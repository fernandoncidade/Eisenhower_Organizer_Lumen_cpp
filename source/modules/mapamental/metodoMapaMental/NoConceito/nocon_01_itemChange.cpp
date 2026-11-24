#include "source/modules/mapamental/metodoMapaMental/NoConceito/nocon_01_itemChange.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_02_LinhaConexao.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"

namespace source::plugin::MapaMental {

QVariant NoConceito::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && !layout_animating_) {
        if (auto *mapa_scene = dynamic_cast<MapaScene *>(scene())) {
            return mapa_scene->snap(value.toPointF());
        }
    }

    if (change == ItemPositionHasChanged) {
        for (LinhaConexao *linha : std::as_const(conexoes_)) {
            if (linha != nullptr) {
                linha->atualizar_posicao();
            }
        }
    }

    return QGraphicsEllipseItem::itemChange(change, value);
}

}  // namespace source::plugin::MapaMental
