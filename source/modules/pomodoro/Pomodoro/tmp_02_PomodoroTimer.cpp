#include "source/modules/pomodoro/Pomodoro/tmp_02_PomodoroTimer.hpp"

#include <exception>

#include <QCoreApplication>
#include <QTimer>

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {

PomodoroTimer::PomodoroTimer(QWidget *parent)
    : PomodoroTimer(QStringLiteral("simple:tabs"), parent)
{
}

PomodoroTimer::PomodoroTimer(const QString &ui_layout_mode_key, QWidget *parent)
    : QWidget(parent),
      logger_(PomodoroTimerInternal::logger()),
      gerenciador_botoes_(this),
      sound_mgr_(compat::SoundManager::instance()),
      ui_layout_mode_key_(PomodoroTimerInternal::normalized_ui_layout_key(ui_layout_mode_key))
{
    try {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &PomodoroTimer::atualizar_timer);

        setup_ui();

        if (auto *app = QCoreApplication::instance()) {
            app->installEventFilter(this);
        }

        _aplicar_tema_dinamico();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao inicializar PomodoroTimer: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao inicializar PomodoroTimer"), true);
    }
}

}  // namespace source::plugin::Pomodoro
