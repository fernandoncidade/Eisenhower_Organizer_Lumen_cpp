#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_15_detectar_secoes_avancado.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QVariantList detectar_secoes_avancado(const QString &texto)
{
    source::plugin::MapaMental::ProcessadorIA processador;
    return processador.analisar_estrutura(texto).value(QStringLiteral("filhos")).toList();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
