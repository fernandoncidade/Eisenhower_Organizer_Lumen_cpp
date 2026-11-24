#pragma once

#include <QDate>
#include <QDialog>
#include <QPoint>
#include <QSet>
#include <QVariantMap>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QCloseEvent;
namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI {

class BadgeCalendarWidget;
class NativeLineEditFrame;

class CalendarDialog : public QDialog {
    Q_OBJECT

public:
    explicit CalendarDialog(source::EisenhowerMatrixApp *app);

    source::EisenhowerMatrixApp *app = nullptr;
    QString _date_format;
    QSet<QString> _highlighted_dates;

    BadgeCalendarWidget *calendar = nullptr;
    QGroupBox *calendar_group = nullptr;
    NativeLineEditFrame *calendar_frame = nullptr;
    QLabel *filter_label = nullptr;
    QComboBox *filter_combo = nullptr;
    QLabel *show_label = nullptr;
    QCheckBox *show_priority_checkbox = nullptr;
    QCheckBox *show_domain_checkbox = nullptr;
    QLabel *tasks_label = nullptr;
    QGroupBox *tasks_group = nullptr;
    QListWidget *tasks_list = nullptr;

    QList<QVariantMap> _collect_tasks() const;
    void update_task_list();
    void _update_badge_counts();
    void _on_language_changed();
    void _apply_locale_to_calendar();
    QSet<QString> _get_task_dates() const;
    void _apply_highlighted_dates();
    void _on_calendar_context_menu(const QPoint &pos);
    void _prompt_edit_task_for_date(const QDate &qdate);
    void _prompt_remove_task_for_date(const QDate &qdate);
    void _open_linked_file(QListWidgetItem *item);

protected:
    void closeEvent(QCloseEvent *event) override;
};

}  // namespace source::GerenciamentoUI
