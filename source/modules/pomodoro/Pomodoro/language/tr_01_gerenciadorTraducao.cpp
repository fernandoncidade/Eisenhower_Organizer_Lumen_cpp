#include "source/modules/pomodoro/Pomodoro/language/tr_01_gerenciadorTraducao.hpp"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QTranslator>

#include "source/utils/ApplicationPathUtils.hpp"
#include "source/utils/LogManager.hpp"

namespace source::plugin::Pomodoro::language {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

QString translations_dir()
{
    const QString app_base = source::utils::get_app_base_path();
    const QString app_root = QFileInfo(app_base).dir().absolutePath();
    const QString source_dir =
        QFileInfo(QString::fromUtf8(__FILE__)).absoluteDir().filePath(QStringLiteral("translations"));

    const QString current = QDir::currentPath();

    const QStringList candidates = {
        QDir(app_base).filePath(QStringLiteral("translations")),
        QDir(app_base).filePath(QStringLiteral("source/language/translations")),
        QDir(app_root).filePath(QStringLiteral("source/language/translations")),
        QDir(current).filePath(QStringLiteral("source/language/translations")),
        QDir(current).absoluteFilePath(QStringLiteral("../source/language/translations")),
    };

    QString first_existing;
    for (const QString &candidate : candidates) {
        const QDir directory(candidate);
        if (!directory.exists()) {
            continue;
        }

        if (first_existing.isEmpty()) {
            first_existing = QFileInfo(candidate).absoluteFilePath();
        }

        if (!directory.entryList(QStringList{QStringLiteral("pomodoro_*.qm")}, QDir::Files).isEmpty()) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }

    if (!first_existing.isEmpty()) {
        return first_existing;
    }

    return QFileInfo(candidates.constFirst()).absoluteFilePath();
}

QStringList translation_file_candidates(const QString &language_code)
{
    return {
        QStringLiteral("pomodoro_%1.qm").arg(language_code),
    };
}

}  // namespace

GerenciadorTraducao::GerenciadorTraducao(QObject *parent)
    : QObject(parent),
      dir_traducoes_(translations_dir())
{
    idiomas_disponiveis_.insert(QStringLiteral("pt_BR"), QStringLiteral("Português (Brasil)"));
    idiomas_disponiveis_.insert(QStringLiteral("en_US"), QStringLiteral("English (United States)"));
}

GerenciadorTraducao::~GerenciadorTraducao()
{
    remover_tradutor_instalado();
}

