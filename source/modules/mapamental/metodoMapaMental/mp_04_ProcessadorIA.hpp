#pragma once

#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace source::plugin::MapaMental {

class ProcessadorIA {
public:
    QString detectar_idioma_texto(const QString &texto) const;
    QString extrair_texto(const QString &caminho) const;
    QVariantMap analisar_estrutura(const QString &texto, bool usar_modelo = true) const;
    QVariantList extrair_conceitos_tfidf(const QString &texto, int max_conceitos = 15) const;
    QVariantList identificar_relacoes_avancadas(const QVariantList &conceitos) const;

private:
    QStringList preprocessar_linhas(const QString &texto) const;
    int inferir_nivel_titulo(const QString &linha) const;
};

}  // namespace source::plugin::MapaMental
