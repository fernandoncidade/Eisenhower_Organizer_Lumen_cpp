#pragma once

class QListWidget;
class QListWidgetItem;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI::ui_06_remove_task {

bool remove_task(source::EisenhowerMatrixApp *app,
                 QListWidgetItem *item,
                 QListWidget *list_widget,
                 bool confirm = true);

}  // namespace source::GerenciamentoUI::ui_06_remove_task
