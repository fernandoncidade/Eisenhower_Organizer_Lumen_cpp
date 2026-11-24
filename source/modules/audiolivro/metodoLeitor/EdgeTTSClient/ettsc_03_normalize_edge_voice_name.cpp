#include "source/modules/audiolivro/metodoLeitor/EdgeTTSClient/ettsc_03_normalize_edge_voice_name.hpp"

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

QString EdgeTTSClient::normalize_edge_voice_name(const QString &voice_id)
{
    const QString trimmed = voice_id.trimmed();
    static const QRegularExpression edge_voice_id(
        QStringLiteral("^([a-z]{2,})-([A-Z]{2,})-(.+Neural)$"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = edge_voice_id.match(trimmed);
    if (!match.hasMatch()) {
        return trimmed;
    }

    const QString lang = match.captured(1);
    QString region = match.captured(2);
    QString name = match.captured(3);
    const int extra_region = name.indexOf(QLatin1Char('-'));
    if (extra_region >= 0) {
        region += QLatin1Char('-') + name.left(extra_region);
        name = name.mid(extra_region + 1);
    }
    return QStringLiteral("Microsoft Server Speech Text to Speech Voice (%1-%2, %3)").arg(lang, region, name);
}

}  // namespace source::plugin::Audiolivro

namespace source::plugin::Audiolivro::EdgeTTSClientFns {

QString normalize_edge_voice_name(const QString &voice_id) { return EdgeTTSClient::normalize_edge_voice_name(voice_id); }

}  // namespace source::plugin::Audiolivro::EdgeTTSClientFns
