#include "GanttTranslationWorker.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace source::gantt::language {

GanttTranslationWorker::GanttTranslationWorker(QObject *parent)
    : QObject(parent)
{
}

void GanttTranslationWorker::prepare_translation(const QString &language_code)
{
    const QString qm_path = find_translation_file(language_code);
    emit translation_prepared(language_code, qm_path);
}

QString GanttTranslationWorker::find_translation_file(const QString &language_code) const
{
    QString base_code = language_code.trimmed();
    if (base_code.isEmpty()) {
        base_code = QStringLiteral("pt_BR");
    }
    base_code.replace(QLatin1Char('-'), QLatin1Char('_'));

    QString alt_code = base_code;
    alt_code.replace(QLatin1Char('_'), QLatin1Char('-'));

    const QString app_dir = QCoreApplication::applicationDirPath();
    const QString current = QDir::currentPath();

    const QStringList candidate_dirs = {
        app_dir + QStringLiteral("/translations"),
        current + QStringLiteral("/source/language/translations"),
        app_dir + QStringLiteral("/source/language/translations"),
        QDir(app_dir).absoluteFilePath(QStringLiteral("../source/language/translations")),
        QDir(current).absoluteFilePath(QStringLiteral("../source/language/translations")),
    };

    const QStringList candidate_files = {
        QStringLiteral("languagegantt_%1.qm").arg(base_code),
        QStringLiteral("languagegantt_%1.qm").arg(alt_code),
        QStringLiteral("gantt_%1.qm").arg(base_code),
        QStringLiteral("gantt_%1.qm").arg(alt_code),
    };

    for (const QString &dir_path : candidate_dirs) {
        const QDir dir(dir_path);
        if (!dir.exists()) {
            continue;
        }
        for (const QString &file_name : candidate_files) {
            const QString full_path = dir.filePath(file_name);
            if (QFile::exists(full_path)) {
                return QFileInfo(full_path).absoluteFilePath();
            }
        }
    }

    return QString();
}

} // namespace source::gantt::language
