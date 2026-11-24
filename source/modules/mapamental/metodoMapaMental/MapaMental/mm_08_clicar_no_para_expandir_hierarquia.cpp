#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_08_clicar_no_para_expandir_hierarquia.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

namespace source::plugin::MapaMental::MapaMentalFns {

void clicar_no_para_expandir_hierarquia(MapaMental &self, NoConceito *no)
{
    Q_UNUSED(no);
    self.refresh_counter();
}

}  // namespace source::plugin::MapaMental::MapaMentalFns
