#include "GanttTaskIntegrationBridge.hpp"

#include <QDate>
#include <QDateTime>
#include <QMessageBox>
#include <QMutexLocker>
#include <QUuid>

#include "../../domain/models/Baseline.hpp"
#include "../../infrastructure/persistence/GanttDatabase.hpp"
#include "../../ui/GanttMainWindow.hpp"
#include "FieldProgressService.hpp"
#include "PlanningEngineService.hpp"
#include "ScheduleService.hpp"
#include "source/utils/TaskProgressHelper.hpp"

namespace source::gantt {

GanttTaskIntegrationBridge::GanttTaskIntegrationBridge(QObject *parent)
    : QObject(parent)
{
}

GanttTaskIntegrationBridge &GanttTaskIntegrationBridge::instance()
{
    static GanttTaskIntegrationBridge s_instance;
    return s_instance;
}

void GanttTaskIntegrationBridge::register_host(IEisenhowerHost *host)
{
    host_ = host;
}

void GanttTaskIntegrationBridge::unregister_host(IEisenhowerHost *host)
{
    if (host_ == host) {
        host_ = nullptr;
    }
}

IEisenhowerHost *GanttTaskIntegrationBridge::host() const
{
    return host_;
}

void GanttTaskIntegrationBridge::register_gantt_window(GanttMainWindow *win)
{
    gantt_win_ = win;
}

void GanttTaskIntegrationBridge::unregister_gantt_window(GanttMainWindow *win)
{
    if (gantt_win_ == win) {
        gantt_win_ = nullptr;
    }
}

GanttMainWindow *GanttTaskIntegrationBridge::gantt_window() const
{
    return gantt_win_;
}

Schedule GanttTaskIntegrationBridge::get_or_load_active_schedule() const
{
    auto &db = GanttDatabase::instance();
    Schedule sched = db.load_default_schedule();
    if (sched.id.isEmpty()) {
        QList<Schedule> all = db.list_schedules();
        if (!all.isEmpty()) {
            sched = all.first();
        }
    }
    return sched;
}

void GanttTaskIntegrationBridge::save_active_schedule(Schedule &schedule)
{
    PlanningEngineService planning;
    planning.recalculate(schedule);

    auto &db = GanttDatabase::instance();
    db.save_schedule(schedule);

    invalidate_cache();

    if (gantt_win_) {
        gantt_win_->load_or_create_default_schedule();
    }
}

void GanttTaskIntegrationBridge::invalidate_cache()
{
    QMutexLocker locker(&cache_mutex_);
    linked_tasks_cache_.clear();
    cache_initialized_ = false;
}

void GanttTaskIntegrationBridge::refresh_cache_if_needed() const
{
    QMutexLocker locker(&cache_mutex_);
    if (cache_initialized_) {
        return;
    }

    linked_tasks_cache_.clear();
    auto &db = GanttDatabase::instance();
    if (!db.is_open()) {
        cache_initialized_ = true;
        return;
    }

    const QList<Schedule> schedules = db.list_schedules();
    for (const auto &sched : schedules) {
        for (const auto &item : sched.items) {
            const QString tid = item.task_id.trimmed();
            if (!tid.isEmpty()) {
                TaskLinkSummary summary;
                summary.schedule_id = sched.id;
                summary.schedule_name = sched.name;
                summary.item_id = item.id;
                summary.activity_name = item.name;
                summary.wbs_code = item.wbs_code;
                summary.progress = item.percent_complete;
                switch (item.status) {
                    case ActivityStatus::Done: summary.status = QStringLiteral("Concluída"); break;
                    case ActivityStatus::InProgress: summary.status = QStringLiteral("Em Andamento"); break;
                    case ActivityStatus::Blocked: summary.status = QStringLiteral("Bloqueada"); break;
                    case ActivityStatus::NotStarted: default: summary.status = QStringLiteral("Não Iniciada"); break;
                }
                linked_tasks_cache_.insert(tid, summary);
            }
        }
    }
    cache_initialized_ = true;
}

bool GanttTaskIntegrationBridge::is_task_linked(const QString &task_id,
                                                QString *out_schedule_id,
                                                QString *out_schedule_name,
                                                QString *out_item_id,
                                                QString *out_activity_name,
                                                QString *out_wbs_code,
                                                double *out_progress,
                                                QString *out_status) const
{
    const QString clean_id = task_id.trimmed();
    if (clean_id.isEmpty()) {
        return false;
    }

    refresh_cache_if_needed();

    QMutexLocker locker(&cache_mutex_);
    auto it = linked_tasks_cache_.constFind(clean_id);
    if (it != linked_tasks_cache_.constEnd()) {
        const auto &s = it.value();
        if (out_schedule_id) *out_schedule_id = s.schedule_id;
        if (out_schedule_name) *out_schedule_name = s.schedule_name;
        if (out_item_id) *out_item_id = s.item_id;
        if (out_activity_name) *out_activity_name = s.activity_name;
        if (out_wbs_code) *out_wbs_code = s.wbs_code;
        if (out_progress) *out_progress = s.progress;
        if (out_status) *out_status = s.status;
        return true;
    }
    return false;
}

QVariantList GanttTaskIntegrationBridge::get_available_eisenhower_tasks() const
{
    if (host_) {
        return host_->get_available_tasks();
    }
    return {};
}

QVariantMap GanttTaskIntegrationBridge::get_task_by_id(const QString &task_id) const
{
    if (host_) {
        return host_->get_task_by_id(task_id);
    }
    return {};
}

void GanttTaskIntegrationBridge::on_eisenhower_task_completed(const QString &task_id, bool is_completed)
{
    if (task_id.trimmed().isEmpty()) {
        return;
    }

    auto &db = GanttDatabase::instance();
    QList<Schedule> schedules = db.list_schedules();
    bool changed = false;

    for (auto &sched : schedules) {
        ScheduleItem *item = sched.find_item_by_task_id(task_id);
        if (item) {
            FieldProgressService fps;
            const double new_percent = is_completed ? 100.0 : (item->percent_complete >= 100.0 ? 50.0 : item->percent_complete);
            fps.record_progress(sched, item->id, new_percent,
                                is_completed ? QStringLiteral("Concluída via tarefa EisenPo") : QStringLiteral("Reaberta via tarefa EisenPo"),
                                QStringLiteral("EisenPo"));
            changed = true;
            save_active_schedule(sched);
            emit progress_synced(task_id, new_percent);
        }
    }

    if (gantt_win_ && changed) {
        gantt_win_->load_or_create_default_schedule();
        gantt_win_->refresh_view();
    }
}

void GanttTaskIntegrationBridge::on_eisenhower_task_in_progress(const QString &task_id)
{
    if (task_id.trimmed().isEmpty()) {
        return;
    }

    const QVariantMap task = get_task_by_id(task_id);
    int target_poms = task.value(QStringLiteral("ciclos_totais"),
                      task.value(QStringLiteral("ciclos_permitidos"),
                      task.value(QStringLiteral("total_ciclos"),
                      task.value(QStringLiteral("ciclos_planejados"),
                      task.value(QStringLiteral("ciclos_alvo"), 4))))).toInt();
    if (target_poms <= 0) target_poms = 4;

    const int completed_poms = task.value(QStringLiteral("pomodoros_completados"),
                               task.value(QStringLiteral("pomodoros"),
                               task.value(QStringLiteral("ciclos_completados"), 0))).toInt();

    double cur = 0.0;
    if (completed_poms > 0) {
        cur = (static_cast<double>(completed_poms) / static_cast<double>(target_poms)) * 100.0;
        if (cur > 100.0) cur = 100.0;
    } else {
        cur = 0.0;
    }

    auto &db = GanttDatabase::instance();
    QList<Schedule> schedules = db.list_schedules();
    bool changed = false;

    for (auto &sched : schedules) {
        ScheduleItem *item = sched.find_item_by_task_id(task_id);
        if (item) {
            FieldProgressService fps;
            item->status = ActivityStatus::InProgress;
            item->percent_complete = cur;
            fps.record_progress(sched, item->id, cur,
                                QStringLiteral("Iniciada em campo via EisenPo"),
                                QStringLiteral("EisenPo"));
            item->status = ActivityStatus::InProgress;
            item->percent_complete = cur;
            changed = true;
            save_active_schedule(sched);
            emit progress_synced(task_id, cur);
        }
    }

    if (gantt_win_ && changed) {
        gantt_win_->load_or_create_default_schedule();
        gantt_win_->refresh_view();
    }
}

void GanttTaskIntegrationBridge::on_eisenhower_task_modified(const QString &task_id)
{
    if (task_id.trimmed().isEmpty()) {
        return;
    }

    const QVariantMap task = get_task_by_id(task_id);
    if (task.isEmpty()) {
        return;
    }

    const QString title = task.value(QStringLiteral("text"),
                          task.value(QStringLiteral("titulo"))).toString().trimmed();
    const QString desc = task.value(QStringLiteral("description"),
                         task.value(QStringLiteral("descricao"))).toString().trimmed();

    double duration = 8.0;
    if (task.contains(QStringLiteral("duracao")) && task.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        duration = task.value(QStringLiteral("duracao")).toDouble() * 8.0;
    } else if (task.contains(QStringLiteral("duration")) && task.value(QStringLiteral("duration")).toDouble() > 0.0) {
        duration = task.value(QStringLiteral("duration")).toDouble() * 8.0;
    }

    bool done = false;
    if (task.contains(QStringLiteral("concluida"))) done = task.value(QStringLiteral("concluida")).toBool();
    else if (task.contains(QStringLiteral("completed"))) done = task.value(QStringLiteral("completed")).toBool();
    else {
        const QString st = task.value(QStringLiteral("status")).toString().toLower();
        done = (st == QStringLiteral("done") || st == QStringLiteral("concluida") ||
                st == QStringLiteral("completed") || st == QStringLiteral("executado"));
    }

    const QString st_str = task.value(QStringLiteral("status")).toString().toLower();
    const QString cat_str = task.value(QStringLiteral("category")).toString().toLower();
    const int completed_poms = task.value(QStringLiteral("pomodoros_completados"),
                               task.value(QStringLiteral("pomodoros"),
                               task.value(QStringLiteral("ciclos_completados"), 0))).toInt();

    bool in_progress = false;
    if (!done) {
        if (st_str.contains(QStringLiteral("progress")) ||
            st_str.contains(QStringLiteral("andamento")) ||
            st_str == QStringLiteral("doing") ||
            st_str.contains(QStringLiteral("doing")) ||
            cat_str.contains(QStringLiteral("progresso")) ||
            cat_str.contains(QStringLiteral("andamento")) ||
            cat_str.contains(QStringLiteral("doing")) ||
            task.value(QStringLiteral("em_progresso")).toBool() ||
            task.value(QStringLiteral("in_progress")).toBool() ||
            task.value(QStringLiteral("em_andamento")).toBool()) {
            in_progress = true;
        }
    }

    int target_poms = task.value(QStringLiteral("ciclos_totais"),
                      task.value(QStringLiteral("ciclos_permitidos"),
                      task.value(QStringLiteral("total_ciclos"),
                      task.value(QStringLiteral("ciclos_planejados"),
                      task.value(QStringLiteral("ciclos_alvo"), 4))))).toInt();
    if (target_poms <= 0) target_poms = 4;

    double cur_pct = 0.0;
    if (done) {
        cur_pct = 100.0;
    } else if (completed_poms > 0) {
        cur_pct = (static_cast<double>(completed_poms) / static_cast<double>(target_poms)) * 100.0;
        if (cur_pct > 100.0) cur_pct = 100.0;
    }

    ActivityStatus new_status = ActivityStatus::NotStarted;
    if (done || cur_pct >= 100.0) {
        new_status = ActivityStatus::Done;
    } else if (in_progress || completed_poms > 0) {
        new_status = ActivityStatus::InProgress;
    } else {
        new_status = ActivityStatus::NotStarted;
    }

    auto &db = GanttDatabase::instance();
    QList<Schedule> schedules = db.list_schedules();
    bool changed = false;

    PlanningEngineService pes;
    for (auto &sched : schedules) {
        ScheduleItem *item = sched.find_item_by_task_id(task_id);
        if (item) {
            if (!title.isEmpty()) item->name = title;
            if (!desc.isEmpty()) item->description = desc;
            item->duration_hours = duration;
            item->is_milestone = (duration == 0.0);
            item->percent_complete = cur_pct;
            item->status = new_status;
            pes.recalculate(sched);
            db.save_schedule(sched);
            changed = true;
            emit progress_synced(task_id, cur_pct);
        }
    }

    if (gantt_win_ && changed) {
        gantt_win_->load_or_create_default_schedule();
        gantt_win_->refresh_view();
    }
}

void GanttTaskIntegrationBridge::on_eisenhower_task_removed(const QString &task_id)
{
    if (task_id.trimmed().isEmpty()) {
        return;
    }

    if (gantt_win_) {
        gantt_win_->on_eisenhower_task_removed(task_id);
    } else {
        auto &db = GanttDatabase::instance();
        QList<Schedule> schedules = db.list_schedules();
        PlanningEngineService pes;
        for (auto &sched : schedules) {
            ScheduleItem *item = sched.find_item_by_task_id(task_id);
            if (item) {
                const QString item_id = item->id;
                sched.remove_item(item_id);
                pes.recalculate(sched);
                db.save_schedule(sched);
            }
        }
    }

    emit task_link_changed(task_id, QString());
}

void GanttTaskIntegrationBridge::on_eisenhower_task_duration_changed(const QString &task_id, double duration_days)
{
    if (task_id.trimmed().isEmpty()) {
        return;
    }

    if (duration_days <= 0.0) {
        duration_days = 1.0;
    }

    if (gantt_win_) {
        gantt_win_->on_eisenhower_task_duration_changed(task_id, duration_days);
    } else {
        Schedule sched = get_or_load_active_schedule();
        if (sched.id.isEmpty()) {
            return;
        }

        ScheduleItem *found_item = nullptr;
        for (auto &it : sched.items) {
            if (it.task_id == task_id) {
                found_item = &it;
                break;
            }
        }

        if (!found_item) {
            return;
        }

        found_item->duration_hours = duration_days * 8.0;
        found_item->is_milestone = (duration_days == 0.0);
        found_item->end_date = sched.calendar.add_working_hours(found_item->start_date, found_item->duration_hours);
        CpmEngine().calculate(sched);
        save_active_schedule(sched);
    }
}

void GanttTaskIntegrationBridge::sync_gantt_progress_to_eisenhower(const QString &task_id, double percent, bool is_done)
{
    if (task_id.trimmed().isEmpty()) {
        return;
    }

    if (host_) {
        host_->sync_task_progress(task_id, percent, is_done);
    }
}

void GanttTaskIntegrationBridge::sync_gantt_duration_to_eisenhower(const QString &task_id, double duration_days)
{
    if (task_id.trimmed().isEmpty()) {
        return;
    }

    if (host_) {
        host_->sync_task_duration(task_id, duration_days);
    }
}

void GanttTaskIntegrationBridge::sync_gantt_title_to_eisenhower(const QString &task_id, const QString &new_title)
{
    if (task_id.trimmed().isEmpty() || new_title.trimmed().isEmpty()) {
        return;
    }

    if (host_) {
        host_->sync_task_title(task_id, new_title);
    }
}

bool GanttTaskIntegrationBridge::remove_eisenhower_task_by_id(const QString &task_id)
{
    if (task_id.trimmed().isEmpty()) {
        return false;
    }

    if (host_) {
        return host_->remove_task_by_id(task_id);
    }
    return false;
}

void GanttTaskIntegrationBridge::show_task_sync_delete_info(const QString &origin, QWidget *parent)
{
    if (host_) {
        host_->show_task_sync_delete_info(origin.trimmed().isEmpty() ? QStringLiteral("eisenhower") : origin, parent);
    }
}

bool GanttTaskIntegrationBridge::convert_eisenhower_task_to_gantt_activity(const QVariantMap &task,
                                                                          QString *out_item_id,
                                                                          QString *error_message)
{
    QString title = task.value(QStringLiteral("text")).toString().trimmed();
    if (title.isEmpty()) {
        title = task.value(QStringLiteral("titulo")).toString().trimmed();
    }
    if (title.isEmpty()) {
        title = task.value(QStringLiteral("title")).toString().trimmed();
    }
    if (title.isEmpty()) {
        if (error_message) *error_message = QStringLiteral("A tarefa não possui título.");
        return false;
    }

    const QString task_id = task.value(QStringLiteral("id")).toString().trimmed();
    if (task_id.isEmpty()) {
        if (error_message) *error_message = QStringLiteral("ID de tarefa inválido.");
        return false;
    }

    Schedule sched = get_or_load_active_schedule();
    if (sched.id.isEmpty()) {
        sched = ScheduleService().create_schedule(QStringLiteral("Cronograma Geral da Obra"),
                                                 QStringLiteral("Cronograma principal integrado"));
    }

    // Check if already linked
    for (const auto &it : sched.items) {
        if (it.task_id == task_id) {
            if (out_item_id) *out_item_id = it.id;
            return true;
        }
    }

    // Parse date/time
    QString date_str = task.value(QStringLiteral("date")).toString().trimmed();
    if (date_str.isEmpty()) date_str = task.value(QStringLiteral("data")).toString().trimmed();

    QString time_str = task.value(QStringLiteral("time")).toString().trimmed();
    if (time_str.isEmpty()) time_str = task.value(QStringLiteral("hora")).toString().trimmed();
    if (time_str.isEmpty()) time_str = task.value(QStringLiteral("horario")).toString().trimmed();

    QDate start_d = QDate::fromString(date_str, Qt::ISODate);
    if (!start_d.isValid()) start_d = QDate::fromString(date_str, QStringLiteral("dd/MM/yyyy"));
    if (!start_d.isValid()) start_d = QDate::fromString(date_str, QStringLiteral("yyyy-MM-dd"));
    if (!start_d.isValid()) start_d = QDate::currentDate();

    QTime start_t = QTime::fromString(time_str, QStringLiteral("HH:mm"));
    if (!start_t.isValid()) start_t = QTime::fromString(time_str, QStringLiteral("H:mm"));
    if (!start_t.isValid()) start_t = QTime::fromString(time_str, QStringLiteral("HH:mm:ss"));
    if (!start_t.isValid()) start_t = QTime(8, 0);

    const QDateTime start_dt(start_d, start_t);
    double duration = 8.0;
    if (task.contains(QStringLiteral("duracao")) && task.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        duration = task.value(QStringLiteral("duracao")).toDouble() * 8.0;
    } else if (task.contains(QStringLiteral("duration")) && task.value(QStringLiteral("duration")).toDouble() > 0.0) {
        duration = task.value(QStringLiteral("duration")).toDouble() * 8.0;
    }

    int quadrant = 0;
    if (task.contains(QStringLiteral("priority")) && task.value(QStringLiteral("priority")).toInt() > 0) {
        quadrant = task.value(QStringLiteral("priority")).toInt();
    } else if (task.contains(QStringLiteral("prioridade")) && task.value(QStringLiteral("prioridade")).toInt() > 0) {
        quadrant = task.value(QStringLiteral("prioridade")).toInt();
    } else if (task.contains(QStringLiteral("quadrante")) && task.value(QStringLiteral("quadrante")).toInt() > 0) {
        quadrant = task.value(QStringLiteral("quadrante")).toInt();
    }
    if (quadrant < 1 || quadrant > 4) quadrant = 1;

    // Count items in this quadrant for WBS code
    int q_count = 1;
    for (const auto &it : sched.items) {
        if (it.wbs_code.startsWith(QStringLiteral("%1.").arg(quadrant))) {
            q_count++;
        }
    }

    const QString new_item_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    ScheduleItem item(title);
    item.id = new_item_id;
    item.start_date = start_dt;
    item.duration_hours = duration;
    item.end_date = sched.calendar.add_working_hours(start_dt, duration);
    item.baseline_start_date = item.start_date;
    item.baseline_end_date = item.end_date;
    item.baseline_duration_hours = item.duration_hours;
    item.task_id = task_id;
    item.description = task.value(QStringLiteral("description"), task.value(QStringLiteral("descricao"))).toString();
    item.wbs_code = QStringLiteral("%1.%2").arg(quadrant).arg(q_count);

    const QDateTime now = QDateTime::currentDateTime();

    bool done = false;
    if (task.contains(QStringLiteral("concluida"))) done = task.value(QStringLiteral("concluida")).toBool();
    else if (task.contains(QStringLiteral("completed"))) done = task.value(QStringLiteral("completed")).toBool();
    else {
        const QString st = task.value(QStringLiteral("status")).toString().toLower();
        done = (st == QStringLiteral("done") || st == QStringLiteral("concluida") || st == QStringLiteral("completed") || st == QStringLiteral("executado"));
    }

    const QString status_str = task.value(QStringLiteral("status")).toString().toLower();
    const QString category_str = task.value(QStringLiteral("category")).toString().toLower();
    const int pomodoros_completados = task.value(QStringLiteral("pomodoros_completados"), task.value(QStringLiteral("pomodoros"), 0)).toInt();
    const int ciclos_completados = task.value(QStringLiteral("ciclos_completados"), task.value(QStringLiteral("ciclos_feitos"), pomodoros_completados)).toInt();

    bool in_progress = false;
    if (!done) {
        if (status_str.contains(QStringLiteral("progress")) ||
            status_str.contains(QStringLiteral("andamento")) ||
            status_str == QStringLiteral("doing") ||
            status_str.contains(QStringLiteral("doing")) ||
            category_str.contains(QStringLiteral("progresso")) ||
            category_str.contains(QStringLiteral("andamento")) ||
            category_str.contains(QStringLiteral("doing")) ||
            ciclos_completados > 0 ||
            task.value(QStringLiteral("em_progresso")).toBool() ||
            task.value(QStringLiteral("in_progress")).toBool() ||
            task.value(QStringLiteral("em_andamento")).toBool()) {
            in_progress = true;
        }
    }

    int ciclos_total = task.value(QStringLiteral("ciclos_totais"),
                       task.value(QStringLiteral("ciclos_permitidos"),
                       task.value(QStringLiteral("total_ciclos"),
                       task.value(QStringLiteral("ciclos_alvo"), 0)))).toInt();
    if (ciclos_total <= 0) {
        ciclos_total = task.value(QStringLiteral("ciclos"), 0).toInt();
    }
    if (ciclos_total <= 0) {
        ciclos_total = 4;
    }

    const double percent = source::utils::calculate_task_completion_percentage(ciclos_completados, ciclos_total, done);
    ActivityStatus act_status = ActivityStatus::NotStarted;

    if (done || percent >= 100.0) {
        act_status = ActivityStatus::Done;
    } else if (in_progress || ciclos_completados > 0) {
        act_status = ActivityStatus::InProgress;
    } else {
        act_status = ActivityStatus::NotStarted;
    }

    item.percent_complete = percent;
    item.status = act_status;

    sched.add_item(item);
    if (!sched.active_baseline_id.isEmpty() && sched.baselines.contains(sched.active_baseline_id)) {
        BaselineItemSnapshot b_item;
        b_item.item_id = item.id;
        b_item.start_date = item.baseline_start_date;
        b_item.end_date = item.baseline_end_date;
        b_item.duration_hours = item.baseline_duration_hours;
        sched.baselines[sched.active_baseline_id].items.insert(item.id, b_item);
    }
    save_active_schedule(sched);

    if (out_item_id) *out_item_id = new_item_id;
    emit task_link_changed(task_id, new_item_id);
    return true;
}

bool GanttTaskIntegrationBridge::unlink_task(const QString &task_id, QString *error_message)
{
    if (task_id.trimmed().isEmpty()) {
        return false;
    }

    auto &db = GanttDatabase::instance();
    QList<Schedule> schedules = db.list_schedules();
    bool unlinked = false;

    for (auto &sched : schedules) {
        for (auto &item : sched.items) {
            if (item.task_id == task_id) {
                item.task_id.clear();
                unlinked = true;
            }
        }
        if (unlinked) {
            save_active_schedule(sched);
        }
    }

    if (!unlinked && error_message) {
        *error_message = QStringLiteral("Vínculo não encontrado.");
    }

    emit task_link_changed(task_id, QString());
    return unlinked;
}

bool GanttTaskIntegrationBridge::link_task_to_activity(const QString &task_id,
                                                      const QString &activity_id,
                                                      const QString &schedule_id,
                                                      QString *error_message)
{
    auto &db = GanttDatabase::instance();
    Schedule sched;
    if (!schedule_id.isEmpty()) {
        if (!db.load_schedule(schedule_id, sched)) {
            sched = get_or_load_active_schedule();
        }
    } else {
        sched = get_or_load_active_schedule();
    }

    if (sched.id.isEmpty()) {
        if (error_message) *error_message = QStringLiteral("Nenhum cronograma ativo encontrado.");
        return false;
    }

    ScheduleItem *target = sched.find_item(activity_id);
    if (!target) {
        if (error_message) *error_message = QStringLiteral("Atividade não encontrada no cronograma.");
        return false;
    }

    // Clear previous task links to this activity or this task to other activities in schedule
    for (auto &it : sched.items) {
        if (it.task_id == task_id) {
            it.task_id.clear();
        }
    }

    target->task_id = task_id;
    save_active_schedule(sched);

    emit task_link_changed(task_id, activity_id);
    return true;
}

bool GanttTaskIntegrationBridge::record_field_progress_for_task(const QString &task_id,
                                                               double percent,
                                                               const QString &notes,
                                                               const QString &reporter,
                                                               QString *error_message)
{
    auto &db = GanttDatabase::instance();
    QList<Schedule> schedules = db.list_schedules();

    for (auto &sched : schedules) {
        ScheduleItem *item = sched.find_item_by_task_id(task_id);
        if (item) {
            FieldProgressService fps;
            const bool ok = fps.record_progress(sched, item->id, percent, notes, reporter, error_message);
            if (ok) {
                save_active_schedule(sched);
                sync_gantt_progress_to_eisenhower(task_id, percent, percent >= 100.0);
                emit progress_synced(task_id, percent);
            }
            return ok;
        }
    }

    if (error_message) *error_message = QStringLiteral("Nenhuma atividade de cronograma vinculada a esta tarefa.");
    return false;
}

void GanttTaskIntegrationBridge::open_gantt_and_highlight_activity(const QString &schedule_item_id, QWidget *parent)
{
    if (!gantt_win_) {
        QWidget *p = parent ? parent : (host_ ? host_->as_widget() : nullptr);
        gantt_win_ = new GanttMainWindow(p);
        gantt_win_->setAttribute(Qt::WA_DeleteOnClose, false);
    }

    gantt_win_->sync_all_eisenhower_tasks(true);
    gantt_win_->show();
    gantt_win_->raise();
    gantt_win_->activateWindow();

    if (!schedule_item_id.isEmpty()) {
        // Highlight in WBS tree
        gantt_win_->on_activity_selected(schedule_item_id);
    }
}

} // namespace source::gantt
