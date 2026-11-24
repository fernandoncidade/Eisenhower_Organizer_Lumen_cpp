#include "tr_01_gerenciadorTraducao.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLocale>

namespace source::gantt::language {

QString obter_diretorio_traducoes()
{
    const QString app_dir = QCoreApplication::applicationDirPath();
    const QString current = QDir::currentPath();
    const QStringList candidatos = {
        app_dir + QStringLiteral("/translations"),
        current + QStringLiteral("/source/language/translations"),
        app_dir + QStringLiteral("/source/language/translations"),
        QDir(app_dir).absoluteFilePath(QStringLiteral("../source/language/translations")),
        QDir(current).absoluteFilePath(QStringLiteral("../source/language/translations")),
    };

    for (const QString &caminho : candidatos) {
        QDir dir(caminho);
        if (dir.exists()) {
            const QStringList qms = dir.entryList({QStringLiteral("languagegantt_*.qm"), QStringLiteral("gantt_*.qm")}, QDir::Files);
            if (!qms.isEmpty()) {
                return dir.absolutePath();
            }
        }
    }

    for (const QString &caminho : candidatos) {
        if (QDir(caminho).exists()) {
            return QDir(caminho).absolutePath();
        }
    }
    return QDir(candidatos.first()).absolutePath();
}

GerenciadorTraducao::GerenciadorTraducao(QObject *parent)
    : QObject(parent)
{
    idiomas_disponiveis.insert(QStringLiteral("pt_BR"), QStringLiteral("Português (Brasil)"));
    idiomas_disponiveis.insert(QStringLiteral("pt-BR"), QStringLiteral("Português (Brasil)"));
    idiomas_disponiveis.insert(QStringLiteral("en_US"), QStringLiteral("English (United States)"));
    idiomas_disponiveis.insert(QStringLiteral("en-US"), QStringLiteral("English (United States)"));

    dir_traducoes_ = obter_diretorio_traducoes();
    QDir().mkpath(dir_traducoes_);
}

GerenciadorTraducao::~GerenciadorTraducao()
{
    _remover_tradutor_instalado();
    qDeleteAll(translators_cache_);
    translators_cache_.clear();
}

void GerenciadorTraducao::_remover_tradutor_instalado()
{
    QCoreApplication *app = QCoreApplication::instance();
    if (app && tradutor_) {
        app->removeTranslator(tradutor_);
    }
    tradutor_ = nullptr;
    idioma_instalado_.clear();
}

bool GerenciadorTraducao::aplicar_traducao(const QString &codigo_idioma)
{
    QCoreApplication *app = QCoreApplication::instance();
    if (!app) {
        return false;
    }

    QString novo_codigo = codigo_idioma.trimmed();
    if (novo_codigo.isEmpty()) {
        novo_codigo = idioma_atual_;
    }

    QString base_code = novo_codigo;
    base_code.replace(QLatin1Char('-'), QLatin1Char('_'));

    const QString locale_code = QString(base_code).replace(QLatin1Char('_'), QLatin1Char('-'));
    QLocale::setDefault(QLocale(locale_code));

    // Idempotência: se o tradutor já estiver instalado para este idioma, não recarrega
    if (tradutor_ != nullptr && idioma_instalado_ == base_code) {
        return true;
    }

    // Cache lookup: se já foi carregado anteriormente, apenas instala em memória (0 I/O de disco)
    if (translators_cache_.contains(base_code)) {
        _remover_tradutor_instalado();
        idioma_atual_ = base_code;
        tradutor_ = translators_cache_.value(base_code);
        if (tradutor_ != nullptr) {
            app->installTranslator(tradutor_);
        }
        idioma_instalado_ = base_code;
        return true;
    }

    _remover_tradutor_instalado();
    idioma_atual_ = base_code;

    QString alt_code = base_code;
    alt_code.replace(QLatin1Char('_'), QLatin1Char('-'));

    const QStringList arquivos = {
        QStringLiteral("languagegantt_%1.qm").arg(base_code),
        QStringLiteral("languagegantt_%1.qm").arg(alt_code),
        QStringLiteral("gantt_%1.qm").arg(base_code),
        QStringLiteral("gantt_%1.qm").arg(alt_code),
    };

    for (const QString &arquivo : arquivos) {
        const QString caminho = QDir(dir_traducoes_).filePath(arquivo);
        if (QFile::exists(caminho)) {
            auto *new_trad = new QTranslator(this);
            if (new_trad->load(caminho)) {
                tradutor_ = new_trad;
                app->installTranslator(tradutor_);
                translators_cache_.insert(base_code, tradutor_);
                idioma_instalado_ = base_code;
                return true;
            }
            delete new_trad;
        }
    }

    // Se não encontrou no diretório inicial em cache, reavalia diretório de traduções
    dir_traducoes_ = obter_diretorio_traducoes();
    for (const QString &arquivo : arquivos) {
        const QString caminho = QDir(dir_traducoes_).filePath(arquivo);
        if (QFile::exists(caminho)) {
            auto *new_trad = new QTranslator(this);
            if (new_trad->load(caminho)) {
                tradutor_ = new_trad;
                app->installTranslator(tradutor_);
                translators_cache_.insert(base_code, tradutor_);
                idioma_instalado_ = base_code;
                return true;
            }
            delete new_trad;
        }
    }

    _remover_tradutor_instalado();
    translators_cache_.insert(base_code, nullptr);
    if (base_code.startsWith(QStringLiteral("pt"), Qt::CaseInsensitive)) {
        idioma_instalado_ = base_code;
        return true;
    }
    return false;
}

bool GerenciadorTraducao::carregar_traducao_direta(const QString &codigo_idioma, const QString &qm_file_path)
{
    auto *app = QCoreApplication::instance();
    if (!app) return false;

    QString base_code = codigo_idioma.trimmed();
    if (base_code.isEmpty()) base_code = QStringLiteral("pt_BR");
    base_code.replace(QLatin1Char('-'), QLatin1Char('_'));

    const QString locale_code = QString(base_code).replace(QLatin1Char('_'), QLatin1Char('-'));
    QLocale::setDefault(QLocale(locale_code));

    if (translators_cache_.contains(base_code) && translators_cache_.value(base_code) != nullptr) {
        _remover_tradutor_instalado();
        idioma_atual_ = base_code;
        tradutor_ = translators_cache_.value(base_code);
        app->installTranslator(tradutor_);
        idioma_instalado_ = base_code;
        return true;
    }

    if (qm_file_path.isEmpty() || !QFile::exists(qm_file_path)) {
        return false;
    }

    auto *new_trad = new QTranslator(this);
    if (new_trad->load(qm_file_path)) {
        _remover_tradutor_instalado();
        idioma_atual_ = base_code;
        tradutor_ = new_trad;
        app->installTranslator(tradutor_);
        translators_cache_.insert(base_code, tradutor_);
        idioma_instalado_ = base_code;
        emit idioma_alterado(base_code);
        return true;
    }
    delete new_trad;
    return false;
}

bool GerenciadorTraducao::definir_idioma(const QString &codigo_idioma)
{
    if (!idiomas_disponiveis.contains(codigo_idioma)) {
        return false;
    }

    idioma_atual_ = codigo_idioma;
    const bool res = aplicar_traducao(codigo_idioma);
    emit idioma_alterado(codigo_idioma);
    return res;
}

QString GerenciadorTraducao::obter_idioma_atual() const
{
    return idioma_atual_;
}

} // namespace source::gantt::language
