#pragma once

#include <memory>

#include <QObject>
#include <QPointer>

#include "source/modules/audiolivro/audiolivro_host_bridge.hpp"
#include "source/modules/audiolivro/audiolivro_plugin_interface.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_03_LeitorAcessivel.hpp"
#include "source/modules/audiolivro/metodoLeitor/language/tr_01_gerenciadorTraducao.hpp"

namespace source::plugin {

class AudiolivroPlugin final : public QObject, public IAudiolivroPlugin {
    Q_OBJECT
    Q_INTERFACES(source::plugin::IAudiolivroPlugin)
#if defined(EISENHOWER_BUILD_AUDIOLIVRO_PLUGIN_DLL)
    Q_PLUGIN_METADATA(IID SOURCE_AUDIOLIVRO_PLUGIN_INTERFACE_IID FILE "audiolivro_plugin.json")
#endif

public:
    QString plugin_id() const override;
    QString display_name() const override;
    QString open_menu_text() const override;
    QString font_settings_menu_text() const override;
    void initialize(AudiolivroHostBridge *host_bridge) override;
    QWidget *create_widget(QWidget *parent = nullptr) override;
    void open_font_settings(QWidget *parent = nullptr) override;
    void retranslate_ui() override;
    void reload_shared_state() override;
    void refresh_task_views() override;
    QVariantList voice_options() const override;
    QString current_voice_id() const override;
    void set_voice_id(const QString &voice_id) override;

private:
    void aplicar_traducao_atual();
    QString stored_voice_id() const;

    AudiolivroHostBridge *host_bridge_ = nullptr;
    QPointer<source::plugin::Audiolivro::LeitorAcessivel> widget_;
    std::unique_ptr<source::plugin::Audiolivro::language::GerenciadorTraducao> translation_manager_;
    QString pending_voice_id_;
};

}  // namespace source::plugin
