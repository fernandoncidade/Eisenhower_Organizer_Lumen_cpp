#include "source/modules/pomodoro/Pomodoro/pomodoro_support.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QAudioOutput>
#include <QCoreApplication>
#include <QDataStream>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMediaPlayer>
#include <QSaveFile>
#include <QUrl>

#include "source/utils/ApplicationPathUtils.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"

namespace source::plugin::Pomodoro::compat {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

source::plugin::PomodoroHostBridge *g_host_bridge = nullptr;
QPointer<SoundManager> g_sound_manager_instance;
QPointer<EventBus> g_event_bus_instance;

QStringList sound_filters()
{
    return {QStringLiteral("*.wav"),
            QStringLiteral("*.mp3"),
            QStringLiteral("*.ogg"),
            QStringLiteral("*.flac"),
            QStringLiteral("*.m4a")};
}

bool ask_yes_no(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(title);
    box.setText(text);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    translate_standard_buttons(&box, parent);
    return box.exec() == QMessageBox::Yes;
}

QString normalize_audio_path(const QString &path)
{
    if (path.trimmed().isEmpty()) {
        return {};
    }

    return QFileInfo(path).canonicalFilePath().isEmpty() ? QFileInfo(path).absoluteFilePath()
                                                         : QFileInfo(path).canonicalFilePath();
}

QString first_existing_directory(const QStringList &candidates)
{
    for (const QString &candidate : candidates) {
        if (QDir(candidate).exists()) {
            return QDir(candidate).absolutePath();
        }
    }

    return {};
}

QString pretty_sound_name(const QString &path)
{
    QString name = QFileInfo(path).completeBaseName();
    name.replace(QLatin1Char('-'), QLatin1Char(' '));
    name.replace(QLatin1Char('_'), QLatin1Char(' '));
    return name.simplified();
}

}  // namespace

GerenciadorBotoesUI::GerenciadorBotoesUI(QWidget *owner) : owner_(owner) {}

QPushButton *GerenciadorBotoesUI::create_button(const QString &text) const
{
    auto *button = new QPushButton(text, owner_);
    return button;
}

void GerenciadorBotoesUI::set_button_text(QPushButton *button, const QString &text) const
{
    if (button != nullptr) {
        button->setText(text);
    }
}

SoundManager::SoundManager(QObject *parent)
    : QObject(parent),
      audio_output_(new QAudioOutput(this)),
      player_(new QMediaPlayer(this))
{
    if (player_ != nullptr) {
        player_->setAudioOutput(audio_output_);

        connect(player_, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
            if (status != QMediaPlayer::EndOfMedia) {
                return;
            }

            if (preview_active_) {
                preview_active_ = false;
                return;
            }

            if (!alarm_active_) {
                return;
            }

            ++completed_alarm_repetitions_;
            if (active_maximum_repetitions_ > 0
                && completed_alarm_repetitions_ >= active_maximum_repetitions_) {
                alarm_active_ = false;
                emit maximum_repetitions_reached();
                return;
            }

            const int interval_ms = qMax(0, alarm_interval_seconds_) * 1000;
            if (interval_ms == 0) {
                start_alarm_cycle();
            } else {
                repeat_timer_.start(interval_ms);
            }
        });
    }

    repeat_timer_.setSingleShot(true);
    connect(&repeat_timer_, &QTimer::timeout, this, [this]() { start_alarm_cycle(); });

    beep_timer_.setSingleShot(true);
    connect(&beep_timer_, &QTimer::timeout, this, [this]() { play_fallback_beep_cycle(); });

    load_config();
}

SoundManager *SoundManager::instance()
{
    if (g_sound_manager_instance == nullptr) {
        auto *app = QCoreApplication::instance();
        g_sound_manager_instance = new SoundManager(app);
        if (app != nullptr) {
            QObject::connect(app, &QCoreApplication::aboutToQuit, g_sound_manager_instance, &SoundManager::stop, Qt::UniqueConnection);
        }
    }

    if (g_sound_manager_instance != nullptr && g_sound_manager_instance->parent() == nullptr) {
        if (auto *app = QCoreApplication::instance()) {
            g_sound_manager_instance->setParent(app);
        }
    }

    return g_sound_manager_instance;
}

