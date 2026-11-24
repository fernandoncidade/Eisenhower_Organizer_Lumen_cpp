#include "source/modules/audiolivro/metodoLeitor/Foco/fc_03_mousePressEvent.hpp"

#include <QMouseEvent>

namespace source::plugin::Audiolivro {

void FocusRuler::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    resize_edge_ = get_resize_edge(event->pos());
    if (resize_edge_ != NoEdge) {
        resizing_ = true;
        global_offset_ = event->globalPosition().toPoint();
        start_geometry_ = geometry();
    } else {
        dragging_ = true;
        offset_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

}  // namespace source::plugin::Audiolivro
