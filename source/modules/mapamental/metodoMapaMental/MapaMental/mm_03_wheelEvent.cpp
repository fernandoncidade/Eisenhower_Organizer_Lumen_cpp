#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_03_wheelEvent.hpp"

#include <QWheelEvent>

namespace source::plugin::MapaMental {

void SmoothGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (event == nullptr) {
        return;
    }

    const int delta = event->angleDelta().y();
    if (delta == 0) {
        QGraphicsView::wheelEvent(event);
        return;
    }

    const qreal factor = delta > 0 ? 1.15 : 1.0 / 1.15;
    const qreal current_scale = transform().m11();
    const qreal next_scale = current_scale * factor;
    const qreal min_scale = min_scale_to_fit_scene();
    const qreal max_scale = 6.0;

    if ((delta < 0 && next_scale < min_scale) || (delta > 0 && next_scale > max_scale)) {
        event->accept();
        return;
    }

    scale(factor, factor);
    event->accept();
}

}  // namespace source::plugin::MapaMental
