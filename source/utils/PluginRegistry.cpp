#include "source/utils/PluginRegistry.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

namespace source::utils::PluginRegistry {
namespace {

QString manifest_file_name(const QString &plugin_key)
{
    return QStringLiteral("%1_plugin.json").arg(plugin_key);
}

QStringList unique_absolute_paths(const QStringList &paths)
{
    QStringList normalized;
    for (const QString &path : paths) {
        const QString absolute = QFileInfo(path).absoluteFilePath();
        if (!normalized.contains(absolute)) {
            normalized.append(absolute);
        }
    }
    return normalized;
}

}  // namespace

QVariantMap manifest_for_key(const QString &plugin_key)
{
    const QString manifest_name = manifest_file_name(plugin_key);
    const QString modules_relative = QDir(QStringLiteral("source/modules")).filePath(
        QDir(plugin_key).filePath(manifest_name));
    const QString plugins_relative = QDir(QStringLiteral("plugins")).filePath(
        QDir(plugin_key).filePath(manifest_name));
    QStringList candidates = {
        QDir(QCoreApplication::applicationDirPath()).filePath(plugins_relative),
        QDir(QDir::currentPath()).filePath(modules_relative),
        QDir(QCoreApplication::applicationDirPath()).filePath(modules_relative),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../%1").arg(modules_relative)),
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../../../%1").arg(modules_relative)),
    };

    for (const QString &candidate : unique_absolute_paths(candidates)) {
        QFile file(candidate);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        if (document.isObject()) {
            QVariantMap manifest = document.object().toVariantMap();
            manifest.insert(QStringLiteral("path"), candidate);
            return manifest;
        }
    }
    return {};
}

QStringList binary_names(const QString &binary_base_name)
{
#if defined(Q_OS_WIN)
    return {QStringLiteral("%1.dll").arg(binary_base_name), QStringLiteral("lib%1.dll").arg(binary_base_name)};
#elif defined(Q_OS_MACOS)
    return {QStringLiteral("lib%1.dylib").arg(binary_base_name), QStringLiteral("%1.dylib").arg(binary_base_name)};
#else
    return {QStringLiteral("lib%1.so").arg(binary_base_name), QStringLiteral("%1.so").arg(binary_base_name)};
#endif
}

QStringList candidate_paths(const QString &plugin_key, const QString &binary_base_name)
{
    const QString app_dir = QCoreApplication::applicationDirPath();
    const QString current_dir = QDir::currentPath();

#if defined(_MSC_VER)
    const QString toolchain_suffix = QStringLiteral("msvc");
#else
    const QString toolchain_suffix = QStringLiteral("mingw");
#endif

    QStringList base_dirs = {
        QDir(app_dir).filePath(QStringLiteral("plugins/%1").arg(plugin_key)),
        QDir(app_dir).filePath(QStringLiteral("../build_%1_%2/plugins/%1").arg(plugin_key, toolchain_suffix)),
        QDir(app_dir).filePath(QStringLiteral("../../build_%1_%2/plugins/%1").arg(plugin_key, toolchain_suffix)),
        QDir(current_dir).filePath(QStringLiteral("build/build_%1_%2/plugins/%1").arg(plugin_key, toolchain_suffix)),
        QDir(current_dir).filePath(QStringLiteral("build_%1_%2/plugins/%1").arg(plugin_key, toolchain_suffix)),
        app_dir,
    };

    QStringList candidates;
    for (const QString &filename : binary_names(binary_base_name)) {
        for (const QString &dir : std::as_const(base_dirs)) {
            candidates << QDir(dir).filePath(filename);
        }
    }
    return unique_absolute_paths(candidates);
}

}  // namespace source::utils::PluginRegistry
