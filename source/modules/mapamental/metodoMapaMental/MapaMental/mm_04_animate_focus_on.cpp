#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_04_animate_focus_on.hpp"

#include <QEasingCurve>
#include <QGraphicsItem>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScrollBar>

namespace source::plugin::MapaMental {

void SmoothGraphicsView::animate_focus_on(QGraphicsItem *item)
{
    if (item == nullptr || viewport() == nullptr || horizontalScrollBar() == nullptr || verticalScrollBar() == nullptr) {
        return;
    }

    if (focus_animation_ != nullptr) {
        focus_animation_->stop();
        focus_animation_->deleteLater();
        focus_animation_.clear();
    }

    const QPointF item_center = item->sceneBoundingRect().center();
    const QPoint current_view_pos = mapFromScene(item_center);
    const QPoint viewport_center = viewport()->rect().center();
    const int dx = current_view_pos.x() - viewport_center.x();
    const int dy = current_view_pos.y() - viewport_center.y();

    auto *group = new QParallelAnimationGroup(this);
    auto *h_anim = new QPropertyAnimation(horizontalScrollBar(), "value", group);
    auto *v_anim = new QPropertyAnimation(verticalScrollBar(), "value", group);

    h_anim->setStartValue(horizontalScrollBar()->value());
    h_anim->setEndValue(horizontalScrollBar()->value() + dx);
    v_anim->setStartValue(verticalScrollBar()->value());
    v_anim->setEndValue(verticalScrollBar()->value() + dy);

    for (QPropertyAnimation *anim : {h_anim, v_anim}) {
        anim->setDuration(520);
        anim->setEasingCurve(QEasingCurve::InOutCubic);
    }

    focus_animation_ = group;
    connect(group, &QParallelAnimationGroup::finished, group, &QObject::deleteLater);
    group->start();
}

}  // namespace source::plugin::MapaMental