QString SoundManager::config_path() const
{
    return QDir(plugin_persistent_dir()).filePath(QStringLiteral("pomodoro_alarm_config.json"));
}

QString SoundManager::sounds_directory() const
{
    const QString app_base = source::utils::get_app_base_path();
    const QString app_root = QFileInfo(app_base).dir().absolutePath();
    const QString source_dir =
        QFileInfo(QString::fromUtf8(__FILE__)).absoluteDir().filePath(QStringLiteral("Alarme/sound"));

    return first_existing_directory(
        {QDir(app_base).filePath(QStringLiteral("sound")),
         QDir(app_base).filePath(QStringLiteral("plugins/pomodoro/sound")),
         QDir(app_base).filePath(QStringLiteral("source/modules/pomodoro/Pomodoro/Alarme/sound")),
         QDir(app_root).filePath(QStringLiteral("source/modules/pomodoro/Pomodoro/Alarme/sound")),
         source_dir});
}

QStringList SoundManager::get_available_sounds() const
{
    const QString directory = sounds_directory();
    if (directory.isEmpty()) {
        return {};
    }

    const QFileInfoList files =
        QDir(directory).entryInfoList(sound_filters(), QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase);

    QStringList sounds;
    sounds.reserve(files.size());
    for (const QFileInfo &file : files) {
        sounds.append(normalize_audio_path(file.absoluteFilePath()));
    }
    return sounds;
}

QString SoundManager::get_sound_display_name(const QString &path) const
{
    return pretty_sound_name(resolve_sound_path(path));
}

QString SoundManager::default_sound_path() const
{
    const QStringList sounds = get_available_sounds();
    return sounds.isEmpty() ? QString() : sounds.constFirst();
}

QString SoundManager::resolve_sound_path(const QString &path) const
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty()) {
        return normalize_audio_path(default_sound_path());
    }

    const QFileInfo direct_info(trimmed);
    if (direct_info.exists()) {
        return normalize_audio_path(direct_info.absoluteFilePath());
    }

    const QString sound_dir = sounds_directory();
    if (!sound_dir.isEmpty()) {
        const QString candidate = QDir(sound_dir).filePath(trimmed);
        if (QFileInfo::exists(candidate)) {
            return normalize_audio_path(candidate);
        }

        const QString file_name = QFileInfo(trimmed).fileName();
        const QString by_name = QDir(sound_dir).filePath(file_name);
        if (QFileInfo::exists(by_name)) {
            return normalize_audio_path(by_name);
        }
    }

    return normalize_audio_path(trimmed);
}

QString SoundManager::get_current_sound() const
{
    const QString resolved = resolve_sound_path(current_sound_);
    return resolved.isEmpty() ? default_sound_path() : resolved;
}

void SoundManager::set_sound(const QString &path)
{
    const QString resolved = resolve_sound_path(path);
    if (resolved == get_current_sound()) {
        return;
    }

    current_sound_ = resolved;
    save_config();
}

int SoundManager::get_alarm_interval() const
{
    return alarm_interval_seconds_;
}

void SoundManager::set_alarm_interval(int seconds)
{
    const int clamped = qBound(0, seconds, 60);
    if (clamped == alarm_interval_seconds_) {
        return;
    }

    alarm_interval_seconds_ = clamped;
    save_config();
}

bool SoundManager::get_automatic_mode_enabled() const
{
    return automatic_mode_enabled_;
}

void SoundManager::set_automatic_mode_enabled(bool enabled)
{
    if (enabled == automatic_mode_enabled_) {
        return;
    }

    automatic_mode_enabled_ = enabled;
    save_config();
}

int SoundManager::get_automatic_alarm_repetitions() const
{
    return automatic_alarm_repetitions_;
}

