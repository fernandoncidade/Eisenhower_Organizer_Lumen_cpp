#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "../domain/models/Schedule.hpp"

namespace source::gantt {

class TaskLinkDialog : public QDialog {
    Q_OBJECT

public:
    TaskLinkDialog(const QString &eisenhower_task_id,
                   const QString &eisenhower_task_title,
                   const Schedule &schedule,
                   QWidget *parent = nullptr);

    QString selected_activity_id() const;

private:
    QString task_id_;
    QString task_title_;
    Schedule schedule_;

    QComboBox *combo_activities_ = nullptr;
    QPushButton *btn_link_ = nullptr;
    QPushButton *btn_cancel_ = nullptr;

    void setup_ui();
};

} // namespace source::gantt
