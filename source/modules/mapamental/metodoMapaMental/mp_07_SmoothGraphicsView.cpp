#include "source/modules/mapamental/metodoMapaMental/mp_07_SmoothGraphicsView.hpp"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QOpenGLWidget>
#include <QPainter>
#include <QtGlobal>

namespace source::plugin::MapaMental {

SmoothGraphicsView::SmoothGraphicsView(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene, parent)
{
    setViewport(new QOpenGLWidget(this));
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
}

qreal SmoothGraphicsView::min_scale_to_fit_scene() const
{
    if (scene() == nullptr || viewport() == nullptr) {
        return 0.05;
    }

    QRectF rect = scene()->itemsBoundingRect();
    if (rect.isEmpty()) {
        return 0.05;
    }

    rect = rect.adjusted(-160.0, -120.0, 160.0, 120.0);
    const QSizeF viewport_size = viewport()->size();
    if (viewport_size.width() <= 0.0 || viewport_size.height() <= 0.0) {
        return 0.05;
    }

    const qreal scale_x = viewport_size.width() / rect.width();
    const qreal scale_y = viewport_size.height() / rect.height();
    const qreal fit_scale = qMin(scale_x, scale_y);
    return qBound(0.02, fit_scale * 0.85, 1.0);
}

}  // namespace source::plugin::MapaMental
