#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_38_modo_layout.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSettings>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_29_aplicar_layout_arvore.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"

namespace source::plugin::MapaMental {
namespace {

constexpr auto kLayoutModeSettingsKey = "layout/mode";
constexpr auto kConfigFileName = "mapamental_config.json";
constexpr auto kLayoutModeConfigKey = "layout_mode";
constexpr auto kSummaryMethodConfigKey = "summary_method";

MapaMental::LayoutMode layout_mode_from_key(QString key)
{
    key = key.trimmed().toLower();
    key.remove(QLatin1Char('_'));
    key.remove(QLatin1Char('-'));
    key.remove(QLatin1Char(' '));

    if (key == QStringLiteral("horizontal")) {
        return MapaMental::LayoutMode::Horizontal;
    }
    if (key == QStringLiteral("radial")) {
        return MapaMental::LayoutMode::Radial;
    }
    if (key == QStringLiteral("tree") || key == QStringLiteral("arvore") || key == QStringLiteral("árvore")) {
        return MapaMental::LayoutMode::Tree;
    }

    return MapaMental::LayoutMode::Vertical;
}

QString layout_mode_to_key(MapaMental::LayoutMode mode)
{
    switch (mode) {
    case MapaMental::LayoutMode::Horizontal:
        return QStringLiteral("horizontal");
    case MapaMental::LayoutMode::Radial:
        return QStringLiteral("radial");
    case MapaMental::LayoutMode::Tree:
        return QStringLiteral("tree");
    case MapaMental::LayoutMode::Vertical:
    default:
        return QStringLiteral("vertical");
    }
}

QString config_path()
{
    return QDir(source::utils::obter_caminho_persistente()).filePath(QString::fromLatin1(kConfigFileName));
}

QJsonObject read_config()
{
    QFile file(config_path());
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    return document.isObject() ? document.object() : QJsonObject{};
}

void write_config(const QJsonObject &config)
{
    const QString path = config_path();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    const QByteArray payload = QJsonDocument(config).toJson(QJsonDocument::Indented);
    if (file.write(payload) == payload.size()) {
        file.commit();
    }
}

QString legacy_layout_mode()
{
    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       QStringLiteral("LumenCpp"),
                       QStringLiteral("MapaMentalPlugin"));
    return settings.value(QString::fromLatin1(kLayoutModeSettingsKey)).toString().trimmed();
}

QString read_layout_mode()
{
    const QJsonObject config = read_config();
    const QString stored = config.value(QString::fromLatin1(kLayoutModeConfigKey)).toString().trimmed();
    if (!stored.isEmpty()) {
        return stored;
    }

    const QString legacy = legacy_layout_mode();
    if (!legacy.isEmpty()) {
        QJsonObject migrated = config;
        migrated.insert(QString::fromLatin1(kLayoutModeConfigKey), legacy);
        write_config(migrated);
        return legacy;
    }

    return QStringLiteral("vertical");
}

void save_layout_mode(const QString &mode_key)
{
    QJsonObject config = read_config();
    config.insert(QString::fromLatin1(kLayoutModeConfigKey), mode_key);
    write_config(config);
}

QString normalized_summary_method(QString method_key)
{
    method_key = method_key.trimmed();
    const QString lower = method_key.toLower();
    if (lower == QStringLiteral("heuristic") || lower == QStringLiteral("heuristico")) {
        return QStringLiteral("heuristic");
    }
    if (lower.startsWith(QStringLiteral("ollama:"))) {
        const QString model_name = method_key.mid(7).trimmed();
        if (!model_name.isEmpty()) {
            return QStringLiteral("ollama:") + model_name;
        }
    }
    return QStringLiteral("qwen");
}

QString read_summary_method()
{
    return normalized_summary_method(
        read_config().value(QString::fromLatin1(kSummaryMethodConfigKey)).toString(QStringLiteral("qwen")));
}

void save_summary_method(const QString &method_key)
{
    QJsonObject config = read_config();
    config.insert(QString::fromLatin1(kSummaryMethodConfigKey), normalized_summary_method(method_key));
    write_config(config);
}

}  // namespace

QString MapaMental::layout_mode_key() const
{
    return layout_mode_to_key(layout_mode_);
}

void MapaMental::set_layout_mode(LayoutMode mode, bool persist)
{
    if (layout_mode_ == mode) {
        if (persist) {
            salvar_preferencia_layout();
        }
        return;
    }

    layout_mode_ = mode;
    if (persist) {
        salvar_preferencia_layout();
    }

    if (!nos_.isEmpty()) {
        aplicar_layout_arvore();
        salvar_backup_automatico();
    }
}

void MapaMental::set_layout_mode_key(const QString &mode_key, bool persist)
{
    set_layout_mode(layout_mode_from_key(mode_key), persist);
}

void MapaMental::carregar_preferencia_layout()
{
    set_layout_mode_key(read_layout_mode(), false);
    set_summary_method_key(read_summary_method(), false);
}

void MapaMental::salvar_preferencia_layout() const
{
    save_layout_mode(layout_mode_key());
}

QString MapaMental::summary_method_key() const
{
    return summary_method_key_;
}

void MapaMental::set_summary_method_key(const QString &method_key, bool persist)
{
    summary_method_key_ = normalized_summary_method(method_key);
    if (persist) {
        save_summary_method(summary_method_key_);
    }
}

}  // namespace source::plugin::MapaMental
