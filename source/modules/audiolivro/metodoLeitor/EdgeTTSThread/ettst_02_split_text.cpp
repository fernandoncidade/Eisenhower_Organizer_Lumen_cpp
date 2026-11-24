#include "source/modules/audiolivro/metodoLeitor/EdgeTTSThread/ettst_02_split_text.hpp"

#include <QRegularExpression>

namespace source::plugin::Audiolivro {

QStringList EdgeTTSThread::split_text(const QString &text, int max_chars) const
{
    QStringList chunks;
    QString current;
    const QStringList paragraphs = text.split(QRegularExpression(QStringLiteral("\\n\\s*\\n")), Qt::SkipEmptyParts);

    for (const QString &paragraph : paragraphs) {
        const QString normalized = paragraph.simplified();
        if (normalized.isEmpty()) {
            continue;
        }

        if (current.size() + normalized.size() + 2 <= max_chars) {
            if (!current.isEmpty()) {
                current += QStringLiteral("\n\n");
            }
            current += normalized;
            continue;
        }

        if (!current.isEmpty()) {
            chunks.append(current);
            current.clear();
        }

        if (normalized.size() <= max_chars) {
            current = normalized;
            continue;
        }

        for (int pos = 0; pos < normalized.size(); pos += max_chars) {
            chunks.append(normalized.mid(pos, max_chars));
        }
    }

    if (!current.isEmpty()) {
        chunks.append(current);
    }

    return chunks;
}

}  // namespace source::plugin::Audiolivro
