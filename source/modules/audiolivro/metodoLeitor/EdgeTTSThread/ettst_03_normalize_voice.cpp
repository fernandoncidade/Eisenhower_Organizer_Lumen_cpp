#include "source/modules/audiolivro/metodoLeitor/EdgeTTSThread/ettst_03_normalize_voice.hpp"

#include <QRegularExpression>

namespace source::plugin::Audiolivro {

QString EdgeTTSThread::normalize_voice(const QString &voz) const
{
    const QString trimmed = voz.trimmed();
    if (trimmed.isEmpty()) {
        return QStringLiteral("pt-BR-AntonioNeural");
    }

    static const QRegularExpression edge_voice_id(
        QStringLiteral("([a-z]{2}-[A-Z]{2}-[A-Za-z0-9]+Neural)"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = edge_voice_id.match(trimmed);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    if (trimmed.startsWith(QStringLiteral("Microsoft "), Qt::CaseInsensitive)
        || trimmed.contains(QStringLiteral(" Desktop"), Qt::CaseInsensitive)) {
        return trimmed;
    }

    if (trimmed.contains(QStringLiteral("Maria"), Qt::CaseInsensitive)) {
        return QStringLiteral("pt-BR-FranciscaNeural");
    }

    if (trimmed.contains(QStringLiteral("Thalita"), Qt::CaseInsensitive)) {
        return QStringLiteral("pt-BR-ThalitaMultilingualNeural");
    }

    if (trimmed.contains(QStringLiteral("Antonio"), Qt::CaseInsensitive)
        || trimmed.contains(QStringLiteral("Antônio"), Qt::CaseInsensitive)) {
        return QStringLiteral("pt-BR-AntonioNeural");
    }

    return trimmed;
}

}  // namespace source::plugin::Audiolivro
