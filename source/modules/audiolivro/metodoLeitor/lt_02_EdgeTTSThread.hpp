#pragma once

#include <QMutex>
#include <QProcess>
#include <QThread>

namespace source::plugin::Audiolivro {

class EdgeTTSThread : public QThread
{
    Q_OBJECT

public:
    explicit EdgeTTSThread(QString texto,
                           QString voz = QStringLiteral("pt-BR-AntonioNeural"),
                           int rate_pct = 0,
                           int volume_pct = 0,
                           QString outdir = QString(),
                           QObject *parent = nullptr);
    ~EdgeTTSThread() override;

    void stop();
    QStringList split_text(const QString &text, int max_chars = 3500) const;
    QString normalize_voice(const QString &voz) const;

signals:
    void chunk_ready(const QString &audio_path);
    void error(const QString &message);

protected:
    void run() override;

private:
    QString texto_;
    QString voz_;
    int rate_pct_ = 0;
    int volume_pct_ = 0;
    QString outdir_;
    bool should_stop_ = false;
    QMutex mutex_;
    QProcess *process_ = nullptr;
};

}  // namespace source::plugin::Audiolivro
