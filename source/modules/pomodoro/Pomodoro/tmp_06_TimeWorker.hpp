#pragma once

#include <QObject>

#include "source/utils/LogManager.hpp"

namespace source::plugin::Pomodoro {

class _TimeWorker : public QObject {
    Q_OBJECT

public:
    explicit _TimeWorker(QObject *parent = nullptr);

public slots:
    void on_pomodoro(const QString &tipo);
    void stop();

signals:
    void pomodoro_completed(const QString &tipo);

private:
    source::utils::LogManager *logger_ = nullptr;
    bool running_ = true;
};

}  // namespace source::plugin::Pomodoro
