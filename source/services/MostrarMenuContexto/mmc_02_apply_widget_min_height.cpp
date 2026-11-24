#include "source/services/MostrarMenuContexto/mmc_02_apply_widget_min_height.hpp"

#include "source/services/incore_14_EditTaskDialog.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {

void apply_widget_min_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name)
{
    source::InterfaceCore::incore_14_EditTaskDialog::apply_widget_min_height(app, widget, name);
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
