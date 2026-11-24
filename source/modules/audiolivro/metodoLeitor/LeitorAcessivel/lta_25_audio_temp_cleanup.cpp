#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_25_audio_temp_cleanup.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::cleanup_paths_with_retry(const QStringList &paths, int retries, int delay_ms)
{
    QStringList pending;
    for (const QString &path : paths) {
        const QString normalized = path.trimmed();
        if (!normalized.isEmpty() && !pending.contains(normalized)) {
            pending.append(normalized);
        }
    }

    for (int attempt = 0; attempt <= qMax(0, retries) && !pending.isEmpty(); ++attempt) {
        QStringList still_pending;
        for (const QString &path : std::as_const(pending)) {
            if (!QFileInfo::exists(path) || QFile::remove(path) || !QFileInfo::exists(path)) {
                continue;
            }
            still_pending.append(path);
        }

        pending = still_pending;
        if (!pending.isEmpty() && attempt < retries && delay_ms > 0) {
            QThread::msleep(static_cast<unsigned long>(delay_ms));
        }
    }
}

void LeitorAcessivel::cleanup_edge_tts_temp_in_outdir(bool remove_mp3, bool remove_part)
{
    QDir dir(audiolivro_persistent_dir());
    const QStringList filters = {remove_mp3 ? QStringLiteral("audiolivro_*.mp3") : QString(),
                                 QStringLiteral("audiolivro_*.wav"),
                                 QStringLiteral("audiolivro_*.txt"),
                                 remove_part ? QStringLiteral("audiolivro_*.part") : QString()};
    for (const QString &filter : filters) {
        if (filter.isEmpty()) {
            continue;
        }
        for (const QString &file : dir.entryList({filter}, QDir::Files)) {
            QFile::remove(dir.filePath(file));
        }
    }
}

void LeitorAcessivel::cleanup_edge_tts_parts_in_outdir()
{
    cleanup_edge_tts_temp_in_outdir(false, true);
}

}  // namespace source::plugin::Audiolivro
