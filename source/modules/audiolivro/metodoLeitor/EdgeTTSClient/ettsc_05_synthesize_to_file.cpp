#include "source/modules/audiolivro/metodoLeitor/EdgeTTSClient/ettsc_05_synthesize_to_file.hpp"

#include "source/modules/audiolivro/metodoLeitor/EdgeTTSClient/ettsc_internal.hpp"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QTimer>
#include <QWebSocket>

namespace source::plugin::Audiolivro {
using namespace EdgeTTSClientInternal;

bool EdgeTTSClient::synthesize_to_file(const QString &text,
                                       const QString &voice_id,
                                       int rate_pct,
                                       int volume_pct,
                                       const QString &outfile,
                                       QString *error,
                                       QVariantList *timestamps,
                                       int timeout_ms)
{
    QFile::remove(outfile);
    const EdgeTTSResult result = synthesize(text, voice_id, rate_pct, volume_pct, timeout_ms);
    if (!result.ok) {
        if (error != nullptr) {
            *error = result.error;
        }
        return false;
    }
    if (timestamps != nullptr) {
        *timestamps = result.timestamps;
    }

    QFile file(outfile);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error != nullptr) {
            *error = file.errorString();
        }
        return false;
    }
    file.write(result.audio);
    file.close();
    return file.size() > 0;
}

}  // namespace source::plugin::Audiolivro

namespace source::plugin::Audiolivro::EdgeTTSClientFns {

bool synthesize_to_file(const QString &text, const QString &voice_id, int rate_pct, int volume_pct, const QString &outfile, QString *error, QVariantList *timestamps, int timeout_ms) { return EdgeTTSClient::synthesize_to_file(text, voice_id, rate_pct, volume_pct, outfile, error, timestamps, timeout_ms); }

}  // namespace source::plugin::Audiolivro::EdgeTTSClientFns
