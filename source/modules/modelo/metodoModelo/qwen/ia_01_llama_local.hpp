#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace source::plugin::Modelo::ProcessadorIAFns {

QVariantMap diagnosticar_llama_local();

QVariantMap analisar_estrutura_com_llama(const QString &texto,
                                         const QString &idioma,
                                         const QVariantList &conceitos,
                                         const QVariantList &relacoes,
                                         const QString &custom_model_path = QString());

// Helpers para gerenciamento e download sob demanda do modelo GGUF local
QString diretorio_modelos_persistente();
QString caminho_destino_modelo_gguf();
QString nome_arquivo_modelo_gguf();
QString url_download_modelo_gguf();
qint64 tamanho_esperado_modelo_gguf();
bool modelo_gguf_instalado();
qint64 tamanho_modelo_gguf_instalado();
bool remover_modelo_gguf_instalado();

}  // namespace source::plugin::Modelo::ProcessadorIAFns
