#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_02_atualizar_traducoes.hpp"

#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_27_pdf_findbar.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_29_text_findbar.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_38_FileDropPDFView.hpp"
#include "source/utils/AccentTabStyle.hpp"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTabWidget>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::atualizar_traducoes()
{
    if (btn_carregar_ != nullptr) btn_carregar_->setText(tr("📁 Carregar PDF"));
    if (btn_play_ != nullptr) btn_play_->setText(tr("▶️ Ler"));
    update_pause_button();
    if (btn_stop_ != nullptr) btn_stop_->setText(tr("⏹️ Parar"));
    if (btn_limpar_ != nullptr) btn_limpar_->setText(tr("🧽 Limpar"));
    if (label_velocidade_titulo_ != nullptr) label_velocidade_titulo_->setText(tr("Velocidade:"));
    if (label_volume_ != nullptr) label_volume_->setText(tr("Volume:"));
    if (btn_new_ != nullptr) btn_new_->setText(tr("📖 Novo"));
    if (btn_save_ != nullptr) btn_save_->setText(tr("💾 Salvar Como"));
    if (btn_undo_ != nullptr) btn_undo_->setText(tr("↩️ Desfazer"));
    if (btn_redo_ != nullptr) btn_redo_->setText(tr("↪️ Refazer"));
    if (btn_find_text_ != nullptr) btn_find_text_->setText(tr("🔎 Buscar"));
    if (btn_font_settings_ != nullptr) btn_font_settings_->setText(tr("🔤 Configurações de Fonte"));
    if (btn_color_tasks_ != nullptr) btn_color_tasks_->setText(tr("🎨 Colorir fonte/texto"));
    if (btn_reset_settings_ != nullptr) btn_reset_settings_->setText(tr("🔄 Restaurar configurações"));
    if (btn_bullets_ != nullptr) btn_bullets_->setText(tr("☑️/🔹 Marcadores"));
    if (label_bullet_style_ != nullptr) label_bullet_style_->setText(tr("Marcador:"));
    if (label_spacing_ != nullptr) label_spacing_->setText(tr("Espacamento:"));
    if (label_indent_ != nullptr) label_indent_->setText(tr("Recuo:"));
    if (label_margin_ != nullptr) label_margin_->setText(tr("Margem:"));
    update_regua_foco_button();
    if (dica_regua_ != nullptr) dica_regua_->setText(tr("💡 Ative uma faixa visual movel para acompanhar linhas de leitura."));
    update_leitura_assistida_button();
    if (label_modo_leitura_ != nullptr) label_modo_leitura_->setText(tr("Modo:"));
    if (dica_leitura_assistida_ != nullptr) dica_leitura_assistida_->setText(tr("💡 Realca o texto enquanto o audio e reproduzido."));
    if (btn_pdf_prev_ != nullptr) btn_pdf_prev_->setText(tr("◀️ Anterior"));
    if (btn_pdf_next_ != nullptr) btn_pdf_next_->setText(tr("▶️ Próxima"));
    if (btn_pdf_zoom_out_ != nullptr) btn_pdf_zoom_out_->setText(tr("🔍−"));
    if (btn_pdf_zoom_in_ != nullptr) btn_pdf_zoom_in_->setText(tr("🔍+"));
    if (label_pdf_zoom_ != nullptr) label_pdf_zoom_->setText(tr("Zoom:"));
    if (btn_pdf_fit_width_ != nullptr) btn_pdf_fit_width_->setText(tr("📄 Largura"));
    if (btn_pdf_fit_page_ != nullptr) btn_pdf_fit_page_->setText(tr("📃 Página"));
    if (btn_pdf_hand_ != nullptr) btn_pdf_hand_->setText(tr("✋ Mão"));
    if (btn_pdf_select_ != nullptr) btn_pdf_select_->setText(tr("📝 Seleção"));
    if (btn_pdf_find_ != nullptr) btn_pdf_find_->setText(tr("🔎 Buscar"));

    if (combo_modo_leitura_ != nullptr) {
        const QSignalBlocker blocker(combo_modo_leitura_);
        const int current_index = combo_modo_leitura_->currentIndex();
        combo_modo_leitura_->clear();
        combo_modo_leitura_->addItems({tr("Palavras"), tr("Frases"), tr("Paragrafos")});
        if (current_index >= 0 && current_index < combo_modo_leitura_->count()) {
            combo_modo_leitura_->setCurrentIndex(current_index);
        }
    }

    if (combo_pdf_zoom_ != nullptr) {
        const QSignalBlocker blocker(combo_pdf_zoom_);
        const QString current_text = combo_pdf_zoom_->currentText();
        const int current_index = combo_pdf_zoom_->currentIndex();
        combo_pdf_zoom_->clear();
        combo_pdf_zoom_->addItems({QStringLiteral("25%"), QStringLiteral("50%"), QStringLiteral("75%"),
                                   QStringLiteral("100%"), QStringLiteral("125%"), QStringLiteral("150%"),
                                   QStringLiteral("175%"), QStringLiteral("200%"), QStringLiteral("225%"),
                                   QStringLiteral("250%"), QStringLiteral("275%"), QStringLiteral("300%"),
                                   tr("Largura"), tr("Pagina")});
        if (current_text.trimmed().endsWith(QLatin1Char('%'))) {
            combo_pdf_zoom_->setCurrentText(current_text);
        } else if (current_index >= 0 && current_index < combo_pdf_zoom_->count()) {
            combo_pdf_zoom_->setCurrentIndex(current_index);
        } else {
            combo_pdf_zoom_->setCurrentText(QStringLiteral("100%"));
        }
    }

    if (text_find_bar_ != nullptr) text_find_bar_->retranslate_ui();
    if (pdf_find_bar_ != nullptr) pdf_find_bar_->retranslate_ui();

    if (content_stack_ != nullptr) {
        content_stack_->setTabText(0, tr("Texto"));
        content_stack_->setTabText(1, tr("PDF"));
        source::utils::AccentTabStyle::set_tab_accent(
            content_stack_, 0, source::utils::AccentTabStyle::accent_blue());
        source::utils::AccentTabStyle::set_tab_accent(
            content_stack_, 1, source::utils::AccentTabStyle::accent_red());
    }

    pdf_update_page_ui(pdf_view_ != nullptr ? pdf_view_->current_page() : 0,
                       pdf_view_ != nullptr ? pdf_view_->page_count() : 0);
}

void LeitorAcessivel::update_regua_foco_button()
{
    if (btn_regua_ != nullptr) {
        btn_regua_->setText(btn_regua_->isChecked() ? tr("📏 Desativar Regua de Foco")
                                                    : tr("📏 Ativar Regua de Foco"));
    }
}

}  // namespace source::plugin::Audiolivro
