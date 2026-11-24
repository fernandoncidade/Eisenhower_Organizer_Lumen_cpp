#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_05_carregar_pdf.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_26_pdf_view.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_38_FileDropPDFView.hpp"

#include <QFileDialog>
#include <QRegularExpression>
#include <QTabWidget>
#include <QTimer>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::carregar_pdf(const QString &arquivo, bool keep_pdf_tab)
{
    QString path = arquivo;
    if (path.trimmed().isEmpty()) {
        path = QFileDialog::getOpenFileName(this, tr("Abrir PDF"), QString(), tr("PDF (*.pdf)"));
    }

    if (path.trimmed().isEmpty() || pdf_view_ == nullptr) {
        return;
    }

    pending_pdf_text_refresh_ = true;
    if (!pdf_view_->load_pdf(path)) {
        pending_pdf_text_refresh_ = false;
        pdf_enable_toolbar(false);
        pdf_update_page_ui(0, 0);
        on_tts_error(tr("Nao foi possivel abrir o PDF selecionado."));
        return;
    }

    last_pdf_path_ = path;
    salvar_pdf_carregado_temp(path);
    if (content_stack_ != nullptr) {
        if (!keep_pdf_tab) {
            content_stack_->setCurrentIndex(1);
        }
    }
    pdf_enable_toolbar(true);
    pdf_update_page_ui(pdf_view_->current_page(), pdf_view_->page_count());
    pdf_refresh_zoom_ui();
    QTimer::singleShot(0, this, [this]() { pdf_refresh_zoom_ui(); });
    sync_text_from_pdf();
}

QString LeitorAcessivel::current_pdf_text(int page_index) const
{
    if (pdf_view_ == nullptr) {
        return QString();
    }
    return reflow_pdf_text(pdf_view_->all_text(page_index));
}

QString LeitorAcessivel::reflow_pdf_text(const QString &text) const
{
    QString out = text;
    out.replace(QRegularExpression(QStringLiteral("[ \\t]+\\n")), QStringLiteral("\n"));
    out.replace(QRegularExpression(QStringLiteral("\\n{3,}")), QStringLiteral("\n\n"));
    out.replace(QRegularExpression(QStringLiteral("([^.!?;:\\n])\\n([^\\n])")), QStringLiteral("\\1 \\2"));
    return out.trimmed();
}

void LeitorAcessivel::sync_text_from_pdf()
{
    if (!pending_pdf_text_refresh_ || pdf_view_ == nullptr || texto_area_ == nullptr) {
        return;
    }

    if (pdf_view_->page_count() <= 0) {
        return;
    }

    pending_pdf_text_refresh_ = false;
    const QString extracted = current_pdf_text(-1);
    if (extracted.trimmed().isEmpty()) {
        return;
    }

    texto_area_->setPlainText(extracted);
    text_find_clear();
}

}  // namespace source::plugin::Audiolivro
