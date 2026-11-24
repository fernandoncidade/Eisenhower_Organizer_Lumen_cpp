#pragma once

#include <QObject>

#include "source/modules/modelo/modelo_plugin_interface.hpp"

namespace source::plugin {

class ModeloPlugin final : public QObject, public IModeloPlugin {
    Q_OBJECT
    Q_INTERFACES(source::plugin::IModeloPlugin)
#if defined(EISENHOWER_BUILD_MODELO_PLUGIN_DLL)
    Q_PLUGIN_METADATA(IID SOURCE_MODELO_PLUGIN_INTERFACE_IID FILE "modelo_plugin.json")
#endif

public:
    QString plugin_id() const override;
    QString display_name() const override;
    QVariantMap capabilities() const override;
    QVariantMap diagnostic() const override;
    bool is_available() const override;
    QVariantMap analyze_document(const QString &text,
                                 const QString &language,
                                 const QVariantList &concepts,
                                 const QVariantList &relations) override;

    QVariantList available_models() const override;
    QString active_model() const override;
    void set_active_model(const QString &model_key) override;
    void refresh_models() override;

    QVariantMap ollama_diagnostic() const override;
    QString gguf_models_dir() const override;
    QString gguf_model_path() const override;
    QString gguf_download_url() const override;
    bool is_gguf_model_installed() const override;
    qint64 gguf_model_size_bytes() const override;
    bool remove_gguf_model() override;

private:
    mutable QVariantMap diagnostic_cache_;
    mutable bool diagnostic_cached_ = false;
    QString active_model_ = QStringLiteral("qwen");
};

}  // namespace source::plugin
