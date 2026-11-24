#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>

namespace source::gantt {

struct WebhookSubscription {
    QString id;
    QString url;
    QString secret;
    QStringList subscribed_events; // e.g., ["schedule.created", "schedule.updated", "activity.updated"]
    bool active = true;
};

struct WebhookDelivery {
    QString id;
    QString subscription_id;
    QString event_name;
    QString idempotency_key;
    QJsonObject payload;
    QDateTime created_at;
    int attempt_count = 0;
    bool delivered = false;
    int response_status_code = 0;
    QString error_message;
};

class WebhookDispatcher {
public:
    WebhookDispatcher();

    // Subscription management
    QString register_subscription(const QString &url, const QString &secret, const QStringList &events);
    bool remove_subscription(const QString &sub_id);
    QList<WebhookSubscription> list_subscriptions() const;

    // Event publishing
    void dispatch_event(const QString &event_name,
                        const QJsonObject &event_data,
                        const QString &custom_idempotency_key = QString());

    // Compute HMAC-SHA256 signature
    static QString compute_signature(const QByteArray &payload, const QString &secret);

    // Delivery log / queue inspection
    QList<WebhookDelivery> list_deliveries(int limit = 50) const;
    int pending_retries_count() const;

    // Process retry queue
    void process_pending_deliveries();

private:
    QList<WebhookSubscription> subscriptions_;
    QList<WebhookDelivery> deliveries_;
};

} // namespace source::gantt
