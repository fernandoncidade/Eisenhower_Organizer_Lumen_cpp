#include "source/modules/audiolivro/metodoLeitor/EdgeTTSWithTimestamps/ettswt_02_split_text.hpp"

#include <QRegularExpression>

namespace source::plugin::Audiolivro {

QStringList EdgeTTSWithTimestamps::split_text(const QString &text, int max_chars) const
{
    QStringList chunks;
    const QString source = text.trimmed();
    if (source.isEmpty()) {
        return chunks;
    }

    auto append_range = [&chunks, &source, max_chars](int start, int end) {
        start = qBound(0, start, source.size());
        end = qBound(start, end, source.size());
        while (start < end && source.at(start).isSpace()) {
            ++start;
        }
        while (end > start && source.at(end - 1).isSpace()) {
            --end;
        }
        if (start >= end) {
            return;
        }

        for (int pos = start; pos < end; pos += max_chars) {
            chunks.append(source.mid(pos, qMin(max_chars, end - pos)));
        }
    };

    QRegularExpression paragraph_break(QStringLiteral("\\n\\s*\\n"));
    QRegularExpressionMatchIterator it = paragraph_break.globalMatch(source);
    int current_start = -1;
    int current_end = -1;
    int segment_start = 0;

    auto consume_paragraph = [&](int raw_start, int raw_end) {
        int paragraph_start = raw_start;
        int paragraph_end = raw_end;
        while (paragraph_start < paragraph_end && source.at(paragraph_start).isSpace()) {
            ++paragraph_start;
        }
        while (paragraph_end > paragraph_start && source.at(paragraph_end - 1).isSpace()) {
            --paragraph_end;
        }
        if (paragraph_start >= paragraph_end) {
            return;
        }

        if (current_start < 0) {
            current_start = paragraph_start;
            current_end = paragraph_end;
            return;
        }

        if (paragraph_end - current_start <= max_chars) {
            current_end = paragraph_end;
            return;
        }

        append_range(current_start, current_end);
        current_start = paragraph_start;
        current_end = paragraph_end;
    };

    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        consume_paragraph(segment_start, match.capturedStart());
        segment_start = match.capturedEnd();
    }
    consume_paragraph(segment_start, source.size());

    if (current_start >= 0) {
        append_range(current_start, current_end);
    }

    QStringList cleaned;
    cleaned.reserve(chunks.size());
    for (const QString &chunk : chunks) {
        const QString trimmed = chunk.trimmed();
        if (!trimmed.isEmpty()) {
            cleaned.append(trimmed);
        }
    }
    return cleaned;
}

}  // namespace source::plugin::Audiolivro
