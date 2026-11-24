#include "source/modules/mapamental/metodoMapaMental/mp_05_ThreadMapaMental.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"

namespace source::plugin::MapaMental {

MapasWorker::MapasWorker(QObject *parent) : QObject(parent) {}

void MapasWorker::process_reorganize(const QStringList &textos,
                                     const QVariantMap &conexoes_existentes,
                                     const QVariantList &visiveis)
{
    ProcessadorIA processador;
    const QString texto = textos.join(QStringLiteral(". "));
    const QVariantList conceitos = processador.extrair_conceitos_tfidf(texto);
    emit reorganize_finished(processador.identificar_relacoes_avancadas(conceitos), conexoes_existentes, visiveis);
}

}  // namespace source::plugin::MapaMental
