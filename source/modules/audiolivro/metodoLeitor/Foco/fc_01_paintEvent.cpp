#include "source/modules/audiolivro/metodoLeitor/Foco/fc_01_paintEvent.hpp"

#include <QColor>
#include <QPainter>

namespace source::plugin::Audiolivro {

void FocusRuler::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(255, 255, 0, 50));
    painter.setPen(QColor(255, 200, 0, 200));
    painter.drawRect(0, 0, width() - 1, height() - 1);
    painter.setBrush(QColor(255, 200, 0, 150));

    constexpr int handle = 6;
    painter.drawEllipse(0, 0, handle, handle);
    painter.drawEllipse(width() - handle, 0, handle, handle);
    painter.drawEllipse(0, height() - handle, handle, handle);
    painter.drawEllipse(width() - handle, height() - handle, handle, handle);

    const int half_width = width() / 2;
    const int half_height = height() / 2;
    painter.drawRect(half_width - handle / 2, 0, handle, handle);
    painter.drawRect(half_width - handle / 2, height() - handle, handle, handle);
    painter.drawRect(0, half_height - handle / 2, handle, handle);
    painter.drawRect(width() - handle, half_height - handle / 2, handle, handle);
}

}  // namespace source::plugin::Audiolivro
