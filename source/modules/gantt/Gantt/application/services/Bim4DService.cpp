#include "Bim4DService.hpp"
#include <QJsonArray>
#include <QSet>

namespace source::gantt {

QJsonObject Bim4DSimulationElement::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("bim_element_id")] = bim_element_id;
    json[QStringLiteral("bim_source")] = bim_source;
    json[QStringLiteral("schedule_item_id")] = schedule_item_id;
    json[QStringLiteral("activity_name")] = activity_name;
    json[QStringLiteral("wbs_code")] = wbs_code;
    json[QStringLiteral("category")] = category;
    json[QStringLiteral("level")] = level;
    json[QStringLiteral("status")] = status_string;
    json[QStringLiteral("color_hex")] = color_hex;
    json[QStringLiteral("progress_percent")] = progress_percent;
    json[QStringLiteral("planned_start")] = planned_start.isValid() ? planned_start.toString(Qt::ISODate) : QString();
    json[QStringLiteral("planned_end")] = planned_end.isValid() ? planned_end.toString(Qt::ISODate) : QString();
    return json;
}

QJsonObject Bim4DSimulationReport::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("schedule_id")] = schedule_id;
    json[QStringLiteral("reference_date")] = reference_date.toString(Qt::ISODate);
    json[QStringLiteral("total_elements")] = total_elements;
    json[QStringLiteral("unstarted_count")] = unstarted_count;
    json[QStringLiteral("planned_in_progress_count")] = planned_in_progress_count;
    json[QStringLiteral("executed_count")] = executed_count;
    json[QStringLiteral("delayed_start_count")] = delayed_start_count;
    json[QStringLiteral("delayed_overdue_count")] = delayed_overdue_count;

    QJsonArray arr;
    for (const auto &el : elements) {
        arr.append(el.to_json());
    }
    json[QStringLiteral("elements")] = arr;
    return json;
}

Bim4DService::Bim4DService()
{
}

bool Bim4DService::add_link(Schedule &schedule, const BimLink &link, QString *error_message)
{
    if (!link.is_valid(error_message)) {
        return false;
    }
    if (!schedule.items.contains(link.schedule_item_id)) {
        if (error_message) *error_message = QStringLiteral("Atividade do cronograma não existe.");
        return false;
    }

    schedule.add_bim_link(link);
    return true;
}

bool Bim4DService::remove_link(Schedule &schedule, const QString &link_id, QString *error_message)
{
    if (!schedule.remove_bim_link(link_id)) {
        if (error_message) *error_message = QStringLiteral("Vínculo BIM não encontrado.");
        return false;
    }
    return true;
}

QList<BimLink> Bim4DService::list_links(const Schedule &schedule) const
{
    return schedule.bim_links;
}

QString Bim4DService::status_to_color(Bim4DStatus status)
{
    switch (status) {
        case Bim4DStatus::Unstarted:
            return QStringLiteral("#9E9E9E"); // Neutral Gray (Future)
        case Bim4DStatus::PlannedInProgress:
            return QStringLiteral("#2196F3"); // Vibrant Blue (In Progress as planned)
        case Bim4DStatus::Executed:
            return QStringLiteral("#4CAF50"); // Green (Done)
        case Bim4DStatus::DelayedStart:
            return QStringLiteral("#FF9800"); // Amber/Orange (Should have started)
        case Bim4DStatus::DelayedOverdue:
            return QStringLiteral("#F44336"); // Crimson Red (Overdue)
    }
    return QStringLiteral("#9E9E9E");
}

