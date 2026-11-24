#pragma once

#include "source/modules/audiolivro/metodoLeitor/lt_07_EdgeTTSClient.hpp"

namespace source::plugin::Audiolivro::EdgeTTSClientFns {

QVariantList available_voice_options(int timeout_ms = 6000);

}  // namespace source::plugin::Audiolivro::EdgeTTSClientFns
