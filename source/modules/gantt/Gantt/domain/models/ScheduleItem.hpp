#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include "GanttEnums.hpp"

namespace source::gantt {

class ScheduleItem {
public:
    ScheduleItem();
    explicit ScheduleItem(const QString &name);

    QString id;
    QString schedule_id;
    QString task_id; // FK to EisenPo task
    QString wbs_code; // e.g., "1.2.3"
    QString name;
    QString description;
    
    QDateTime start_date;
    QDateTime end_date;
    double duration_hours = 8.0; // default 1 day = 8h
    double percent_complete = 0.0; // 0.0 to 100.0
    ActivityStatus status = ActivityStatus::NotStarted;
    bool is_milestone = false;
    bool critical = false;

    // CPM calculated fields
    QDateTime early_start;
    QDateTime early_finish;
    QDateTime late_start;
    QDateTime late_finish;
    double total_float_hours = 0.0;
    double free_float_hours = 0.0;

    // Baseline snapshot comparison
    QDateTime baseline_start_date;
    QDateTime baseline_end_date;
    double baseline_duration_hours = 0.0;

    // External integration (ERP / Revit / MS Project)
    QString external_system;
    QString external_id;
    int version = 1;

    QDateTime created_at;
    QDateTime updated_at;

    // Business rule validations
    bool is_valid(QString *error_message = nullptr) const;
    void sanitize();

    // Serialization
    QJsonObject to_json() const;
    static ScheduleItem from_json(const QJsonObject &json);

    // Helpers
    double variance_start_hours() const;
    double variance_end_hours() const;
    bool is_delayed() const;
};

} // namespace source::gantt
