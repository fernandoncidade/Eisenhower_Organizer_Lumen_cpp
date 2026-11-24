#include "source/modules/audiolivro/metodoLeitor/EdgeTTSThread/ettst_04_run.hpp"

#include "source/modules/audiolivro/metodoLeitor/edge_voice_candidates.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_07_EdgeTTSClient.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QUuid>

namespace {

QString safe_suffix(QString text)
{
    text.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]+")), QStringLiteral("_"));
    return text.left(64);
}

bool generated_file_ok(const QString &outfile)
{
    const QFileInfo info(outfile);
    return info.exists() && info.size() > 0;
}

bool run_windows_sapi(QProcess &process,
                      const QString &text,
                      const QString &outfile,
                      int rate_pct,
                      int volume_pct,
                      const QString &voice)
{
    QFile::remove(outfile);

    QTemporaryFile text_file(QDir(QFileInfo(outfile).absolutePath())
                                 .filePath(QStringLiteral("audiolivro_edge_fallback_XXXXXX.txt")));
    text_file.setAutoRemove(false);
    if (!text_file.open()) {
        return false;
    }

    text_file.write(text.toUtf8());
    const QString text_path = text_file.fileName();
    text_file.close();

    QString script;
    script += QStringLiteral("& { param($textPath,$outPath,$rate,$volume,$voice) ");
    script += QStringLiteral("Add-Type -AssemblyName System.Speech;");
    script += QStringLiteral("$text=[System.IO.File]::ReadAllText($textPath);");
    script += QStringLiteral("$s=New-Object System.Speech.Synthesis.SpeechSynthesizer;");
    script += QStringLiteral("if(-not [string]::IsNullOrWhiteSpace($voice)){try{$s.SelectVoice($voice)}catch{Write-Error ('Voz selecionada nao encontrada: '+$voice);exit 2}};");
    script += QStringLiteral("$s.Rate=[Math]::Max(-10,[Math]::Min(10,[int]$rate));");
    script += QStringLiteral("$s.Volume=[Math]::Max(0,[Math]::Min(100,[int]$volume));");
    script += QStringLiteral("$s.SetOutputToWaveFile($outPath);$s.Speak($text);$s.Dispose();");
    script += QStringLiteral("}");

    const int sapi_rate = qBound(-10, rate_pct / 5, 10);
    const int sapi_volume = qBound(0, 100 + volume_pct, 100);
    process.start(QStringLiteral("powershell"),
                  {QStringLiteral("-NoProfile"),
                   QStringLiteral("-ExecutionPolicy"),
                   QStringLiteral("Bypass"),
                   QStringLiteral("-Command"),
                   script,
                   text_path,
                   outfile,
                   QString::number(sapi_rate),
                   QString::number(sapi_volume),
                   voice});
    if (!process.waitForStarted(3000)) {
        QFile::remove(text_path);
        return false;
    }
    if (!process.waitForFinished(180000)) {
        process.kill();
        process.waitForFinished(1000);
        QFile::remove(text_path);
        return false;
    }
    QFile::remove(text_path);
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0 && generated_file_ok(outfile);
}

}  // namespace

namespace source::plugin::Audiolivro {

void EdgeTTSThread::run()
{
    const QStringList chunks = split_text(texto_);
    const QString voice = normalize_voice(voz_);
    QStringList voice_candidates;
    const bool selected_edge_voice = EdgeTTSClient::is_edge_voice_id(voice);
    if (selected_edge_voice) {
        voice_candidates = EdgeVoiceCandidates::build_voice_candidates(voice);
    } else if (voz_.trimmed().isEmpty()) {
        EdgeVoiceCandidates::append_voice_candidate(voice_candidates, QStringLiteral("pt-BR-AntonioNeural"));
    }
    int index = 0;

    for (const QString &chunk : chunks) {
        {
            QMutexLocker locker(&mutex_);
            if (should_stop_) {
                return;
            }
        }

        const QString base = QStringLiteral("audiolivro_%1_%2").arg(QUuid::createUuid().toString(QUuid::Id128)).arg(index++);
        const QString fallback_file = QDir(outdir_).filePath(base + QStringLiteral(".wav"));

        QProcess process;
        {
            QMutexLocker locker(&mutex_);
            process_ = &process;
        }

        bool ok = false;
        QString output;
        QString edge_error;
        for (const QString &candidate : voice_candidates) {
            const QString edge_file = QDir(outdir_).filePath(base + QLatin1Char('_') + safe_suffix(candidate) + QStringLiteral(".mp3"));
            ok = EdgeTTSClient::synthesize_to_file(chunk,
                                                   candidate,
                                                   rate_pct_,
                                                   volume_pct_,
                                                   edge_file,
                                                   &edge_error);
            if (ok) {
                output = edge_file;
                break;
            }
        }
        if (!ok && !selected_edge_voice) {
            output = fallback_file;
            ok = run_windows_sapi(process,
                                  chunk,
                                  output,
                                  rate_pct_,
                                  volume_pct_,
                                  voz_.trimmed().isEmpty() ? voice : voz_.trimmed());
        }

        {
            QMutexLocker locker(&mutex_);
            process_ = nullptr;
            if (should_stop_) {
                return;
            }
        }

        if (!ok) {
            const QString stderr_text = QString::fromUtf8(process.readAllStandardError()).trimmed();
            if (!stderr_text.isEmpty()) {
                emit error(stderr_text);
            } else if (!edge_error.trimmed().isEmpty()) {
                emit error(edge_error);
            } else {
                emit error(tr("Falha ao gerar audio do texto."));
            }
            return;
        }

        emit chunk_ready(output);
    }
}

}  // namespace source::plugin::Audiolivro
