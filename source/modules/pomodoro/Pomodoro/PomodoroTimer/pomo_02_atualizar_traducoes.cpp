#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_02_atualizar_traducoes.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

#include <QEvent>

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

void PomodoroTimer::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event != nullptr && event->type() == QEvent::LanguageChange) {
        atualizar_traducoes();
    }
}

void PomodoroTimer::atualizar_traducoes()
{
    try {
        const int ciclos_total = qMax(1, ciclos_permitidos);

        if (em_foco) {
            if (ciclos_completados >= ciclos_total) {
                label_status->setText(
                    QCoreApplication::translate("PomodoroPlugin", "✅ Meta concluida | Ciclo: {ciclo}/{total}")
                        .replace(QStringLiteral("{ciclo}"), QString::number(ciclos_total))
                        .replace(QStringLiteral("{total}"), QString::number(ciclos_total)));
            } else {
                const int ciclo_atual = (ciclos_completados % ciclos_total) + 1;
                label_status->setText(
                    QCoreApplication::translate("PomodoroPlugin", "🎯 Modo: Foco | Ciclo: {ciclo}/{total}")
                        .replace(QStringLiteral("{ciclo}"), QString::number(ciclo_atual))
                        .replace(QStringLiteral("{total}"), QString::number(ciclos_total)));
            }
        } else {
            int ciclo_concluido = ciclos_completados % ciclos_total;
            if (ciclo_concluido == 0 && ciclos_completados > 0) {
                ciclo_concluido = ciclos_total;
            } else if (ciclo_concluido == 0 && ciclos_completados == 0) {
                ciclo_concluido = 1;
            }

            const QString tipo_descanso =
                (ciclos_completados % ciclos_total == 0)
                    ? QCoreApplication::translate("PomodoroPlugin", "Longo")
                    : QCoreApplication::translate("PomodoroPlugin", "Curto");

            label_status->setText(
                QCoreApplication::translate("PomodoroPlugin", "☕ Modo: Descanso {tipo} | Ciclo: {ciclo}/{total}")
                    .replace(QStringLiteral("{tipo}"), tipo_descanso)
                    .replace(QStringLiteral("{ciclo}"), QString::number(ciclo_concluido))
                    .replace(QStringLiteral("{total}"), QString::number(ciclos_total)));
        }

        gerenciador_botoes_.set_button_text(
            btn_iniciar,
            timer_ativo ? QCoreApplication::translate("PomodoroPlugin", "⏸️ Pausar")
                        : QCoreApplication::translate("PomodoroPlugin", "▶️ Iniciar"));
        gerenciador_botoes_.set_button_text(btn_resetar, QCoreApplication::translate("PomodoroPlugin", "⏱️ Resetar Relogio"));
        gerenciador_botoes_.set_button_text(btn_pular, QCoreApplication::translate("PomodoroPlugin", "⏭️ Pular"));
        gerenciador_botoes_.set_button_text(btn_resetar_ciclo, QCoreApplication::translate("PomodoroPlugin", "🔄 Resetar Ciclo"));

        if (timer_group != nullptr) {
            timer_group->setTitle(QCoreApplication::translate("PomodoroPlugin", "⏱️ Temporizador"));
        }
        config_group->setTitle(QCoreApplication::translate("PomodoroPlugin", "⚙️ Configuracoes"));
        label_foco->setText(QCoreApplication::translate("PomodoroPlugin", "Foco (min):"));
        label_descanso->setText(QCoreApplication::translate("PomodoroPlugin", "Descanso (min):"));
        label_ciclos->setText(
            QCoreApplication::translate("PomodoroPlugin", "Ciclo 1/{total}:").replace(QStringLiteral("{total}"), QString::number(ciclos_total)));
        chk_modo_automatico->setText(QCoreApplication::translate("PomodoroPlugin", "Modo automatico"));

        int ref_height = input_ref_height_;
        for (QWidget *widget : {static_cast<QWidget *>(btn_iniciar),
                                static_cast<QWidget *>(btn_resetar),
                                static_cast<QWidget *>(btn_resetar_ciclo),
                                static_cast<QWidget *>(btn_pular),
                                static_cast<QWidget *>(spin_foco),
                                static_cast<QWidget *>(spin_descanso),
                                static_cast<QWidget *>(spin_ciclos),
                                static_cast<QWidget *>(label_foco),
                                static_cast<QWidget *>(label_descanso),
                                static_cast<QWidget *>(label_ciclos)}) {
            if (widget != nullptr) {
                ref_height = qMax(ref_height, widget->sizeHint().height());
            }
        }

        if (ref_height > 0) {
            input_ref_height_ = ref_height;

            const auto fix_control_height = [ref_height](QWidget *widget) {
                if (widget == nullptr) {
                    return;
                }

                QSizePolicy policy = widget->sizePolicy();
                policy.setVerticalPolicy(QSizePolicy::Fixed);
                widget->setSizePolicy(policy);
                widget->setFixedHeight(ref_height);
            };

            for (QPushButton *button : {btn_iniciar, btn_resetar, btn_resetar_ciclo, btn_pular}) {
                fix_control_height(button);
            }
            for (QComboBox *combo : {spin_foco, spin_descanso, spin_ciclos}) {
                fix_control_height(combo);
            }
            for (QLabel *label : {label_foco, label_descanso, label_ciclos}) {
                fix_control_height(label);
            }
        }

        if (config_group != nullptr) {
            if (QLayout *config_layout = config_group->layout()) {
                config_layout->invalidate();
            }
            const int config_height = config_group->sizeHint().height();
            if (config_height > 0) {
                config_group->setMinimumHeight(config_height);
            }
            config_group->updateGeometry();
        }

        if (QLayout *root_layout = layout()) {
            root_layout->invalidate();
            const int min_height = root_layout->minimumSize().height();
            if (min_height > 0) {
                setMinimumHeight(min_height);
            }
        }
        updateGeometry();

        _aplicar_tema_dinamico();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao atualizar traduções do PomodoroTimer: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao atualizar traduções do PomodoroTimer"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void atualizar_traducoes(PomodoroTimer &self) { self.atualizar_traducoes(); } }
