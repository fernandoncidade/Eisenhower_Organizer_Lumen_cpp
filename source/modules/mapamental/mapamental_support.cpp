#include "source/modules/mapamental/mapamental_support.hpp"

namespace source::plugin::MapaMental::compat {
namespace {
source::plugin::MapaMentalHostBridge *g_host_bridge = nullptr;
}

void set_host_bridge(source::plugin::MapaMentalHostBridge *host_bridge)
{
    g_host_bridge = host_bridge;
}

source::plugin::MapaMentalHostBridge *host_bridge()
{
    return g_host_bridge;
}

}  // namespace source::plugin::MapaMental::compat
