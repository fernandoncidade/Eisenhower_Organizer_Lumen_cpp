#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_13_eventFilter.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

bool PomodoroTimer::eventFilter(QObject *obj, QEvent *event)
{
    try {
        if (event != nullptr) {
            switch (event->type()) {
            case QEvent::ApplicationPaletteChange:
            case QEvent::PaletteChange:
            case QEvent::StyleChange:
                _aplicar_tema_dinamico();
                break;
            case QEvent::LanguageChange:
                atualizar_traducoes();
                break;
            default:
                break;
            }
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro no eventFilter PomodoroTimer: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro no eventFilter PomodoroTimer"), true);
    }

    return QWidget::eventFilter(obj, event);
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { bool eventFilter(PomodoroTimer &self, QObject *obj, QEvent *event) { return self.eventFilter(obj, event); } }
