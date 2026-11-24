#pragma once

#include <memory>

#include <QObject>
#include <QPointer>

#include "source/modules/agenda/agenda_host_bridge.hpp"
#include "source/modules/agenda/agenda_plugin_interface.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoInterfaceGrafica/Interface_Grafica.hpp"

namespace source::plugin {

class AgendaPlugin final : public QObject, public IAgendaPlugin {
    Q_OBJECT
    Q_INTERFACES(source::plugin::IAgendaPlugin)
#if defined(EISENHOWER_BUILD_AGENDA_PLUGIN_DLL)
    Q_PLUGIN_METADATA(IID SOURCE_AGENDA_PLUGIN_INTERFACE_IID FILE "agenda_plugin.json")
#endif

public:
    QString plugin_id() const override;
    QString display_name() const override;
    QString open_menu_text() const override;
    void initialize(AgendaHostBridge *host_bridge) override;
    void open_window(QWidget *parent = nullptr) override;
    void populate_delete_item_menu(QMenu *menu) override;
    void populate_color_menu(QMenu *menu) override;
    void retranslate_ui() override;

private:
    InterfaceGerenciadorAtividades *ensure_window(QWidget *parent = nullptr);
    void apply_host_language();

    AgendaHostBridge *host_bridge_ = nullptr;
    QMetaObject::Connection host_destroyed_connection_;
    QPointer<InterfaceGerenciadorAtividades> window_;
};

}  // namespace source::plugin
