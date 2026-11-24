#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_06_atualizar_display.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

void PomodoroTimer::atualizar_display()
{
    try {
        const int minutos = tempo_restante / 60;
        const int segundos = tempo_restante % 60;
        label_tempo->setText(QStringLiteral("%1:%2")
                                 .arg(minutos, 2, 10, QLatin1Char('0'))
                                 .arg(segundos, 2, 10, QLatin1Char('0')));
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao atualizar display do timer: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao atualizar display do timer"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void atualizar_display(PomodoroTimer &self) { self.atualizar_display(); } }
