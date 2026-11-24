#pragma once

namespace source::plugin::MapaMental {
class MapaMental;
class NoConceito;
namespace MapaMentalFns {
void aplicar_visibilidade_por_foco(MapaMental &self, NoConceito *foco, bool abrir_foco);
}
}  // namespace source::plugin::MapaMental
