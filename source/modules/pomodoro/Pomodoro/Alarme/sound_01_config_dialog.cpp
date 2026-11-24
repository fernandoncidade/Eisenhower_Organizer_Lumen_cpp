#include "source/modules/pomodoro/Pomodoro/Alarme/sound_01_config_dialog.hpp"

#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace source::plugin::Pomodoro::Alarme {

SoundConfigDialog::SoundConfigDialog(QWidget *parent)
    : QDialog(parent),
      logger_(source::utils::LogManager::get_logger()),
      sound_manager_(compat::SoundManager::instance())
{
    setMinimumWidth(450);

    auto *root_layout = new QVBoxLayout(this);

    sound_group_ = new QGroupBox(this);
    auto *sound_layout = new QVBoxLayout(sound_group_);

    sound_hint_label_ = new QLabel(sound_group_);
    sound_hint_label_->setWordWrap(true);
    sound_layout->addWidget(sound_hint_label_);

    sound_combo_ = new QComboBox(sound_group_);
    const QStringList paths = sound_manager_ != nullptr ? sound_manager_->get_available_sounds() : QStringList{};
    for (const QString &path : paths) {
        sound_combo_->addItem(sound_manager_->get_sound_display_name(path), path);
    }

    if (sound_manager_ != nullptr) {
        const QString current_sound = sound_manager_->get_current_sound();
        const int index = sound_combo_->findData(current_sound);
        if (index >= 0) {
            sound_combo_->setCurrentIndex(index);
        }
    }

    connect(sound_combo_,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            [this](int index) { on_sound_changed(index); });
    sound_layout->addWidget(sound_combo_);

    duration_label_ = new QLabel(sound_group_);
    duration_label_->setStyleSheet(QStringLiteral("color: #666; font-size: 9pt;"));
    sound_layout->addWidget(duration_label_);

    auto *preview_layout = new QHBoxLayout();
    play_button_ = new QPushButton(sound_group_);
    connect(play_button_, &QPushButton::clicked, this, [this]() { on_play(); });
    preview_layout->addWidget(play_button_);

    stop_preview_button_ = new QPushButton(sound_group_);
    connect(stop_preview_button_, &QPushButton::clicked, this, [this]() { on_stop_preview(); });
    preview_layout->addWidget(stop_preview_button_);
    sound_layout->addLayout(preview_layout);

    root_layout->addWidget(sound_group_);

    interval_group_ = new QGroupBox(this);
    interval_layout_ = new QFormLayout(interval_group_);

    interval_spin_ = new QSpinBox(interval_group_);
    interval_spin_->setRange(0, 60);
    interval_spin_->setValue(sound_manager_ != nullptr ? sound_manager_->get_alarm_interval() : 0);
    connect(interval_spin_, &QSpinBox::valueChanged, this, [this](int) { update_total_label(); });
    interval_layout_->addRow(new QLabel(interval_group_), interval_spin_);

    total_label_ = new QLabel(interval_group_);
    total_label_->setStyleSheet(QStringLiteral("color: #007ACC; font-size: 9pt; font-weight: bold;"));
    interval_layout_->addRow(total_label_);

    hint_label_ = new QLabel(interval_group_);
    hint_label_->setWordWrap(true);
    hint_label_->setStyleSheet(QStringLiteral("color: gray; font-size: 9pt;"));
    interval_layout_->addRow(hint_label_);

    root_layout->addWidget(interval_group_);

    auto *actions_layout = new QHBoxLayout();
    actions_layout->addStretch();

    save_button_ = new QPushButton(this);
    connect(save_button_, &QPushButton::clicked, this, [this]() { on_save(); });
    actions_layout->addWidget(save_button_);

    cancel_button_ = new QPushButton(this);
    connect(cancel_button_, &QPushButton::clicked, this, &QDialog::reject);
    actions_layout->addWidget(cancel_button_);

    root_layout->addLayout(actions_layout);

    retranslate_ui();
    update_duration_label();
    update_total_label();
}

void SoundConfigDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event != nullptr && event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
}

