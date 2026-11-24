#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_10_resetar_ciclo.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

void PomodoroTimer::resetar_ciclo()
{
    try {
        timer->stop();
        timer_ativo = false;
        em_foco = true;
        ciclos_completados = 0;
        tempo_restante = tempo_foco;
        progress->setMaximum(tempo_foco);
        progress->setValue(tempo_foco);
        atualizar_display();
        mismatch_acknowledged_ = false;
        gerenciador_botoes_.set_button_text(btn_iniciar, QCoreApplication::translate("PomodoroPlugin", "▶️ Iniciar"));
        atualizar_traducoes();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao resetar ciclo: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao resetar ciclo"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void resetar_ciclo(PomodoroTimer &self) { self.resetar_ciclo(); } }
