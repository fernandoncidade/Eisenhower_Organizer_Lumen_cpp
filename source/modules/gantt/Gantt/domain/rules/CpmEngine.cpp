#include "CpmEngine.hpp"
#include "DependencyValidator.hpp"
#include <algorithm>

namespace source::gantt {

CpmEngine::CpmResult CpmEngine::calculate(Schedule &schedule, const QDateTime &forced_start_date)
{
    CpmResult result;

    if (schedule.items.isEmpty()) {
        result.success = true;
        return result;
    }

    QList<QString> item_ids = schedule.items.keys();

    // 1. Cycle detection
    DependencyValidator::CycleCheckResult cycle_check =
        DependencyValidator::check_cycles(item_ids, schedule.dependencies);

    if (cycle_check.has_cycle) {
        result.success = false;
        result.error_message = cycle_check.error_message;
        return result;
    }

    // 2. Topological sort
    QList<QString> sorted_ids;
    QString sort_error;
    if (!DependencyValidator::topological_sort(item_ids, schedule.dependencies, sorted_ids, &sort_error)) {
        result.success = false;
        result.error_message = sort_error;
        return result;
    }

    // 3. Determine base start date
    QDateTime base_start = forced_start_date;
    if (!base_start.isValid()) {
        for (const QString &id : item_ids) {
            const ScheduleItem &item = schedule.items[id];
            if (item.start_date.isValid()) {
                if (!base_start.isValid() || item.start_date < base_start) {
                    base_start = item.start_date;
                }
            }
        }
    }
    if (!base_start.isValid()) {
        base_start = QDateTime::currentDateTime();
        base_start.setTime(schedule.calendar.morning_start);
    }
    base_start = schedule.calendar.normalize_to_working_time(base_start);

    // 4. FORWARD PASS
    QDateTime project_ef = base_start;

    for (const QString &id : sorted_ids) {
        ScheduleItem &item = schedule.items[id];
        item.sanitize();

        QDateTime earliest_start = base_start;

        // If activity has an explicit user start date and no predecessors, preserve it if valid
        QList<Dependency> preds = schedule.get_predecessors_of(id);
        if (preds.isEmpty() && item.start_date.isValid()) {
            earliest_start = item.start_date;
            while (!schedule.calendar.is_working_day(earliest_start.date())) {
                earliest_start = earliest_start.addDays(1);
            }
        }

        for (const auto &dep : preds) {
            if (!schedule.items.contains(dep.predecessor_item_id)) continue;
            const ScheduleItem &pred = schedule.items[dep.predecessor_item_id];

            QDateTime constraint_start;
            switch (dep.type) {
                case DependencyType::FinishToStart:
                    constraint_start = schedule.calendar.add_working_hours(pred.early_finish, dep.lag_hours);
                    break;
                case DependencyType::StartToStart:
                    constraint_start = schedule.calendar.add_working_hours(pred.early_start, dep.lag_hours);
                    break;
                case DependencyType::FinishToFinish: {
                    // Successor finish must be >= predecessor finish + lag
                    QDateTime required_finish = schedule.calendar.add_working_hours(pred.early_finish, dep.lag_hours);
                    double dur = item.is_milestone ? 0.0 : item.duration_hours;
                    // Approximate earliest start by backing up duration
                    constraint_start = required_finish.addSecs(-static_cast<qint64>(dur * 3600.0 * 1.5));
                    break;
                }
                case DependencyType::StartToFinish: {
                    QDateTime required_finish = schedule.calendar.add_working_hours(pred.early_start, dep.lag_hours);
                    double dur = item.is_milestone ? 0.0 : item.duration_hours;
                    constraint_start = required_finish.addSecs(-static_cast<qint64>(dur * 3600.0 * 1.5));
                    break;
                }
            }

            if (constraint_start > earliest_start) {
                earliest_start = constraint_start;
            }
        }

        if (preds.isEmpty() && item.start_date.isValid()) {
            item.early_start = earliest_start;
        } else {
            item.early_start = schedule.calendar.normalize_to_working_time(earliest_start);
        }

        if (item.is_milestone) {
            item.early_finish = item.early_start;
        } else {
            item.early_finish = schedule.calendar.add_working_hours(item.early_start, item.duration_hours);
        }

        // Additional pass for FF constraints to ensure finish is properly respected
        for (const auto &dep : preds) {
            if (!schedule.items.contains(dep.predecessor_item_id)) continue;
            const ScheduleItem &pred = schedule.items[dep.predecessor_item_id];

            if (dep.type == DependencyType::FinishToFinish) {
                QDateTime req_finish = schedule.calendar.add_working_hours(pred.early_finish, dep.lag_hours);
                if (item.early_finish < req_finish) {
                    item.early_finish = req_finish;
                    // Adjust early start backwards
                    double hours_diff = schedule.calendar.working_hours_between(item.early_start, item.early_finish);
                    if (hours_diff > item.duration_hours) {
                        double shift = hours_diff - item.duration_hours;
                        item.early_start = schedule.calendar.add_working_hours(item.early_start, shift);
                    }
                }
            }
        }

        if (item.early_finish > project_ef) {
            project_ef = item.early_finish;
        }
    }

    result.project_early_start = base_start;
    result.project_early_finish = project_ef;
    result.total_duration_hours = schedule.calendar.working_hours_between(base_start, project_ef);

    // 5. BACKWARD PASS
    for (int i = sorted_ids.size() - 1; i >= 0; --i) {
        const QString &id = sorted_ids[i];
        ScheduleItem &item = schedule.items[id];

        QList<Dependency> succs = schedule.get_successors_of(id);

        if (succs.isEmpty()) {
            item.late_finish = project_ef;
        } else {
            QDateTime latest_finish = project_ef;
            bool first = true;

            for (const auto &dep : succs) {
                if (!schedule.items.contains(dep.successor_item_id)) continue;
                const ScheduleItem &succ = schedule.items[dep.successor_item_id];

                QDateTime constraint_finish;
                switch (dep.type) {
                    case DependencyType::FinishToStart:
                        // pred LF <= succ LS - lag
                        constraint_finish = schedule.calendar.subtract_working_hours(succ.late_start, dep.lag_hours);
                        break;
                    case DependencyType::FinishToFinish:
                        constraint_finish = schedule.calendar.subtract_working_hours(succ.late_finish, dep.lag_hours);
                        break;
                    case DependencyType::StartToStart:
                        constraint_finish = schedule.calendar.subtract_working_hours(succ.late_start, dep.lag_hours);
                        constraint_finish = schedule.calendar.add_working_hours(constraint_finish, item.duration_hours);
                        break;
                    case DependencyType::StartToFinish:
                        constraint_finish = schedule.calendar.subtract_working_hours(succ.late_finish, dep.lag_hours);
                        constraint_finish = schedule.calendar.add_working_hours(constraint_finish, item.duration_hours);
                        break;
                }

                if (first || constraint_finish < latest_finish) {
                    latest_finish = constraint_finish;
                    first = false;
                }
            }
            item.late_finish = latest_finish;
        }

        if (item.is_milestone) {
            item.late_start = item.late_finish;
        } else {
            item.late_start = schedule.calendar.subtract_working_hours(item.late_finish, item.duration_hours);
        }

        // 6. FLOAT & CRITICAL PATH
        double tf = 0.0;
        if (item.late_finish > item.early_finish) {
            tf = schedule.calendar.working_hours_between(item.early_finish, item.late_finish);
        } else if (item.late_finish < item.early_finish) {
            tf = -schedule.calendar.working_hours_between(item.late_finish, item.early_finish);
        }
        item.total_float_hours = tf;

        // An activity can only be on the Critical Path if it belongs to an interconnected
        // dependency network (has predecessors or successors) and the schedule defines dependencies.
        // Isolated activities without dependencies are standalone tasks whose calendar dates do not
        // form a critical dependency chain.
        QList<Dependency> preds = schedule.get_predecessors_of(id);
        const bool has_network_links = (!preds.isEmpty() || !succs.isEmpty());
        if (!has_network_links || schedule.dependencies.isEmpty()) {
            item.critical = false;
        } else {
            item.critical = (std::abs(tf) < 0.1);
        }

        // Update working dates
        item.start_date = item.early_start;
        item.end_date = item.early_finish;

        if (item.critical) {
            result.critical_activities_count++;
            result.critical_path_ids.append(item.id);
        }
    }

    result.success = true;
    schedule.updated_at = QDateTime::currentDateTime();
    return result;
}

CpmEngine::CpmResult CpmEngine::reschedule_from_item(Schedule &schedule, const QString &changed_item_id)
{
    // Full CPM recalculation propagates all changes consistently
    Q_UNUSED(changed_item_id);
    return calculate(schedule);
}

} // namespace source::gantt
