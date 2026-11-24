#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_02_snap.hpp"

namespace source::plugin::MapaMental {

QPointF MapaScene::snap(const QPointF &pos) const
{
    if (!snap_enabled_ || grid_step_ <= 0) {
        return pos;
    }

    const qreal x = qRound(pos.x() / grid_step_) * grid_step_;
    const qreal y = qRound(pos.y() / grid_step_) * grid_step_;
    return {x, y};
}

}  // namespace source::plugin::MapaMental
