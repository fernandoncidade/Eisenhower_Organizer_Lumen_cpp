#pragma once

#include <QGraphicsView>
#include <QPointer>

class QParallelAnimationGroup;

namespace source::plugin::MapaMental {

class SmoothGraphicsView final : public QGraphicsView {
public:
    explicit SmoothGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);

    void animate_focus_on(QGraphicsItem *item);

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    qreal min_scale_to_fit_scene() const;
    QPointer<QParallelAnimationGroup> focus_animation_;
};

}  // namespace source::plugin::MapaMental
