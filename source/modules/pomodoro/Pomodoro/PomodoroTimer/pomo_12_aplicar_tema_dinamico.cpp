#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_12_aplicar_tema_dinamico.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

void PomodoroTimer::_aplicar_tema_dinamico()
{
    try {
        auto *app = QCoreApplication::instance();
        if (app == nullptr) {
            return;
        }

        const QColor window_color = qApp->palette().color(QPalette::Window);
        const bool light_theme = is_light_color(window_color);

        for (QLabel *label : {label_status, label_tempo}) {
            if (label != nullptr) {
                label->setPalette(qApp->palette());
                label->setAutoFillBackground(false);
            }
        }

        const auto apply_combo_base_as_window = [&](QComboBox *combo) {
            if (combo == nullptr) {
                return;
            }

            QPalette palette = combo->palette();
            palette.setColor(QPalette::Button, window_color);
            combo->setPalette(palette);

            if (auto *view = combo->view()) {
                QPalette popup_palette = view->palette();
                const QColor popup_base = light_theme ? window_color : window_color.darker(115);
                popup_palette.setColor(QPalette::Base, popup_base);
                popup_palette.setColor(QPalette::AlternateBase, popup_base);
                view->setPalette(popup_palette);
                view->viewport()->setAutoFillBackground(true);
                view->viewport()->setPalette(popup_palette);
            }
        };

        apply_combo_base_as_window(spin_foco);
        apply_combo_base_as_window(spin_descanso);
        apply_combo_base_as_window(spin_ciclos);
        update();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao aplicar tema dinâmico PomodoroTimer: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao aplicar tema dinâmico PomodoroTimer"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void aplicar_tema_dinamico(PomodoroTimer &self) { self._aplicar_tema_dinamico(); } }
