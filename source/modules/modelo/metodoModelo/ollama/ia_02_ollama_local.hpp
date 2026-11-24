#pragma once

#include <QList>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace source::plugin::Modelo::ProcessadorIAFns {

struct OllamaModelInfo {
    QString name;          // e.g. "llama3.2:3b"
    QString displayName;   // e.g. "llama3.2:3b (2.0 GB)"
    QString digest;        // e.g. "sha256-dde5aa3fc5ff..."
    QString blobPath;      // Caminho absoluto para o arquivo blob GGUF
    qint64 sizeBytes = 0;
    QString family;        // e.g. "llama", "qwen2", "gemma"
    QString parameterSize; // e.g. "3.2B", "7B"
    bool isBlobGguf = false;
};

// Descobre todos os modelos instalados no Ollama, combinando a API REST (caso ativa)
// com a varredura direta dos manifests em disco (~/.ollama/models).
QList<OllamaModelInfo> descobrir_modelos_ollama(bool forcar_recarga = false);

// Retorna as informações de um modelo específico pelo nome (ex: "llama3.2:3b").
OllamaModelInfo obter_info_modelo_ollama(const QString &model_name);

// Testa se a API REST do Ollama (localhost:11434) está respondendo.
bool testar_conexao_ollama_api(int timeout_ms = 1500);

// Gera relatório de diagnóstico do Ollama (status da API, modelos instalados, caminhos).
QVariantMap diagnosticar_ollama();

// Analisa o documento e gera a árvore de tópicos hierárquica usando o modelo Ollama indicado.
// Tenta primeiro a API REST do Ollama (aceleração GPU e JSON Schema).
// Caso o serviço esteja desativado, utiliza diretamente o blob GGUF via runtime local llama-cli.
QVariantMap analisar_estrutura_com_ollama(const QString &model_name,
                                         const QString &texto,
                                         const QString &idioma,
                                         const QVariantList &conceitos,
                                         const QVariantList &relacoes);

}  // namespace source::plugin::Modelo::ProcessadorIAFns
