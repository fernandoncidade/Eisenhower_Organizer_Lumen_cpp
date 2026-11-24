#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_09_habilitar_navegacao_hierarquia.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

namespace source::plugin::MapaMental::MapaMentalFns {

void habilitar_navegacao_hierarquia(MapaMental &self, NoConceito *raiz_no)
{
    Q_UNUSED(raiz_no);
    self.refresh_counter();
}

}  // namespace source::plugin::MapaMental::MapaMentalFns
