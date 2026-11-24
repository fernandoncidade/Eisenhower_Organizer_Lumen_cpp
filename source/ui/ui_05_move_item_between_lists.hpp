#pragma once

#include <Qt>

class QListWidget;
class QListWidgetItem;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI::ui_05_move_item_between_lists {

void move_item_between_lists(source::EisenhowerMatrixApp *app,
                             QListWidgetItem *item,
                             QListWidget *source,
                             QListWidget *target,
                             Qt::CheckState new_check_state);

}  // namespace source::GerenciamentoUI::ui_05_move_item_between_lists
