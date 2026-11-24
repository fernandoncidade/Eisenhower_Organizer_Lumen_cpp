#include "source/utils/CaminhoPersistenteUtils.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

#include "source/utils/AppConfig.hpp"

namespace source::utils {
namespace {

bool copy_file_if_missing(const QString &source_path, const QString &destination_path)
{
    if (!QFileInfo::exists(source_path) || QFileInfo::exists(destination_path)) {
        return false;
    }

    QDir().mkpath(QFileInfo(destination_path).absolutePath());
    return QFile::copy(source_path, destination_path);
}

bool copy_directory_files_if_missing(const QString &source_path, const QString &destination_path)
{
    const QDir source_dir(source_path);
    if (!source_dir.exists()) {
        return false;
    }

    bool copied = false;
    QDir().mkpath(destination_path);
    const QFileInfoList entries =
        source_dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &entry : entries) {
        const QString destination_entry = QDir(destination_path).filePath(entry.fileName());
        if (entry.isDir()) {
            copied = copy_directory_files_if_missing(entry.absoluteFilePath(), destination_entry) || copied;
        } else {
            copied = copy_file_if_missing(entry.absoluteFilePath(), destination_entry) || copied;
        }
    }
    return copied;
}

void migrate_lumen_files_if_needed(const QString &root, const QString &config_dir)
{
    if (!AppConfig::is_lumen()) {
        return;
    }

    const QString source_dir_path = QDir(root).filePath(QStringLiteral("EisenhowerOrganizer"));
    if (!QDir(source_dir_path).exists()
        || QFileInfo(source_dir_path).absoluteFilePath() == QFileInfo(config_dir).absoluteFilePath()) {
        return;
    }

    const QString marker_path = QDir(config_dir).filePath(QStringLiteral(".lumen_persistence_migrated"));
    if (QFileInfo::exists(marker_path)) {
        return;
    }

    const QStringList files = {
        QStringLiteral("audiolivro_config.json"),
        QStringLiteral("language.json"),
        QStringLiteral("layout.json"),
        QStringLiteral("mapa_mental_autosave.json"),
        QStringLiteral("mapamental_config.json"),
        QStringLiteral("pomodoro_alarm_config.json"),
        QStringLiteral("tarefas.json"),
        QStringLiteral("tarefas.json.bak"),
    };

    bool copied = false;
    for (const QString &file_name : files) {
        copied = copy_file_if_missing(QDir(source_dir_path).filePath(file_name), QDir(config_dir).filePath(file_name))
                 || copied;
    }
    copied = copy_directory_files_if_missing(QDir(source_dir_path).filePath(QStringLiteral("logs")),
                                             QDir(config_dir).filePath(QStringLiteral("logs")))
             || copied;

    if (copied) {
        QFile marker(marker_path);
        if (marker.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            marker.write("migrated\n");
        }
    }
}

}  // namespace

QString obter_caminho_persistente()
{
    const QString local_app_data = qEnvironmentVariable("LOCALAPPDATA");
    const QString root = local_app_data.isEmpty() ? QDir::homePath() : local_app_data;
    const QString config_dir = QDir(root).filePath(AppConfig::local_app_data_dir_name());

    if (!QDir(config_dir).exists() && !QDir().mkpath(config_dir)) {
        qCritical() << "Erro ao criar diretório de configuração:" << config_dir;
    }
    migrate_lumen_files_if_needed(root, config_dir);

    return config_dir;
}

}  // namespace source::utils
