#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_09_resetar_timer.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

void PomodoroTimer::resetar_timer()
{
    try {
        const int ciclos_total = qMax(1, ciclos_permitidos);
        timer->stop();
        timer_ativo = false;
        tempo_restante =
            em_foco ? tempo_foco : ((ciclos_completados % ciclos_total == 0) ? tempo_descanso_longo : tempo_descanso_curto);
        progress->setMaximum(tempo_restante);
        progress->setValue(tempo_restante);
        atualizar_display();
        mismatch_acknowledged_ = false;
        gerenciador_botoes_.set_button_text(btn_iniciar, QCoreApplication::translate("PomodoroPlugin", "▶️ Iniciar"));
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao resetar timer: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao resetar timer"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void resetar_timer(PomodoroTimer &self) { self.resetar_timer(); } }
