#include "source/ui/ui_05_move_item_between_lists.hpp"

#include "source/ui/agregador_01_ui.hpp"

namespace source::GerenciamentoUI::ui_05_move_item_between_lists {

void move_item_between_lists(source::EisenhowerMatrixApp *app,
                             QListWidgetItem *item,
                             QListWidget *source,
                             QListWidget *target,
                             Qt::CheckState new_check_state)
{
    source::GerenciamentoUI::move_item_between_lists(app, item, source, target, new_check_state);
}

}  // namespace source::GerenciamentoUI::ui_05_move_item_between_lists