void SoundManager::set_automatic_alarm_repetitions(int repetitions)
{
    const int clamped = qBound(1, repetitions, 20);
    if (clamped == automatic_alarm_repetitions_) {
        return;
    }

    automatic_alarm_repetitions_ = clamped;
    save_config();
}

void SoundManager::load_config()
{
    current_sound_.clear();
    alarm_interval_seconds_ = 0;
    automatic_mode_enabled_ = false;
    automatic_alarm_repetitions_ = 3;

    QFile file(config_path());
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        const QJsonObject object = document.object();
        current_sound_ = resolve_sound_path(object.value(QStringLiteral("sound_file")).toString());
        if (current_sound_.isEmpty()) {
            current_sound_ = resolve_sound_path(object.value(QStringLiteral("sound_path")).toString());
        }
        alarm_interval_seconds_ = qBound(0, object.value(QStringLiteral("alarm_interval")).toInt(0), 60);
        automatic_mode_enabled_ = object.value(QStringLiteral("automatic_mode_enabled")).toBool(false);
        automatic_alarm_repetitions_ =
            qBound(1, object.value(QStringLiteral("automatic_alarm_repetitions")).toInt(3), 20);
    }

    if (current_sound_.isEmpty()) {
        current_sound_ = default_sound_path();
    }
}

void SoundManager::save_config() const
{
    const QString path = config_path();
    QDir().mkpath(QFileInfo(path).absolutePath());

    const QString current = get_current_sound();
    const QString sounds_dir = sounds_directory();
    QString stored_file = QFileInfo(current).fileName();
    QString stored_path = current;
    if (!sounds_dir.isEmpty() && QFileInfo(current).absolutePath() == QDir(sounds_dir).absolutePath()) {
        stored_path = stored_file;
    }

    QJsonObject object;
    object.insert(QStringLiteral("sound_file"), stored_file);
    object.insert(QStringLiteral("sound_path"), stored_path);
    object.insert(QStringLiteral("alarm_interval"), alarm_interval_seconds_);
    object.insert(QStringLiteral("automatic_mode_enabled"), automatic_mode_enabled_);
    object.insert(QStringLiteral("automatic_alarm_repetitions"), automatic_alarm_repetitions_);

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        logger()->warning(QStringLiteral("Não foi possível salvar configuração de alarme em %1").arg(path));
        return;
    }

    const QByteArray payload = QJsonDocument(object).toJson(QJsonDocument::Indented);
    if (file.write(payload) != payload.size() || !file.commit()) {
        logger()->warning(QStringLiteral("Não foi possível confirmar configuração de alarme em %1").arg(path));
    }
}

qint64 SoundManager::probe_wav_duration_ms(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }

    if (file.read(4) != "RIFF") {
        return 0;
    }

    file.seek(8);
    if (file.read(4) != "WAVE") {
        return 0;
    }

    quint32 byte_rate = 0;
    quint32 data_size = 0;

    while (!file.atEnd()) {
        const QByteArray chunk_id = file.read(4);
        if (chunk_id.size() < 4) {
            break;
        }

        QByteArray chunk_size_bytes = file.read(4);
        if (chunk_size_bytes.size() < 4) {
            break;
        }

        QDataStream stream(&chunk_size_bytes, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        quint32 chunk_size = 0;
        stream >> chunk_size;

        if (chunk_id == "fmt ") {
            QByteArray fmt_chunk = file.read(static_cast<qint64>(chunk_size));
            if (fmt_chunk.size() >= 12) {
                QDataStream fmt_stream(&fmt_chunk, QIODevice::ReadOnly);
                fmt_stream.setByteOrder(QDataStream::LittleEndian);
                quint16 audio_format = 0;
                quint16 channels = 0;
                quint32 sample_rate = 0;
                fmt_stream >> audio_format >> channels >> sample_rate >> byte_rate;
                Q_UNUSED(audio_format);
                Q_UNUSED(channels);
                Q_UNUSED(sample_rate);
            }
        } else if (chunk_id == "data") {
            data_size = chunk_size;
            file.seek(file.pos() + chunk_size);
        } else {
            file.seek(file.pos() + chunk_size);
        }

        if (chunk_size % 2 != 0) {
            file.seek(file.pos() + 1);
        }

        if (byte_rate > 0 && data_size > 0) {
            return static_cast<qint64>((static_cast<double>(data_size) * 1000.0) / static_cast<double>(byte_rate));
        }
    }

    return 0;
}

