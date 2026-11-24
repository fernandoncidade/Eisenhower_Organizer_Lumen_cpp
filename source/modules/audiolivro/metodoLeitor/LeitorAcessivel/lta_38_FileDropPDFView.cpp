#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_38_FileDropPDFView.hpp"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

namespace source::plugin::Audiolivro {

FileDropPDFView::FileDropPDFView(QWidget *parent)
    : PDFView(parent)
{
    setAcceptDrops(true);
}

void FileDropPDFView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }
    PDFView::dragEnterEvent(event);
}

void FileDropPDFView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }
    PDFView::dragMoveEvent(event);
}

void FileDropPDFView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile() && url.toLocalFile().endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive)) {
                emit file_dropped(url.toLocalFile());
                event->acceptProposedAction();
                return;
            }
        }
    }
    PDFView::dropEvent(event);
}

}  // namespace source::plugin::Audiolivro
