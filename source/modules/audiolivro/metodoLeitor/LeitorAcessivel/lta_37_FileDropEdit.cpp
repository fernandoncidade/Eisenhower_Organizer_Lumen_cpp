#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

namespace source::plugin::Audiolivro {

FileDropEdit::FileDropEdit(QWidget *parent)
    : QTextEdit(parent)
{
    setAcceptDrops(true);
}

void FileDropEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() || event->mimeData()->hasText()) {
        event->acceptProposedAction();
        return;
    }
    QTextEdit::dragEnterEvent(event);
}

void FileDropEdit::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls() || event->mimeData()->hasText()) {
        event->acceptProposedAction();
        return;
    }
    QTextEdit::dragMoveEvent(event);
}

void FileDropEdit::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QStringList paths;
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                paths.append(url.toLocalFile());
            }
        }
        if (!paths.isEmpty()) {
            emit files_dropped(paths);
            event->acceptProposedAction();
            return;
        }
    }
    QTextEdit::dropEvent(event);
}

}  // namespace source::plugin::Audiolivro