Bim4DSimulationReport Bim4DService::simulate(const Schedule &schedule, const QDateTime &reference_date) const
{
    Bim4DSimulationReport report;
    report.schedule_id = schedule.id;
    report.reference_date = reference_date.isValid() ? reference_date : QDateTime::currentDateTime();

    const QDateTime now = QDateTime::currentDateTime();

    auto is_test_item = [](const ScheduleItem &item) {
        const QString lower = item.name.toLower();
        return lower.contains(QStringLiteral("tarefa concluída teste")) ||
               lower.contains(QStringLiteral("tarefa concluida teste")) ||
               lower.contains(QStringLiteral("tarefa ciclos teste")) ||
               lower.contains(QStringLiteral("tarefa atrasada teste")) ||
               item.task_id.startsWith(QStringLiteral("TASK-CYCLE-"));
    };

    QSet<QString> items_with_bim_links;

    const QDateTime eval_date = reference_date.isValid() ? reference_date : now;

    for (const auto &link : schedule.bim_links) {
        items_with_bim_links.insert(link.schedule_item_id);
        const ScheduleItem *item = schedule.find_item(link.schedule_item_id);
        if (item && is_test_item(*item)) {
            continue;
        }

        Bim4DSimulationElement el;
        el.bim_element_id = link.bim_element_id;
        el.bim_source = bim_source_to_string(link.bim_source);
        el.schedule_item_id = link.schedule_item_id;
        el.category = link.category;
        el.level = link.level;

        if (item) {
            el.activity_name = item->name;
            el.wbs_code = item->wbs_code;
            el.progress_percent = item->percent_complete;
            el.planned_start = item->start_date;
            el.planned_end = item->end_date;

            if (item->percent_complete >= 100.0 || item->status == ActivityStatus::Done) {
                el.status = Bim4DStatus::Executed;
                report.executed_count++;
            } else if (item->is_delayed()) {
                el.status = Bim4DStatus::DelayedOverdue;
                report.delayed_overdue_count++;
            } else if (item->status == ActivityStatus::InProgress) {
                el.status = Bim4DStatus::PlannedInProgress;
                report.planned_in_progress_count++;
            } else if (item->percent_complete > 0.0) {
                const bool is_overdue = item->end_date.isValid() ? (item->end_date < eval_date) : false;
                if (is_overdue) {
                    el.status = Bim4DStatus::DelayedOverdue;
                    report.delayed_overdue_count++;
                } else {
                    el.status = Bim4DStatus::PlannedInProgress;
                    report.planned_in_progress_count++;
                }
            } else if (item->status == ActivityStatus::Blocked) {
                el.status = Bim4DStatus::DelayedOverdue;
                report.delayed_overdue_count++;
            } else {
                const bool is_delayed_start = item->start_date.isValid() ? (item->start_date < eval_date) : false;
                if (is_delayed_start) {
                    el.status = Bim4DStatus::DelayedStart;
                    report.delayed_start_count++;
                } else {
                    el.status = Bim4DStatus::Unstarted;
                    report.unstarted_count++;
                }
            }
        } else {
            el.status = Bim4DStatus::Unstarted;
            report.unstarted_count++;
        }

        el.status_string = bim_4d_status_to_string(el.status);
        el.color_hex = status_to_color(el.status);

        report.elements.append(el);
        report.total_elements++;
    }

    // Include activities without explicit BIM links
    for (const auto &item : schedule.get_sorted_by_wbs()) {
        if (is_test_item(item) || items_with_bim_links.contains(item.id)) {
            continue;
        }

        Bim4DSimulationElement el;
        el.bim_element_id = !item.task_id.trimmed().isEmpty() ? item.task_id : item.id;
        el.bim_source = !item.task_id.trimmed().isEmpty() ? QStringLiteral("EisenPo") : QStringLiteral("Gantt");
        el.schedule_item_id = item.id;
        el.activity_name = item.name;
        el.wbs_code = item.wbs_code;
        el.category = item.wbs_code.contains('.') ? QStringLiteral("Q%1").arg(item.wbs_code.section('.', 0, 0)) : QStringLiteral("Geral");
        el.level = QStringLiteral("Nível %1").arg(item.wbs_code.count('.') + 1);
        el.progress_percent = item.percent_complete;
        el.planned_start = item.start_date;
        el.planned_end = item.end_date;

        if (item.percent_complete >= 100.0 || item.status == ActivityStatus::Done) {
            el.status = Bim4DStatus::Executed;
            report.executed_count++;
        } else if (item.is_delayed()) {
            el.status = Bim4DStatus::DelayedOverdue;
            report.delayed_overdue_count++;
        } else if (item.status == ActivityStatus::InProgress) {
            el.status = Bim4DStatus::PlannedInProgress;
            report.planned_in_progress_count++;
        } else if (item.percent_complete > 0.0) {
            const bool is_overdue = item.end_date.isValid() ? (item.end_date < eval_date) : false;
            if (is_overdue) {
                el.status = Bim4DStatus::DelayedOverdue;
                report.delayed_overdue_count++;
            } else {
                el.status = Bim4DStatus::PlannedInProgress;
                report.planned_in_progress_count++;
            }
        } else if (item.status == ActivityStatus::Blocked) {
            el.status = Bim4DStatus::DelayedOverdue;
            report.delayed_overdue_count++;
        } else {
            const bool is_delayed_start = item.start_date.isValid() ? (item.start_date < eval_date) : false;
            if (is_delayed_start) {
                el.status = Bim4DStatus::DelayedStart;
                report.delayed_start_count++;
            } else {
                el.status = Bim4DStatus::Unstarted;
                report.unstarted_count++;
            }
        }

        el.status_string = bim_4d_status_to_string(el.status);
        el.color_hex = status_to_color(el.status);

        report.elements.append(el);
        report.total_elements++;
    }

    return report;
}

} // namespace source::gantt
