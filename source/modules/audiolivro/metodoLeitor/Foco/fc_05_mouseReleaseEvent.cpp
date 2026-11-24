#include "source/modules/audiolivro/metodoLeitor/Foco/fc_05_mouseReleaseEvent.hpp"

#include <QMouseEvent>

namespace source::plugin::Audiolivro {

void FocusRuler::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragging_ = false;
        resizing_ = false;
        resize_edge_ = NoEdge;
        update_cursor(event->pos());
    }
}

}  // namespace source::plugin::Audiolivro
