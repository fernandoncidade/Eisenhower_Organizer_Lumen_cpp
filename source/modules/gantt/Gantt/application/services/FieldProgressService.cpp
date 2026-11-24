#include "FieldProgressService.hpp"

namespace source::gantt {

FieldProgressService::FieldProgressService()
{
}

void FieldProgressService::set_host_sync_callback(HostTaskSyncCallback callback)
{
    host_sync_cb_ = callback;
}

bool FieldProgressService::record_progress(Schedule &schedule,
                                           const QString &item_id,
                                           double percent,
                                           const QString &notes,
                                           const QString &reporter,
                                           QString *error_message)
{
    ScheduleItem *item = schedule.find_item(item_id);
    if (!item) {
        if (error_message) *error_message = QStringLiteral("Atividade não encontrada.");
        return false;
    }

    if (percent < 0.0 || percent > 100.0) {
        if (error_message) *error_message = QStringLiteral("Percentual deve estar entre 0 e 100.");
        return false;
    }

    ProgressUpdate update(item_id, percent, reporter);
    update.previous_percent = item->percent_complete;
    update.notes = notes;

    QDateTime now = QDateTime::currentDateTime();

    if (item->percent_complete == 0.0 && percent > 0.0) {
        update.actual_start = now;
        item->status = ActivityStatus::InProgress;
    }
    if (percent >= 100.0) {
        update.actual_end = now;
        item->status = ActivityStatus::Done;
    } else if (percent > 0.0) {
        item->status = ActivityStatus::InProgress;
    } else {
        item->status = ActivityStatus::NotStarted;
    }

    item->percent_complete = percent;
    item->updated_at = now;
    schedule.progress_updates.append(update);
    schedule.updated_at = now;

    // Trigger two-way sync to host EisenPo task if linked
    if (!item->task_id.isEmpty() && host_sync_cb_) {
        host_sync_cb_(item->task_id, percent, (percent >= 100.0));
    }

    return true;
}

bool FieldProgressService::link_eisenhower_task(Schedule &schedule,
                                                const QString &item_id,
                                                const QString &eisenhower_task_id,
                                                QString *error_message)
{
    ScheduleItem *item = schedule.find_item(item_id);
    if (!item) {
        if (error_message) *error_message = QStringLiteral("Atividade não encontrada.");
        return false;
    }

    item->task_id = eisenhower_task_id;
    item->updated_at = QDateTime::currentDateTime();
    schedule.updated_at = QDateTime::currentDateTime();
    return true;
}

bool FieldProgressService::unlink_eisenhower_task(Schedule &schedule,
                                                  const QString &item_id,
                                                  QString *error_message)
{
    ScheduleItem *item = schedule.find_item(item_id);
    if (!item) {
        if (error_message) *error_message = QStringLiteral("Atividade não encontrada.");
        return false;
    }

    item->task_id.clear();
    item->updated_at = QDateTime::currentDateTime();
    schedule.updated_at = QDateTime::currentDateTime();
    return true;
}

void FieldProgressService::on_eisenhower_task_status_changed(Schedule &schedule,
                                                             const QString &eisenhower_task_id,
                                                             bool is_completed)
{
    ScheduleItem *item = schedule.find_item_by_task_id(eisenhower_task_id);
    if (item) {
        double new_percent = is_completed ? 100.0 : (item->percent_complete >= 100.0 ? 50.0 : item->percent_complete);
        record_progress(schedule, item->id, new_percent, QStringLiteral("Atualização via sincronização de tarefa EisenPo"), QStringLiteral("EisenPo"));
    }
}

} // namespace source::gantt
