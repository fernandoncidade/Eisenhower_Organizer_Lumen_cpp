#include "source/modules/pomodoro/Pomodoro/tmp_06_TimeWorker.hpp"

namespace source::plugin::Pomodoro {

_TimeWorker::_TimeWorker(QObject *parent)
    : QObject(parent), logger_(source::utils::LogManager::get_logger())
{
}

void _TimeWorker::on_pomodoro(const QString &tipo)
{
    try {
        logger_->debug(QStringLiteral("Worker de tempo recebeu evento de pomodoro: %1").arg(tipo));
        emit pomodoro_completed(tipo);
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro no TimeWorker.on_pomodoro: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro no TimeWorker.on_pomodoro"), true);
    }
}

void _TimeWorker::stop()
{
    running_ = false;
}

}  // namespace source::plugin::Pomodoro
