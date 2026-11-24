#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include "GanttEnums.hpp"

namespace source::gantt {

class AuditLog {
public:
    AuditLog();
    AuditLog(const QString &type, const QString &id, const QString &act, const QString &user = QStringLiteral("system"), UserRole role = UserRole::Engineer);

    QString id;
    QString entity_type; // "Schedule", "ScheduleItem", "Dependency", "Baseline", "BimLink"
    QString entity_id;
    QString action;      // "CREATE", "UPDATE", "DELETE", "RECALCULATE", "FREEZE_BASELINE", "UNFREEZE_BASELINE"
    QDateTime timestamp;
    QString user_id;
    UserRole user_role = UserRole::Engineer;
    QJsonObject details;

    QJsonObject to_json() const;
    static AuditLog from_json(const QJsonObject &json);
};

} // namespace source::gantt
