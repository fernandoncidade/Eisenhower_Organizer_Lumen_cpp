#include "source/modules/pomodoro/Pomodoro/tmp_05_GerenciadorTempo.hpp"

#include <exception>

#include <QSizePolicy>
#include <QThread>
#include <QWidget>

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_02_PomodoroTimer.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_06_TimeWorker.hpp"

namespace source::plugin::Pomodoro {

GerenciadorTempo::GerenciadorTempo(QWidget *parent)
    : QWidget(parent), logger_(GerenciadorTempoInternal::logger())
{
    try {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMinimumHeight(0);
        tempo_thread_ = new QThread();
        tempo_worker_ = new _TimeWorker();
        tempo_worker_->moveToThread(tempo_thread_);
        connect(tempo_worker_, &_TimeWorker::pomodoro_completed, this, &GerenciadorTempo::registrar_pomodoro);
        tempo_thread_->start();

        setup_ui();
        _setup_integracao_eisenhower();
        installEventFilter(this);

        _aplicar_tema_dinamico_inputs();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao inicializar GerenciadorTempo: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao inicializar GerenciadorTempo"), true);
    }
}

GerenciadorTempo::~GerenciadorTempo()
{
    cleanup();
}

}  // namespace source::plugin::Pomodoro
