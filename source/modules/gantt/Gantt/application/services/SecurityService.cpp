#include "SecurityService.hpp"
#include <QUuid>
#include <algorithm>

namespace source::gantt {

SecurityService::SecurityService()
{
    // Seed standard development & testing tokens
    issue_token(QStringLiteral("admin"), UserRole::Admin, 8760);       // 1 year
    issue_token(QStringLiteral("engenheiro"), UserRole::Engineer, 8760);
    issue_token(QStringLiteral("gestor"), UserRole::Manager, 8760);
    issue_token(QStringLiteral("leitor"), UserRole::Viewer, 8760);

    // Hardcode convenient dev tokens
    UserSession dev_admin;
    dev_admin.user_id = QStringLiteral("admin");
    dev_admin.token = QStringLiteral("eisen-admin-token-2026");
    dev_admin.role = UserRole::Admin;
    dev_admin.expires_at = QDateTime::currentDateTime().addYears(5);
    active_tokens_.insert(dev_admin.token, dev_admin);

    UserSession dev_eng;
    dev_eng.user_id = QStringLiteral("engenheiro_obra");
    dev_eng.token = QStringLiteral("eisen-engineer-token-2026");
    dev_eng.role = UserRole::Engineer;
    dev_eng.expires_at = QDateTime::currentDateTime().addYears(5);
    active_tokens_.insert(dev_eng.token, dev_eng);
}

QString SecurityService::issue_token(const QString &user_id, UserRole role, int validity_hours)
{
    QString token = QUuid::createUuid().toString(QUuid::WithoutBraces).remove(QLatin1Char('-'));
    UserSession session;
    session.user_id = user_id;
    session.token = token;
    session.role = role;
    session.expires_at = QDateTime::currentDateTime().addSecs(validity_hours * 3600);

    active_tokens_.insert(token, session);
    return token;
}

bool SecurityService::validate_token(const QString &token, UserRole &out_role, QString &out_user_id) const
{
    if (token.isEmpty()) {
        return false;
    }

    auto it = active_tokens_.find(token);
    if (it == active_tokens_.end()) {
        return false;
    }

    if (QDateTime::currentDateTime() > it->expires_at) {
        return false;
    }

    out_role = it->role;
    out_user_id = it->user_id;
    return true;
}

bool SecurityService::can_view(UserRole role) const
{
    Q_UNUSED(role);
    return true; // All authenticated roles can view
}

bool SecurityService::can_edit(UserRole role) const
{
    return (role == UserRole::Admin || role == UserRole::Engineer);
}

bool SecurityService::can_manage_baselines(UserRole role) const
{
    return (role == UserRole::Admin || role == UserRole::Engineer);
}

bool SecurityService::can_administer(UserRole role) const
{
    return (role == UserRole::Admin);
}

bool SecurityService::check_rate_limit(const QString &client_identifier, int max_per_minute)
{
    QDateTime now = QDateTime::currentDateTime();
    RateBucket &bucket = rate_buckets_[client_identifier];

    if (!bucket.last_refill.isValid()) {
        bucket.tokens = max_per_minute;
        bucket.last_refill = now;
    } else {
        qint64 secs = bucket.last_refill.secsTo(now);
        if (secs > 0) {
            double refill = (static_cast<double>(secs) / 60.0) * max_per_minute;
            bucket.tokens = std::min(max_per_minute, bucket.tokens + static_cast<int>(refill));
            bucket.last_refill = now;
        }
    }

    if (bucket.tokens > 0) {
        bucket.tokens--;
        return true;
    }

    return false;
}

} // namespace source::gantt
