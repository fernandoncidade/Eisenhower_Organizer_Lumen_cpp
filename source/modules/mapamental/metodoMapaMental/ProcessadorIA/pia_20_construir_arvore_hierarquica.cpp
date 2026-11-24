#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_20_construir_arvore_hierarquica.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QVariantMap construir_arvore_hierarquica(const QString &texto)
{
    source::plugin::MapaMental::ProcessadorIA processador;
    return processador.analisar_estrutura(texto);
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
