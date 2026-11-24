#include "source/ui/ui_06_remove_task.hpp"

#include "source/ui/agregador_01_ui.hpp"

namespace source::GerenciamentoUI::ui_06_remove_task {

bool remove_task(source::EisenhowerMatrixApp *app,
                 QListWidgetItem *item,
                 QListWidget *list_widget,
                 bool confirm)
{
    return source::GerenciamentoUI::remove_task(app, item, list_widget, confirm);
}

}  // namespace source::GerenciamentoUI::ui_06_remove_task
