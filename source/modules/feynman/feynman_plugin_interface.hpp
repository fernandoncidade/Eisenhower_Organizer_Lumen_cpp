#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QtPlugin>

class QWidget;

namespace source::plugin {

class FeynmanHostBridge;

class IFeynmanPlugin {
public:
    virtual ~IFeynmanPlugin() = default;

    virtual QString plugin_id() const = 0;
    virtual QString display_name() const = 0;
    virtual QString open_menu_text() const = 0;
    virtual void initialize(FeynmanHostBridge *host_bridge) = 0;
    virtual QWidget *create_widget(QWidget *parent = nullptr) = 0;
    virtual void retranslate_ui() = 0;
    virtual void receive_mindmap_concept(const QVariantMap &mindmap_concept) = 0;
    virtual void receive_task(const QVariantMap &task) = 0;
    virtual void reload_shared_state() = 0;
    virtual void refresh_task_views() = 0;
    virtual int selected_task_count() const = 0;
    virtual QString selected_task_color() const = 0;
    virtual int apply_color_to_selected_tasks(const QString &color) = 0;
    virtual void reset_task_font_overrides(bool target_linked) { (void)target_linked; }
    virtual QVariantList get_task_font_overrides(bool target_linked) const { (void)target_linked; return {}; }
    virtual void on_task_duration_changed(const QString &task_id, double duration_days) { (void)task_id; (void)duration_days; }
    virtual void reset_in_progress_task_cycles(int new_cycles, const QVariantList &affected_tasks) { (void)new_cycles; (void)affected_tasks; }
};

}  // namespace source::plugin

#define SOURCE_FEYNMAN_PLUGIN_INTERFACE_IID "source.plugin.IFeynmanPlugin/2026.11.22.0"

Q_DECLARE_INTERFACE(source::plugin::IFeynmanPlugin, SOURCE_FEYNMAN_PLUGIN_INTERFACE_IID)
