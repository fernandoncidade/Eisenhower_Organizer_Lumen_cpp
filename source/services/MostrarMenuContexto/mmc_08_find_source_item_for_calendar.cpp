#include "source/services/MostrarMenuContexto/mmc_08_find_source_item_for_calendar.hpp"

#include "source/ui/agregador_01_ui.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {

QPair<QListWidget *, QListWidgetItem *> find_source_item_for_calendar(source::EisenhowerMatrixApp *app,
                                                                      const QVariantMap &task_dict)
{
    return source::GerenciamentoUI::task_utils::find_source_item_for_calendar(app, task_dict);
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