qint64 SoundManager::get_duration_for_path(const QString &path) const
{
    const QString resolved = resolve_sound_path(path);
    if (resolved.isEmpty()) {
        return 0;
    }

    const auto it = duration_cache_.constFind(resolved);
    if (it != duration_cache_.constEnd()) {
        return it.value();
    }

    qint64 duration_ms = 0;
    if (QFileInfo(resolved).suffix().compare(QStringLiteral("wav"), Qt::CaseInsensitive) == 0) {
        duration_ms = probe_wav_duration_ms(resolved);
    }

    duration_cache_.insert(resolved, duration_ms);
    return duration_ms;
}

void SoundManager::preview(const QString &path)
{
    stop();
    preview_active_ = true;
    active_sound_ = resolve_sound_path(path);

    if (active_sound_.isEmpty() || !QFileInfo::exists(active_sound_) || player_ == nullptr) {
        QApplication::beep();
        preview_active_ = false;
        return;
    }

    player_->setSource(QUrl::fromLocalFile(active_sound_));
    player_->setPosition(0);
    player_->play();
}

void SoundManager::play_fallback_beep_cycle()
{
    if (!alarm_active_) {
        return;
    }

    QApplication::beep();
    ++completed_alarm_repetitions_;
    if (active_maximum_repetitions_ > 0
        && completed_alarm_repetitions_ >= active_maximum_repetitions_) {
        alarm_active_ = false;
        emit maximum_repetitions_reached();
        return;
    }

    const int interval_ms = qMax(1000, alarm_interval_seconds_ * 1000);
    beep_timer_.start(interval_ms);
}

void SoundManager::start_alarm_cycle()
{
    if (!alarm_active_) {
        return;
    }

    repeat_timer_.stop();
    beep_timer_.stop();

    active_sound_ = get_current_sound();
    if (active_sound_.isEmpty() || !QFileInfo::exists(active_sound_) || player_ == nullptr) {
        play_fallback_beep_cycle();
        return;
    }

    player_->stop();
    player_->setSource(QUrl::fromLocalFile(active_sound_));
    player_->setPosition(0);
    player_->play();
}

void SoundManager::play_looping(int maximum_repetitions)
{
    stop();
    active_maximum_repetitions_ = qMax(0, maximum_repetitions);
    completed_alarm_repetitions_ = 0;
    alarm_active_ = true;
    start_alarm_cycle();
}

void SoundManager::stop()
{
    preview_active_ = false;
    alarm_active_ = false;
    active_maximum_repetitions_ = 0;
    completed_alarm_repetitions_ = 0;
    active_sound_.clear();
    repeat_timer_.stop();
    beep_timer_.stop();
    if (player_ != nullptr) {
        player_->stop();
    }
}

MessageBox *MessageBox::create(QWidget *parent)
{
    auto *box = new MessageBox(parent);
    box->setAttribute(Qt::WA_DeleteOnClose, false);
    box->setStandardButtons(QMessageBox::Ok);
    translate_standard_buttons(box, parent);
    return box;
}

void MessageBox::information(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle(title);
    box.setText(text);
    box.setStandardButtons(QMessageBox::Ok);
    translate_standard_buttons(&box, parent);
    box.exec();
}

void MessageBox::warning(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(title);
    box.setText(text);
    box.setStandardButtons(QMessageBox::Ok);
    translate_standard_buttons(&box, parent);
    box.exec();
}

void MessageBox::warning_file_not_found(QWidget *parent)
{
    warning(parent,
            QCoreApplication::translate("PomodoroPlugin", "Arquivo não encontrado"),
            QCoreApplication::translate("PomodoroPlugin", "O arquivo vinculado não foi encontrado."));
}

