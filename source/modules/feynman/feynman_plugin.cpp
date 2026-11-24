#include "source/modules/feynman/feynman_plugin.hpp"

#include <QCoreApplication>
#include <QLocale>
#include <QWidget>

#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"
#include "source/modules/feynman/Feynman/feynman_support.hpp"
#include "source/modules/feynman/Feynman/language/tr_01_gerenciadorTraducao.hpp"
#include "source/modules/feynman/mod_05_metodo_feynman.hpp"

namespace source::plugin {
namespace {

QString effective_language_code(FeynmanHostBridge *host_bridge)
{
    if (host_bridge == nullptr) {
        return QLocale::system().name();
    }

    const QString code = host_bridge->language_code().trimmed();
    return code.isEmpty() ? QLocale::system().name() : code;
}

}  // namespace

QString FeynmanPlugin::plugin_id() const
{
    return QStringLiteral("feynman");
}

QString FeynmanPlugin::display_name() const
{
    return QCoreApplication::translate("FeynmanPlugin", "Feynman");
}

QString FeynmanPlugin::open_menu_text() const
{
    return QCoreApplication::translate("FeynmanPlugin", "Abrir Feynman");
}

void FeynmanPlugin::initialize(FeynmanHostBridge *host_bridge)
{
    host_bridge_ = host_bridge;
    if (translation_manager_ == nullptr) {
        translation_manager_ = new source::plugin::Feynman::language::GerenciadorTraducao(this);
    }
    translation_manager_->aplicar_traducao(effective_language_code(host_bridge_));
    source::plugin::Feynman::compat::set_host_bridge(host_bridge_);
}

QWidget *FeynmanPlugin::create_widget(QWidget *parent)
{
    if (translation_manager_ != nullptr) {
        translation_manager_->aplicar_traducao(effective_language_code(host_bridge_));
    }

    if (widget_ == nullptr) {
        widget_ = new source::plugin::Feynman::MetodoFeynman(parent);
        for (const QVariantMap &pending : pending_concepts_) {
            widget_->receber_conceito_mapa_mental(pending);
        }
        pending_concepts_.clear();
        for (const QVariantMap &pending : pending_tasks_) {
            widget_->receber_tarefa_compartilhada(pending);
        }
        pending_tasks_.clear();
    } else if (widget_->parentWidget() != parent) {
        widget_->setParent(parent);
    }

    widget_->atualizar_traducoes();
    return widget_;
}

void FeynmanPlugin::retranslate_ui()
{
    if (translation_manager_ != nullptr) {
        translation_manager_->aplicar_traducao(effective_language_code(host_bridge_));
    }

    if (widget_ != nullptr) {
        widget_->atualizar_traducoes();
        widget_->update();
    }
}

void FeynmanPlugin::receive_mindmap_concept(const QVariantMap &mindmap_concept)
{
    if (widget_ != nullptr) {
        widget_->receber_conceito_mapa_mental(mindmap_concept);
    } else {
        pending_concepts_.append(mindmap_concept);
    }
}

void FeynmanPlugin::receive_task(const QVariantMap &task)
{
    if (widget_ != nullptr) {
        widget_->receber_tarefa_compartilhada(task);
    } else {
        pending_tasks_.append(task);
    }
}

void FeynmanPlugin::reload_shared_state()
{
    if (widget_ != nullptr) {
        widget_->carregar_conceitos();
    }
}

void FeynmanPlugin::refresh_task_views()
{
    if (widget_ != nullptr) {
        widget_->atualizar_lista();
    }
}

int FeynmanPlugin::selected_task_count() const
{
    return widget_ != nullptr ? widget_->selected_task_count() : 0;
}

QString FeynmanPlugin::selected_task_color() const
{
    return widget_ != nullptr ? widget_->selected_task_color() : QString();
}

int FeynmanPlugin::apply_color_to_selected_tasks(const QString &color)
{
    return widget_ != nullptr ? widget_->apply_color_to_selected_tasks(color) : 0;
}

void FeynmanPlugin::reset_task_font_overrides(bool target_linked)
{
    if (widget_ == nullptr) {
        return;
    }

    bool changed = false;
    for (int i = 0; i < widget_->conceitos_.size(); ++i) {
        QVariantMap conceito = source::plugin::Feynman::normalized_concept(widget_->conceitos_.at(i).toMap());
        const bool has_linked = !conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed().isEmpty();
        if (has_linked == target_linked && conceito.contains(QStringLiteral("font_config"))) {
            conceito.remove(QStringLiteral("font_config"));
            widget_->conceitos_[i] = conceito;
            changed = true;
        }
    }

    if (changed) {
        widget_->salvar_conceitos();
        widget_->atualizar_lista();
    }
}

QVariantList FeynmanPlugin::get_task_font_overrides(bool target_linked) const
{
    if (widget_ == nullptr) {
        return {};
    }

    QVariantList res;
    for (const QVariant &var : widget_->conceitos_) {
        QVariantMap conceito = source::plugin::Feynman::normalized_concept(var.toMap());
        const bool has_linked = !conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed().isEmpty();
        if (has_linked == target_linked) {
            res.append(conceito.value(QStringLiteral("font_config")).toMap());
        }
    }
    return res;
}

void FeynmanPlugin::on_task_duration_changed(const QString &task_id, double duration_days)
{
    if (widget_ != nullptr) {
        widget_->on_task_duration_changed(task_id, duration_days);
    }
}

void FeynmanPlugin::reset_in_progress_task_cycles(int new_cycles, const QVariantList &affected_tasks)
{
    if (widget_ != nullptr) {
        widget_->reset_in_progress_task_cycles(new_cycles, affected_tasks);
    }
}

}  // namespace source::plugin
