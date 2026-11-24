#include "source/modules/audiolivro/metodoLeitor/EdgeTTSWithTimestamps/ettswt_06_run.hpp"

#include "source/modules/audiolivro/metodoLeitor/edge_voice_candidates.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_07_EdgeTTSClient.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QUuid>

#include <algorithm>

namespace {

struct ChunkToken {
    int start = 0;
    int length = 0;
    QString text;
};

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
                                 .filePath(QStringLiteral("audiolivro_timestamps_fallback_XXXXXX.txt")));
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

QVector<ChunkToken> source_tokens_for_chunk(const QString &chunk)
{
    QVector<ChunkToken> tokens;
    int token_start = -1;
    for (int i = 0; i <= chunk.size(); ++i) {
        const bool in_token = i < chunk.size() && chunk.at(i).isLetterOrNumber();
        if (in_token && token_start < 0) {
            token_start = i;
            continue;
        }

        if ((!in_token || i == chunk.size()) && token_start >= 0) {
            const int token_length = i - token_start;
            tokens.append(ChunkToken{token_start, token_length, chunk.mid(token_start, token_length)});
            token_start = -1;
        }
    }
    return tokens;
}

QString normalized_boundary_word(QString text)
{
    text = text.trimmed();
    while (!text.isEmpty() && !text.front().isLetterOrNumber()) {
        text.remove(0, 1);
    }
    while (!text.isEmpty() && !text.back().isLetterOrNumber()) {
        text.chop(1);
    }
    return text.toCaseFolded();
}

bool token_matches_boundary_word(const ChunkToken &token, const QString &word)
{
    const QString normalized_word = normalized_boundary_word(word);
    if (normalized_word.isEmpty()) {
        return false;
    }

    return normalized_boundary_word(token.text) == normalized_word;
}

bool token_has_digit(const ChunkToken &token)
{
    for (const QChar ch : token.text) {
        if (ch.isDigit()) {
            return true;
        }
    }
    return false;
}

bool token_has_letter(const ChunkToken &token)
{
    for (const QChar ch : token.text) {
        if (ch.isLetter()) {
            return true;
        }
    }
    return false;
}

bool token_has_alnum(const ChunkToken &token)
{
    for (const QChar ch : token.text) {
        if (ch.isLetterOrNumber()) {
            return true;
        }
    }
    return false;
}

QString timestamp_word(const QVariantList &timestamps, int index)
{
    if (index < 0 || index >= timestamps.size()) {
        return QString();
    }
    return timestamps.at(index).toMap().value(QStringLiteral("text")).toString();
}

bool token_can_consume_multiple_boundaries(const ChunkToken &token)
{
    return !token_has_alnum(token)
           || token.text.contains(QLatin1Char('-'))
           || token.text.contains(QLatin1Char('_'))
           || token.text.contains(QLatin1Char('.'))
           || token.text.contains(QLatin1Char('/'));
}

double token_skip_cost(const ChunkToken &token)
{
    if (!token_has_alnum(token)) {
        return 0.05;
    }
    if (token_has_digit(token)) {
        return 1.8;
    }
    return 4.0;
}

double skipped_tokens_cost(const QVector<ChunkToken> &tokens, int first, int last_exclusive)
{
    double cost = 0.0;
    for (int i = qMax(0, first); i < qMin(tokens.size(), last_exclusive); ++i) {
        cost += token_skip_cost(tokens.at(i));
    }
    return cost;
}

double token_boundary_alignment_cost(const ChunkToken &token, const QString &word)
{
    const QString normalized_token = normalized_boundary_word(token.text);
    const QString normalized_word = normalized_boundary_word(word);
    if (normalized_word.isEmpty()) {
        return 8.0;
    }

    if (normalized_token == normalized_word) {
        return 0.0;
    }

    if (!normalized_token.isEmpty()
        && (normalized_token.contains(normalized_word) || normalized_word.contains(normalized_token))
        && qMin(normalized_token.size(), normalized_word.size()) >= 3) {
        return 0.35;
    }

    const bool word_has_digit = std::any_of(normalized_word.cbegin(), normalized_word.cend(), [](QChar ch) {
        return ch.isDigit();
    });
    if (token_has_digit(token)) {
        return word_has_digit ? 0.7 : 1.05;
    }
    if (!token_has_alnum(token)) {
        return 1.35;
    }
    if (token_can_consume_multiple_boundaries(token) && token_has_letter(token)) {
        return 2.5;
    }
    return 6.0;
}

