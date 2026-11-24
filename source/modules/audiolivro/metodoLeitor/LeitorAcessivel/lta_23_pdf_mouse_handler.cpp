#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_23_pdf_mouse_handler.hpp"

#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_26_pdf_view.hpp"

#include <QApplication>
#include <QClipboard>
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>

namespace source::plugin::Audiolivro {

PDFMouseHandler::PDFMouseHandler(PDFView *view, QObject *parent)
    : QObject(parent), view_(view)
{
    if (view_ != nullptr) {
        view_->viewport()->installEventFilter(this);
    }
}

bool PDFMouseHandler::copy_selection()
{
    if (view_ == nullptr) {
        return false;
    }
    const QString text = view_->has_text_selection() ? view_->selected_text() : view_->all_text(view_->current_page());
    if (text.trimmed().isEmpty()) {
        return false;
    }
    QApplication::clipboard()->setText(text);
    return true;
}

bool PDFMouseHandler::eventFilter(QObject *watched, QEvent *event)
{
    if (view_ == nullptr || watched != view_->viewport()) {
        return QObject::eventFilter(watched, event);
    }

    if (event->type() == QEvent::MouseButtonPress) {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->button() == Qt::LeftButton && view_->viewport()->cursor().shape() == Qt::OpenHandCursor) {
            hand_dragging_ = true;
            last_pos_ = mouse->pos();
            view_->viewport()->setCursor(Qt::ClosedHandCursor);
            return true;
        }
    } else if (event->type() == QEvent::MouseMove && hand_dragging_) {
        auto *mouse = static_cast<QMouseEvent *>(event);
        const QPoint delta = mouse->pos() - last_pos_;
        view_->horizontalScrollBar()->setValue(view_->horizontalScrollBar()->value() - delta.x());
        view_->verticalScrollBar()->setValue(view_->verticalScrollBar()->value() - delta.y());
        last_pos_ = mouse->pos();
        return true;
    } else if (event->type() == QEvent::MouseButtonRelease && hand_dragging_) {
        hand_dragging_ = false;
        view_->viewport()->setCursor(Qt::OpenHandCursor);
        return true;
    }

    return QObject::eventFilter(watched, event);
}

}  // namespace source::plugin::Audiolivro
