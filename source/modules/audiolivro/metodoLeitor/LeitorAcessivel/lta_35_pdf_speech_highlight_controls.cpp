#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_35_pdf_speech_highlight_controls.hpp"

#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_34_pdf_speech_highlight.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_26_pdf_view.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_38_FileDropPDFView.hpp"

namespace source::plugin::Audiolivro {

void LeitorAcessivel::init_pdf_speech_highlight()
{
}

QString LeitorAcessivel::setup_pdf_speech_highlight_for_reading(const QString &pdf_path,
                                                                const QString &text,
                                                                int page_index)
{
    QString page_text = pdf_view_ != nullptr ? pdf_view_->all_text(-1) : QString();
    if (page_text.trimmed().isEmpty() && page_index >= 0) {
        page_text = pdf_view_ != nullptr ? pdf_view_->all_text(page_index) : QString();
    }
    if (page_text.trimmed().isEmpty()) {
        page_text = current_pdf_text(-1);
    }
    if (page_text.trimmed().isEmpty() && page_index >= 0) {
        page_text = current_pdf_text(page_index);
    }
    if (page_text.trimmed().isEmpty() && !pdf_path.trimmed().isEmpty()) {
        page_text = extract_word_positions_from_pdf(pdf_path, -1);
    }
    if (page_text.trimmed().isEmpty()) {
        page_text = text;
    }
    if (page_text.trimmed().isEmpty() && texto_area_ != nullptr) {
        page_text = texto_area_->toPlainText();
    }
    current_reading_text_ = page_text;
    current_reading_pdf_path_ = pdf_path;
    current_timestamp_index_ = -1;
    last_speech_word_offset_ = -1;
    last_speech_highlight_start_ = -1;
    current_speech_highlight_start_ = -1;
    current_speech_highlight_length_ = 0;
    speech_highlight_force_refresh_ = false;
    last_speech_render_mode_.clear();
    current_timestamps_.clear();
    if (texto_area_ != nullptr && texto_area_->toPlainText() != page_text) {
        texto_area_->setPlainText(page_text);
        text_find_clear();
    } else if (texto_area_ != nullptr) {
        apply_text_extra_selections();
    }
    return page_text;
}

void LeitorAcessivel::on_pdf_timestamps_received(const QVariantList &timestamps)
{
    current_timestamps_ = timestamps;
    current_timestamp_index_ = -1;
    last_speech_word_offset_ = -1;
    last_speech_highlight_start_ = -1;
    current_speech_highlight_start_ = -1;
    current_speech_highlight_length_ = 0;
    speech_highlight_force_refresh_ = false;
    last_speech_render_mode_.clear();
}

void LeitorAcessivel::start_pdf_speech_highlight()
{
    if (pdf_speech_enabled_) {
        speech_timer_.start();
    }
}

void LeitorAcessivel::pause_pdf_speech_highlight()
{
    speech_timer_.stop();
}

void LeitorAcessivel::resume_pdf_speech_highlight()
{
    start_pdf_speech_highlight();
}

void LeitorAcessivel::stop_pdf_speech_highlight()
{
    if (!speech_enabled_) {
        speech_timer_.stop();
    }
}

void LeitorAcessivel::toggle_pdf_speech_highlight()
{
    pdf_speech_enabled_ = !pdf_speech_enabled_;
}

void LeitorAcessivel::set_pdf_speech_highlight_mode(const QString &mode)
{
    pdf_speech_mode_ = mode;
}

bool LeitorAcessivel::pdf_speech_highlight_enabled() const
{
    return pdf_speech_enabled_;
}

}  // namespace source::plugin::Audiolivro
