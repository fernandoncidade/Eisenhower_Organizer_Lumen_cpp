#pragma once

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include "IEisenhowerHost.hpp"
#include "../../domain/models/Schedule.hpp"

class QWidget;

namespace source::gantt {

class GanttMainWindow;

class GanttTaskIntegrationBridge : public QObject {
    Q_OBJECT

public:
    static GanttTaskIntegrationBridge &instance();

    // Host & Window Registration
    void register_host(IEisenhowerHost *host);
    void unregister_host(IEisenhowerHost *host);
    IEisenhowerHost *host() const;

    void register_gantt_window(GanttMainWindow *win);
    void unregister_gantt_window(GanttMainWindow *win);
    GanttMainWindow *gantt_window() const;

    // Link Queries
    bool is_task_linked(const QString &task_id,
                        QString *out_schedule_id = nullptr,
                        QString *out_schedule_name = nullptr,
                        QString *out_item_id = nullptr,
                        QString *out_activity_name = nullptr,
                        QString *out_wbs_code = nullptr,
                        double *out_progress = nullptr,
                        QString *out_status = nullptr) const;

    // Operational Task Queries
    QVariantList get_available_eisenhower_tasks() const;
    QVariantMap get_task_by_id(const QString &task_id) const;

    // Field Execution Events (EisenPo -> Gantt)
    void on_eisenhower_task_completed(const QString &task_id, bool is_completed);
    void on_eisenhower_task_in_progress(const QString &task_id);
    void on_eisenhower_task_removed(const QString &task_id);
    void on_eisenhower_task_duration_changed(const QString &task_id, double duration_days);
    void on_eisenhower_task_modified(const QString &task_id);

    // Schedule Events (Gantt -> EisenPo)
    void sync_gantt_progress_to_eisenhower(const QString &task_id, double percent, bool is_done);
    void sync_gantt_duration_to_eisenhower(const QString &task_id, double duration_days);
    void sync_gantt_title_to_eisenhower(const QString &task_id, const QString &new_title);
    bool remove_eisenhower_task_by_id(const QString &task_id);
    void show_task_sync_delete_info(const QString &origin = QString(), QWidget *parent = nullptr);

    // Operations
    bool convert_eisenhower_task_to_gantt_activity(const QVariantMap &task,
                                                    QString *out_item_id = nullptr,
                                                    QString *error_message = nullptr);

    bool unlink_task(const QString &task_id, QString *error_message = nullptr);

    bool link_task_to_activity(const QString &task_id,
                               const QString &activity_id,
                               const QString &schedule_id = QString(),
                               QString *error_message = nullptr);

    bool record_field_progress_for_task(const QString &task_id,
                                        double percent,
                                        const QString &notes,
                                        const QString &reporter = QStringLiteral("Campo"),
                                        QString *error_message = nullptr);

    // Navigation
    void open_gantt_and_highlight_activity(const QString &schedule_item_id, QWidget *parent = nullptr);

signals:
    void task_link_changed(const QString &task_id, const QString &activity_id);
    void progress_synced(const QString &task_id, double percent);

private:
    GanttTaskIntegrationBridge(QObject *parent = nullptr);
    ~GanttTaskIntegrationBridge() override = default;

    IEisenhowerHost *host_ = nullptr;
    QPointer<GanttMainWindow> gantt_win_;

    Schedule get_or_load_active_schedule() const;
    void save_active_schedule(Schedule &schedule);

    struct TaskLinkSummary {
        QString schedule_id;
        QString schedule_name;
        QString item_id;
        QString activity_name;
        QString wbs_code;
        double progress = 0.0;
        QString status;
    };

    void invalidate_cache();
    void refresh_cache_if_needed() const;

    mutable QHash<QString, TaskLinkSummary> linked_tasks_cache_;
    mutable bool cache_initialized_ = false;
    mutable QMutex cache_mutex_;
};

} // namespace source::gantt
