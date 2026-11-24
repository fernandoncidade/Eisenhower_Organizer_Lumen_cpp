#include "source/modules/pomodoro/pomodoro_plugin.hpp"

#include <QCoreApplication>
#include <QLocale>
#include <QWidget>

#include "source/modules/pomodoro/Pomodoro/Alarme/sound_01_config_dialog.hpp"
#include "source/modules/pomodoro/Pomodoro/language/tr_01_gerenciadorTraducao.hpp"
#include "source/modules/pomodoro/Pomodoro/pomodoro_support.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_02_PomodoroTimer.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_03_GerenciadorTarefas.hpp"

namespace source::plugin {
namespace {

QString effective_language_code(source::plugin::PomodoroHostBridge *host_bridge)
{
    if (host_bridge == nullptr) {
        return QLocale::system().name();
    }

    const QString code = host_bridge->language_code().trimmed();
    return code.isEmpty() ? QLocale::system().name() : code;
}

}  // namespace

QString PomodoroPlugin::plugin_id() const
{
    return QStringLiteral("pomodoro");
}

QString PomodoroPlugin::display_name() const
{
    return QCoreApplication::translate("PomodoroPlugin", "Pomodoro");
}

QString PomodoroPlugin::open_menu_text() const
{
    return QCoreApplication::translate("PomodoroPlugin", "Abrir Pomodoro");
}

QString PomodoroPlugin::alarm_menu_text() const
{
    return QCoreApplication::translate("PomodoroPlugin", "Alarme");
}

QString PomodoroPlugin::automatic_mode_menu_text() const
{
    return QCoreApplication::translate("PomodoroPlugin", "Modo automatico");
}

void PomodoroPlugin::initialize(PomodoroHostBridge *host_bridge)
{
    host_bridge_ = host_bridge;
    if (translation_manager_ == nullptr) {
        translation_manager_ = new source::plugin::Pomodoro::language::GerenciadorTraducao(this);
    }
    translation_manager_->aplicar_traducao(effective_language_code(host_bridge_));
    source::plugin::Pomodoro::compat::set_host_bridge(host_bridge_);
}

QWidget *PomodoroPlugin::create_widget(QWidget *parent)
{
    if (translation_manager_ != nullptr) {
        translation_manager_->aplicar_traducao(effective_language_code(host_bridge_));
    }

    if (widget_ == nullptr) {
        widget_ = new source::plugin::Pomodoro::GerenciadorTempo(parent);
    } else if (widget_->parentWidget() != parent) {
        widget_->setParent(parent);
    }

    return widget_;
}

void PomodoroPlugin::open_alarm_settings(QWidget *parent)
{
    QWidget *dialog_parent = parent != nullptr ? parent : widget_;
    source::plugin::Pomodoro::Alarme::SoundConfigDialog dialog(dialog_parent);
    dialog.exec();
}

void PomodoroPlugin::open_automatic_mode_settings(QWidget *parent)
{
    QWidget *dialog_parent = parent != nullptr ? parent : widget_;
    source::plugin::Pomodoro::Alarme::AutomaticModeConfigDialog dialog(dialog_parent);
    dialog.exec();
}

void PomodoroPlugin::retranslate_ui()
{
    if (translation_manager_ != nullptr) {
        translation_manager_->aplicar_traducao(effective_language_code(host_bridge_));
    }

    if (widget_ != nullptr) {
        widget_->atualizar_traducoes();
        widget_->update();
    }
}

void PomodoroPlugin::receive_eisenhower_task(const QVariantMap &task)
{
    QVariantMap payload = task;
    if (payload.value(QStringLiteral("origin")).toString().trimmed().isEmpty()) {
        payload.insert(QStringLiteral("origin"), QStringLiteral("eisenhower"));
    }

    source::plugin::Pomodoro::compat::get_event_bus()->receive_tarefa_from_host(payload);
}

void PomodoroPlugin::reload_shared_state()
{
    if (widget_ == nullptr || widget_->tarefas == nullptr) {
        return;
    }

    widget_->tarefas->tarefas = widget_->tarefas->carregar_tarefas();
    widget_->tarefas->atualizar_listas();
    widget_->atualizar_coluna_doing_timer();
}

void PomodoroPlugin::refresh_task_views()
{
    if (widget_ == nullptr || widget_->tarefas == nullptr) {
        return;
    }

    widget_->tarefas->atualizar_listas();
    widget_->atualizar_coluna_doing_timer();
}

int PomodoroPlugin::selected_task_count() const
{
    return widget_ != nullptr ? widget_->selected_task_count() : 0;
}

QString PomodoroPlugin::selected_task_color() const
{
    return widget_ != nullptr ? widget_->selected_task_color() : QString();
}

int PomodoroPlugin::apply_color_to_selected_tasks(const QString &color)
{
    return widget_ != nullptr ? widget_->apply_color_to_selected_tasks(color) : 0;
}

void PomodoroPlugin::reset_task_font_overrides(bool target_linked)
{
    if (widget_ == nullptr || widget_->tarefas == nullptr) {
        return;
    }

    bool changed = false;
    for (source::plugin::Pomodoro::Tarefa &task : widget_->tarefas->tarefas) {
        const bool has_linked = !task.file_path.trimmed().isEmpty();
        if (has_linked == target_linked && !task.font_config.isEmpty()) {
            task.font_config.clear();
            changed = true;
        }
    }

    if (changed) {
        widget_->tarefas->salvar_tarefas();
        widget_->tarefas->atualizar_listas();
        widget_->atualizar_coluna_doing_timer();
    }
}

QVariantList PomodoroPlugin::get_task_font_overrides(bool target_linked) const
{
    if (widget_ == nullptr || widget_->tarefas == nullptr) {
        return {};
    }

    QVariantList res;
    for (const source::plugin::Pomodoro::Tarefa &task : widget_->tarefas->tarefas) {
        const bool has_linked = !task.file_path.trimmed().isEmpty();
        if (has_linked == target_linked) {
            res.append(task.font_config);
        }
    }
    return res;
}

void PomodoroPlugin::on_task_duration_changed(const QString &task_id, double duration_days)
{
    if (widget_ != nullptr && widget_->tarefas != nullptr) {
        widget_->tarefas->on_task_duration_changed(task_id, duration_days);
    }
}

int PomodoroPlugin::default_pomodoro_cycle_count() const
{
    if (widget_ != nullptr && widget_->pomodoro != nullptr) {
        return qMax(1, widget_->pomodoro->ciclos_permitidos);
    }
    return source::plugin::Pomodoro::PomodoroTimer::obter_ciclos_permitidos_persistidos();
}

}  // namespace source::plugin
