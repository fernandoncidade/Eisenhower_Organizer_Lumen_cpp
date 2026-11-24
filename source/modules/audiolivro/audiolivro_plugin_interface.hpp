#pragma once

#include <QString>
#include <QtPlugin>
#include <QVariantList>

class QWidget;

namespace source::plugin {

class AudiolivroHostBridge;

class IAudiolivroPlugin {
public:
    virtual ~IAudiolivroPlugin() = default;

    virtual QString plugin_id() const = 0;
    virtual QString display_name() const = 0;
    virtual QString open_menu_text() const = 0;
    virtual QString font_settings_menu_text() const = 0;
    virtual void initialize(AudiolivroHostBridge *host_bridge) = 0;
    virtual QWidget *create_widget(QWidget *parent = nullptr) = 0;
    virtual void open_font_settings(QWidget *parent = nullptr) = 0;
    virtual void retranslate_ui() = 0;
    virtual void reload_shared_state() = 0;
    virtual void refresh_task_views() = 0;
    virtual QVariantList voice_options() const = 0;
    virtual QString current_voice_id() const = 0;
    virtual void set_voice_id(const QString &voice_id) = 0;
};

}  // namespace source::plugin

#define SOURCE_AUDIOLIVRO_PLUGIN_INTERFACE_IID "source.plugin.IAudiolivroPlugin/2026.11.22.0"

Q_DECLARE_INTERFACE(source::plugin::IAudiolivroPlugin, SOURCE_AUDIOLIVRO_PLUGIN_INTERFACE_IID)
