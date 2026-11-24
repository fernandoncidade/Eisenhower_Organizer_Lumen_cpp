#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_18_e_titulo_isolado.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

bool e_titulo_isolado(const QString &linha)
{
    return linha.trimmed().size() <= 72 && !linha.contains(QLatin1Char('.'));
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
