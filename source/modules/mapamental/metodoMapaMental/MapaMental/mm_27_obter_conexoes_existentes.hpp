#pragma once

#include <QVariantMap>

namespace source::plugin::MapaMental {
class MapaMental;
namespace MapaMentalFns {
QVariantMap obter_conexoes_existentes(const MapaMental &self);
}
}  // namespace source::plugin::MapaMental
