#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_26_identificar_relacoes_semanticas.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QVariantList identificar_relacoes_semanticas(const QVariantList &conceitos)
{
    source::plugin::MapaMental::ProcessadorIA processador;
    return processador.identificar_relacoes_avancadas(conceitos);
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
