#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QtPlugin>

class QWidget;

namespace source::plugin {

class PomodoroHostBridge;

class IPomodoroPlugin {
public:
    virtual ~IPomodoroPlugin() = default;

    virtual QString plugin_id() const = 0;
    virtual QString display_name() const = 0;
    virtual QString open_menu_text() const = 0;
    virtual QString alarm_menu_text() const = 0;
    virtual QString automatic_mode_menu_text() const = 0;
    virtual void initialize(PomodoroHostBridge *host_bridge) = 0;
    virtual QWidget *create_widget(QWidget *parent = nullptr) = 0;
    virtual void open_alarm_settings(QWidget *parent = nullptr) = 0;
    virtual void open_automatic_mode_settings(QWidget *parent = nullptr) = 0;
    virtual void retranslate_ui() = 0;
    virtual void receive_eisenhower_task(const QVariantMap &task) = 0;
    virtual void reload_shared_state() = 0;
    virtual void refresh_task_views() = 0;
    virtual int selected_task_count() const = 0;
    virtual QString selected_task_color() const = 0;
    virtual int apply_color_to_selected_tasks(const QString &color) = 0;
    virtual void reset_task_font_overrides(bool target_linked) { (void)target_linked; }
    virtual QVariantList get_task_font_overrides(bool target_linked) const { (void)target_linked; return {}; }
    virtual void on_task_duration_changed(const QString &task_id, double duration_days) { (void)task_id; (void)duration_days; }
    virtual int default_pomodoro_cycle_count() const { return 4; }
};

}  // namespace source::plugin

#define SOURCE_POMODORO_PLUGIN_INTERFACE_IID "source.plugin.IPomodoroPlugin/2026.11.22.0"

Q_DECLARE_INTERFACE(source::plugin::IPomodoroPlugin, SOURCE_POMODORO_PLUGIN_INTERFACE_IID)
