#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_15_clicar_no_para_conectar.hpp"

#include <QPushButton>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_25_criar_relacoes_ia.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"

namespace source::plugin::MapaMental {

void MapaMental::clicar_no_para_conectar(NoConceito *no)
{
    if (no == nullptr) {
        return;
    }

    if (no_origem_ == nullptr) {
        no_origem_ = no;
        no->setSelected(true);
        return;
    }

    if (no_origem_ != no && !conexao_existe(no_origem_, no)) {
        criar_conexao(no_origem_, no);
    }

    no_origem_ = nullptr;
    modo_conexao_ = false;
    if (btn_conectar_ != nullptr) {
        btn_conectar_->setChecked(false);
    }
    atualizar_traducoes();
    salvar_backup_automatico();
}

}  // namespace source::plugin::MapaMental
