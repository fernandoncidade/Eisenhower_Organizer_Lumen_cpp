#include "source/utils/ApplicationPathUtils.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "source/utils/AppConfig.hpp"
#include "source/utils/LogManager.hpp"

namespace source::utils {
namespace {

QStringList base_candidates()
{
    const QString app_dir = QCoreApplication::applicationDirPath();
    const QString current_dir = QDir::currentPath();

    QStringList candidates;
    candidates << QDir(app_dir).filePath(QStringLiteral("source"))
               << QDir(current_dir).filePath(QStringLiteral("source"))
               << QDir(app_dir).filePath(QStringLiteral("../source"))
               << QDir(app_dir).filePath(QStringLiteral("../../source"))
               << QDir(app_dir).filePath(QStringLiteral("../../../source"))
               << app_dir;

    QStringList normalized;
    for (const QString &candidate : candidates) {
        const QString absolute = QFileInfo(candidate).absoluteFilePath();
        if (!normalized.contains(absolute)) {
            normalized << absolute;
        }
    }
    return normalized;
}

QStringList asset_base_candidates()
{
    const QString base_path = get_app_base_path();
    if (base_path.isEmpty()) {
        return {};
    }

    const QString parent_base = QFileInfo(base_path).dir().absolutePath();
    const QString assets_subdir = AppConfig::assets_subdir_name();
    QStringList candidates;

    auto add_path = [&](const QString &path) {
        const QString absolute = QFileInfo(path).absoluteFilePath();
        if (!candidates.contains(absolute)) {
            candidates << absolute;
        }
    };

    add_path(QDir(base_path).filePath(QStringLiteral("assets/%1").arg(assets_subdir)));
    add_path(QDir(base_path).filePath(QStringLiteral("assets")));
    add_path(QDir(base_path).filePath(QStringLiteral("_internal/assets/%1").arg(assets_subdir)));
    add_path(QDir(base_path).filePath(QStringLiteral("_internal/assets")));
    add_path(QDir(parent_base).filePath(QStringLiteral("assets/%1").arg(assets_subdir)));
    add_path(QDir(parent_base).filePath(QStringLiteral("assets")));
    add_path(QDir(base_path).filePath(QStringLiteral("main.dist/assets/%1").arg(assets_subdir)));
    add_path(QDir(base_path).filePath(QStringLiteral("main.dist/assets")));
    add_path(QDir(base_path).filePath(QStringLiteral("dist/main.dist/assets/%1").arg(assets_subdir)));
    add_path(QDir(base_path).filePath(QStringLiteral("dist/main.dist/assets")));

    return candidates;
}

}  // namespace

QString get_app_base_path()
{
    try {
        for (const QString &candidate : base_candidates()) {
            if (QDir(candidate).exists()) {
                return candidate;
            }
        }

        return QFileInfo(QCoreApplication::applicationDirPath()).absoluteFilePath();
    } catch (...) {
        LogManager::get_logger()->error(QStringLiteral("Erro ao obter caminho base do app."));
        return QString();
    }
}

QString get_assets_base_path()
{
    try {
        const QStringList candidates = asset_base_candidates();
        for (const QString &candidate : candidates) {
            if (QDir(candidate).exists()) {
                return candidate;
            }
        }

        return candidates.isEmpty() ? QString() : candidates.constFirst();
    } catch (...) {
        LogManager::get_logger()->error(QStringLiteral("Erro ao obter caminho de assets do app."));
        return QString();
    }
}

QString get_text_file_path(const QString &filename, const QString &folder)
{
    try {
        const QString base_path = get_app_base_path();
        if (base_path.isEmpty()) {
            return QString();
        }

        const QString parent_base = QFileInfo(base_path).dir().absolutePath();
        QStringList possible_paths;

        auto add_path = [&](const QString &path) {
            const QString absolute = QFileInfo(path).absoluteFilePath();
            if (!possible_paths.contains(absolute)) {
                possible_paths << absolute;
            }
        };

        const QString relative_path =
            folder.isEmpty() ? filename : QStringLiteral("%1/%2").arg(folder, filename);

        for (const QString &assets_base : asset_base_candidates()) {
            add_path(QDir(assets_base).filePath(relative_path));
        }
        add_path(QDir(base_path).filePath(relative_path));
        add_path(QDir(base_path).filePath(QStringLiteral("_internal/%1").arg(relative_path)));
        add_path(QDir(parent_base).filePath(relative_path));

        for (const QString &path : possible_paths) {
            if (QFileInfo::exists(path)) {
                return path;
            }
        }

        return possible_paths.isEmpty() ? QString() : possible_paths.constFirst();
    } catch (...) {
        LogManager::get_logger()->error(
            QStringLiteral("Erro ao obter caminho do arquivo de texto '%1'.").arg(filename));
        return QString();
    }
}

QString load_text_file(const QString &filename, const QString &folder, const QByteArray &encoding)
{
    const QString file_path = get_text_file_path(filename, folder);
    if (file_path.isEmpty()) {
        LogManager::get_logger()->error(
            QStringLiteral("Arquivo de texto '%1' não encontrado.").arg(filename));
        return QString();
    }

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly)) {
        LogManager::get_logger()->error(
            QStringLiteral("Erro ao carregar arquivo de texto '%1'.").arg(filename));
        return QString();
    }

    const QByteArray data = file.readAll();
    const QByteArray normalized = encoding.toLower();
    if (normalized == QByteArrayLiteral("utf-16")) {
        return QString::fromUtf16(reinterpret_cast<const char16_t *>(data.constData()), data.size() / 2);
    }
    if (normalized == QByteArrayLiteral("latin-1")) {
        return QString::fromLatin1(data);
    }
    return QString::fromUtf8(data);
}

}  // namespace source::utils