bool MessageBox::tempo_confirm_attach_file(QWidget *parent, const QString &file_name)
{
    return ask_yes_no(
        parent,
        QCoreApplication::translate("PomodoroPlugin", "Vincular arquivo"),
        QCoreApplication::translate("PomodoroPlugin", "Deseja vincular o arquivo \"%1\" a esta tarefa?").arg(file_name));
}

bool MessageBox::tempo_confirm_delete_task(QWidget *parent, const QString &task_name)
{
    return ask_yes_no(
        parent,
        QCoreApplication::translate("PomodoroPlugin", "Remover tarefa"),
        QCoreApplication::translate("PomodoroPlugin", "Deseja remover a tarefa \"%1\"?").arg(task_name));
}

bool MessageBox::tempo_confirm_delete_tasks(QWidget *parent, int total)
{
    return ask_yes_no(
        parent,
        QCoreApplication::translate("PomodoroPlugin", "Remover tarefas"),
        QCoreApplication::translate("PomodoroPlugin", "Deseja remover %1 tarefa(s)?").arg(total));
}

void MessageBox::tempo_delete_task_success(QWidget *parent)
{
    information(parent,
                QCoreApplication::translate("PomodoroPlugin", "Tarefa removida"),
                QCoreApplication::translate("PomodoroPlugin", "A tarefa foi removida com sucesso."));
}

void MessageBox::tempo_delete_task_failure(QWidget *parent)
{
    information(parent,
                QCoreApplication::translate("PomodoroPlugin", "Nenhuma alteração"),
                QCoreApplication::translate("PomodoroPlugin", "Nenhuma tarefa foi removida."));
}

void MessageBox::tempo_delete_task_error(QWidget *parent)
{
    information(parent,
                QCoreApplication::translate("PomodoroPlugin", "Erro"),
                QCoreApplication::translate("PomodoroPlugin", "Não foi possível remover a tarefa."));
}

void MessageBox::tempo_delete_tasks_success(QWidget *parent, int total)
{
    information(
        parent,
        QCoreApplication::translate("PomodoroPlugin", "Tarefas removidas"),
        QCoreApplication::translate("PomodoroPlugin", "%1 tarefa(s) foram removidas com sucesso.").arg(total));
}

void MessageBox::tempo_delete_tasks_failure(QWidget *parent)
{
    information(parent,
                QCoreApplication::translate("PomodoroPlugin", "Nenhuma alteração"),
                QCoreApplication::translate("PomodoroPlugin", "Nenhuma tarefa foi removida."));
}

void MessageBox::tempo_delete_tasks_error(QWidget *parent)
{
    information(parent,
                QCoreApplication::translate("PomodoroPlugin", "Erro"),
                QCoreApplication::translate("PomodoroPlugin", "Não foi possível remover as tarefas selecionadas."));
}

void MessageBox::tempo_edit_task_success(QWidget *parent)
{
    information(parent,
                QCoreApplication::translate("PomodoroPlugin", "✏️ Editar Tarefa"),
                QCoreApplication::translate("PomodoroPlugin", "Alterações aplicadas com sucesso."));
}

void MessageBox::tempo_share_eisenhower_success(QWidget *parent, int total)
{
    information(
        parent,
        QCoreApplication::translate("PomodoroPlugin", "Compartilhamento concluído"),
        QCoreApplication::translate("PomodoroPlugin", "%1 tarefa(s) foram compartilhadas com Eisenhower.").arg(total));
}

void MessageBox::tempo_share_feynman_success(QWidget *parent, int total)
{
    information(
        parent,
        QCoreApplication::translate("PomodoroPlugin", "Compartilhamento concluído"),
        QCoreApplication::translate("PomodoroPlugin", "%1 tarefa(s) foram compartilhadas com Feynman.").arg(total));
}

void MessageBox::tempo_no_tasks_in_column(QWidget *parent)
{
    information(parent,
                QCoreApplication::translate("PomodoroPlugin", "Sem tarefas"),
                QCoreApplication::translate("PomodoroPlugin", "Não há tarefas na coluna selecionada."));
}

