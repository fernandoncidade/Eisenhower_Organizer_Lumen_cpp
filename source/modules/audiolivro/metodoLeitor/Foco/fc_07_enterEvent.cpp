#include "source/modules/audiolivro/metodoLeitor/Foco/fc_07_enterEvent.hpp"

#include <QEnterEvent>

namespace source::plugin::Audiolivro {

void FocusRuler::enterEvent(QEnterEvent *event)
{
    update_cursor(event->position().toPoint());
    QWidget::enterEvent(event);
}

}  // namespace source::plugin::Audiolivro
