#pragma once

#include <QGraphicsScene>
#include <QPen>

namespace source::plugin::MapaMental {

class MapaScene final : public QGraphicsScene {
public:
    explicit MapaScene(QObject *parent = nullptr);

    QPointF snap(const QPointF &pos) const;
    void set_snap_enabled(bool enabled) { snap_enabled_ = enabled; }
    bool snap_enabled() const { return snap_enabled_; }

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    int grid_step_ = 40;
    QPen grid_pen_light_;
    QPen grid_pen_dark_;
    bool snap_enabled_ = true;
};

}  // namespace source::plugin::MapaMental
