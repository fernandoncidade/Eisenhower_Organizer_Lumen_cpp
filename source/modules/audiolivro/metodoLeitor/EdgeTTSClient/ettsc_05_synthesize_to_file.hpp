#pragma once

#include "source/modules/audiolivro/metodoLeitor/lt_07_EdgeTTSClient.hpp"

namespace source::plugin::Audiolivro::EdgeTTSClientFns {

bool synthesize_to_file(const QString &text, const QString &voice_id, int rate_pct, int volume_pct, const QString &outfile, QString *error = nullptr, QVariantList *timestamps = nullptr, int timeout_ms = 180000);

}  // namespace source::plugin::Audiolivro::EdgeTTSClientFns
