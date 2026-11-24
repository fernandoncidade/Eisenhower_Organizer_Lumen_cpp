#include "AuditService.hpp"

namespace source::gantt {

AuditService::AuditService()
{
}

void AuditService::record_action(const QString &entity_type,
                                 const QString &entity_id,
                                 const QString &action,
                                 const QString &user_id,
                                 UserRole role,
                                 const QJsonObject &details)
{
    AuditLog log(entity_type, entity_id, action, user_id, role);
    log.details = details;
    logs_.prepend(log); // newest first

    // Limit in-memory logs
    if (logs_.size() > 5000) {
        logs_.removeLast();
    }
}

QList<AuditLog> AuditService::get_logs_for_entity(const QString &entity_type, const QString &entity_id) const
{
    QList<AuditLog> result;
    for (const auto &log : logs_) {
        if (log.entity_type == entity_type && log.entity_id == entity_id) {
            result.append(log);
        }
    }
    return result;
}

QList<AuditLog> AuditService::list_recent_logs(int limit) const
{
    QList<AuditLog> result;
    int count = 0;
    for (const auto &log : logs_) {
        result.append(log);
        count++;
        if (count >= limit) break;
    }
    return result;
}

} // namespace source::gantt
