#pragma once

#include <QList>
#include <QObject>
#include <QPointer>

#include "source/modules/feynman/feynman_host_bridge.hpp"
#include "source/modules/feynman/feynman_plugin_interface.hpp"
#include "source/modules/feynman/mod_05_metodo_feynman.hpp"

namespace source::plugin {

namespace Feynman::language {
class GerenciadorTraducao;
}

class FeynmanPlugin final : public QObject, public IFeynmanPlugin {
    Q_OBJECT
    Q_INTERFACES(source::plugin::IFeynmanPlugin)
#if defined(EISENHOWER_BUILD_FEYNMAN_PLUGIN_DLL)
    Q_PLUGIN_METADATA(IID SOURCE_FEYNMAN_PLUGIN_INTERFACE_IID FILE "feynman_plugin.json")
#endif

public:
    QString plugin_id() const override;
    QString display_name() const override;
    QString open_menu_text() const override;
    void initialize(FeynmanHostBridge *host_bridge) override;
    QWidget *create_widget(QWidget *parent = nullptr) override;
    void retranslate_ui() override;
    void receive_mindmap_concept(const QVariantMap &mindmap_concept) override;
    void receive_task(const QVariantMap &task) override;
    void reload_shared_state() override;
    void refresh_task_views() override;
    int selected_task_count() const override;
    QString selected_task_color() const override;
    int apply_color_to_selected_tasks(const QString &color) override;
    void reset_task_font_overrides(bool target_linked) override;
    QVariantList get_task_font_overrides(bool target_linked) const override;
    void on_task_duration_changed(const QString &task_id, double duration_days) override;
    void reset_in_progress_task_cycles(int new_cycles, const QVariantList &affected_tasks) override;

private:
    FeynmanHostBridge *host_bridge_ = nullptr;
    source::plugin::Feynman::language::GerenciadorTraducao *translation_manager_ = nullptr;
    QPointer<source::plugin::Feynman::MetodoFeynman> widget_;
    QList<QVariantMap> pending_concepts_;
    QList<QVariantMap> pending_tasks_;
};

}  // namespace source::plugin
