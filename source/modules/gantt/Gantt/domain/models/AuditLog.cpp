#include "AuditLog.hpp"
#include <QUuid>

namespace source::gantt {

AuditLog::AuditLog()
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      timestamp(QDateTime::currentDateTime())
{
}

AuditLog::AuditLog(const QString &type, const QString &ent_id, const QString &act, const QString &user, UserRole role)
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      entity_type(type),
      entity_id(ent_id),
      action(act),
      timestamp(QDateTime::currentDateTime()),
      user_id(user),
      user_role(role)
{
}

QJsonObject AuditLog::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("id")] = id;
    json[QStringLiteral("entity_type")] = entity_type;
    json[QStringLiteral("entity_id")] = entity_id;
    json[QStringLiteral("action")] = action;
    json[QStringLiteral("timestamp")] = timestamp.toString(Qt::ISODate);
    json[QStringLiteral("user_id")] = user_id;
    json[QStringLiteral("user_role")] = user_role_to_string(user_role);
    json[QStringLiteral("details")] = details;
    return json;
}

AuditLog AuditLog::from_json(const QJsonObject &json)
{
    AuditLog log;
    if (json.contains(QStringLiteral("id"))) log.id = json[QStringLiteral("id")].toString();
    if (json.contains(QStringLiteral("entity_type"))) log.entity_type = json[QStringLiteral("entity_type")].toString();
    if (json.contains(QStringLiteral("entity_id"))) log.entity_id = json[QStringLiteral("entity_id")].toString();
    if (json.contains(QStringLiteral("action"))) log.action = json[QStringLiteral("action")].toString();
    if (json.contains(QStringLiteral("timestamp"))) log.timestamp = QDateTime::fromString(json[QStringLiteral("timestamp")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("user_id"))) log.user_id = json[QStringLiteral("user_id")].toString();
    if (json.contains(QStringLiteral("user_role"))) log.user_role = string_to_user_role(json[QStringLiteral("user_role")].toString());
    if (json.contains(QStringLiteral("details"))) log.details = json[QStringLiteral("details")].toObject();
    return log;
}

} // namespace source::gantt
