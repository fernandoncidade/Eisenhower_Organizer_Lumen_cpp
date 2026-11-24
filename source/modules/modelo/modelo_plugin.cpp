#include "source/modules/modelo/modelo_plugin.hpp"

#include <QCoreApplication>

#include "source/modules/modelo/metodoModelo/qwen/ia_01_llama_local.hpp"
#include "source/modules/modelo/metodoModelo/ollama/ia_02_ollama_local.hpp"

namespace source::plugin {

QString ModeloPlugin::plugin_id() const
{
    return QStringLiteral("modelo");
}

QString ModeloPlugin::display_name() const
{
    if (active_model_.startsWith(QStringLiteral("ollama:"))) {
        return QStringLiteral("Ollama (%1)").arg(active_model_.mid(7));
    }
    return QCoreApplication::translate("ModeloPlugin", "Modelo Qwen");
}

QVariantMap ModeloPlugin::capabilities() const
{
    return {
        {QStringLiteral("text_analysis"), true},
        {QStringLiteral("mindmap_hierarchy"), true},
        {QStringLiteral("hierarchical_summaries"), true},
        {QStringLiteral("provider"), active_model_.startsWith(QStringLiteral("ollama:")) ? QStringLiteral("ollama") : QStringLiteral("qwen")},
        {QStringLiteral("format"), QStringLiteral("gguf")},
        {QStringLiteral("ollama_support"), true},
    };
}

QVariantMap ModeloPlugin::diagnostic() const
{
    if (diagnostic_cached_) {
        return diagnostic_cache_;
    }

    QVariantMap result = Modelo::ProcessadorIAFns::diagnosticar_llama_local();
    QVariantMap ollama_diag = Modelo::ProcessadorIAFns::diagnosticar_ollama();
    result.insert(QStringLiteral("ollama"), ollama_diag);
    result.insert(QStringLiteral("plugin"), plugin_id());
    result.insert(QStringLiteral("provider"), active_model_.startsWith(QStringLiteral("ollama:")) ? QStringLiteral("ollama") : QStringLiteral("qwen"));
    const bool qwen_avail = result.value(QStringLiteral("modelo_disponivel")).toBool()
                            && result.value(QStringLiteral("runtime_disponivel")).toBool();
    const bool ollama_avail = ollama_diag.value(QStringLiteral("total_modelos")).toInt() > 0;
    result.insert(QStringLiteral("available"), qwen_avail || ollama_avail);
    result.insert(QStringLiteral("qwen_available"), qwen_avail);
    result.insert(QStringLiteral("ollama_available"), ollama_avail);
    diagnostic_cache_ = result;
    diagnostic_cached_ = true;
    return diagnostic_cache_;
}

bool ModeloPlugin::is_available() const
{
    if (active_model_.compare(QStringLiteral("heuristic"), Qt::CaseInsensitive) == 0) {
        return false;
    }
    if (active_model_.startsWith(QStringLiteral("ollama:"), Qt::CaseInsensitive)) {
        const QString m_name = active_model_.mid(7).trimmed();
        if (m_name.isEmpty()) {
            return false;
        }
        const auto info = Modelo::ProcessadorIAFns::obter_info_modelo_ollama(m_name);
        return !info.name.isEmpty();
    }
    const QVariantMap diag = diagnostic();
    return diag.value(QStringLiteral("qwen_available")).toBool();
}

QVariantMap ModeloPlugin::analyze_document(const QString &text,
                                           const QString &language,
                                           const QVariantList &concepts,
                                           const QVariantList &relations)
{
    if (text.trimmed().isEmpty()) {
        return {};
    }

    if (active_model_.compare(QStringLiteral("heuristic"), Qt::CaseInsensitive) == 0) {
        return {};
    }

    if (active_model_.startsWith(QStringLiteral("ollama:"), Qt::CaseInsensitive)) {
        const QString m_name = active_model_.mid(7).trimmed();
        if (m_name.isEmpty()) {
            return {};
        }
        QVariantMap result = Modelo::ProcessadorIAFns::analisar_estrutura_com_ollama(
            m_name, text, language, concepts, relations);
        if (!result.isEmpty()) {
            result.insert(QStringLiteral("motor_ia"), active_model_);
            result.insert(QStringLiteral("plugin_modelo"), true);
        }
        return result;
    }

    // Padrão Qwen local
    QVariantMap result = Modelo::ProcessadorIAFns::analisar_estrutura_com_llama(
        text, language, concepts, relations);
    if (!result.isEmpty()) {
        result.insert(QStringLiteral("motor_ia"), QStringLiteral("modelo.qwen"));
        result.insert(QStringLiteral("plugin_modelo"), true);
    }
    return result;
}

QVariantList ModeloPlugin::available_models() const
{
    QVariantList list;

    // Modelo Qwen local
    QVariantMap qwen_item;
    qwen_item.insert(QStringLiteral("key"), QStringLiteral("qwen"));
    qwen_item.insert(QStringLiteral("display"), QStringLiteral("Qwen 2.5 (3B) [GGUF local]"));
    qwen_item.insert(QStringLiteral("provider"), QStringLiteral("qwen"));
    list.append(qwen_item);

    // Modelos instalados no Ollama
    const QList<Modelo::ProcessadorIAFns::OllamaModelInfo> ollama_models =
        Modelo::ProcessadorIAFns::descobrir_modelos_ollama(false);
    for (const auto &info : ollama_models) {
        QVariantMap item;
        item.insert(QStringLiteral("key"), QStringLiteral("ollama:%1").arg(info.name));
        item.insert(QStringLiteral("name"), info.name);
        item.insert(QStringLiteral("display"), info.displayName);
        item.insert(QStringLiteral("provider"), QStringLiteral("ollama"));
        item.insert(QStringLiteral("size_bytes"), info.sizeBytes);
        list.append(item);
    }
    return list;
}

QString ModeloPlugin::active_model() const
{
    return active_model_;
}

void ModeloPlugin::set_active_model(const QString &model_key)
{
    const QString normalized = model_key.trimmed().isEmpty() ? QStringLiteral("qwen") : model_key.trimmed();
    if (active_model_ != normalized) {
        active_model_ = normalized;
        diagnostic_cached_ = false;
    }
}

void ModeloPlugin::refresh_models()
{
    Modelo::ProcessadorIAFns::descobrir_modelos_ollama(true);
    diagnostic_cached_ = false;
}

QVariantMap ModeloPlugin::ollama_diagnostic() const
{
    return Modelo::ProcessadorIAFns::diagnosticar_ollama();
}

QString ModeloPlugin::gguf_models_dir() const
{
    return Modelo::ProcessadorIAFns::diretorio_modelos_persistente();
}

QString ModeloPlugin::gguf_model_path() const
{
    return Modelo::ProcessadorIAFns::caminho_destino_modelo_gguf();
}

QString ModeloPlugin::gguf_download_url() const
{
    return Modelo::ProcessadorIAFns::url_download_modelo_gguf();
}

bool ModeloPlugin::is_gguf_model_installed() const
{
    return Modelo::ProcessadorIAFns::modelo_gguf_instalado();
}

qint64 ModeloPlugin::gguf_model_size_bytes() const
{
    return Modelo::ProcessadorIAFns::tamanho_modelo_gguf_instalado();
}

bool ModeloPlugin::remove_gguf_model()
{
    diagnostic_cached_ = false;
    return Modelo::ProcessadorIAFns::remover_modelo_gguf_instalado();
}

}  // namespace source::plugin
