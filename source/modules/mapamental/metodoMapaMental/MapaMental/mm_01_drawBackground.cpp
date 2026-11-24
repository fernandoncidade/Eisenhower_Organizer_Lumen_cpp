#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_01_drawBackground.hpp"

#include <cmath>

#include <QApplication>
#include <QPainter>
#include <QPalette>

namespace source::plugin::MapaMental {

void MapaScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (painter == nullptr || grid_step_ <= 0) {
        QGraphicsScene::drawBackground(painter, rect);
        return;
    }

    QGraphicsScene::drawBackground(painter, rect);

    const QPalette palette = QApplication::palette();
    const bool light = palette.color(QPalette::Window).lightnessF() >= 0.55;
    painter->setPen(light ? grid_pen_dark_ : grid_pen_light_);

    const int left = static_cast<int>(std::floor(rect.left() / grid_step_)) * grid_step_;
    const int top = static_cast<int>(std::floor(rect.top() / grid_step_)) * grid_step_;

    for (int x = left; x < rect.right(); x += grid_step_) {
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    }
    for (int y = top; y < rect.bottom(); y += grid_step_) {
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
    }
}

}  // namespace source::plugin::MapaMental
