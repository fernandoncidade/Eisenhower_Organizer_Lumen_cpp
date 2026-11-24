#pragma once

#include <QString>
#include <QtPlugin>

class QMenu;
class QWidget;

namespace source::plugin {

class AgendaHostBridge;

class IAgendaPlugin {
public:
    virtual ~IAgendaPlugin() = default;

    virtual QString plugin_id() const = 0;
    virtual QString display_name() const = 0;
    virtual QString open_menu_text() const = 0;
    virtual void initialize(AgendaHostBridge *host_bridge) = 0;
    virtual void open_window(QWidget *parent = nullptr) = 0;
    virtual void populate_delete_item_menu(QMenu *menu) = 0;
    virtual void populate_color_menu(QMenu *menu) = 0;
    virtual void retranslate_ui() = 0;
};

}  // namespace source::plugin

#define SOURCE_AGENDA_PLUGIN_INTERFACE_IID "source.plugin.IAgendaPlugin/2026.11.22.0"

Q_DECLARE_INTERFACE(source::plugin::IAgendaPlugin, SOURCE_AGENDA_PLUGIN_INTERFACE_IID)
