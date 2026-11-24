#include "source/services/MostrarMenuContexto/mmc_03_effective_locale.hpp"

#include "source/services/incore_14_EditTaskDialog.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {

QLocale effective_locale(source::EisenhowerMatrixApp *app)
{
    return source::InterfaceCore::incore_14_EditTaskDialog::effective_locale(app);
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
