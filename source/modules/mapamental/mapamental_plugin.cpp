#include "source/modules/mapamental/mapamental_plugin.hpp"

#include <QCoreApplication>

#include "source/modules/mapamental/mapamental_support.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

namespace source::plugin {

QString MapaMentalPlugin::plugin_id() const
{
    return QStringLiteral("mapamental");
}

QString MapaMentalPlugin::display_name() const
{
    return QCoreApplication::translate("MapaMentalPlugin", "Mapa Mental");
}

QString MapaMentalPlugin::open_menu_text() const
{
    return QCoreApplication::translate("MapaMentalPlugin", "Abrir Mapa Mental");
}

void MapaMentalPlugin::initialize(MapaMentalHostBridge *host_bridge)
{
    host_bridge_ = host_bridge;
    source::plugin::MapaMental::compat::set_host_bridge(host_bridge_);
    aplicar_traducao_atual();
    if (widget_ != nullptr) {
        widget_->atualizar_traducoes();
    }
}

QWidget *MapaMentalPlugin::create_widget(QWidget *parent)
{
    aplicar_traducao_atual();

    if (widget_ == nullptr) {
        widget_ = new source::plugin::MapaMental::MapaMental(parent);
        if (has_pending_layout_mode_) {
            widget_->set_layout_mode_key(pending_layout_mode_key_);
        } else {
            pending_layout_mode_key_ = widget_->layout_mode_key();
        }
        if (has_pending_summary_method_) {
            widget_->set_summary_method_key(pending_summary_method_key_);
        } else {
            pending_summary_method_key_ = widget_->summary_method_key();
        }
    } else if (widget_->parentWidget() != parent) {
        widget_->setParent(parent);
    }

    widget_->atualizar_traducoes();
    return widget_;
}

void MapaMentalPlugin::retranslate_ui()
{
    aplicar_traducao_atual();
    if (widget_ != nullptr) {
        widget_->atualizar_traducoes();
        widget_->update();
    }
}

void MapaMentalPlugin::reload_shared_state()
{
    if (widget_ != nullptr) {
        widget_->carregar_backup_automatico();
    }
}

void MapaMentalPlugin::refresh_task_views()
{
    if (widget_ != nullptr) {
        widget_->refresh_fonts();
        widget_->refresh_counter();
    }
}

QString MapaMentalPlugin::layout_mode_key() const
{
    return widget_ != nullptr ? widget_->layout_mode_key() : pending_layout_mode_key_;
}

void MapaMentalPlugin::set_layout_mode_key(const QString &mode_key)
{
    pending_layout_mode_key_ = mode_key;
    has_pending_layout_mode_ = true;
    if (widget_ != nullptr) {
        widget_->set_layout_mode_key(mode_key);
        pending_layout_mode_key_ = widget_->layout_mode_key();
    }
}

QString MapaMentalPlugin::summary_method_key() const
{
    return widget_ != nullptr ? widget_->summary_method_key() : pending_summary_method_key_;
}

void MapaMentalPlugin::set_summary_method_key(const QString &method_key)
{
    pending_summary_method_key_ = method_key;
    has_pending_summary_method_ = true;
    if (widget_ != nullptr) {
        widget_->set_summary_method_key(method_key);
        pending_summary_method_key_ = widget_->summary_method_key();
    }
}

void MapaMentalPlugin::aplicar_traducao_atual()
{
    if (translation_manager_ == nullptr) {
        translation_manager_ = std::make_unique<source::plugin::MapaMental::language::GerenciadorTraducao>(this);
    }

    const QString idioma = host_bridge_ != nullptr ? host_bridge_->language_code() : QString();
    translation_manager_->aplicar_traducao(idioma);
}

void MapaMentalPlugin::reset_task_font_overrides(bool target_linked)
{
    if (widget_ == nullptr) {
        return;
    }

    bool changed = false;
    for (source::plugin::MapaMental::NoConceito *no : widget_->nos()) {
        if (no != nullptr) {
            const bool has_linked = !no->arquivo_vinculado().trimmed().isEmpty();
            if (has_linked == target_linked && !no->font_config().isEmpty()) {
                no->set_font_config(QVariantMap());
                changed = true;
            }
        }
    }

    if (changed) {
        widget_->salvar_backup_automatico();
        widget_->refresh_fonts();
    }
}

QVariantList MapaMentalPlugin::get_task_font_overrides(bool target_linked) const
{
    if (widget_ == nullptr) {
        return {};
    }

    QVariantList res;
    for (source::plugin::MapaMental::NoConceito *no : widget_->nos()) {
        if (no != nullptr) {
            const bool has_linked = !no->arquivo_vinculado().trimmed().isEmpty();
            if (has_linked == target_linked) {
                res.append(no->font_config());
            }
        }
    }
    return res;
}

}  // namespace source::plugin
