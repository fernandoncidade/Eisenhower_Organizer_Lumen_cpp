#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_05_corrigir_palavras_conhecidas.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QString corrigir_palavras_conhecidas(const QString &linha)
{
    return linha.simplified();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