void SoundConfigDialog::retranslate_ui()
{
    setWindowTitle(QCoreApplication::translate("PomodoroPlugin", "🔔 Configurar Som do Pomodoro"));
    sound_group_->setTitle(QCoreApplication::translate("PomodoroPlugin", "🔊 Campainha"));
    sound_hint_label_->setText(
        QCoreApplication::translate("PomodoroPlugin",
                                    "Selecione a campainha para tocar ao final do Pomodoro / Pausa:"));
    play_button_->setText(QCoreApplication::translate("PomodoroPlugin", "▶️ Tocar"));
    stop_preview_button_->setText(QCoreApplication::translate("PomodoroPlugin", "⏹️ Parar"));

    interval_group_->setTitle(QCoreApplication::translate("PomodoroPlugin", "⏱️ Repeticao do Alarme"));
    auto *interval_label = qobject_cast<QLabel *>(interval_layout_->labelForField(interval_spin_));
    if (interval_label != nullptr) {
        interval_label->setText(QCoreApplication::translate("PomodoroPlugin", "Intervalo entre repeticoes:"));
    }
    interval_spin_->setSuffix(QCoreApplication::translate("PomodoroPlugin", " segundos"));

    hint_label_->setText(QCoreApplication::translate(
        "PomodoroPlugin",
        "O alarme tocara por completo e, apos o intervalo definido,\n"
        "repetira ate voce fechar a janela de alerta ou ate o limite do modo automatico.\n"
        "Defina 0 para repetir imediatamente apos o audio terminar."));

    save_button_->setText(QCoreApplication::translate("PomodoroPlugin", "💾 Salvar"));
    cancel_button_->setText(QCoreApplication::translate("PomodoroPlugin", "Cancelar"));

    update_duration_label();
    update_total_label();
}

void SoundConfigDialog::on_sound_changed(int index)
{
    Q_UNUSED(index);
    on_stop_preview();
    update_duration_label();
    update_total_label();
}

void SoundConfigDialog::update_duration_label()
{
    if (sound_combo_->count() == 0 || sound_manager_ == nullptr) {
        duration_label_->setText(QCoreApplication::translate("PomodoroPlugin", "Nenhum audio encontrado para o alarme."));
        return;
    }

    const QString path = sound_combo_->currentData().toString();
    const qint64 duration_ms = sound_manager_->get_duration_for_path(path);
    if (duration_ms <= 0) {
        duration_label_->setText(QCoreApplication::translate("PomodoroPlugin", "⏱️ Duracao do audio: indisponivel"));
        return;
    }

    const QString duration_seconds = QString::number(duration_ms / 1000.0, 'f', 2);
    duration_label_->setText(
        QCoreApplication::translate("PomodoroPlugin", "⏱️ Duracao do audio: {duration} segundos")
            .replace(QStringLiteral("{duration}"), duration_seconds));
}

void SoundConfigDialog::update_total_label()
{
    if (sound_combo_->count() == 0 || sound_manager_ == nullptr) {
        total_label_->setText(QCoreApplication::translate("PomodoroPlugin", "🔄 Repeticao a cada: indisponivel"));
        return;
    }

    const QString path = sound_combo_->currentData().toString();
    const qint64 duration_ms = sound_manager_->get_duration_for_path(path);
    const double total_seconds = (duration_ms / 1000.0) + interval_spin_->value();
    const QString total = duration_ms > 0 ? QString::number(total_seconds, 'f', 2)
                                          : QString::number(interval_spin_->value());

    total_label_->setText(
        QCoreApplication::translate("PomodoroPlugin", "🔄 Repeticao a cada: {total} segundos")
            .replace(QStringLiteral("{total}"), total));
}

void SoundConfigDialog::on_play()
{
    try {
        if (sound_manager_ == nullptr || sound_combo_->count() == 0) {
            return;
        }

        sound_manager_->preview(sound_combo_->currentData().toString());
        logger_->debug(QStringLiteral("Preview iniciado para o alarme do Pomodoro."));
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao reproduzir preview do alarme: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao reproduzir preview do alarme"), true);
    }
}

