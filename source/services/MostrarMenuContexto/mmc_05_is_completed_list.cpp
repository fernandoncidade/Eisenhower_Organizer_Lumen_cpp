#include "source/services/MostrarMenuContexto/mmc_05_is_completed_list.hpp"

#include "source/eisenhower_app.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {

bool is_completed_list(source::EisenhowerMatrixApp *app, QListWidget *list)
{
    return app != nullptr && app->is_completed_list(list);
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