QVector<int> align_boundaries_to_tokens(const QVector<ChunkToken> &tokens, const QVariantList &timestamps)
{
    const int token_count = tokens.size();
    const int boundary_count = timestamps.size();
    QVector<int> fallback;
    if (token_count <= 0 || boundary_count <= 0) {
        return fallback;
    }

    constexpr double kInf = 1.0e100;
    constexpr int kLookbackWindow = 96;
    constexpr double kSkippedTokenPenalty = 0.22;

    QVector<QVector<int>> parents(boundary_count, QVector<int>(token_count, -1));
    QVector<double> previous(token_count, kInf);
    QVector<double> current(token_count, kInf);

    const QString first_word = timestamp_word(timestamps, 0);
    for (int token = 0; token < token_count; ++token) {
        previous[token] = skipped_tokens_cost(tokens, 0, token)
                          + kSkippedTokenPenalty * token
                          + token_boundary_alignment_cost(tokens.at(token), first_word);
    }

    for (int boundary = 1; boundary < boundary_count; ++boundary) {
        std::fill(current.begin(), current.end(), kInf);
        const QString word = timestamp_word(timestamps, boundary);
        for (int token = 0; token < token_count; ++token) {
            double best = kInf;
            int best_parent = -1;

            const double same_token_cost = token_can_consume_multiple_boundaries(tokens.at(token)) ? 0.03 : 7.5;
            if (previous.at(token) + same_token_cost < best) {
                best = previous.at(token) + same_token_cost;
                best_parent = token;
            }

            const int first_parent = qMax(0, token - kLookbackWindow);
            for (int parent = first_parent; parent < token; ++parent) {
                const int skipped_count = token - parent - 1;
                const double transition = skipped_tokens_cost(tokens, parent + 1, token)
                                          + kSkippedTokenPenalty * skipped_count;
                const double candidate = previous.at(parent) + transition;
                if (candidate < best) {
                    best = candidate;
                    best_parent = parent;
                }
            }

            current[token] = best + token_boundary_alignment_cost(tokens.at(token), word);
            parents[boundary][token] = best_parent;
        }
        previous.swap(current);
    }

    double best = kInf;
    int best_token = 0;
    for (int token = 0; token < token_count; ++token) {
        const double candidate = previous.at(token) + skipped_tokens_cost(tokens, token + 1, token_count);
        if (candidate < best) {
            best = candidate;
            best_token = token;
        }
    }

    QVector<int> aligned(boundary_count, best_token);
    for (int boundary = boundary_count - 1; boundary > 0; --boundary) {
        best_token = parents.at(boundary).at(best_token);
        if (best_token < 0) {
            best_token = aligned.at(boundary);
        }
        aligned[boundary - 1] = best_token;
    }
    return aligned;
}

int locate_chunk_text_offset(const QString &full_text, const QString &chunk, int search_start, int fallback_offset)
{
    if (chunk.isEmpty()) {
        return qBound(0, fallback_offset, full_text.size());
    }

    const int clamped_search_start = qBound(0, search_start, full_text.size());
    const int fallback = qBound(0, fallback_offset, full_text.size());
    const int max_reasonable_jump = 256;
    const int exact = full_text.indexOf(chunk, clamped_search_start, Qt::CaseSensitive);
    if (exact >= 0 && exact <= clamped_search_start + max_reasonable_jump) {
        return exact;
    }

    return fallback;
}

QVariantList timestamps_with_global_text_offsets(const QVariantList &timestamps, const QString &chunk, int global_offset)
{
    QVariantList adjusted;
    const QVector<ChunkToken> source_tokens = source_tokens_for_chunk(chunk);
    if (source_tokens.isEmpty()) {
        return adjusted;
    }

    const QVector<int> aligned_tokens = align_boundaries_to_tokens(source_tokens, timestamps);
    for (int i = 0; i < timestamps.size(); ++i) {
        QVariantMap timestamp = timestamps.at(i).toMap();
        const int token_index = i < aligned_tokens.size() ? aligned_tokens.at(i) : qMin(i, source_tokens.size() - 1);
        const ChunkToken &token = source_tokens.at(qBound(0, token_index, source_tokens.size() - 1));

        timestamp.insert(QStringLiteral("spoken_text"), timestamp.value(QStringLiteral("text")).toString());
        timestamp.insert(QStringLiteral("text"), token.text);
        timestamp.insert(QStringLiteral("text_offset"), global_offset + token.start);
        timestamp.insert(QStringLiteral("source_length"), token.length);
        adjusted.append(timestamp);
    }

    return adjusted;
}

}  // namespace

namespace source::plugin::Audiolivro {

void EdgeTTSWithTimestamps::run()
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
    int next_text_search_offset = 0;

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
        QVariantList edge_timestamps;
        for (const QString &candidate : voice_candidates) {
            const QString edge_file = QDir(outdir_).filePath(base + QLatin1Char('_') + safe_suffix(candidate) + QStringLiteral(".mp3"));
            ok = EdgeTTSClient::synthesize_to_file(chunk,
                                                   candidate,
                                                   rate_pct_,
                                                   volume_pct_,
                                                   edge_file,
                                                   &edge_error,
                                                   &edge_timestamps);
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

        const int chunk_text_offset = locate_chunk_text_offset(texto_, chunk, next_text_search_offset, next_text_search_offset);
        const QVariantList timestamps = edge_timestamps.isEmpty()
                                            ? build_estimated_timestamps(chunk, chunk_text_offset)
                                            : timestamps_with_global_text_offsets(edge_timestamps, chunk, chunk_text_offset);
        emit chunk_ready(output, timestamps);
        next_text_search_offset = qMax(next_text_search_offset, chunk_text_offset + chunk.size());
    }
}

}  // namespace source::plugin::Audiolivro
