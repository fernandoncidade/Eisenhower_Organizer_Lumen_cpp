#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_17_desativar_regua_foco.hpp"

#include "source/modules/audiolivro/metodoLeitor/Foco/agregador_01_Foco.hpp"

#include <QPushButton>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::desativar_regua_foco()
{
    if (regua_ != nullptr) {
        regua_->hide();
    }
    if (btn_regua_ != nullptr) {
        btn_regua_->setChecked(false);
        update_regua_foco_button();
    }
}

}  // namespace source::plugin::Audiolivro
