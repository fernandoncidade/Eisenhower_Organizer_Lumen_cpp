#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_21_processar_hierarquia_completa.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QVariantMap processar_hierarquia_completa(const QString &texto)
{
    source::plugin::MapaMental::ProcessadorIA processador;
    return processador.analisar_estrutura(texto);
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
