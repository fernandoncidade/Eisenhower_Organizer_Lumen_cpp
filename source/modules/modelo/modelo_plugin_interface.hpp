#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QtPlugin>

namespace source::plugin {

class IModeloPlugin {
public:
    virtual ~IModeloPlugin() = default;

    virtual QString plugin_id() const = 0;
    virtual QString display_name() const = 0;
    virtual QVariantMap capabilities() const = 0;
    virtual QVariantMap diagnostic() const = 0;
    virtual bool is_available() const = 0;
    virtual QVariantMap analyze_document(const QString &text,
                                         const QString &language,
                                         const QVariantList &concepts,
                                         const QVariantList &relations) = 0;

    virtual QVariantList available_models() const { return {}; }
    virtual QString active_model() const { return QStringLiteral("qwen"); }
    virtual void set_active_model(const QString &model_key) { Q_UNUSED(model_key); }
    virtual void refresh_models() {}

    virtual QVariantMap ollama_diagnostic() const { return {}; }
    virtual QString gguf_models_dir() const { return {}; }
    virtual QString gguf_model_path() const { return {}; }
    virtual QString gguf_download_url() const { return {}; }
    virtual bool is_gguf_model_installed() const { return false; }
    virtual qint64 gguf_model_size_bytes() const { return 0; }
    virtual bool remove_gguf_model() { return false; }
};

}  // namespace source::plugin

#define SOURCE_MODELO_PLUGIN_INTERFACE_IID "source.plugin.IModeloPlugin/2026.11.22.0"

Q_DECLARE_INTERFACE(source::plugin::IModeloPlugin, SOURCE_MODELO_PLUGIN_INTERFACE_IID)