void MessageBox::tempo_no_tasks_anywhere(QWidget *parent)
{
    information(parent,
                QCoreApplication::translate("PomodoroPlugin", "Sem tarefas"),
                QCoreApplication::translate("PomodoroPlugin", "Não há tarefas cadastradas."));
}

void EventBus::send_conceito(const QVariantMap &dados)
{
    pending_conceitos_.append(dados);
    emit conceito_atualizado(dados);
}

void EventBus::drain_pending_conceitos()
{
    const auto pending = pending_conceitos_;
    pending_conceitos_.clear();

    for (const QVariantMap &item : pending) {
        emit conceito_atualizado(item);
    }
}

EventBus *EventBus::instance()
{
    if (g_event_bus_instance == nullptr) {
        g_event_bus_instance = new EventBus(QCoreApplication::instance());
    }

    if (g_event_bus_instance != nullptr && g_event_bus_instance->parent() == nullptr) {
        if (auto *app = QCoreApplication::instance()) {
            g_event_bus_instance->setParent(app);
        }
    }

    return g_event_bus_instance;
}

void EventBus::send_tarefa(const QVariantMap &dados)
{
    emit tarefa_integracao(dados);
    if (g_host_bridge != nullptr) {
        g_host_bridge->publish_task_to_eisenhower(dados);
    }
}

void EventBus::receive_tarefa_from_host(const QVariantMap &dados)
{
    pending_tarefas_from_host_.append(dados);
    emit tarefa_integracao(dados);
}

void EventBus::drain_pending_tarefas()
{
    const auto pending = pending_tarefas_from_host_;
    pending_tarefas_from_host_.clear();

    for (const QVariantMap &item : pending) {
        emit tarefa_integracao(item);
    }
}

void EventBus::clear_pending()
{
    pending_conceitos_.clear();
    pending_tarefas_from_host_.clear();
}

EventBus *get_event_bus()
{
    return EventBus::instance();
}

void set_host_bridge(source::plugin::PomodoroHostBridge *host_bridge)
{
    g_host_bridge = host_bridge;
}

source::plugin::PomodoroHostBridge *host_bridge()
{
    return g_host_bridge;
}

QString plugin_persistent_dir()
{
    const QString persistent_dir = source::utils::obter_caminho_persistente();
    if (!QDir(persistent_dir).exists() && !QDir().mkpath(persistent_dir)) {
        logger()->warning(
            QStringLiteral("Não foi possível criar diretório persistente do Pomodoro: %1").arg(persistent_dir));
    }
    return persistent_dir;
}

QString effective_language_code(const QWidget *context)
{
    Q_UNUSED(context);
    if (g_host_bridge != nullptr) {
        return g_host_bridge->language_code();
    }

    return QLocale::system().name();
}

QLocale effective_locale(const QWidget *context)
{
    const QString code = effective_language_code(context);
    if (code.startsWith(QStringLiteral("pt"))) {
        return QLocale(QLocale::Portuguese, QLocale::Brazil);
    }

    if (code.startsWith(QStringLiteral("en"))) {
        return QLocale(QLocale::English, QLocale::UnitedStates);
    }

    return QLocale::system();
}

QString effective_date_format(const QWidget *context)
{
    const QString code = effective_language_code(context);
    if (code.startsWith(QStringLiteral("en"))) {
        return QStringLiteral("MM/dd/yyyy");
    }

    return QStringLiteral("dd/MM/yyyy");
}

