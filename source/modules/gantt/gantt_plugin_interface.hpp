#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QtPlugin>

class QWidget;

namespace source::plugin {

class GanttHostBridge;

class IGanttPlugin {
public:
    virtual ~IGanttPlugin() = default;

    virtual QString plugin_id() const = 0;
    virtual QString display_name() const = 0;
    virtual QString open_menu_text() const = 0;
    virtual void initialize(GanttHostBridge *host_bridge) = 0;
    virtual void open_window(QWidget *parent = nullptr) = 0;
    virtual void open_gantt_and_highlight_activity(const QString &schedule_item_id, QWidget *parent = nullptr) = 0;

    virtual bool is_task_linked(const QString &task_id) const = 0;
    virtual bool get_task_gantt_summary(const QString &task_id,
                                        QString *out_wbs_and_name,
                                        QString *out_sched_name,
                                        double *out_progress,
                                        QString *out_status) const = 0;
    virtual bool convert_eisenhower_task_to_gantt_activity(const QVariantMap &task,
                                                           QString *out_item_id = nullptr,
                                                           QString *error_message = nullptr) = 0;
    virtual bool link_task_to_activity(const QString &task_id,
                                       const QString &activity_id,
                                       const QString &schedule_id = QString(),
                                       QString *error_message = nullptr) = 0;
    virtual bool unlink_task(const QString &task_id, QString *error_message = nullptr) = 0;
    virtual bool open_task_link_dialog(const QString &task_id,
                                       const QString &task_title,
                                       QWidget *parent = nullptr) = 0;
    virtual bool record_field_progress_for_task(const QString &task_id,
                                                double percent,
                                                const QString &notes,
                                                const QString &reporter = QStringLiteral("Campo"),
                                                QString *error_message = nullptr) = 0;

    virtual void on_eisenhower_task_completed(const QString &task_id, bool is_completed) = 0;
    virtual void on_eisenhower_task_status_changed(const QString &task_id, const QString &status) = 0;
    virtual void on_eisenhower_task_duration_changed(const QString &task_id, double duration_days) {
        Q_UNUSED(task_id);
        Q_UNUSED(duration_days);
    }
    virtual void on_eisenhower_task_removed(const QString &task_id) = 0;
    virtual void on_eisenhower_task_modified(const QString &task_id) {
        Q_UNUSED(task_id);
        sync_all_tasks();
    }
    virtual void sync_all_tasks() = 0;
    virtual void retranslate_ui() = 0;
    virtual void close_window() = 0;
};

} // namespace source::plugin

#define SOURCE_GANTT_PLUGIN_INTERFACE_IID "source.plugin.IGanttPlugin/2026.11.22.0"

Q_DECLARE_INTERFACE(source::plugin::IGanttPlugin, SOURCE_GANTT_PLUGIN_INTERFACE_IID)
