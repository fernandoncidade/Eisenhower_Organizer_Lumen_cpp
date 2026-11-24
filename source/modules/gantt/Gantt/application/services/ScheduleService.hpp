#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include "../../domain/models/Schedule.hpp"

namespace source::gantt {

class ScheduleService {
public:
    ScheduleService();

    // Schedule CRUD
    Schedule create_schedule(const QString &name, const QString &project_id = QString(), const QString &desc = QString());
    void add_schedule(const Schedule &schedule);
    Schedule *get_schedule(const QString &schedule_id);
    const Schedule *get_schedule(const QString &schedule_id) const;
    QList<Schedule> list_schedules() const;
    bool delete_schedule(const QString &schedule_id);

    // Activity CRUD
    bool add_activity(const QString &schedule_id, const ScheduleItem &item, QString *error_message = nullptr);
    bool update_activity(const QString &schedule_id, const ScheduleItem &item, QString *error_message = nullptr);
    bool delete_activity(const QString &schedule_id, const QString &item_id, QString *error_message = nullptr);
    ScheduleItem *get_activity(const QString &schedule_id, const QString &item_id);

    // Dependency CRUD
    bool add_dependency(const QString &schedule_id, const Dependency &dep, QString *error_message = nullptr);
    bool delete_dependency(const QString &schedule_id, const QString &dep_id, QString *error_message = nullptr);

    // Template duplication by construction typology
    Schedule create_from_template(const QString &typology_name, const QString &project_id, const QString &new_schedule_name);

private:
    QHash<QString, Schedule> schedules_;
    void seed_default_templates();
};

} // namespace source::gantt