void SoundConfigDialog::on_stop_preview()
{
    try {
        if (sound_manager_ != nullptr) {
            sound_manager_->stop();
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao parar preview do alarme: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao parar preview do alarme"), true);
    }
}

void SoundConfigDialog::on_save()
{
    try {
        on_stop_preview();

        if (sound_manager_ != nullptr) {
            if (sound_combo_->count() > 0) {
                sound_manager_->set_sound(sound_combo_->currentData().toString());
            }
            sound_manager_->set_alarm_interval(interval_spin_->value());
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao salvar configuração do alarme: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao salvar configuração do alarme"), true);
    }

    accept();
}

void SoundConfigDialog::closeEvent(QCloseEvent *event)
{
    on_stop_preview();
    QDialog::closeEvent(event);
}

AutomaticModeConfigDialog::AutomaticModeConfigDialog(QWidget *parent)
    : QDialog(parent),
      logger_(source::utils::LogManager::get_logger()),
      sound_manager_(compat::SoundManager::instance())
{
    setMinimumWidth(430);

    auto *root_layout = new QVBoxLayout(this);

    automatic_group_ = new QGroupBox(this);
    automatic_layout_ = new QFormLayout(automatic_group_);

    repetitions_spin_ = new QSpinBox(automatic_group_);
    repetitions_spin_->setRange(1, 20);
    repetitions_spin_->setValue(sound_manager_ != nullptr ? sound_manager_->get_automatic_alarm_repetitions() : 3);
    automatic_layout_->addRow(new QLabel(automatic_group_), repetitions_spin_);

    hint_label_ = new QLabel(automatic_group_);
    hint_label_->setWordWrap(true);
    hint_label_->setStyleSheet(QStringLiteral("color: gray; font-size: 9pt;"));
    automatic_layout_->addRow(hint_label_);
    root_layout->addWidget(automatic_group_);

    auto *actions_layout = new QHBoxLayout();
    actions_layout->addStretch();

    save_button_ = new QPushButton(this);
    connect(save_button_, &QPushButton::clicked, this, [this]() { on_save(); });
    actions_layout->addWidget(save_button_);

    cancel_button_ = new QPushButton(this);
    connect(cancel_button_, &QPushButton::clicked, this, &QDialog::reject);
    actions_layout->addWidget(cancel_button_);
    root_layout->addLayout(actions_layout);

    retranslate_ui();
}

void AutomaticModeConfigDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event != nullptr && event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
}

void AutomaticModeConfigDialog::retranslate_ui()
{
    setWindowTitle(QCoreApplication::translate("PomodoroPlugin", "⚙️ Configurar modo automatico"));
    automatic_group_->setTitle(QCoreApplication::translate("PomodoroPlugin", "🔁 Avanco automatico"));

    auto *repetitions_label = qobject_cast<QLabel *>(automatic_layout_->labelForField(repetitions_spin_));
    if (repetitions_label != nullptr) {
        repetitions_label->setText(
            QCoreApplication::translate("PomodoroPlugin", "Maximo de repeticoes do alarme:"));
    }
    repetitions_spin_->setSuffix(QCoreApplication::translate("PomodoroPlugin", " repeticoes"));
    hint_label_->setText(QCoreApplication::translate(
        "PomodoroPlugin",
        "Quando o modo automatico estiver marcado no Pomodoro, o aviso sera fechado apos esta quantidade de "
        "reproducoes completas do alarme e a proxima etapa sera iniciada."));

    save_button_->setText(QCoreApplication::translate("PomodoroPlugin", "💾 Salvar"));
    cancel_button_->setText(QCoreApplication::translate("PomodoroPlugin", "Cancelar"));
}

void AutomaticModeConfigDialog::on_save()
{
    try {
        if (sound_manager_ != nullptr) {
            sound_manager_->set_automatic_alarm_repetitions(repetitions_spin_->value());
        }
    } catch (const std::exception &e) {
        logger_->error(
            QStringLiteral("Erro ao salvar configuração do modo automático: %1").arg(QString::fromUtf8(e.what())),
            true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao salvar configuração do modo automático"), true);
    }

    accept();
}

}  // namespace source::plugin::Pomodoro::Alarme
