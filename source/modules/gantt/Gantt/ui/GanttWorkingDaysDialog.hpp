#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QSet>
#include "../domain/models/WorkCalendar.hpp"

namespace source::gantt {

class GanttWorkingDaysDialog : public QDialog {
    Q_OBJECT

public:
    explicit GanttWorkingDaysDialog(const WorkCalendar &current_cal, QWidget *parent = nullptr);

    QSet<int> selected_working_days() const;
    double selected_hours_per_day() const;

private slots:
    void on_preset_mon_fri();
    void on_preset_mon_sat();
    void on_preset_all_days();
    void on_validate_and_accept();

private:
    void setup_ui(const WorkCalendar &cal);

    QCheckBox *chk_days_[7] = {nullptr};
    QDoubleSpinBox *spin_hours_ = nullptr;
};

} // namespace source::gantt
