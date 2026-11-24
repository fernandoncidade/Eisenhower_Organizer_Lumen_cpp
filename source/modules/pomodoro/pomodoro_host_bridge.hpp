#pragma once

#include <functional>
#include <utility>

#include <QDialog>
#include <QLocale>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class QObject;
class QWidget;

namespace source::plugin {

class PomodoroHostBridge {
public:
    using TaskHandler = std::function<void(const QVariantMap &)>;
    using FeynmanTaskHandler = std::function<bool(const QVariantMap &)>;
    using AvailabilityChecker = std::function<bool()>;
    using ReservedSpaceChecker = std::function<bool()>;
    using WatermarkChecker = std::function<bool()>;
    using TaskSyncEnabledChecker = std::function<bool()>;
    using StandardButtonsHandler = std::function<void(QObject *)>;
    using SharedStateChangedHandler = std::function<void(const QString &)>;
    using SyncOperationInfoHandler = std::function<void(const QString &, const QString &)>;
    using TaskDeletedHandler = std::function<void(const QVariantMap &, const QString &)>;
    using TaskSortStateChecker = std::function<bool()>;
    using TaskSortLessHandler = std::function<bool(const QVariantMap &, const QVariantMap &)>;
    using GanttAvailableChecker = std::function<bool()>;
    using GanttLinkDialogHandler = std::function<bool(const QString &, const QString &, QWidget *)>;
    using GanttSummaryHandler = std::function<bool(const QString &, QString *, QString *, double *, QString *)>;
    using GanttHighlightHandler = std::function<void(const QString &, QWidget *)>;
    using GanttRecordProgressHandler = std::function<bool(const QString &, double, const QString &, const QString &, QString *)>;
    using GanttUnlinkHandler = std::function<bool(const QString &)>;
    using TaskDurationSyncHandler = std::function<void(const QString &, double)>;
    using DefaultCycleCountChangedHandler = std::function<void(int)>;
    using ResetInProgressCyclesHandler = std::function<void(int, const QVariantList &)>;

    void set_language_code(const QString &language_code)
    {
        language_code_ = language_code.trimmed().isEmpty() ? QLocale::system().name() : language_code.trimmed();
    }

    QString language_code() const
    {
        return language_code_;
    }

    QLocale locale() const
    {
        if (language_code_.startsWith(QStringLiteral("pt"), Qt::CaseInsensitive)) {
            return QLocale(QLocale::Portuguese, QLocale::Brazil);
        }
        if (language_code_.startsWith(QStringLiteral("en"), Qt::CaseInsensitive)) {
            return QLocale(QLocale::English, QLocale::UnitedStates);
        }
        return QLocale::system();
    }

    QString date_format() const
    {
        return language_code_.startsWith(QStringLiteral("en"), Qt::CaseInsensitive) ? QStringLiteral("MM/dd/yyyy")
                                                                                     : QStringLiteral("dd/MM/yyyy");
    }

    void set_task_to_eisenhower_handler(TaskHandler handler)
    {
        task_to_eisenhower_handler_ = std::move(handler);
    }

    void publish_task_to_eisenhower(const QVariantMap &task) const
    {
        if (task_to_eisenhower_handler_) {
            task_to_eisenhower_handler_(task);
        }
    }

    void set_task_to_feynman_handler(FeynmanTaskHandler handler)
    {
        task_to_feynman_handler_ = std::move(handler);
    }

    bool publish_task_to_feynman(const QVariantMap &task) const
    {
        return task_to_feynman_handler_ ? task_to_feynman_handler_(task) : false;
    }

    void set_feynman_available_checker(AvailabilityChecker checker)
    {
        feynman_available_checker_ = std::move(checker);
    }

    bool can_publish_task_to_feynman() const
    {
        if (!task_to_feynman_handler_) {
            return false;
        }

        return !feynman_available_checker_ || feynman_available_checker_();
    }

    void set_reserved_space_checker(ReservedSpaceChecker checker)
    {
        reserved_space_checker_ = std::move(checker);
    }

    bool reserved_space_enabled() const
    {
        return reserved_space_checker_ ? reserved_space_checker_() : false;
    }

    void set_watermark_checker(WatermarkChecker checker)
    {
        watermark_checker_ = std::move(checker);
    }

    bool watermark_enabled() const
    {
        return watermark_checker_ ? watermark_checker_() : false;
    }

    void set_task_sync_enabled_checker(TaskSyncEnabledChecker checker)
    {
        task_sync_enabled_checker_ = std::move(checker);
    }

    bool task_sync_enabled() const
    {
        return task_sync_enabled_checker_ ? task_sync_enabled_checker_() : true;
    }

    void set_standard_buttons_handler(StandardButtonsHandler handler)
    {
        standard_buttons_handler_ = std::move(handler);
    }

    void translate_standard_buttons(QObject *dialog) const
    {
        if (standard_buttons_handler_) {
            standard_buttons_handler_(dialog);
        }
    }

    void set_shared_state_changed_handler(SharedStateChangedHandler handler)
    {
        shared_state_changed_handler_ = std::move(handler);
    }

    void notify_shared_state_changed(const QString &origin) const
    {
        if (shared_state_changed_handler_) {
            shared_state_changed_handler_(origin);
        }
    }

    void set_sync_operation_info_handler(SyncOperationInfoHandler handler)
    {
        sync_operation_info_handler_ = std::move(handler);
    }

    void show_sync_operation_info(const QString &operation, const QString &origin) const
    {
        if (sync_operation_info_handler_) {
            sync_operation_info_handler_(operation, origin);
        }
    }

