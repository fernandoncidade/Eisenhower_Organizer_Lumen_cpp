#include "IconUtils.hpp"

#include "source/utils/LogManager.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QHash>

QString get_icon_app_base_path() {
    static QString s_cached_base_path;
    if (!s_cached_base_path.isEmpty() && QDir(s_cached_base_path).exists()) {
        return s_cached_base_path;
    }
    const QString app_dir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        app_dir,
        QDir(app_dir).absoluteFilePath("agenda"),
        QDir(app_dir).absoluteFilePath("plugins/agenda"),
        QDir::currentPath(),
        QDir(QDir::currentPath()).absoluteFilePath("source/modules/agenda/AgendaAvaliacoes"),
    };

    for (const QString &candidate : candidates) {
        const QString clean = QDir::cleanPath(candidate);
        if (QDir(clean + "/assets/icones").exists() || QDir(clean + "/icones").exists()) {
            s_cached_base_path = QDir(clean).absolutePath();
            return s_cached_base_path;
        }
    }

    s_cached_base_path = QDir(app_dir).absolutePath();
    return s_cached_base_path;
}

QString get_icon_path(const QString &icon_name) {
    static QHash<QString, QString> s_icon_cache;
    auto it = s_icon_cache.constFind(icon_name);
    if (it != s_icon_cache.constEnd()) {
        return it.value();
    }
    const QString basePath = get_icon_app_base_path();
    const QStringList possiblePaths = {basePath + "/assets/icones/" + icon_name, basePath + "/icones/" + icon_name,
                                       basePath + "/_internal/icones/" + icon_name,
                                       QDir(basePath).absoluteFilePath("../assets/icones/" + icon_name),
                                       QDir(basePath).absoluteFilePath("../icones/" + icon_name)};
    for (const QString &path : possiblePaths) {
        const QString abs = QDir::cleanPath(path);
        if (QFile::exists(abs)) {
            s_icon_cache.insert(icon_name, abs);
            return abs;
        }
    }
    const QString fallback = possiblePaths.isEmpty() ? QString() : QDir::cleanPath(possiblePaths.first());
    s_icon_cache.insert(icon_name, fallback);
    return fallback;
}
