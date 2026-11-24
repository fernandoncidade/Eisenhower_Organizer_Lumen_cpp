#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_36_limpeza_painel.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_26_pdf_view.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_38_FileDropPDFView.hpp"

namespace source::plugin::Audiolivro {

void LeitorAcessivel::limpar_painel()
{
    parar_leitura();
    if (texto_area_ != nullptr) {
        texto_area_->clear();
    }
    if (pdf_view_ != nullptr) {
        pdf_view_->clear_pdf();
    }
    pdf_enable_toolbar(false);
    pdf_find_clear();
    pdf_update_page_ui(0, 0);
    last_pdf_path_.clear();
    salvar_pdf_carregado_temp(QString());
    limpar_texto_carregado_temp();
    current_reading_pdf_path_.clear();
    current_reading_text_.clear();
    pending_pdf_text_refresh_ = false;
    generated_queue_.clear();
    generated_timestamps_.clear();
    current_generated_.clear();
    current_timestamps_.clear();
    current_reading_is_pdf_ = false;
    text_find_clear();
}

}  // namespace source::plugin::Audiolivro
