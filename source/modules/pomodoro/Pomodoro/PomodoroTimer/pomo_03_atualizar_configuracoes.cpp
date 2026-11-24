#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_03_atualizar_configuracoes.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

int PomodoroTimer::obter_ciclos_permitidos_persistidos()
{
    return carregar_ciclos_permitidos_persistidos();
}

void PomodoroTimer::atualizar_configuracoes()
{
    try {
        const int novo_foco = spin_foco->currentText().toInt() * 60;
        const int novo_descanso_curto = spin_descanso->currentText().toInt() * 60;
        const int novos_ciclos = qMax(1, spin_ciclos->currentText().toInt());
        const int ciclos_atuais = qMax(1, ciclos_permitidos);

        const int total_antigo =
            em_foco ? tempo_foco
                    : ((ciclos_completados % ciclos_atuais == 0) ? tempo_descanso_longo : tempo_descanso_curto);

        const int decorrido = qMax(0, total_antigo - tempo_restante);
        const bool ciclos_mudaram = (novos_ciclos != ciclos_atuais);
        if (ciclos_mudaram) {
            mismatch_acknowledged_ = false;
        }

        tempo_foco = novo_foco;
        tempo_descanso_curto = novo_descanso_curto;
        ciclos_permitidos = novos_ciclos;

        const int total_novo =
            em_foco ? tempo_foco
                    : ((ciclos_completados % novos_ciclos == 0) ? tempo_descanso_longo : tempo_descanso_curto);

        tempo_restante = qMax(total_novo - decorrido, 0);
        progress->setMaximum(total_novo);
        progress->setValue(tempo_restante);
        atualizar_display();
        if (label_ciclos != nullptr) {
            label_ciclos->setText(
                QCoreApplication::translate("PomodoroPlugin", "Ciclo 1/{total}:")
                    .replace(QStringLiteral("{total}"), QString::number(ciclos_permitidos)));
        }
        if (label_status != nullptr) {
            if (em_foco) {
                if (ciclos_completados >= ciclos_permitidos) {
                    label_status->setText(
                        QCoreApplication::translate("PomodoroPlugin", "✅ Meta concluida | Ciclo: {ciclo}/{total}")
                            .replace(QStringLiteral("{ciclo}"), QString::number(ciclos_permitidos))
                            .replace(QStringLiteral("{total}"), QString::number(ciclos_permitidos)));
                } else {
                    const int ciclo_atual = (ciclos_completados % ciclos_permitidos) + 1;
                    label_status->setText(
                        QCoreApplication::translate("PomodoroPlugin", "🎯 Modo: Foco | Ciclo: {ciclo}/{total}")
                            .replace(QStringLiteral("{ciclo}"), QString::number(ciclo_atual))
                            .replace(QStringLiteral("{total}"), QString::number(ciclos_permitidos)));
                }
            } else {
                int ciclo_concluido = ciclos_completados % ciclos_permitidos;
                if (ciclo_concluido == 0 && ciclos_completados > 0) {
                    ciclo_concluido = ciclos_permitidos;
                } else if (ciclo_concluido == 0 && ciclos_completados == 0) {
                    ciclo_concluido = 1;
                }
                const QString tipo_descanso =
                    (ciclos_completados % ciclos_permitidos == 0)
                        ? QCoreApplication::translate("PomodoroPlugin", "Longo")
                        : QCoreApplication::translate("PomodoroPlugin", "Curto");
                label_status->setText(
                    QCoreApplication::translate("PomodoroPlugin", "☕ Modo: Descanso {tipo} | Ciclo: {ciclo}/{total}")
                        .replace(QStringLiteral("{tipo}"), tipo_descanso)
                        .replace(QStringLiteral("{ciclo}"), QString::number(ciclo_concluido))
                        .replace(QStringLiteral("{total}"), QString::number(ciclos_permitidos)));
            }
        }

        // Otimização de desempenho: resposta visual imediata (0ms); persistência e broadcast inter-módulos debounced (150ms)
        if (config_sync_timer_ == nullptr) {
            config_sync_timer_ = new QTimer(this);
            config_sync_timer_->setSingleShot(true);
            connect(config_sync_timer_, &QTimer::timeout, this, [this]() {
                salvar_configuracoes_timer_persistidas(ciclos_permitidos, tempo_foco, tempo_descanso_curto);
                if (last_broadcasted_ciclos_ != ciclos_permitidos) {
                    last_broadcasted_ciclos_ = ciclos_permitidos;
                    mismatch_acknowledged_ = false;
                    emit ciclos_permitidos_alterados(ciclos_permitidos);
                    if (compat::host_bridge() != nullptr) {
                        compat::host_bridge()->notify_default_cycle_count_changed(ciclos_permitidos);
                    }
                }
            });
        }
        config_sync_timer_->start(150);
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao atualizar configurações do timer: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao atualizar configurações do timer"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void atualizar_configuracoes(PomodoroTimer &self) { self.atualizar_configuracoes(); } }
