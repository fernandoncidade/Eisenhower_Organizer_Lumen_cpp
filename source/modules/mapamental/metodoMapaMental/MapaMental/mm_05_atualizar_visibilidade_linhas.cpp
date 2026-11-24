#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_05_atualizar_visibilidade_linhas.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

namespace source::plugin::MapaMental::MapaMentalFns {

void atualizar_visibilidade_linhas(MapaMental &self)
{
    self.refresh_counter();
}

}  // namespace source::plugin::MapaMental::MapaMentalFns
