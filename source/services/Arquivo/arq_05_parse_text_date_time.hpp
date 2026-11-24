#pragma once

#include "source/services/Arquivo/agregador_01_Arquivo.hpp"

namespace source::InterfaceCore {
class Arquivo;
}

namespace source::InterfaceCore::ArquivoModules {

ParsedTextDateTime parse_text_date_time(Arquivo *self, const QString &raw_text);

}  // namespace source::InterfaceCore::ArquivoModules
