#include "source/services/MostrarMenuContexto/mmc_04_apply_locale_to_dateedit.hpp"

#include "source/services/incore_14_EditTaskDialog.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {

void apply_locale_to_dateedit(source::EisenhowerMatrixApp *app, QDateEdit *date_edit)
{
    source::InterfaceCore::incore_14_EditTaskDialog::apply_locale_to_dateedit(app, date_edit);
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
