#include "source/modules/mapamental/metodoMapaMental/NoConceito/nocon_02_mousePressEvent.hpp"

#include <QGraphicsSceneMouseEvent>

#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

namespace source::plugin::MapaMental {

void NoConceito::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (owner_ != nullptr && owner_->modo_conexao()) {
        owner_->clicar_no_para_conectar(this);
        if (event != nullptr) {
            event->accept();
        }
        return;
    }

    if (owner_ != nullptr && event != nullptr && event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ControlModifier) {
            ctrl_selection_click_ = true;
            setSelected(!isSelected());
            owner_->refresh_counter();
            event->accept();
            return;
        }

        ctrl_selection_click_ = false;
        owner_->clicar_no_para_expandir_hierarquia(this);
        owner_->focar_no_para_qa(this);
    }

    QGraphicsEllipseItem::mousePressEvent(event);
}

}  // namespace source::plugin::MapaMental
