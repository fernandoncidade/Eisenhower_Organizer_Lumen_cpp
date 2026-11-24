#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_14_toggle_modo_conexao.hpp"

#include <QPushButton>

namespace source::plugin::MapaMental {

void MapaMental::toggle_modo_conexao()
{
    modo_conexao_ = !modo_conexao_;
    no_origem_ = nullptr;
    if (btn_conectar_ != nullptr) {
        btn_conectar_->setChecked(modo_conexao_);
    }
    atualizar_traducoes();
}

}  // namespace source::plugin::MapaMental
