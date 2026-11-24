#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_18_regua_fechada.hpp"

#include <QPushButton>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::regua_fechada()
{
    if (btn_regua_ != nullptr) {
        btn_regua_->setChecked(false);
        update_regua_foco_button();
    }
}

}  // namespace source::plugin::Audiolivro
