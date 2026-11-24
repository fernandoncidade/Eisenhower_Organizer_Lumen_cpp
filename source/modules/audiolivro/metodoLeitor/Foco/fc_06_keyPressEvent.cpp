#include "source/modules/audiolivro/metodoLeitor/Foco/fc_06_keyPressEvent.hpp"

#include <QKeyEvent>

namespace source::plugin::Audiolivro {

void FocusRuler::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        emit closed();
        event->accept();
        return;
    }

    QWidget::keyPressEvent(event);
}

}  // namespace source::plugin::Audiolivro
