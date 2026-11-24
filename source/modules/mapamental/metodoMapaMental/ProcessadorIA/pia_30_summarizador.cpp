#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_30_summarizador.hpp"

#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_22_gerar_resumo_contextual.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QVariantMap gerar_resumo_avancado(const QString &texto)
{
    QVariantMap result;
    result.insert(QStringLiteral("resumo"), gerar_resumo_contextual(texto, 600));
    return result;
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
