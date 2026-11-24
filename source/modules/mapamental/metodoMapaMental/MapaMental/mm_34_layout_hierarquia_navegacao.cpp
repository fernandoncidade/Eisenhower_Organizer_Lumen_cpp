#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_34_layout_hierarquia_navegacao.hpp"

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_26_reorganizar_com_ia.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

namespace source::plugin::MapaMental::MapaMentalFns {

void layout_hierarquia_navegacao(MapaMental &self)
{
    self.reorganizar_com_ia();
}

}  // namespace source::plugin::MapaMental::MapaMentalFns
