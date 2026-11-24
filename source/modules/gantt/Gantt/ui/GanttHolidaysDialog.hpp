#pragma once

#include <QCalendarWidget>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include "../domain/models/WorkCalendar.hpp"

namespace source::gantt {

class GanttHolidaysDialog : public QDialog {
    Q_OBJECT

public:
    explicit GanttHolidaysDialog(const WorkCalendar &current_cal, QWidget *parent = nullptr);

    QList<CalendarException> updated_exceptions() const;

private slots:
    void on_calendar_selection_changed();
    void on_tree_selection_changed();
    void on_add_or_update_holiday();
    void on_remove_selected_holiday();
    void on_delete_tree_row();
    void on_clear_all_holidays();
    void on_apply_country_holidays();

private:
    void setup_ui();
    void refresh_calendar_formats();
    void refresh_tree();
    void update_inputs_for_date(const QDate &date);

    WorkCalendar calendar_copy_;
    QCalendarWidget *calendar_widget_ = nullptr;
    QLabel *lbl_selected_date_ = nullptr;
    QLineEdit *edit_name_ = nullptr;
    QPushButton *btn_add_update_ = nullptr;
    QPushButton *btn_remove_ = nullptr;
    QTreeWidget *tree_holidays_ = nullptr;
    QComboBox *combo_countries_ = nullptr;
    QPushButton *btn_apply_country_ = nullptr;
    QPushButton *btn_delete_tree_ = nullptr;
    QPushButton *btn_clear_ = nullptr;
};

} // namespace source::gantt
