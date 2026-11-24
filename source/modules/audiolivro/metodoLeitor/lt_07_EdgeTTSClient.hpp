#pragma once

#include <QByteArray>
#include <QString>
#include <QVariantList>

namespace source::plugin::Audiolivro {

struct EdgeTTSResult {
    bool ok = false;
    QString error;
    QByteArray audio;
    QVariantList timestamps;
};

class EdgeTTSClient
{
public:
    static EdgeTTSResult synthesize(const QString &text,
                                    const QString &voice_id,
                                    int rate_pct,
                                    int volume_pct,
                                    int timeout_ms = 180000);
    static bool synthesize_to_file(const QString &text,
                                   const QString &voice_id,
                                   int rate_pct,
                                   int volume_pct,
                                   const QString &outfile,
                                   QString *error = nullptr,
                                   QVariantList *timestamps = nullptr,
                                   int timeout_ms = 180000);
    static QVariantList available_voice_options(int timeout_ms = 6000);
    static bool is_edge_voice_id(const QString &voice_id);
    static QString locale_for_voice_id(const QString &voice_id);
    static QString normalize_edge_voice_name(const QString &voice_id);
};

}  // namespace source::plugin::Audiolivro
