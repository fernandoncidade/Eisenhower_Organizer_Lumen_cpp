#pragma once

#include <QDateTime>
#include <QHash>
#include <QString>
#include "../../domain/models/GanttEnums.hpp"

namespace source::gantt {

struct UserSession {
    QString user_id;
    QString token;
    UserRole role = UserRole::Viewer;
    QDateTime expires_at;
};

class SecurityService {
public:
    SecurityService();

    // Session / Token Management
    QString issue_token(const QString &user_id, UserRole role, int validity_hours = 24);
    bool validate_token(const QString &token, UserRole &out_role, QString &out_user_id) const;

    // RBAC Permissions
    bool can_view(UserRole role) const;
    bool can_edit(UserRole role) const;
    bool can_manage_baselines(UserRole role) const;
    bool can_administer(UserRole role) const;

    // Rate Limiting (Token Bucket)
    bool check_rate_limit(const QString &client_identifier, int max_per_minute = 60);

private:
    QHash<QString, UserSession> active_tokens_;

    struct RateBucket {
        int tokens = 60;
        QDateTime last_refill;
    };
    mutable QHash<QString, RateBucket> rate_buckets_;
};

} // namespace source::gantt
