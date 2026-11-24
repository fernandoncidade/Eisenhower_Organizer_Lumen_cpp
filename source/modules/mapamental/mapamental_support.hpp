#pragma once

namespace source::plugin {
class MapaMentalHostBridge;
}

namespace source::plugin::MapaMental::compat {

void set_host_bridge(source::plugin::MapaMentalHostBridge *host_bridge);
source::plugin::MapaMentalHostBridge *host_bridge();

}  // namespace source::plugin::MapaMental::compat
