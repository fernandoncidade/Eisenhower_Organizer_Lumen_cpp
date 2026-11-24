#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_26_reorganizar_com_ia.hpp"

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_29_aplicar_layout_arvore.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"

namespace source::plugin::MapaMental {

void MapaMental::reorganizar_com_ia()
{
    aplicar_layout_arvore();
    salvar_backup_automatico();
}

}  // namespace source::plugin::MapaMental
