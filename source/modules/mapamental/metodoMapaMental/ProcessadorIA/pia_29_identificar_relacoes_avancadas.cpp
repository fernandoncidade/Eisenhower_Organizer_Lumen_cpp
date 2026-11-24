#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_29_identificar_relacoes_avancadas.hpp"

namespace source::plugin::MapaMental {

QVariantList ProcessadorIA::identificar_relacoes_avancadas(const QVariantList &conceitos) const
{
    QVariantList relacoes;
    for (int i = 1; i < conceitos.size(); ++i) {
        QVariantMap relation;
        relation.insert(QStringLiteral("origem"), conceitos.at(i - 1).toMap().value(QStringLiteral("texto")).toString());
        relation.insert(QStringLiteral("destino"), conceitos.at(i).toMap().value(QStringLiteral("texto")).toString());
        relation.insert(QStringLiteral("tipo"), QStringLiteral("associado"));
        relacoes.append(relation);
    }
    return relacoes;
}

}  // namespace source::plugin::MapaMental
