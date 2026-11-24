#include "source/modules/audiolivro/metodoLeitor/lt_01_TTSThread.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"

#include <QDir>
#include <QFile>
#include <QMutexLocker>
#include <QTemporaryFile>

namespace source::plugin::Audiolivro {

TTSThread::TTSThread(QString texto, int velocidade, double volume, QString voz, QObject *parent)
    : QThread(parent),
      texto_(std::move(texto)),
      velocidade_(velocidade),
      volume_(volume),
      voz_(std::move(voz))
{
}

TTSThread::~TTSThread()
{
    stop();
    if (!wait(2000)) {
        requestInterruption();
        quit();
        wait(500);
    }
}

void TTSThread::stop()
{
    requestInterruption();
    QMutexLocker locker(&mutex_);
    should_stop_ = true;
    if (process_ != nullptr) {
        process_->kill();
    }
}

void TTSThread::run()
{
    const QString temp_dir = audiolivro_persistent_dir();
    QDir().mkpath(temp_dir);

    QTemporaryFile text_file(QDir(temp_dir).filePath(QStringLiteral("audiolivro_tts_XXXXXX.txt")));
    text_file.setAutoRemove(false);
    if (!text_file.open()) {
        emit error(tr("Nao foi possivel criar arquivo temporario para sintese de voz."));
        return;
    }

    text_file.write(texto_.toUtf8());
    const QString text_path = text_file.fileName();
    text_file.close();

    const QString out_path =
        QDir(temp_dir).filePath(QStringLiteral("audiolivro_tts_%1.wav").arg(quintptr(QThread::currentThreadId())));
    const int sapi_rate = qBound(-10, (velocidade_ - 150) / 10, 10);
    const int sapi_volume = qBound(0, int(volume_ * 100.0), 100);

    QString script;
    script += QStringLiteral("& { param($textPath,$outPath,$rate,$volume,$voice) ");
    script += QStringLiteral("Add-Type -AssemblyName System.Speech;");
    script += QStringLiteral("$text=[System.IO.File]::ReadAllText($textPath);");
    script += QStringLiteral("$s=New-Object System.Speech.Synthesis.SpeechSynthesizer;");
    script += QStringLiteral("$s.Rate=[Math]::Max(-10,[Math]::Min(10,[int]$rate));");
    script += QStringLiteral("$s.Volume=[Math]::Max(0,[Math]::Min(100,[int]$volume));");
    if (!voz_.trimmed().isEmpty()) {
        script += QStringLiteral("try{$s.SelectVoice($voice)}catch{Write-Error ('Voz selecionada nao encontrada: '+$voice);exit 2};");
    }
    script += QStringLiteral("$s.SetOutputToWaveFile($outPath);$s.Speak($text);$s.Dispose();");
    script += QStringLiteral("}");

    QProcess process;
    {
        QMutexLocker locker(&mutex_);
        if (should_stop_ || isInterruptionRequested()) {
            QFile::remove(text_path);
            return;
        }
        process_ = &process;
    }

    process.start(QStringLiteral("powershell"),
                  {QStringLiteral("-NoProfile"),
                   QStringLiteral("-ExecutionPolicy"),
                   QStringLiteral("Bypass"),
                   QStringLiteral("-Command"),
                   script,
                   text_path,
                   out_path,
                   QString::number(sapi_rate),
                   QString::number(sapi_volume),
                   voz_});
    process.waitForFinished(-1);

    {
        QMutexLocker locker(&mutex_);
        process_ = nullptr;
    }

    QFile::remove(text_path);
    QFile::remove(out_path);

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        emit error(QString::fromUtf8(process.readAllStandardError()).trimmed());
    }
}

}  // namespace source::plugin::Audiolivro
