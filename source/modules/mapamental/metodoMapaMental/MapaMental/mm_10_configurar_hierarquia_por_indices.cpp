#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_10_configurar_hierarquia_por_indices.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

namespace source::plugin::MapaMental::MapaMentalFns {

void configurar_hierarquia_por_indices(MapaMental &self, const QVariantMap &hierarquia)
{
    Q_UNUSED(hierarquia);
    self.refresh_counter();
}

}  // namespace source::plugin::MapaMental::MapaMentalFns
