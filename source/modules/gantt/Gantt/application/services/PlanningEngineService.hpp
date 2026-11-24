#pragma once

#include <QList>
#include <QString>
#include "../../domain/models/Schedule.hpp"
#include "../../domain/rules/CpmEngine.hpp"

namespace source::gantt {

struct VarianceReportItem {
    QString item_id;
    QString name;
    QString wbs_code;
    bool critical = false;
    double current_duration_days = 0.0;
    double baseline_duration_days = 0.0;
    double start_variance_days = 0.0; // Positive = delayed start
    double finish_variance_days = 0.0; // Positive = delayed finish
    double percent_complete = 0.0;
    bool is_delayed = false;
    bool is_critical_delayed = false;
};

struct BaselineComparisonReport {
    QString schedule_id;
    QString baseline_id;
    QString baseline_name;
    QDateTime report_date;
    double total_project_delay_days = 0.0;
    int critical_delayed_count = 0;
    QList<VarianceReportItem> items;
    QStringList critical_alerts;
};

class PlanningEngineService {
public:
    PlanningEngineService();

    // CPM calculation
    CpmEngine::CpmResult recalculate(Schedule &schedule);

    // Baseline management
    Baseline create_baseline(Schedule &schedule, const QString &name, const QString &author = QStringLiteral("engenheiro"));
    bool freeze_baseline(Schedule &schedule, const QString &baseline_id, QString *error_message = nullptr);
    bool unfreeze_baseline(Schedule &schedule, const QString &baseline_id, bool is_admin, QString *error_message = nullptr);

    // Baseline comparison and variance analysis
    BaselineComparisonReport compare_baseline(const Schedule &schedule, const QString &baseline_id = QString());
};

} // namespace source::gantt