void translate_standard_buttons(QDialog *dialog, const QWidget *context)
{
    if (dialog == nullptr) {
        return;
    }

    if (g_host_bridge != nullptr) {
        g_host_bridge->translate_standard_buttons(dialog);
    }

    if (auto *message_box = qobject_cast<QMessageBox *>(dialog)) {
        const struct ButtonText {
            QMessageBox::StandardButton button;
            const char *text;
        } buttons[] = {
            {QMessageBox::Ok, "OK"},
            {QMessageBox::Cancel, "Cancelar"},
            {QMessageBox::Yes, "Sim"},
            {QMessageBox::No, "Não"},
            {QMessageBox::Abort, "Abortar"},
            {QMessageBox::Retry, "Tentar Novamente"},
            {QMessageBox::Ignore, "Ignorar"},
            {QMessageBox::Close, "Fechar"},
            {QMessageBox::Help, "Ajuda"},
            {QMessageBox::Apply, "Aplicar"},
            {QMessageBox::Reset, "Redefinir"},
            {QMessageBox::RestoreDefaults, "Restaurar Padrões"},
            {QMessageBox::Save, "Salvar"},
            {QMessageBox::SaveAll, "Salvar Tudo"},
            {QMessageBox::Open, "Abrir"},
        };

        for (const ButtonText &button : buttons) {
            if (QAbstractButton *widget = message_box->button(button.button)) {
                widget->setText(QCoreApplication::translate("PomodoroPlugin", button.text));
            }
        }
    }

    Q_UNUSED(context);
}

MissingLinkedFileChoice prompt_missing_linked_file(QWidget *parent)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(QCoreApplication::translate("PomodoroPlugin", "Aviso"));
    box.setText(QCoreApplication::translate("PomodoroPlugin", "O arquivo vinculado não foi encontrado ou não existe mais."));
    QAbstractButton *remove_button =
        box.addButton(QCoreApplication::translate("PomodoroPlugin", "Remover Vínculo"), QMessageBox::ActionRole);
    QAbstractButton *search_button =
        box.addButton(QCoreApplication::translate("PomodoroPlugin", "Buscar"), QMessageBox::ActionRole);
    box.addButton(QCoreApplication::translate("PomodoroPlugin", "OK"), QMessageBox::AcceptRole);
    box.exec();

    if (box.clickedButton() == remove_button) {
        return MissingLinkedFileChoice::RemoveLink;
    }
    if (box.clickedButton() == search_button) {
        return MissingLinkedFileChoice::Search;
    }
    return MissingLinkedFileChoice::None;
}

QString browse_linked_file_replacement(QWidget *parent, const QString &current_path)
{
    const QString initial_dir = QFileInfo(current_path).absolutePath();
    return QFileDialog::getOpenFileName(parent,
                                        QCoreApplication::translate("PomodoroPlugin", "Buscar arquivo"),
                                        initial_dir,
                                        QStringLiteral("*"));
}

void show_link_removed_success(QWidget *parent)
{
    MessageBox::information(parent,
                            QCoreApplication::translate("PomodoroPlugin", "Sucesso"),
                            QCoreApplication::translate("PomodoroPlugin", "Vínculo removido com sucesso!"));
}

bool open_local_path(const QString &path, QWidget *parent)
{
    if (path.isEmpty()) {
        return false;
    }

    if (!QFileInfo::exists(path)) {
        MessageBox::warning_file_not_found(parent);
        return false;
    }

    if (QDesktopServices::openUrl(QUrl::fromLocalFile(path))) {
        return true;
    }

    logger()->error(QStringLiteral("Falha ao abrir arquivo vinculado: %1").arg(path));
    return false;
}

QString strip_priority_emoji(const QString &text)
{
    QString result = text.split(QLatin1Char('\n')).value(0).trimmed();
    result.remove(QChar(0x0332));
    result.replace(QStringLiteral("🔴 "), QString());
    result.replace(QStringLiteral("🟠 "), QString());
    result.replace(QStringLiteral("🟡 "), QString());
    result.replace(QStringLiteral("🟢 "), QString());
    result.replace(QStringLiteral("🥉 "), QString());
    result.replace(QStringLiteral("🥈 "), QString());
    result.replace(QStringLiteral("🥇 "), QString());
    const int sep = result.indexOf(QStringLiteral(" — "));
    if (sep >= 0) {
        result = result.left(sep);
    }
    return result.trimmed();
}

QString normalized_task_text(const QString &text)
{
    return strip_priority_emoji(text).simplified().trimmed().toCaseFolded();
}

}  // namespace source::plugin::Pomodoro::compat
