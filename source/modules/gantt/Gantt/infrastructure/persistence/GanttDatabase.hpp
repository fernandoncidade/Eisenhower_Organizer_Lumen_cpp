#pragma once

#include <QSqlDatabase>
#include <QString>
#include <QList>
#include "../../domain/models/AuditLog.hpp"
#include "../../domain/models/Schedule.hpp"

namespace source::gantt {

class GanttDatabase {
public:
    GanttDatabase();
    ~GanttDatabase();

    static GanttDatabase &instance();

    bool initialize(const QString &db_path = QString(), QString *error_message = nullptr);
    void close();

    bool is_open() const;

    // Transactional Schedule Persistence
    bool save_schedule(const Schedule &schedule, QString *error_message = nullptr);
    bool load_schedule(const QString &schedule_id, Schedule &out_schedule, QString *error_message = nullptr);
    Schedule load_default_schedule(QString *error_message = nullptr);
    QList<Schedule> list_schedules(QString *error_message = nullptr);
    bool delete_schedule(const QString &schedule_id, QString *error_message = nullptr);

    // Audit Logging
    bool save_audit_log(const AuditLog &log, QString *error_message = nullptr);
    QList<AuditLog> load_audit_logs(const QString &entity_type = QString(), const QString &entity_id = QString(), int limit = 100);

private:
    QString connection_name_;
    bool run_migrations(QSqlDatabase &db, QString *error_message);
};

} // namespace source::gantt
