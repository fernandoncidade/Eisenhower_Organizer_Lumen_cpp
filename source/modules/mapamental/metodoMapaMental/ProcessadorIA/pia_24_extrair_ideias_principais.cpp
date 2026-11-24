#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_24_extrair_ideias_principais.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QVariantList extrair_ideias_principais(const QString &texto)
{
    source::plugin::MapaMental::ProcessadorIA processador;
    return processador.extrair_conceitos_tfidf(texto, 8);
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
