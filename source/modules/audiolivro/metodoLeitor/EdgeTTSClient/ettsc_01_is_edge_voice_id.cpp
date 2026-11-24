#include "source/modules/audiolivro/metodoLeitor/EdgeTTSClient/ettsc_01_is_edge_voice_id.hpp"

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

bool EdgeTTSClient::is_edge_voice_id(const QString &voice_id)
{
    static const QRegularExpression edge_voice_id(
        QStringLiteral("^([a-z]{2,})-([A-Z]{2,})-(.+Neural)$"),
        QRegularExpression::CaseInsensitiveOption);
    return edge_voice_id.match(voice_id.trimmed()).hasMatch()
           || voice_id.trimmed().startsWith(QStringLiteral("Microsoft Server Speech Text to Speech Voice"),
                                            Qt::CaseInsensitive);
}

}  // namespace source::plugin::Audiolivro

namespace source::plugin::Audiolivro::EdgeTTSClientFns {

bool is_edge_voice_id(const QString &voice_id) { return EdgeTTSClient::is_edge_voice_id(voice_id); }

}  // namespace source::plugin::Audiolivro::EdgeTTSClientFns
