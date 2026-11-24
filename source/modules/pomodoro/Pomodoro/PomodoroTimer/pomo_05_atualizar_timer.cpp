#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_05_atualizar_timer.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

void PomodoroTimer::atualizar_timer()
{
    try {
        --tempo_restante;
        progress->setValue(tempo_restante);
        atualizar_display();

        if (tempo_restante <= 0) {
            timer->stop();
            finalizar_ciclo();
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao atualizar timer: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao atualizar timer"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void atualizar_timer(PomodoroTimer &self) { self.atualizar_timer(); } }
