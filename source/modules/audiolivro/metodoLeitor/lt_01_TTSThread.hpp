#pragma once

#include <QMutex>
#include <QProcess>
#include <QThread>

namespace source::plugin::Audiolivro {

class TTSThread : public QThread
{
    Q_OBJECT

public:
    explicit TTSThread(QString texto,
                       int velocidade = 150,
                       double volume = 1.0,
                       QString voz = QString(),
                       QObject *parent = nullptr);
    ~TTSThread() override;

    void stop();

signals:
    void error(const QString &message);

protected:
    void run() override;

private:
    QString texto_;
    int velocidade_ = 150;
    double volume_ = 1.0;
    QString voz_;
    bool should_stop_ = false;
    QMutex mutex_;
    QProcess *process_ = nullptr;
};

}  // namespace source::plugin::Audiolivro
