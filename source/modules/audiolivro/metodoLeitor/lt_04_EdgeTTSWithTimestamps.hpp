#pragma once

#include "source/modules/audiolivro/metodoLeitor/lt_05_WordTimestamp.hpp"

#include <QMutex>
#include <QProcess>
#include <QThread>
#include <QVariantList>

namespace source::plugin::Audiolivro {

class EdgeTTSWithTimestamps : public QThread
{
    Q_OBJECT

public:
    explicit EdgeTTSWithTimestamps(QString texto,
                                   QString voz = QStringLiteral("pt-BR-AntonioNeural"),
                                   int rate_pct = 0,
                                   int volume_pct = 0,
                                   QString outdir = QString(),
                                   QObject *parent = nullptr);
    ~EdgeTTSWithTimestamps() override;

    void stop();
    QStringList split_text(const QString &text, int max_chars = 3500) const;
    QString normalize_voice(const QString &voz) const;
    int calculate_text_offset(const QString &word, const QString &segment, int last_offset) const;
    WordTimestamp find_word_in_full_text(const QString &word, const QString &full_text, int start) const;

signals:
    void chunk_ready(const QString &audio_path, const QVariantList &timestamps);
    void error(const QString &message);

protected:
    void run() override;

private:
    QVariantList build_estimated_timestamps(const QString &segment, int global_offset) const;

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
