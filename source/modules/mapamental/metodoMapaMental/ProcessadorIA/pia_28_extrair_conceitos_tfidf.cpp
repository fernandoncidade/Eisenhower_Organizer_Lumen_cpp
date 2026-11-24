#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_28_extrair_conceitos_tfidf.hpp"

#include <algorithm>

#include <QRegularExpression>
#include <QSet>

namespace source::plugin::MapaMental {
namespace {

QString simplified_token(QString token)
{
    token = token.toCaseFolded();
    token.remove(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}_-]")));
    return token.trimmed();
}

QSet<QString> stopwords()
{
    return {
        QStringLiteral("para"), QStringLiteral("como"), QStringLiteral("com"), QStringLiteral("que"),
        QStringLiteral("uma"), QStringLiteral("uns"), QStringLiteral("das"), QStringLiteral("dos"),
        QStringLiteral("por"), QStringLiteral("sobre"), QStringLiteral("entre"), QStringLiteral("esta"),
        QStringLiteral("este"), QStringLiteral("isso"), QStringLiteral("mais"), QStringLiteral("menos"),
        QStringLiteral("the"), QStringLiteral("and"), QStringLiteral("for"), QStringLiteral("with"),
        QStringLiteral("from"), QStringLiteral("that"), QStringLiteral("this"), QStringLiteral("into"),
        QStringLiteral("your"), QStringLiteral("their"), QStringLiteral("when"), QStringLiteral("were"),
        QStringLiteral("been"), QStringLiteral("are"), QStringLiteral("was"),
    };
}

}  // namespace

QVariantList ProcessadorIA::extrair_conceitos_tfidf(const QString &texto, int max_conceitos) const
{
    QHash<QString, int> freq;
    const QSet<QString> ignored = stopwords();
    const QStringList raw_tokens = texto.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    for (const QString &raw : raw_tokens) {
        const QString token = simplified_token(raw);
        if (token.size() < 4 || ignored.contains(token)) {
            continue;
        }
        ++freq[token];
    }

    QList<QPair<QString, int>> ranked;
    ranked.reserve(freq.size());
    for (auto it = freq.constBegin(); it != freq.constEnd(); ++it) {
        ranked.append({it.key(), it.value()});
    }
    std::sort(ranked.begin(), ranked.end(), [](const auto &a, const auto &b) {
        if (a.second == b.second) {
            return a.first < b.first;
        }
        return a.second > b.second;
    });

    QVariantList conceitos;
    const int limit = qMin(max_conceitos, ranked.size());
    for (int i = 0; i < limit; ++i) {
        QVariantMap item;
        item.insert(QStringLiteral("texto"), ranked.at(i).first);
        item.insert(QStringLiteral("peso"), ranked.at(i).second);
        conceitos.append(item);
    }
    return conceitos;
}

}  // namespace source::plugin::MapaMental
