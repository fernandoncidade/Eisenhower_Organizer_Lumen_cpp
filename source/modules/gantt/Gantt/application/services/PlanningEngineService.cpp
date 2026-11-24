#include "PlanningEngineService.hpp"

namespace source::gantt {

PlanningEngineService::PlanningEngineService()
{
}

CpmEngine::CpmResult PlanningEngineService::recalculate(Schedule &schedule)
{
    return CpmEngine::calculate(schedule);
}

Baseline PlanningEngineService::create_baseline(Schedule &schedule, const QString &name, const QString &author)
{
    Baseline b(name, schedule.id);
    b.created_by = author;

    for (const auto &item : schedule.items) {
        BaselineItemSnapshot snap;
        snap.item_id = item.id;
        snap.start_date = item.start_date;
        snap.end_date = item.end_date;
        snap.duration_hours = item.duration_hours;
        b.items.insert(snap.item_id, snap);

        // Also update activity's cached baseline dates
        ScheduleItem *it = schedule.find_item(item.id);
        if (it) {
            it->baseline_start_date = item.start_date;
            it->baseline_end_date = item.end_date;
            it->baseline_duration_hours = item.duration_hours;
        }
    }

    b.freeze(); // Freeze upon creation
    schedule.baselines.insert(b.id, b);
    schedule.active_baseline_id = b.id;
    schedule.updated_at = QDateTime::currentDateTime();
    return b;
}

bool PlanningEngineService::freeze_baseline(Schedule &schedule, const QString &baseline_id, QString *error_message)
{
    if (!schedule.baselines.contains(baseline_id)) {
        if (error_message) *error_message = QStringLiteral("Baseline não encontrada.");
        return false;
    }
    schedule.baselines[baseline_id].freeze();
    schedule.updated_at = QDateTime::currentDateTime();
    return true;
}

bool PlanningEngineService::unfreeze_baseline(Schedule &schedule, const QString &baseline_id, bool is_admin, QString *error_message)
{
    if (!schedule.baselines.contains(baseline_id)) {
        if (error_message) *error_message = QStringLiteral("Baseline não encontrada.");
        return false;
    }
    return schedule.baselines[baseline_id].unfreeze(is_admin, error_message);
}

BaselineComparisonReport PlanningEngineService::compare_baseline(const Schedule &schedule, const QString &baseline_id)
{
    BaselineComparisonReport report;
    report.schedule_id = schedule.id;
    report.report_date = QDateTime::currentDateTime();

    QString target_id = baseline_id.isEmpty() ? schedule.active_baseline_id : baseline_id;
    if (target_id.isEmpty() || !schedule.baselines.contains(target_id)) {
        report.critical_alerts.append(QStringLiteral("Nenhuma baseline ativa disponível para comparação."));
        return report;
    }

    const Baseline &base = schedule.baselines[target_id];
    report.baseline_id = base.id;
    report.baseline_name = base.name;

    QDateTime max_base_finish;
    QDateTime max_current_finish;

    for (const auto &item : schedule.items) {
        VarianceReportItem rep_item;
        rep_item.item_id = item.id;
        rep_item.name = item.name;
        rep_item.wbs_code = item.wbs_code;
        rep_item.critical = item.critical;
        rep_item.percent_complete = item.percent_complete;
        rep_item.current_duration_days = item.duration_hours / 8.0;

        if (base.items.contains(item.id)) {
            const BaselineItemSnapshot &snap = base.items[item.id];
            rep_item.baseline_duration_days = snap.duration_hours / 8.0;

            if (snap.start_date.isValid() && item.start_date.isValid()) {
                rep_item.start_variance_days = schedule.calendar.working_days_between(snap.start_date, item.start_date);
                if (item.start_date < snap.start_date) {
                    rep_item.start_variance_days = -schedule.calendar.working_days_between(item.start_date, snap.start_date);
                }
            }

            if (snap.end_date.isValid() && item.end_date.isValid()) {
                rep_item.finish_variance_days = schedule.calendar.working_days_between(snap.end_date, item.end_date);
                if (item.end_date < snap.end_date) {
                    rep_item.finish_variance_days = -schedule.calendar.working_days_between(item.end_date, snap.end_date);
                }
            }

            if (!max_base_finish.isValid() || snap.end_date > max_base_finish) {
                max_base_finish = snap.end_date;
            }
        }

        if (!max_current_finish.isValid() || item.end_date > max_current_finish) {
            max_current_finish = item.end_date;
        }

        rep_item.is_delayed = (rep_item.finish_variance_days > 0.01) || item.is_delayed();
        rep_item.is_critical_delayed = rep_item.critical && rep_item.is_delayed;

        if (rep_item.is_critical_delayed) {
            report.critical_delayed_count++;
            report.critical_alerts.append(
                QStringLiteral("ALERTA CRÍTICO: Atividade '%1' (WBS %2) atrasada em %.1f dias.")
                    .arg(item.name, item.wbs_code)
                    .arg(rep_item.finish_variance_days)
            );
        }

        report.items.append(rep_item);
    }

    if (max_base_finish.isValid() && max_current_finish.isValid()) {
        if (max_current_finish > max_base_finish) {
            report.total_project_delay_days = schedule.calendar.working_days_between(max_base_finish, max_current_finish);
        }
    }

    return report;
}

} // namespace source::gantt
