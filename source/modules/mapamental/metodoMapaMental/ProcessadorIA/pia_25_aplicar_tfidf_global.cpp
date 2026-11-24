#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_25_aplicar_tfidf_global.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QVariantList aplicar_tfidf_global(const QString &texto, int max_conceitos)
{
    source::plugin::MapaMental::ProcessadorIA processador;
    return processador.extrair_conceitos_tfidf(texto, max_conceitos);
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
