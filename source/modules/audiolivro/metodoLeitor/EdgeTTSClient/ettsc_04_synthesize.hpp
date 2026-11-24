#pragma once

#include "source/modules/audiolivro/metodoLeitor/lt_07_EdgeTTSClient.hpp"

namespace source::plugin::Audiolivro::EdgeTTSClientFns {

EdgeTTSResult synthesize(const QString &text, const QString &voice_id, int rate_pct, int volume_pct, int timeout_ms = 180000);

}  // namespace source::plugin::Audiolivro::EdgeTTSClientFns
