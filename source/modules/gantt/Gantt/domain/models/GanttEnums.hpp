#pragma once

#include <QString>

namespace source::gantt {

enum class DependencyType {
    FinishToStart,  // FS (predecessor finishes before successor starts)
    StartToStart,   // SS (predecessor starts before successor starts)
    FinishToFinish, // FF (predecessor finishes before successor finishes)
    StartToFinish   // SF (predecessor starts before successor finishes)
};

enum class ActivityStatus {
    NotStarted,
    InProgress,
    Blocked,
    Done
};

enum class BimSource {
    IFC,
    Revit,
    Other
};

enum class Bim4DStatus {
    Unstarted,          // Reference date is prior to planned start
    PlannedInProgress,  // Reference date is between planned start & end, work ongoing
    Executed,           // Finished on time or ahead
    DelayedStart,       // Reference date past start, but 0% complete
    DelayedOverdue      // Reference date past planned end, but < 100% complete
};

enum class UserRole {
    Admin,
    Engineer,
    Manager,
    Viewer
};

inline QString dependency_type_to_string(DependencyType type) {
    switch (type) {
        case DependencyType::FinishToStart: return QStringLiteral("FS");
        case DependencyType::StartToStart: return QStringLiteral("SS");
        case DependencyType::FinishToFinish: return QStringLiteral("FF");
        case DependencyType::StartToFinish: return QStringLiteral("SF");
    }
    return QStringLiteral("FS");
}

inline DependencyType string_to_dependency_type(const QString &str) {
    const QString upper = str.trimmed().toUpper();
    if (upper == QStringLiteral("SS")) return DependencyType::StartToStart;
    if (upper == QStringLiteral("FF")) return DependencyType::FinishToFinish;
    if (upper == QStringLiteral("SF")) return DependencyType::StartToFinish;
    return DependencyType::FinishToStart;
}

inline QString activity_status_to_string(ActivityStatus status) {
    switch (status) {
        case ActivityStatus::NotStarted: return QStringLiteral("not_started");
        case ActivityStatus::InProgress: return QStringLiteral("in_progress");
        case ActivityStatus::Blocked:    return QStringLiteral("blocked");
        case ActivityStatus::Done:       return QStringLiteral("done");
    }
    return QStringLiteral("not_started");
}

inline ActivityStatus string_to_activity_status(const QString &str) {
    QString lower = str.trimmed().toLower();
    lower.replace(QLatin1Char('_'), QLatin1Char(' '));
    lower.replace(QLatin1Char('-'), QLatin1Char(' '));
    while (lower.contains(QStringLiteral("  "))) {
        lower.replace(QStringLiteral("  "), QStringLiteral(" "));
    }
    if (lower == QStringLiteral("in progress") ||
        lower == QStringLiteral("em andamento") ||
        lower == QStringLiteral("em progresso") ||
        lower == QStringLiteral("progresso") ||
        lower == QStringLiteral("andamento") ||
        lower == QStringLiteral("doing") ||
        lower == QStringLiteral("doing timer")) {
        return ActivityStatus::InProgress;
    }
    if (lower == QStringLiteral("blocked") || lower == QStringLiteral("bloqueado") || lower == QStringLiteral("bloqueada") || lower == QStringLiteral("atrasado") || lower == QStringLiteral("atrasada")) {
        return ActivityStatus::Blocked;
    }
    if (lower == QStringLiteral("done") || lower == QStringLiteral("concluido") || lower == QStringLiteral("concluído") || lower == QStringLiteral("concluida") || lower == QStringLiteral("concluída") || lower == QStringLiteral("completed") || lower == QStringLiteral("executado") || lower == QStringLiteral("executada")) {
        return ActivityStatus::Done;
    }
    return ActivityStatus::NotStarted;
}

inline QString bim_source_to_string(BimSource source) {
    switch (source) {
        case BimSource::IFC: return QStringLiteral("ifc");
        case BimSource::Revit: return QStringLiteral("revit");
        case BimSource::Other: return QStringLiteral("other");
    }
    return QStringLiteral("ifc");
}

inline BimSource string_to_bim_source(const QString &str) {
    const QString lower = str.trimmed().toLower();
    if (lower == QStringLiteral("revit")) return BimSource::Revit;
    if (lower == QStringLiteral("other") || lower == QStringLiteral("outro")) return BimSource::Other;
    return BimSource::IFC;
}

inline QString bim_4d_status_to_string(Bim4DStatus status) {
    switch (status) {
        case Bim4DStatus::Unstarted: return QStringLiteral("unstarted");
        case Bim4DStatus::PlannedInProgress: return QStringLiteral("planned_in_progress");
        case Bim4DStatus::Executed: return QStringLiteral("executed");
        case Bim4DStatus::DelayedStart: return QStringLiteral("delayed_start");
        case Bim4DStatus::DelayedOverdue: return QStringLiteral("delayed_overdue");
    }
    return QStringLiteral("unstarted");
}

inline QString user_role_to_string(UserRole role) {
    switch (role) {
        case UserRole::Admin: return QStringLiteral("admin");
        case UserRole::Engineer: return QStringLiteral("engineer");
        case UserRole::Manager: return QStringLiteral("manager");
        case UserRole::Viewer: return QStringLiteral("viewer");
    }
    return QStringLiteral("viewer");
}

inline UserRole string_to_user_role(const QString &str) {
    const QString lower = str.trimmed().toLower();
    if (lower == QStringLiteral("admin")) return UserRole::Admin;
    if (lower == QStringLiteral("engineer") || lower == QStringLiteral("engenheiro")) return UserRole::Engineer;
    if (lower == QStringLiteral("manager") || lower == QStringLiteral("gestor")) return UserRole::Manager;
    return UserRole::Viewer;
}

} // namespace source::gantt
