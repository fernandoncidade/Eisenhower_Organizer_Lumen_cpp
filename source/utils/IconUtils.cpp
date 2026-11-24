#include "source/utils/IconUtils.hpp"

#include <QDir>
#include <QFileInfo>

#include "source/utils/AppConfig.hpp"
#include "source/utils/ApplicationPathUtils.hpp"
#include "source/utils/LogManager.hpp"

namespace source::utils {
namespace {

QStringList icon_name_candidates(const QString &icon_name)
{
    QStringList names;
    auto add_name = [&](const QString &name) {
        if (!name.isEmpty() && !names.contains(name)) {
            names << name;
        }
    };

    add_name(icon_name);

    const QString normalized = icon_name.trimmed().toCaseFolded();
    if (AppConfig::is_lumen()
        && (normalized == QStringLiteral("organizador.ico")
            || normalized == QStringLiteral("logotipo.ico"))) {
        add_name(AppConfig::default_window_icon_name());
    } else if (AppConfig::is_lumen()
               && (normalized == QStringLiteral("organizador.png")
                   || normalized == QStringLiteral("logotipo.png")
                   || normalized == QStringLiteral("slogan.png"))) {
        add_name(AppConfig::default_window_icon_image_name());
    }

    return names;
}

}  // namespace

QString get_default_window_icon_name()
{
    return AppConfig::default_window_icon_name();
}

QString get_default_window_icon_path()
{
    return get_icon_path(get_default_window_icon_name());
}

QString get_icon_path(const QString &icon_name)
{
    try {
        const QString base_path = get_app_base_path();
        const QString assets_base_path = get_assets_base_path();
        const QString source_dir = QFileInfo(base_path).dir().absolutePath();

        QStringList possible_paths;
        auto add_path = [&](const QString &path) {
            const QString absolute = QFileInfo(path).absoluteFilePath();
            if (!possible_paths.contains(absolute)) {
                possible_paths << absolute;
            }
        };

        for (const QString &candidate_name : icon_name_candidates(icon_name)) {
            if (!assets_base_path.isEmpty()) {
                add_path(QDir(assets_base_path).filePath(QStringLiteral("icones/%1").arg(candidate_name)));
            }
            add_path(QDir(base_path).filePath(
                QStringLiteral("assets/%1/icones/%2").arg(AppConfig::assets_subdir_name(), candidate_name)));
            add_path(QDir(base_path).filePath(QStringLiteral("assets/icones/%1").arg(candidate_name)));
            add_path(QDir(base_path).filePath(QStringLiteral("icones/%1").arg(candidate_name)));
            add_path(QDir(base_path).filePath(QStringLiteral("_internal/assets/%1/icones/%2")
                                                  .arg(AppConfig::assets_subdir_name(), candidate_name)));
            add_path(QDir(base_path).filePath(QStringLiteral("_internal/assets/icones/%1").arg(candidate_name)));
            add_path(QDir(base_path).filePath(QStringLiteral("_internal/icones/%1").arg(candidate_name)));
            add_path(QDir(source_dir).filePath(
                QStringLiteral("assets/%1/icones/%2").arg(AppConfig::assets_subdir_name(), candidate_name)));
            add_path(QDir(source_dir).filePath(QStringLiteral("assets/icones/%1").arg(candidate_name)));
            add_path(QDir(source_dir).filePath(QStringLiteral("icones/%1").arg(candidate_name)));
        }

        for (const QString &icon_path : possible_paths) {
            const QString absolute = QFileInfo(icon_path).absoluteFilePath();
            if (QFileInfo::exists(absolute)) {
                return absolute;
            }
        }

        return possible_paths.isEmpty() ? QString() : possible_paths.constFirst();
    } catch (...) {
        LogManager::get_logger()->error(
            QStringLiteral("Erro ao obter caminho do ícone '%1'.").arg(icon_name));
        return QString();
    }
}

}  // namespace source::utils
