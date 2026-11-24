#pragma once

#include <QDialog>
#include <QPair>

class QListWidget;
class QListWidgetItem;
class QCheckBox;
class QDateEdit;
class QTimeEdit;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI {

class EditDateTimeDialog : public QDialog {
public:
    EditDateTimeDialog(source::EisenhowerMatrixApp *app,
                       const QString &current_date_iso,
                       const QString &current_time_str,
                       QWidget *parent = nullptr);

    QPair<QString, QString> values() const;

private:
    source::EisenhowerMatrixApp *app_ = nullptr;
    QCheckBox *date_enabled_ = nullptr;
    QDateEdit *date_edit_ = nullptr;
    QCheckBox *time_enabled_ = nullptr;
    QTimeEdit *time_edit_ = nullptr;
};

}  // namespace source::GerenciamentoUI

namespace source::GerenciamentoUI::ui_15_edit_task {

void edit_task_datetime(source::EisenhowerMatrixApp *app, QListWidgetItem *item, QListWidget *list_widget);
void move_task_to_quadrant(source::EisenhowerMatrixApp *app,
                           QListWidgetItem *item,
                           QListWidget *source_list,
                           QListWidget *target_list);

}  // namespace source::GerenciamentoUI::ui_15_edit_task
