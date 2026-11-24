#pragma once

#include <QObject>
#include <QPointer>

#include "source/modules/gantt/gantt_host_bridge.hpp"
#include "source/modules/gantt/gantt_plugin_interface.hpp"

class QThread;

namespace source::gantt {
class GanttMainWindow;
namespace language {
class GerenciadorTraducao;
class GanttTranslationWorker;
}
}

namespace source::plugin {

class GanttPlugin final : public QObject, public IGanttPlugin {
    Q_OBJECT
    Q_INTERFACES(source::plugin::IGanttPlugin)
#if defined(EISENHOWER_BUILD_GANTT_PLUGIN_DLL)
    Q_PLUGIN_METADATA(IID SOURCE_GANTT_PLUGIN_INTERFACE_IID FILE "gantt_plugin.json")
#endif

public:
    explicit GanttPlugin(QObject *parent = nullptr);
    ~GanttPlugin() override;

    QString plugin_id() const override;
    QString display_name() const override;
    QString open_menu_text() const override;
    void initialize(GanttHostBridge *host_bridge) override;
    void open_window(QWidget *parent = nullptr) override;
    void open_gantt_and_highlight_activity(const QString &schedule_item_id, QWidget *parent = nullptr) override;

    bool is_task_linked(const QString &task_id) const override;
    bool get_task_gantt_summary(const QString &task_id,
                                QString *out_wbs_and_name,
                                QString *out_sched_name,
                                double *out_progress,
                                QString *out_status) const override;
    bool convert_eisenhower_task_to_gantt_activity(const QVariantMap &task,
                                                   QString *out_item_id = nullptr,
                                                   QString *error_message = nullptr) override;
    bool link_task_to_activity(const QString &task_id,
                               const QString &activity_id,
                               const QString &schedule_id = QString(),
                               QString *error_message = nullptr) override;
    bool unlink_task(const QString &task_id, QString *error_message = nullptr) override;
    bool open_task_link_dialog(const QString &task_id,
                               const QString &task_title,
                               QWidget *parent = nullptr) override;
    bool record_field_progress_for_task(const QString &task_id,
                                        double percent,
                                        const QString &notes,
                                        const QString &reporter = QStringLiteral("Campo"),
                                        QString *error_message = nullptr) override;

    void on_eisenhower_task_completed(const QString &task_id, bool is_completed) override;
    void on_eisenhower_task_status_changed(const QString &task_id, const QString &status) override;
    void on_eisenhower_task_duration_changed(const QString &task_id, double duration_days) override;
    void on_eisenhower_task_removed(const QString &task_id) override;
    void on_eisenhower_task_modified(const QString &task_id) override;
    void sync_all_tasks() override;
    void retranslate_ui() override;
    void close_window() override;

private:
    source::gantt::GanttMainWindow *ensure_window(QWidget *parent = nullptr);
    void apply_host_language();

private slots:
    void on_translation_prepared(const QString &language_code, const QString &qm_file_path);

private:
    GanttHostBridge *host_bridge_ = nullptr;
    QMetaObject::Connection host_destroyed_connection_;
    QPointer<source::gantt::GanttMainWindow> window_;
    source::gantt::language::GerenciadorTraducao *translation_manager_ = nullptr;
    QThread *translation_thread_ = nullptr;
    source::gantt::language::GanttTranslationWorker *translation_worker_ = nullptr;
    bool needs_eisenhower_sync_ = true;
};

} // namespace source::plugin
