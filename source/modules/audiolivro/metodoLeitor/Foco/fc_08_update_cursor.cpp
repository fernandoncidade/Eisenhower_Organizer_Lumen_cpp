#include "source/modules/audiolivro/metodoLeitor/Foco/fc_08_update_cursor.hpp"

namespace source::plugin::Audiolivro {

void FocusRuler::update_cursor(const QPoint &pos)
{
    const ResizeEdges edge = get_resize_edge(pos);
    if ((edge.testFlag(LeftEdge) && edge.testFlag(TopEdge))
        || (edge.testFlag(RightEdge) && edge.testFlag(BottomEdge))) {
        setCursor(Qt::SizeFDiagCursor);
    } else if ((edge.testFlag(RightEdge) && edge.testFlag(TopEdge))
               || (edge.testFlag(LeftEdge) && edge.testFlag(BottomEdge))) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (edge.testFlag(LeftEdge) || edge.testFlag(RightEdge)) {
        setCursor(Qt::SizeHorCursor);
    } else if (edge.testFlag(TopEdge) || edge.testFlag(BottomEdge)) {
        setCursor(Qt::SizeVerCursor);
    } else {
        setCursor(Qt::OpenHandCursor);
    }
}

}  // namespace source::plugin::Audiolivro
