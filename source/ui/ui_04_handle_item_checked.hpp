#pragma once

class QListWidget;
class QListWidgetItem;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI::ui_04_handle_item_checked {

void handle_item_checked(source::EisenhowerMatrixApp *app,
                         QListWidgetItem *item,
                         QListWidget *source_list,
                         QListWidget *target_list);

}  // namespace source::GerenciamentoUI::ui_04_handle_item_checked
