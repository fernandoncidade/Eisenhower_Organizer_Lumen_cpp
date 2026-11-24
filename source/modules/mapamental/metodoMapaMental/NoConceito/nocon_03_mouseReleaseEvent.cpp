#include "source/modules/mapamental/metodoMapaMental/NoConceito/nocon_03_mouseReleaseEvent.hpp"

#include <QGraphicsSceneMouseEvent>

#include "source/modules/mapamental/metodoMapaMental/mp_02_LinhaConexao.hpp"

namespace source::plugin::MapaMental {

void NoConceito::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (ctrl_selection_click_) {
        ctrl_selection_click_ = false;
        for (LinhaConexao *linha : std::as_const(conexoes_)) {
            if (linha != nullptr) {
                linha->atualizar_posicao();
            }
        }
        if (event != nullptr) {
            event->accept();
        }
        return;
    }

    QGraphicsEllipseItem::mouseReleaseEvent(event);
    for (LinhaConexao *linha : std::as_const(conexoes_)) {
        if (linha != nullptr) {
            linha->atualizar_posicao();
        }
    }
}

}  // namespace source::plugin::MapaMental
