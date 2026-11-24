#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_28_pdf_find_actions.hpp"

#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_26_pdf_view.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_27_pdf_findbar.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_38_FileDropPDFView.hpp"

namespace source::plugin::Audiolivro {

void LeitorAcessivel::pdf_find_show()
{
    if (pdf_find_bar_ != nullptr) {
        pdf_find_bar_->show_bar();
    }
}

void LeitorAcessivel::pdf_find_toggle()
{
    if (pdf_find_bar_ == nullptr) {
        return;
    }
    pdf_find_bar_->isVisible() ? pdf_find_bar_->hide_bar() : pdf_find_bar_->show_bar();
}

void LeitorAcessivel::pdf_find_clear()
{
    if (pdf_view_ != nullptr) {
        pdf_view_->find_text(QString());
    }
    if (pdf_find_bar_ != nullptr) {
        pdf_find_bar_->set_result_text(QString());
    }
}

void LeitorAcessivel::pdf_find_set_highlight_all(bool enable)
{
    highlight_all_pdf_hits_ = enable;
    if (pdf_view_ != nullptr) {
        pdf_view_->set_search_highlight_all(enable);
    }
}

void LeitorAcessivel::pdf_find_apply(const QString &query, bool match_case, bool whole_words)
{
    if (pdf_view_ == nullptr) {
        return;
    }

    const QVector<PageHit> hits = pdf_view_->find_text(query, match_case, whole_words);
    if (pdf_find_bar_ != nullptr) {
        pdf_find_bar_->set_result_text(hits.isEmpty() ? tr("0 resultados") : tr("%1 resultados").arg(hits.size()));
    }
}

void LeitorAcessivel::pdf_find_next()
{
    if (pdf_view_ != nullptr && pdf_view_->hit_count() > 0) {
        pdf_view_->set_current_hit(pdf_view_->current_hit() + 1);
    }
}

void LeitorAcessivel::pdf_find_prev()
{
    if (pdf_view_ != nullptr && pdf_view_->hit_count() > 0) {
        pdf_view_->set_current_hit(pdf_view_->current_hit() - 1);
    }
}

}  // namespace source::plugin::Audiolivro
