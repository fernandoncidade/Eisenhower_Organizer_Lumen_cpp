#pragma once

#include <QList>
#include <QString>
#include "../../domain/models/AuditLog.hpp"

namespace source::gantt {

class AuditService {
public:
    AuditService();

    void record_action(const QString &entity_type,
                       const QString &entity_id,
                       const QString &action,
                       const QString &user_id = QStringLiteral("system"),
                       UserRole role = UserRole::Engineer,
                       const QJsonObject &details = QJsonObject());

    QList<AuditLog> get_logs_for_entity(const QString &entity_type, const QString &entity_id) const;
    QList<AuditLog> list_recent_logs(int limit = 100) const;

private:
    QList<AuditLog> logs_;
};

} // namespace source::gantt
