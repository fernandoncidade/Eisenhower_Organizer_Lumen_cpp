#include "source/modules/audiolivro/metodoLeitor/EdgeTTSClient/ettsc_02_locale_for_voice_id.hpp"

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

QString EdgeTTSClient::locale_for_voice_id(const QString &voice_id)
{
    const QString trimmed = voice_id.trimmed();

    static const QRegularExpression short_name(
        QStringLiteral("^([a-z]{2,})-([A-Z]{2,})(?:-.+Neural)$"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch short_match = short_name.match(trimmed);
    if (short_match.hasMatch()) {
        return QStringLiteral("%1-%2")
            .arg(short_match.captured(1).toLower(), short_match.captured(2).toUpper());
    }

    static const QRegularExpression microsoft_name(
        QStringLiteral("\\(([a-z]{2,}-[A-Z]{2,})\\s*,"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch microsoft_match = microsoft_name.match(trimmed);
    if (microsoft_match.hasMatch()) {
        const QString locale = microsoft_match.captured(1);
        return QStringLiteral("%1-%2").arg(locale.left(2).toLower(), locale.mid(3, 2).toUpper());
    }

    return QStringLiteral("pt-BR");
}

}  // namespace source::plugin::Audiolivro

namespace source::plugin::Audiolivro::EdgeTTSClientFns {

QString locale_for_voice_id(const QString &voice_id) { return EdgeTTSClient::locale_for_voice_id(voice_id); }

}  // namespace source::plugin::Audiolivro::EdgeTTSClientFns
