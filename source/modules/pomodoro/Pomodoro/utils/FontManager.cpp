#include "source/modules/pomodoro/Pomodoro/utils/FontManager.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>

#include "source/modules/pomodoro/Pomodoro/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/LogManager.hpp"

namespace source::plugin::Pomodoro::utils {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

}  // namespace

QJsonObject FontManager::default_config()
{
    return {
        {QStringLiteral("family"), QStringLiteral("Arial")},
        {QStringLiteral("size"), 10},
        {QStringLiteral("bold"), false},
        {QStringLiteral("italic"), false},
        {QStringLiteral("underline"), false},
    };
}

QString FontManager::config_path()
{
    return QDir(obter_caminho_persistente()).filePath(QStringLiteral("font_config.json"));
}

QJsonObject FontManager::get_config()
{
    try {
        QFile file(config_path());
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
            return default_config();
        }

        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        const QJsonObject config = doc.object();
        QJsonObject merged = default_config();
        for (auto it = config.begin(); it != config.end(); ++it) {
            merged.insert(it.key(), it.value());
        }
        return merged;
    } catch (const std::exception &e) {
        logger()->error(QStringLiteral("Erro ao ler configuração de fontes: %1").arg(QString::fromUtf8(e.what())), true);
        return default_config();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao ler configuração de fontes"), true);
        return default_config();
    }
}

bool FontManager::save_config(const QJsonObject &config)
{
    try {
        QDir().mkpath(QFileInfo(config_path()).absolutePath());
        QSaveFile file(config_path());
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }

        const QByteArray payload = QJsonDocument(config).toJson(QJsonDocument::Indented);
        return file.write(payload) == payload.size() && file.commit();
    } catch (const std::exception &e) {
        logger()->error(QStringLiteral("Erro ao salvar configuração de fontes: %1").arg(QString::fromUtf8(e.what())), true);
        return false;
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao salvar configuração de fontes"), true);
        return false;
    }
}

QFont FontManager::get_font()
{
    const QJsonObject config = get_config();
    QFont font(config.value(QStringLiteral("family")).toString(QStringLiteral("Arial")),
               config.value(QStringLiteral("size")).toInt(10));
    font.setBold(config.value(QStringLiteral("bold")).toBool(false));
    font.setItalic(config.value(QStringLiteral("italic")).toBool(false));
    font.setUnderline(config.value(QStringLiteral("underline")).toBool(false));
    return font;
}

QString FontManager::get_html_style()
{
    const QJsonObject config = get_config();
    return QStringLiteral(
               "<style>body{font-family:'%1', monospace;font-size:%2pt;font-weight:%3;font-style:%4;text-decoration:%5;white-space:pre-wrap;word-wrap:break-word;margin:0;padding:4px;}pre{font-family:'%1', monospace;font-size:%2pt;font-weight:%3;font-style:%4;}</style>")
        .arg(config.value(QStringLiteral("family")).toString(QStringLiteral("Arial")),
             QString::number(config.value(QStringLiteral("size")).toInt(10)),
             config.value(QStringLiteral("bold")).toBool(false) ? QStringLiteral("bold") : QStringLiteral("normal"),
             config.value(QStringLiteral("italic")).toBool(false) ? QStringLiteral("italic") : QStringLiteral("normal"),
             config.value(QStringLiteral("underline")).toBool(false) ? QStringLiteral("underline") : QStringLiteral("none"));
}

bool FontManager::reset_to_default()
{
    return save_config(default_config());
}

}  // namespace source::plugin::Pomodoro::utils
