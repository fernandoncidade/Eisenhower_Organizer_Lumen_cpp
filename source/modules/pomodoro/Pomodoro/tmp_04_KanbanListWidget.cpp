#include "source/modules/pomodoro/Pomodoro/tmp_04_KanbanListWidget.hpp"

#include <QDropEvent>

#include "source/utils/LogManager.hpp"

namespace source::plugin::Pomodoro {
_KanbanListWidget::_KanbanListWidget(QWidget *parent) : QListWidget(parent) {}

void _KanbanListWidget::dropEvent(QDropEvent *event)
{
    auto *logger = source::utils::LogManager::get_logger();

    try {
        QListWidget::dropEvent(event);
        emit tarefas_soltadas();
    } catch (const std::exception &e) {
        logger->error(QStringLiteral("Erro no KanbanListWidget.dropEvent: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger->error(QStringLiteral("Erro no KanbanListWidget.dropEvent"), true);
    }
}

}  // namespace source::plugin::Pomodoro
