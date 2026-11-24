#pragma once

#include <QDate>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QVariantMap>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QHBoxLayout;
class QGroupBox;
class QLabel;
class QListWidget;
class QResizeEvent;
class QShowEvent;
class QStackedWidget;
class QTimer;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI {

class BadgeCalendarWidget;
class Calendar;
class CalendarDialog;
class NativeLineEditFrame;
class YearCalendarWidget;

class CalendarPanel : public QWidget {
    Q_OBJECT

public:
    explicit CalendarPanel(source::EisenhowerMatrixApp *app, QWidget *parent = nullptr);

    source::EisenhowerMatrixApp *app = nullptr;
    QString _date_format;
    QSet<QString> _highlighted_dates;
    QString _calendar_view_mode;
    int _calendar_content_width = 300;
    bool _syncing_month_calendars = false;
    QList<BadgeCalendarWidget *> _extra_month_calendars;
    QSet<QString> _extra_highlighted_dates;
    bool _syncing_year_calendars = false;
    QList<YearCalendarWidget *> _extra_year_calendars;
    bool _year_sync_pending = false;
    QList<YearCalendarWidget *> _year_calendar_pool;
    QTimer *_year_resize_timer = nullptr;
    QDate _selected_reference_date;
    QStringList _selected_reference_dates;
    QList<QVariantMap> _task_index_records;
    QMap<QString, QList<QVariantMap>> _task_index_by_date;
    QSet<QString> _task_index_dates;

    BadgeCalendarWidget *calendar = nullptr;
    QGroupBox *calendar_group = nullptr;
    QList<QGroupBox *> _extra_month_groups;
    YearCalendarWidget *year_calendar = nullptr;
    QGroupBox *year_calendar_group = nullptr;
    QList<QGroupBox *> _extra_year_groups;
    QList<QGroupBox *> _year_group_pool;
    QWidget *months_container = nullptr;
    QHBoxLayout *months_layout = nullptr;
    QWidget *years_container = nullptr;
    QHBoxLayout *years_layout = nullptr;
    QStackedWidget *calendar_stack = nullptr;
    NativeLineEditFrame *calendar_frame = nullptr;
    QWidget *controls_container = nullptr;
    QLabel *filter_label = nullptr;
    QComboBox *filter_combo = nullptr;
    QLabel *show_label = nullptr;
    QCheckBox *show_priority_checkbox = nullptr;
    QCheckBox *show_domain_checkbox = nullptr;
    QLabel *tasks_label = nullptr;
    QGroupBox *tasks_group = nullptr;
    QListWidget *tasks_list = nullptr;

    void set_calendar_content_width(int width);
    void update_calendars_for_width();
    QList<QVariantMap> _collect_tasks() const;
    QList<BadgeCalendarWidget *> _iter_calendars() const;
    void update_task_list();
    void _update_badge_counts();
    QSet<QString> _get_task_dates() const;
    void _apply_highlighted_dates();
    void _on_language_changed();
    void _apply_locale_to_calendar();
    void _on_calendar_context_menu(const QPoint &pos);
    void _prompt_edit_task_for_date(const QDate &qdate);
    void _prompt_remove_task_for_date(const QDate &qdate);
    QDate get_filter_reference_date() const;
    QStringList get_filter_reference_dates() const;
    void set_calendar_view_mode(const QString &mode);
    void toggle_month_filter_view_mode();
    bool is_month_filter_active() const;
    QString calendar_view_mode() const;

signals:
    void month_view_toggle_visibility_changed(bool visible);
    void month_view_mode_changed(const QString &mode);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    friend class Calendar;

    bool _is_month_view_active() const;
    bool _is_year_view_active() const;
    void _schedule_year_calendars_update(int delay_ms = 24);
    int _available_calendar_width() const;
    int _available_months_inner_width() const;
    int _available_years_inner_width() const;
    int _desired_month_calendar_count() const;
    void _apply_month_calendar_width_distribution();
    void _create_extra_month_calendar();
    void _remove_last_extra_month_calendar();
    void _ensure_month_calendar_count(int target_count);
    void _update_month_calendars_for_width();
    int _desired_year_calendar_count() const;
    void _apply_year_calendar_width_distribution();
    void _create_extra_year_calendar();
    void _configure_year_calendar_widget(YearCalendarWidget *year_calendar_widget);
    void _remove_last_extra_year_calendar();
    bool _ensure_year_calendar_count(int target_count);
    bool _reconcile_year_calendar_count(int target_count, bool sync_after_change = true);
    void _update_year_calendars_for_width();
    bool _ctrl_multi_select_active() const;
    void _update_reference_selection(const QDate &qdate, bool allow_multi_on_ctrl = true);
    void _on_any_calendar_date_clicked(BadgeCalendarWidget *calendar_widget, const QDate &qdate);
    void _on_primary_calendar_selection_changed();
    void _on_primary_calendar_page_changed(int year, int month);
    void _on_extra_calendar_selection_changed(BadgeCalendarWidget *calendar_widget);
    void _on_extra_calendar_page_changed(BadgeCalendarWidget *calendar_widget, int year, int month);
    void _sync_month_calendar_pages_from(BadgeCalendarWidget *source_calendar, int year, int month);
    void _sync_month_calendar_pages_from_primary();
    void _sync_month_calendars_visual_data();
    void _on_any_calendar_context_menu(BadgeCalendarWidget *calendar_widget, const QPoint &pos);
    void _on_filter_mode_changed();
    void _on_year_month_selected(const QDate &qdate);
    void _on_year_month_double_clicked(const QDate &qdate);
    void _sync_year_calendar_counts();
    void _sync_year_calendar_pages_from_primary();
    void _sync_year_calendars_visual_data();
    void _rebuild_task_date_index();
    bool eventFilter(QObject *obj, QEvent *event) override;
};

}  // namespace source::GerenciamentoUI
