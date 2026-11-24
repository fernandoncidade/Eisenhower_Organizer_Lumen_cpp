#pragma once

#include <QVariantMap>

namespace source::plugin::MapaMental {
class MapaMental;
namespace MapaMentalFns {
void configurar_hierarquia_por_indices(MapaMental &self, const QVariantMap &hierarquia);
}
}  // namespace source::plugin::MapaMental