    void set_task_deleted_handler(TaskDeletedHandler handler)
    {
        task_deleted_handler_ = std::move(handler);
    }

    void publish_task_deleted(const QVariantMap &task, const QString &origin) const
    {
        if (task_deleted_handler_) {
            task_deleted_handler_(task, origin);
        }
    }

    void set_task_sort_state_checker(TaskSortStateChecker checker)
    {
        task_sort_state_checker_ = std::move(checker);
    }

    bool has_active_task_sort() const
    {
        return task_sort_state_checker_ ? task_sort_state_checker_() : false;
    }

    void set_task_sort_less_handler(TaskSortLessHandler handler)
    {
        task_sort_less_handler_ = std::move(handler);
    }

    bool task_sort_less(const QVariantMap &left, const QVariantMap &right) const
    {
        return task_sort_less_handler_ ? task_sort_less_handler_(left, right) : false;
    }

    void set_gantt_available_checker(GanttAvailableChecker checker)
    {
        gantt_available_checker_ = std::move(checker);
    }

    bool can_publish_task_to_gantt() const
    {
        return gantt_available_checker_ ? gantt_available_checker_() : false;
    }

    void set_gantt_link_handler(GanttLinkDialogHandler handler)
    {
        gantt_link_handler_ = std::move(handler);
    }

    bool open_gantt_link_dialog(const QString &task_id, const QString &task_title, QWidget *parent = nullptr) const
    {
        return gantt_link_handler_ ? gantt_link_handler_(task_id, task_title, parent) : false;
    }

    void set_task_duration_sync_handler(TaskDurationSyncHandler handler)
    {
        task_duration_sync_handler_ = std::move(handler);
    }

    void sync_task_duration(const QString &task_id, double duration_days) const
    {
        if (task_duration_sync_handler_) {
            task_duration_sync_handler_(task_id, duration_days);
        }
    }

    void set_gantt_summary_handler(GanttSummaryHandler handler)
    {
        gantt_summary_handler_ = std::move(handler);
    }

    bool get_gantt_summary(const QString &task_id, QString *wbs_and_name, QString *sched_name, double *progress, QString *status) const
    {
        return gantt_summary_handler_ ? gantt_summary_handler_(task_id, wbs_and_name, sched_name, progress, status) : false;
    }

    void set_gantt_highlight_handler(GanttHighlightHandler handler)
    {
        gantt_highlight_handler_ = std::move(handler);
    }

    void open_gantt_and_highlight_activity(const QString &task_id, QWidget *parent = nullptr) const
    {
        if (gantt_highlight_handler_) {
            gantt_highlight_handler_(task_id, parent);
        }
    }

    void set_gantt_record_progress_handler(GanttRecordProgressHandler handler)
    {
        gantt_record_progress_handler_ = std::move(handler);
    }

    bool record_field_progress_for_task(const QString &task_id, double percent, const QString &note, const QString &author, QString *error_msg = nullptr) const
    {
        return gantt_record_progress_handler_ ? gantt_record_progress_handler_(task_id, percent, note, author, error_msg) : false;
    }

    void set_gantt_unlink_handler(GanttUnlinkHandler handler)
    {
        gantt_unlink_handler_ = std::move(handler);
    }

    bool unlink_task(const QString &task_id) const
    {
        return gantt_unlink_handler_ ? gantt_unlink_handler_(task_id) : false;
    }

    void set_default_cycle_count_changed_handler(DefaultCycleCountChangedHandler handler)
    {
        default_cycle_count_changed_handler_ = std::move(handler);
    }

    void notify_default_cycle_count_changed(int new_count) const
    {
        if (default_cycle_count_changed_handler_) {
            default_cycle_count_changed_handler_(new_count);
        }
    }

    void set_reset_in_progress_cycles_handler(ResetInProgressCyclesHandler handler)
    {
        reset_in_progress_cycles_handler_ = std::move(handler);
    }

    void reset_in_progress_task_cycles(int new_cycles, const QVariantList &affected_tasks) const
    {
        if (reset_in_progress_cycles_handler_) {
            reset_in_progress_cycles_handler_(new_cycles, affected_tasks);
        }
    }

private:
    QString language_code_ = QLocale::system().name();
    TaskHandler task_to_eisenhower_handler_;
    FeynmanTaskHandler task_to_feynman_handler_;
    AvailabilityChecker feynman_available_checker_;
    ReservedSpaceChecker reserved_space_checker_;
    WatermarkChecker watermark_checker_;
    TaskSyncEnabledChecker task_sync_enabled_checker_;
    StandardButtonsHandler standard_buttons_handler_;
    SharedStateChangedHandler shared_state_changed_handler_;
    SyncOperationInfoHandler sync_operation_info_handler_;
    TaskDeletedHandler task_deleted_handler_;
    TaskSortStateChecker task_sort_state_checker_;
    TaskSortLessHandler task_sort_less_handler_;
    GanttAvailableChecker gantt_available_checker_;
    GanttLinkDialogHandler gantt_link_handler_;
    TaskDurationSyncHandler task_duration_sync_handler_;
    GanttSummaryHandler gantt_summary_handler_;
    GanttHighlightHandler gantt_highlight_handler_;
    GanttRecordProgressHandler gantt_record_progress_handler_;
    GanttUnlinkHandler gantt_unlink_handler_;
    DefaultCycleCountChangedHandler default_cycle_count_changed_handler_;
    ResetInProgressCyclesHandler reset_in_progress_cycles_handler_;
};

}  // namespace source::plugin
