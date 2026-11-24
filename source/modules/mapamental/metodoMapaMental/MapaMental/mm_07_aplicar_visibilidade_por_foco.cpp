#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_07_aplicar_visibilidade_por_foco.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

namespace source::plugin::MapaMental::MapaMentalFns {

void aplicar_visibilidade_por_foco(MapaMental &self, NoConceito *foco, bool abrir_foco)
{
    Q_UNUSED(foco);
    Q_UNUSED(abrir_foco);
    self.refresh_counter();
}

}  // namespace source::plugin::MapaMental::MapaMentalFns
