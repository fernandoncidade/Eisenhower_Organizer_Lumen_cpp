#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_32_util_texto.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QString normalizar_texto(const QString &texto)
{
    return texto.simplified().trimmed();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
