#pragma once

#include <QDateTime>
#include <QHash>
#include <QList>
#include <QJsonObject>
#include <QString>

#include "Baseline.hpp"
#include "BimLink.hpp"
#include "Dependency.hpp"
#include "ProgressUpdate.hpp"
#include "ScheduleItem.hpp"
#include "WorkCalendar.hpp"

namespace source::gantt {

class Schedule {
public:
    Schedule();
    explicit Schedule(const QString &schedule_name, const QString &proj_id = QString());

    QString id;
    QString project_id;
    QString name;
    QString description;
    QString active_baseline_id;
    QDateTime created_at;
    QDateTime updated_at;

    WorkCalendar calendar;
    QHash<QString, ScheduleItem> items;
    QList<Dependency> dependencies;
    QHash<QString, Baseline> baselines;
    QList<BimLink> bim_links;
    QList<ProgressUpdate> progress_updates;

    // Entity lookups & queries
    ScheduleItem *find_item(const QString &item_id);
    const ScheduleItem *find_item(const QString &item_id) const;
    ScheduleItem *find_item_by_task_id(const QString &eisenhower_task_id);

    QList<Dependency> get_predecessors_of(const QString &item_id) const;
    QList<Dependency> get_successors_of(const QString &item_id) const;

    QList<ScheduleItem> get_sorted_by_wbs() const;
    QList<ScheduleItem> get_critical_path_items() const;

    // Modification methods
    bool add_item(const ScheduleItem &item, QString *error_message = nullptr);
    bool update_item(const ScheduleItem &item, QString *error_message = nullptr);
    bool remove_item(const QString &item_id, QString *error_message = nullptr);

    bool add_dependency(const Dependency &dep, QString *error_message = nullptr);
    bool remove_dependency(const QString &dep_id, QString *error_message = nullptr);

    void add_bim_link(const BimLink &link);
    bool remove_bim_link(const QString &link_id);
    QList<BimLink> get_bim_links_for_item(const QString &item_id) const;

    // Metrics
    QDateTime project_start_date() const;
    QDateTime project_finish_date() const;
    double total_duration_hours() const;
    double overall_percent_complete() const;
    int critical_items_count() const;
    int delayed_items_count() const;

    QJsonObject to_json() const;
    static Schedule from_json(const QJsonObject &json);
};

} // namespace source::gantt
