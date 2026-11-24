#include "WebhookDispatcher.hpp"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QMessageAuthenticationCode>
#include <QUuid>

namespace source::gantt {

WebhookDispatcher::WebhookDispatcher()
{
}

QString WebhookDispatcher::register_subscription(const QString &url, const QString &secret, const QStringList &events)
{
    WebhookSubscription sub;
    sub.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    sub.url = url;
    sub.secret = secret;
    sub.subscribed_events = events;
    sub.active = true;

    subscriptions_.append(sub);
    return sub.id;
}

bool WebhookDispatcher::remove_subscription(const QString &sub_id)
{
    qsizetype removed = subscriptions_.removeIf([&sub_id](const WebhookSubscription &s) {
        return s.id == sub_id;
    });
    return (removed > 0);
}

QList<WebhookSubscription> WebhookDispatcher::list_subscriptions() const
{
    return subscriptions_;
}

QString WebhookDispatcher::compute_signature(const QByteArray &payload, const QString &secret)
{
    QByteArray hmac = QMessageAuthenticationCode::hash(
        payload,
        secret.toUtf8(),
        QCryptographicHash::Sha256
    );
    return QString::fromLatin1(hmac.toHex());
}

void WebhookDispatcher::dispatch_event(const QString &event_name,
                                      const QJsonObject &event_data,
                                      const QString &custom_idempotency_key)
{
    QString idemp_key = custom_idempotency_key.isEmpty()
                            ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                            : custom_idempotency_key;

    if (!custom_idempotency_key.isEmpty()) {
        for (const auto &d : deliveries_) {
            if (d.idempotency_key == custom_idempotency_key) {
                // Idempotent: event with this key was already dispatched
                return;
            }
        }
    }

    QJsonObject envelope;
    envelope[QStringLiteral("event")] = event_name;
    envelope[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    envelope[QStringLiteral("idempotency_key")] = idemp_key;
    envelope[QStringLiteral("data")] = event_data;

    for (const auto &sub : subscriptions_) {
        if (!sub.active) continue;
        if (!sub.subscribed_events.isEmpty() && !sub.subscribed_events.contains(event_name) && !sub.subscribed_events.contains(QStringLiteral("*"))) {
            continue;
        }

        WebhookDelivery delivery;
        delivery.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        delivery.subscription_id = sub.id;
        delivery.event_name = event_name;
        delivery.idempotency_key = idemp_key;
        delivery.payload = envelope;
        delivery.created_at = QDateTime::currentDateTime();
        delivery.attempt_count = 1;
        delivery.delivered = true; // In synchronous dispatch simulation / queue
        delivery.response_status_code = 200;

        deliveries_.prepend(delivery);
    }

    if (deliveries_.size() > 500) {
        deliveries_.removeLast();
    }
}

QList<WebhookDelivery> WebhookDispatcher::list_deliveries(int limit) const
{
    QList<WebhookDelivery> result;
    int count = 0;
    for (const auto &d : deliveries_) {
        result.append(d);
        count++;
        if (count >= limit) break;
    }
    return result;
}

int WebhookDispatcher::pending_retries_count() const
{
    int count = 0;
    for (const auto &d : deliveries_) {
        if (!d.delivered && d.attempt_count < 5) {
            count++;
        }
    }
    return count;
}

void WebhookDispatcher::process_pending_deliveries()
{
    for (auto &d : deliveries_) {
        if (!d.delivered && d.attempt_count < 5) {
            d.attempt_count++;
            d.delivered = true;
            d.response_status_code = 200;
        }
    }
}

} // namespace source::gantt
