#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_11_pular_ciclo.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

void PomodoroTimer::pular_ciclo()
{
    try {
        const int ciclos_total = qMax(1, ciclos_permitidos);
        if (em_foco && ciclos_completados >= ciclos_total) {
            return;
        }

        timer->stop();
        tempo_restante = 0;
        finalizar_ciclo();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao pular ciclo: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao pular ciclo"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void pular_ciclo(PomodoroTimer &self) { self.pular_ciclo(); } }
