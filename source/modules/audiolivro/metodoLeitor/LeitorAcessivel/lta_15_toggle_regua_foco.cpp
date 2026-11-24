#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_15_toggle_regua_foco.hpp"

#include "source/modules/audiolivro/metodoLeitor/Foco/agregador_01_Foco.hpp"

#include <QPushButton>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::toggle_regua_foco()
{
    if (regua_ != nullptr && regua_->isVisible()) {
        desativar_regua_foco();
    } else {
        ativar_regua_foco();
    }
}

}  // namespace source::plugin::Audiolivro
