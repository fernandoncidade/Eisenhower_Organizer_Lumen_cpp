#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QtPlugin>

class QWidget;

namespace source::plugin {

class MapaMentalHostBridge;

class IMapaMentalPlugin {
public:
    virtual ~IMapaMentalPlugin() = default;

    virtual QString plugin_id() const = 0;
    virtual QString display_name() const = 0;
    virtual QString open_menu_text() const = 0;
    virtual void initialize(MapaMentalHostBridge *host_bridge) = 0;
    virtual QWidget *create_widget(QWidget *parent = nullptr) = 0;
    virtual void retranslate_ui() = 0;
    virtual void reload_shared_state() = 0;
    virtual void refresh_task_views() = 0;
    virtual QString layout_mode_key() const = 0;
    virtual void set_layout_mode_key(const QString &mode_key) = 0;
    virtual QString summary_method_key() const = 0;
    virtual void set_summary_method_key(const QString &method_key) = 0;
    virtual void reset_task_font_overrides(bool target_linked) { (void)target_linked; }
    virtual QVariantList get_task_font_overrides(bool target_linked) const { (void)target_linked; return {}; }
};

}  // namespace source::plugin

#define SOURCE_MAPAMENTAL_PLUGIN_INTERFACE_IID "source.plugin.IMapaMentalPlugin/2026.11.22.0"

Q_DECLARE_INTERFACE(source::plugin::IMapaMentalPlugin, SOURCE_MAPAMENTAL_PLUGIN_INTERFACE_IID)