QString GerenciadorTraducao::normalizar_codigo_idioma(const QString &codigo_idioma) const
{
    QString normalized = codigo_idioma.trimmed();
    if (normalized.isEmpty()) {
        normalized = QLocale::system().name();
    }

    normalized.replace(QLatin1Char('-'), QLatin1Char('_'));
    if (normalized.compare(QStringLiteral("pt"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("pt_BR");
    }
    if (normalized.compare(QStringLiteral("en"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("en_US");
    }
    if (normalized.startsWith(QStringLiteral("pt"), Qt::CaseInsensitive)) {
        return QStringLiteral("pt_BR");
    }
    if (normalized.startsWith(QStringLiteral("en"), Qt::CaseInsensitive)) {
        return QStringLiteral("en_US");
    }

    return idiomas_disponiveis_.contains(normalized) ? normalized : QStringLiteral("pt_BR");
}

bool GerenciadorTraducao::aplicar_traducao(const QString &codigo_idioma)
{
    auto *app = QCoreApplication::instance();
    if (app == nullptr) {
        return false;
    }

    remover_tradutor_instalado();

    idioma_atual_ = normalizar_codigo_idioma(codigo_idioma);
    const QString locale_code = QString(idioma_atual_).replace(QLatin1Char('_'), QLatin1Char('-'));
    QLocale::setDefault(QLocale(locale_code));
    load_qt_translator_for_locale(idioma_atual_);

    bool applied = false;
    for (const QString &translation_file : translation_file_candidates(idioma_atual_)) {
        const QString translation_path = QDir(dir_traducoes_).filePath(translation_file);
        if (!QFileInfo::exists(translation_path)) {
            continue;
        }

        tradutor_ = new QTranslator(this);
        if (tradutor_->load(translation_path)) {
            app->installTranslator(tradutor_);
            applied = true;
            break;
        }

        logger()->warning(QStringLiteral("Falha ao carregar tradução do plugin Pomodoro: %1").arg(translation_path));
        tradutor_->deleteLater();
        tradutor_ = nullptr;
    }

    if (applied || qt_translator_ != nullptr) {
        return true;
    }

    logger()->warning(QStringLiteral("Nenhum arquivo .qm do plugin Pomodoro encontrado para %1 em %2")
                          .arg(idioma_atual_, dir_traducoes_));
    return false;
}

QString GerenciadorTraducao::obter_idioma_atual() const
{
    return idioma_atual_;
}

void GerenciadorTraducao::traduzir_botoes_padrao(QObject *dialogo) const
{
    if (auto *message_box = qobject_cast<QMessageBox *>(dialogo)) {
        const struct ButtonText {
            QMessageBox::StandardButton button;
            const char *text;
        } buttons[] = {
            {QMessageBox::Ok, QT_TRANSLATE_NOOP("PomodoroPlugin", "OK")},
            {QMessageBox::Cancel, QT_TRANSLATE_NOOP("PomodoroPlugin", "Cancelar")},
            {QMessageBox::Yes, QT_TRANSLATE_NOOP("PomodoroPlugin", "Sim")},
            {QMessageBox::No, QT_TRANSLATE_NOOP("PomodoroPlugin", "Não")},
            {QMessageBox::Abort, QT_TRANSLATE_NOOP("PomodoroPlugin", "Abortar")},
            {QMessageBox::Retry, QT_TRANSLATE_NOOP("PomodoroPlugin", "Tentar Novamente")},
            {QMessageBox::Ignore, QT_TRANSLATE_NOOP("PomodoroPlugin", "Ignorar")},
            {QMessageBox::Close, QT_TRANSLATE_NOOP("PomodoroPlugin", "Fechar")},
            {QMessageBox::Help, QT_TRANSLATE_NOOP("PomodoroPlugin", "Ajuda")},
            {QMessageBox::Apply, QT_TRANSLATE_NOOP("PomodoroPlugin", "Aplicar")},
            {QMessageBox::Reset, QT_TRANSLATE_NOOP("PomodoroPlugin", "Redefinir")},
            {QMessageBox::RestoreDefaults, QT_TRANSLATE_NOOP("PomodoroPlugin", "Restaurar Padrões")},
            {QMessageBox::Save, QT_TRANSLATE_NOOP("PomodoroPlugin", "Salvar")},
            {QMessageBox::SaveAll, QT_TRANSLATE_NOOP("PomodoroPlugin", "Salvar Tudo")},
            {QMessageBox::Open, QT_TRANSLATE_NOOP("PomodoroPlugin", "Abrir")},
        };

        for (const ButtonText &button : buttons) {
            if (QAbstractButton *widget = message_box->button(button.button)) {
                widget->setText(QCoreApplication::translate("PomodoroPlugin", button.text));
            }
        }
        return;
    }

    if (auto *button_box = qobject_cast<QDialogButtonBox *>(dialogo)) {
        const struct ButtonText {
            QDialogButtonBox::StandardButton button;
            const char *text;
        } buttons[] = {
            {QDialogButtonBox::Ok, QT_TRANSLATE_NOOP("PomodoroPlugin", "OK")},
            {QDialogButtonBox::Cancel, QT_TRANSLATE_NOOP("PomodoroPlugin", "Cancelar")},
            {QDialogButtonBox::Yes, QT_TRANSLATE_NOOP("PomodoroPlugin", "Sim")},
            {QDialogButtonBox::No, QT_TRANSLATE_NOOP("PomodoroPlugin", "Não")},
            {QDialogButtonBox::Abort, QT_TRANSLATE_NOOP("PomodoroPlugin", "Abortar")},
            {QDialogButtonBox::Retry, QT_TRANSLATE_NOOP("PomodoroPlugin", "Tentar Novamente")},
            {QDialogButtonBox::Ignore, QT_TRANSLATE_NOOP("PomodoroPlugin", "Ignorar")},
            {QDialogButtonBox::Close, QT_TRANSLATE_NOOP("PomodoroPlugin", "Fechar")},
            {QDialogButtonBox::Help, QT_TRANSLATE_NOOP("PomodoroPlugin", "Ajuda")},
            {QDialogButtonBox::Apply, QT_TRANSLATE_NOOP("PomodoroPlugin", "Aplicar")},
            {QDialogButtonBox::Reset, QT_TRANSLATE_NOOP("PomodoroPlugin", "Redefinir")},
            {QDialogButtonBox::RestoreDefaults, QT_TRANSLATE_NOOP("PomodoroPlugin", "Restaurar Padrões")},
            {QDialogButtonBox::Save, QT_TRANSLATE_NOOP("PomodoroPlugin", "Salvar")},
            {QDialogButtonBox::SaveAll, QT_TRANSLATE_NOOP("PomodoroPlugin", "Salvar Tudo")},
            {QDialogButtonBox::Open, QT_TRANSLATE_NOOP("PomodoroPlugin", "Abrir")},
        };

        for (const ButtonText &button : buttons) {
            if (QPushButton *widget = button_box->button(button.button)) {
                widget->setText(QCoreApplication::translate("PomodoroPlugin", button.text));
            }
        }
    }
}

const QMap<QString, QString> &GerenciadorTraducao::idiomas_disponiveis() const
{
    return idiomas_disponiveis_;
}

void GerenciadorTraducao::remover_tradutor_instalado()
{
    auto *app = QCoreApplication::instance();
    if (app == nullptr) {
        return;
    }

    if (tradutor_ != nullptr) {
        app->removeTranslator(tradutor_);
        tradutor_->deleteLater();
        tradutor_ = nullptr;
    }

    if (qt_translator_ != nullptr) {
        app->removeTranslator(qt_translator_);
        qt_translator_->deleteLater();
        qt_translator_ = nullptr;
    }
}

bool GerenciadorTraducao::load_qt_translator_for_locale(const QString &codigo_idioma)
{
    auto *app = QCoreApplication::instance();
    if (app == nullptr) {
        return false;
    }

    const QString translations_path = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (!QDir(translations_path).exists()) {
        return false;
    }

    qt_translator_ = new QTranslator(this);
    if (!qt_translator_->load(QLocale(codigo_idioma), QStringLiteral("qtbase"), QStringLiteral("_"), translations_path)) {
        qt_translator_->deleteLater();
        qt_translator_ = nullptr;
        return false;
    }

    app->installTranslator(qt_translator_);
    return true;
}

}  // namespace source::plugin::Pomodoro::language
