#include "source/modules/audiolivro/metodoLeitor/EdgeTTSClient/ettsc_06_available_voice_options.hpp"

#include "source/modules/audiolivro/metodoLeitor/EdgeTTSClient/ettsc_internal.hpp"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSet>
#include <QTimer>
#include <QWebSocket>

namespace source::plugin::Audiolivro {
using namespace EdgeTTSClientInternal;

QVariantList EdgeTTSClient::available_voice_options(int timeout_ms)
{
    QNetworkAccessManager manager;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QNetworkReply *reply = manager.get(voice_list_request());
    QObject::connect(&timer, &QTimer::timeout, &loop, [&] {
        reply->abort();
        loop.quit();
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(timeout_ms);
    loop.exec();
    timer.stop();

    QVariantList voices;
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return voices;
    }

    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    if (!document.isArray()) {
        return voices;
    }

    QSet<QString> seen_ids;
    for (const QJsonValue &value : document.array()) {
        const QJsonObject object = value.toObject();
        const QString id = object.value(QStringLiteral("ShortName")).toString(
            object.value(QStringLiteral("Name")).toString());
        if (id.trimmed().isEmpty() || seen_ids.contains(id.trimmed())) {
            continue;
        }
        if (id.contains(QStringLiteral("Multilingual"), Qt::CaseInsensitive)) {
            continue;
        }
        seen_ids.insert(id.trimmed());

        const QString locale = object.value(QStringLiteral("Locale")).toString(id.section(QLatin1Char('-'), 0, 1));
        QVariantMap item;
        item.insert(QStringLiteral("id"), id);
        item.insert(QStringLiteral("language"), language_for_locale(locale));
        item.insert(QStringLiteral("locale"), locale);
        item.insert(QStringLiteral("gender"), object.value(QStringLiteral("Gender")).toString());
        item.insert(QStringLiteral("name"), friendly_name_from_id(id));
        item.insert(QStringLiteral("engine"), QStringLiteral("Edge TTS"));
        voices.append(item);
    }

    return voices;
}

}  // namespace source::plugin::Audiolivro

namespace source::plugin::Audiolivro::EdgeTTSClientFns {

QVariantList available_voice_options(int timeout_ms) { return EdgeTTSClient::available_voice_options(timeout_ms); }

}  // namespace source::plugin::Audiolivro::EdgeTTSClientFns
