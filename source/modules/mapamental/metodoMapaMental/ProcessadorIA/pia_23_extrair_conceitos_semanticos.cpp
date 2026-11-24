#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_23_extrair_conceitos_semanticos.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QVariantList extrair_conceitos_semanticos(const QString &texto)
{
    source::plugin::MapaMental::ProcessadorIA processador;
    return processador.extrair_conceitos_tfidf(texto);
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
