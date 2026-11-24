#include "source/modules/audiolivro/metodoLeitor/lt_04_EdgeTTSWithTimestamps.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"

#include <QDir>
#include <QMutexLocker>
#include <QRegularExpression>

namespace source::plugin::Audiolivro {

EdgeTTSWithTimestamps::EdgeTTSWithTimestamps(QString texto,
                                             QString voz,
                                             int rate_pct,
                                             int volume_pct,
                                             QString outdir,
                                             QObject *parent)
    : QThread(parent),
      texto_(std::move(texto)),
      voz_(std::move(voz)),
      rate_pct_(rate_pct),
      volume_pct_(volume_pct),
      outdir_(std::move(outdir))
{
    if (outdir_.trimmed().isEmpty()) {
        outdir_ = audiolivro_persistent_dir();
    }
    QDir().mkpath(outdir_);
}

EdgeTTSWithTimestamps::~EdgeTTSWithTimestamps()
{
    stop();
    wait(1500);
}

QVariantList EdgeTTSWithTimestamps::build_estimated_timestamps(const QString &segment, int global_offset) const
{
    QVariantList timestamps;
    const int wpm = qBound(80, 150 + (rate_pct_ * 2), 260);
    const int ms_per_word = qMax(180, 60000 / wpm);

    QRegularExpression word_re(QStringLiteral("\\S+"));
    QRegularExpressionMatchIterator it = word_re.globalMatch(segment);
    int index = 0;
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        WordTimestamp timestamp;
        timestamp.text = match.captured();
        timestamp.offset_ms = qint64(index) * ms_per_word;
        timestamp.duration_ms = qMax(120, ms_per_word - 40);
        timestamp.text_offset = global_offset + match.capturedStart();
        QVariantMap data = timestamp.to_map();
        data.insert(QStringLiteral("source_length"), match.capturedLength());
        timestamps.append(data);
        ++index;
    }

    return timestamps;
}

}  // namespace source::plugin::Audiolivro
