#pragma once

#include <memory>

#include <QObject>
#include <QPointer>

#include "source/modules/mapamental/mapamental_host_bridge.hpp"
#include "source/modules/mapamental/mapamental_plugin_interface.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"
#include "source/modules/mapamental/metodoMapaMental/language/tr_01_gerenciadorTraducao.hpp"

namespace source::plugin {

class MapaMentalPlugin final : public QObject, public IMapaMentalPlugin {
    Q_OBJECT
    Q_INTERFACES(source::plugin::IMapaMentalPlugin)
#if defined(EISENHOWER_BUILD_MAPAMENTAL_PLUGIN_DLL)
    Q_PLUGIN_METADATA(IID SOURCE_MAPAMENTAL_PLUGIN_INTERFACE_IID FILE "mapamental_plugin.json")
#endif

public:
    QString plugin_id() const override;
    QString display_name() const override;
    QString open_menu_text() const override;
    void initialize(MapaMentalHostBridge *host_bridge) override;
    QWidget *create_widget(QWidget *parent = nullptr) override;
    void retranslate_ui() override;
    void reload_shared_state() override;
    void refresh_task_views() override;
    QString layout_mode_key() const override;
    void set_layout_mode_key(const QString &mode_key) override;
    QString summary_method_key() const override;
    void set_summary_method_key(const QString &method_key) override;
    void reset_task_font_overrides(bool target_linked) override;
    QVariantList get_task_font_overrides(bool target_linked) const override;

private:
    void aplicar_traducao_atual();

    MapaMentalHostBridge *host_bridge_ = nullptr;
    QPointer<source::plugin::MapaMental::MapaMental> widget_;
    std::unique_ptr<source::plugin::MapaMental::language::GerenciadorTraducao> translation_manager_;
    QString pending_layout_mode_key_ = QStringLiteral("vertical");
    bool has_pending_layout_mode_ = false;
    QString pending_summary_method_key_ = QStringLiteral("qwen");
    bool has_pending_summary_method_ = false;
};

}  // namespace source::plugin
