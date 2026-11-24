#include "source/modules/audiolivro/metodoLeitor/Foco/fc_02_get_resize_edge.hpp"

namespace source::plugin::Audiolivro {

FocusRuler::ResizeEdges FocusRuler::get_resize_edge(const QPoint &pos) const
{
    ResizeEdges edge = NoEdge;
    if (pos.x() <= resize_margin_) {
        edge |= LeftEdge;
    } else if (pos.x() >= width() - resize_margin_) {
        edge |= RightEdge;
    }

    if (pos.y() <= resize_margin_) {
        edge |= TopEdge;
    } else if (pos.y() >= height() - resize_margin_) {
        edge |= BottomEdge;
    }

    return edge;
}

}  // namespace source::plugin::Audiolivro
