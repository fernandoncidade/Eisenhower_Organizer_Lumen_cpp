#include "source/modules/audiolivro/metodoLeitor/Foco/fc_04_mouseMoveEvent.hpp"

#include <QMouseEvent>

namespace source::plugin::Audiolivro {

void FocusRuler::mouseMoveEvent(QMouseEvent *event)
{
    if (resizing_) {
        const QPoint delta = event->globalPosition().toPoint() - global_offset_;
        QRect next = start_geometry_;

        if (resize_edge_.testFlag(LeftEdge)) {
            next.setLeft(next.left() + delta.x());
        }
        if (resize_edge_.testFlag(RightEdge)) {
            next.setRight(next.right() + delta.x());
        }
        if (resize_edge_.testFlag(TopEdge)) {
            next.setTop(next.top() + delta.y());
        }
        if (resize_edge_.testFlag(BottomEdge)) {
            next.setBottom(next.bottom() + delta.y());
        }

        if (next.width() >= 120 && next.height() >= 32) {
            setGeometry(next);
        }
        return;
    }

    if (dragging_) {
        move(event->globalPosition().toPoint() - offset_);
        return;
    }

    update_cursor(event->pos());
}

}  // namespace source::plugin::Audiolivro
