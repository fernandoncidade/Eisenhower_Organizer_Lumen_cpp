#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_08_finalizar_ciclo.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

void PomodoroTimer::finalizar_ciclo()
{
    try {
        const int ciclos_total = qMax(1, ciclos_permitidos);
        bool proxima_etapa_disponivel = false;

        if (em_foco) {
            ++ciclos_completados;
            emit ciclo_completado(QStringLiteral("foco"));

            _executar_alerta(
                QCoreApplication::translate("PomodoroPlugin", "🎉 Ciclo Completo!"),
                QCoreApplication::translate(
                    "PomodoroPlugin",
                    "Parabéns! Você completou um ciclo de foco.\nCiclos hoje: {ciclos}\n\nHora de descansar! 😊")
                    .replace(QStringLiteral("{ciclos}"), QString::number(ciclos_completados)),
                true);

            if (ciclos_completados >= ciclos_total) {
                mismatch_acknowledged_ = false;
                em_foco = true;
                tempo_restante = tempo_foco;
                label_status->setText(
                    QCoreApplication::translate("PomodoroPlugin", "✅ Meta concluída | Ciclo: {ciclo}/{total}")
                        .replace(QStringLiteral("{ciclo}"), QString::number(ciclos_total))
                        .replace(QStringLiteral("{total}"), QString::number(ciclos_total)));
                progress->setMaximum(tempo_foco);
                progress->setValue(tempo_foco);
            } else {
                QString tipo_descanso;
                if (ciclos_completados % ciclos_total == 0) {
                    tempo_restante = tempo_descanso_longo;
                    tipo_descanso = QCoreApplication::translate("PomodoroPlugin", "Longo");
                } else {
                    tempo_restante = tempo_descanso_curto;
                    tipo_descanso = QCoreApplication::translate("PomodoroPlugin", "Curto");
                }

                em_foco = false;
                int ciclo_concluido = ciclos_completados % ciclos_total;
                if (ciclo_concluido == 0) {
                    ciclo_concluido = ciclos_total;
                }

                label_status->setText(
                    QCoreApplication::translate("PomodoroPlugin", "☕ Modo: Descanso {tipo} | Ciclo: {ciclo}/{total}")
                        .replace(QStringLiteral("{tipo}"), tipo_descanso)
                        .replace(QStringLiteral("{ciclo}"), QString::number(ciclo_concluido))
                        .replace(QStringLiteral("{total}"), QString::number(ciclos_total)));
                progress->setMaximum(tempo_restante);
                progress->setValue(tempo_restante);
                proxima_etapa_disponivel = true;
            }
        } else {
            emit ciclo_completado(QStringLiteral("descanso"));

            _executar_alerta(
                QCoreApplication::translate("PomodoroPlugin", "✅ Descanso Completo"),
                QCoreApplication::translate("PomodoroPlugin", "Descanso finalizado!\nVamos para o próximo ciclo de foco! 💪"),
                true);

            em_foco = true;
            tempo_restante = tempo_foco;

            if (ciclos_completados >= ciclos_total) {
                label_status->setText(
                    QCoreApplication::translate("PomodoroPlugin", "✅ Meta concluída | Ciclo: {ciclo}/{total}")
                        .replace(QStringLiteral("{ciclo}"), QString::number(ciclos_total))
                        .replace(QStringLiteral("{total}"), QString::number(ciclos_total)));
            } else {
                const int ciclo_atual = (ciclos_completados % ciclos_total) + 1;
                label_status->setText(
                    QCoreApplication::translate("PomodoroPlugin", "🎯 Modo: Foco | Ciclo: {ciclo}/{total}")
                        .replace(QStringLiteral("{ciclo}"), QString::number(ciclo_atual))
                        .replace(QStringLiteral("{total}"), QString::number(ciclos_total)));
            }

            progress->setMaximum(tempo_foco);
            progress->setValue(tempo_foco);
            proxima_etapa_disponivel = ciclos_completados < ciclos_total;
        }

        atualizar_display();
        const bool iniciar_automaticamente = proxima_etapa_disponivel && chk_modo_automatico != nullptr
            && chk_modo_automatico->isChecked();
        timer_ativo = iniciar_automaticamente;
        if (iniciar_automaticamente) {
            timer->start(1000);
        }
        gerenciador_botoes_.set_button_text(
            btn_iniciar,
            iniciar_automaticamente ? QCoreApplication::translate("PomodoroPlugin", "⏸️ Pausar")
                                    : QCoreApplication::translate("PomodoroPlugin", "▶️ Iniciar"));
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao finalizar ciclo: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao finalizar ciclo"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void finalizar_ciclo(PomodoroTimer &self) { self.finalizar_ciclo(); } }
