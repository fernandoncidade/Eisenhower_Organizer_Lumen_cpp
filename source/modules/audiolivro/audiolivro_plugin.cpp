#include "source/modules/audiolivro/audiolivro_plugin.hpp"

#include "source/modules/audiolivro/audiolivro_font_settings_dialog.hpp"
#include "source/modules/audiolivro/audiolivro_settings.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_06_VoiceCatalog.hpp"

#include <QCoreApplication>

namespace source::plugin {

QString AudiolivroPlugin::plugin_id() const
{
    return QStringLiteral("audiolivro");
}

QString AudiolivroPlugin::display_name() const
{
    return QCoreApplication::translate("AudiolivroPlugin", "Audiolivro");
}

QString AudiolivroPlugin::open_menu_text() const
{
    return QCoreApplication::translate("AudiolivroPlugin", "Abrir Audiolivro");
}

QString AudiolivroPlugin::font_settings_menu_text() const
{
    return QCoreApplication::translate("AudiolivroPlugin", "Configuracoes de Fonte");
}

void AudiolivroPlugin::initialize(AudiolivroHostBridge *host_bridge)
{
    host_bridge_ = host_bridge;
    aplicar_traducao_atual();
    if (widget_ != nullptr) {
        widget_->atualizar_traducoes();
    }
}

QWidget *AudiolivroPlugin::create_widget(QWidget *parent)
{
    aplicar_traducao_atual();

    if (widget_ == nullptr) {
        widget_ = new source::plugin::Audiolivro::LeitorAcessivel(parent);
    } else if (widget_->parentWidget() != parent) {
        widget_->setParent(parent);
    }

    widget_->definir_voz(stored_voice_id());
    widget_->atualizar_traducoes();
    return widget_;
}

void AudiolivroPlugin::open_font_settings(QWidget *parent)
{
    aplicar_traducao_atual();
    QWidget *dialog_parent = parent != nullptr ? parent : widget_.data();
    source::plugin::Audiolivro::FontSettingsDialog dialog(
        source::plugin::Audiolivro::carregar_fonte_configurada(),
        dialog_parent);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    source::plugin::Audiolivro::salvar_fonte_configurada(dialog.selected_font());
    if (widget_ != nullptr) {
        widget_->aplicar_fonte_configurada();
    }
}

void AudiolivroPlugin::retranslate_ui()
{
    aplicar_traducao_atual();
    if (widget_ != nullptr) {
        widget_->atualizar_traducoes();
        widget_->update();
    }
}

void AudiolivroPlugin::reload_shared_state()
{
    if (widget_ != nullptr) {
        widget_->reload_shared_state();
    }
}

void AudiolivroPlugin::refresh_task_views()
{
    if (widget_ != nullptr) {
        widget_->refresh_task_views();
    }
}

QVariantList AudiolivroPlugin::voice_options() const
{
    return source::plugin::Audiolivro::available_voice_options();
}

QString AudiolivroPlugin::current_voice_id() const
{
    if (widget_ != nullptr) {
        return widget_->voz_atual();
    }
    return stored_voice_id();
}

void AudiolivroPlugin::set_voice_id(const QString &voice_id)
{
    QString normalized = voice_id.trimmed();
    if (normalized.isEmpty()) {
        normalized = source::plugin::Audiolivro::default_voice_id();
    }

    pending_voice_id_ = normalized;
    source::plugin::Audiolivro::salvar_configuracao_texto(QStringLiteral("voice_id"), normalized);
    if (widget_ != nullptr) {
        widget_->definir_voz(normalized);
    }
}

QString AudiolivroPlugin::stored_voice_id() const
{
    if (!pending_voice_id_.trimmed().isEmpty()) {
        return pending_voice_id_;
    }

    const QString stored = source::plugin::Audiolivro::carregar_configuracao_texto(
                               QStringLiteral("voice_id"),
                               source::plugin::Audiolivro::default_voice_id())
                               .trimmed();
    return stored.isEmpty() ? source::plugin::Audiolivro::default_voice_id() : stored;
}

void AudiolivroPlugin::aplicar_traducao_atual()
{
    if (translation_manager_ == nullptr) {
        translation_manager_ = std::make_unique<source::plugin::Audiolivro::language::GerenciadorTraducao>(this);
    }

    const QString idioma = host_bridge_ != nullptr ? host_bridge_->language_code() : QString();
    translation_manager_->aplicar_traducao(idioma);
}

}  // namespace source::plugin
