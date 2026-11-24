#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"

namespace source::plugin::MapaMental {

MapaScene::MapaScene(QObject *parent)
    : QGraphicsScene(parent),
      grid_pen_light_(QColor(255, 255, 255, 18), 1),
      grid_pen_dark_(QColor(0, 0, 0, 22), 1)
{
    setSceneRect(-2400, -1800, 4800, 3600);
}

}  // namespace source::plugin::MapaMental
