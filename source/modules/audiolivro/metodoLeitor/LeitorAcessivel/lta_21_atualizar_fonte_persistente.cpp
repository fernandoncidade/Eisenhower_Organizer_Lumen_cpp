#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_21_atualizar_fonte_persistente.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"

#include <QComboBox>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::atualizar_fonte_persistente()
{
    if (texto_area_ == nullptr) {
        return;
    }
    salvar_fonte_configurada(texto_area_->font());
}

}  // namespace source::plugin::Audiolivro
