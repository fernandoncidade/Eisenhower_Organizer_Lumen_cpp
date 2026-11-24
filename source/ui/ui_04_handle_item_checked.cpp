#include "source/ui/ui_04_handle_item_checked.hpp"

#include "source/ui/agregador_01_ui.hpp"

namespace source::GerenciamentoUI::ui_04_handle_item_checked {

void handle_item_checked(source::EisenhowerMatrixApp *app,
                         QListWidgetItem *item,
                         QListWidget *source_list,
                         QListWidget *target_list)
{
    source::GerenciamentoUI::handle_item_checked(app, item, source_list, target_list);
}

}  // namespace source::GerenciamentoUI::ui_04_handle_item_checked
