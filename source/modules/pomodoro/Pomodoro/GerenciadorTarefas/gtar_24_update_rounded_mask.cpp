#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_24_update_rounded_mask.hpp"

#include <QPainterPath>
#include <QRegion>
#include <QWidget>

namespace source::plugin::Pomodoro::GerenciadorTarefasFns {

void update_rounded_mask(GerenciadorTarefas &self, QWidget *widget, int radius)
{
    Q_UNUSED(self);
    if (widget == nullptr) {
        return;
    }

    QPainterPath path;
    path.addRoundedRect(widget->rect(), radius, radius);
    widget->setMask(QRegion(path.toFillPolygon().toPolygon()));
}

}  // namespace source::plugin::Pomodoro::GerenciadorTarefasFns
