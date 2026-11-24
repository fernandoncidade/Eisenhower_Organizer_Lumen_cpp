#include "source/services/MostrarMenuContexto/mmc_01_compute_widget_height.hpp"

#include "source/services/incore_14_EditTaskDialog.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {

int compute_widget_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name)
{
    return source::InterfaceCore::incore_14_EditTaskDialog::compute_widget_height(app, widget, name);
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
