#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_22_gerar_resumo_contextual.hpp"

#include <QtGlobal>

#include <algorithm>
#include <cmath>

#include <QHash>
#include <QRegularExpression>
#include <QSet>

namespace source::plugin::MapaMental::ProcessadorIAFns {
namespace {

struct Unit {
    QString text;
    QSet<QString> tokens;
    int index = 0;
    int length = 0;
    double relevance = 0.0;
};

const QSet<QString> &stopwords()
{
    static const QSet<QString> words = {
        QStringLiteral("a"),     QStringLiteral("ao"),    QStringLiteral("aos"),
        QStringLiteral("as"),    QStringLiteral("com"),   QStringLiteral("como"),
        QStringLiteral("da"),    QStringLiteral("das"),   QStringLiteral("de"),
        QStringLiteral("do"),    QStringLiteral("dos"),   QStringLiteral("e"),
        QStringLiteral("ela"),   QStringLiteral("ele"),   QStringLiteral("eles"),
        QStringLiteral("em"),    QStringLiteral("entre"), QStringLiteral("era"),
        QStringLiteral("essa"),  QStringLiteral("esse"),  QStringLiteral("esta"),
        QStringLiteral("este"),  QStringLiteral("foi"),   QStringLiteral("mais"),
        QStringLiteral("mas"),   QStringLiteral("menos"), QStringLiteral("na"),
        QStringLiteral("nas"),   QStringLiteral("no"),    QStringLiteral("nos"),
        QStringLiteral("o"),     QStringLiteral("os"),    QStringLiteral("ou"),
        QStringLiteral("para"),  QStringLiteral("pela"),  QStringLiteral("pelo"),
        QStringLiteral("por"),   QStringLiteral("porque"),QStringLiteral("que"),
        QStringLiteral("se"),    QStringLiteral("sem"),   QStringLiteral("ser"),
        QStringLiteral("sua"),   QStringLiteral("suas"),  QStringLiteral("tem"),
        QStringLiteral("um"),    QStringLiteral("uma"),   QStringLiteral("the"),
        QStringLiteral("and"),   QStringLiteral("are"),   QStringLiteral("as"),
        QStringLiteral("at"),    QStringLiteral("be"),    QStringLiteral("been"),
        QStringLiteral("by"),    QStringLiteral("for"),   QStringLiteral("from"),
        QStringLiteral("has"),   QStringLiteral("in"),    QStringLiteral("into"),
        QStringLiteral("is"),    QStringLiteral("it"),    QStringLiteral("of"),
        QStringLiteral("on"),    QStringLiteral("or"),    QStringLiteral("that"),
        QStringLiteral("their"), QStringLiteral("this"),  QStringLiteral("to"),
        QStringLiteral("was"),   QStringLiteral("were"),  QStringLiteral("when"),
        QStringLiteral("with"),  QStringLiteral("your"),
    };
    return words;
}

QString normalized_token(QString token)
{
    token = token.toCaseFolded();
    token.remove(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]") ));
    return token.trimmed();
}

QSet<QString> meaningful_tokens(const QString &text)
{
    QSet<QString> result;
    const QStringList raw = text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    for (const QString &part : raw) {
        const QString token = normalized_token(part);
        if (token.size() >= 3 && !stopwords().contains(token)) {
            result.insert(token);
        }
    }
    return result;
}

QStringList split_sentences(const QString &paragraph)
{
    static const QRegularExpression boundary(
        QStringLiteral("(?<=[\\.!?])\\s+(?=[\\p{Lu}\\p{N}\\\"“‘(])"));
    return paragraph.split(boundary, Qt::SkipEmptyParts);
}

bool looks_like_noise(const QString &text)
{
    const QString simplified = text.simplified().trimmed();
    if (simplified.size() < 24 || simplified.size() > 900) {
        return true;
    }
    if (QRegularExpression(QStringLiteral("^(?:p[aá]gina\\s+)?\\d{1,4}$"),
                           QRegularExpression::CaseInsensitiveOption)
            .match(simplified)
            .hasMatch()) {
        return true;
    }
    if (simplified.contains(QRegularExpression(QStringLiteral("(?:ISBN|https?://|www\\.)"),
                                                QRegularExpression::CaseInsensitiveOption))) {
        return true;
    }
    if (simplified.contains(QRegularExpression(QStringLiteral("\\.{4,}\\s*\\d+\\s*$")))) {
        return true;
    }
    const int letters = simplified.count(QRegularExpression(QStringLiteral("\\p{L}")));
    return letters < simplified.size() / 3;
}

QVector<Unit> extract_units(const QString &text)
{
    QVector<Unit> units;
    const QStringList lines = text.split(QRegularExpression(QStringLiteral("[\\r\\n]+")),
                                         Qt::SkipEmptyParts);
    QString paragraph;
    int index = 0;
    static const QRegularExpression list_prefix(QStringLiteral("^(?:[-*•]|\\d+[.)])\\s+"));

    const auto append_text = [&](QString candidate) {
        candidate = candidate.simplified().trimmed();
        candidate.remove(list_prefix);
        if (looks_like_noise(candidate)) {
            return;
        }
        const QSet<QString> tokens = meaningful_tokens(candidate);
        if (tokens.size() < 3) {
            return;
        }
        units.push_back(Unit{candidate,
                             tokens,
                             index++,
                             static_cast<int>(candidate.size()),
                             0.0});
    };

    const auto flush = [&]() {
        if (!paragraph.trimmed().isEmpty()) {
            const QStringList sentences = split_sentences(paragraph);
            for (const QString &sentence : sentences) {
                append_text(sentence);
            }
        }
        paragraph.clear();
    };

    for (const QString &raw_line : lines) {
        const QString line = raw_line.simplified().trimmed();
        if (line.isEmpty()) {
            flush();
            continue;
        }
        if (list_prefix.match(line).hasMatch()) {
            flush();
            append_text(line);
        } else {
            if (!paragraph.isEmpty()) {
                paragraph.append(QLatin1Char(' '));
            }
            paragraph.append(line);
        }
    }
    flush();
    return units;
}

double jaccard(const QSet<QString> &a, const QSet<QString> &b)
{
    if (a.isEmpty() || b.isEmpty()) {
        return 0.0;
    }
    int intersection = 0;
    for (const QString &token : a) {
        if (b.contains(token)) {
            ++intersection;
        }
    }
    const int union_size = a.size() + b.size() - intersection;
    return union_size > 0 ? static_cast<double>(intersection) / union_size : 0.0;
}

QString normalized_sentence_key(QString text)
{
    text = text.toCaseFolded();
    text.remove(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")));
    return text.left(240);
}

void calculate_relevance(QVector<Unit> &units, const QString &context)
{
    QHash<QString, int> frequency;
    QHash<QString, int> sentence_repetitions;
    for (const Unit &unit : units) {
        for (const QString &token : unit.tokens) {
            ++frequency[token];
        }
        ++sentence_repetitions[normalized_sentence_key(unit.text)];
    }

    const QSet<QString> context_tokens = meaningful_tokens(context);
    static const QRegularExpression discourse_cue(
        QStringLiteral("\\b(?:objetivo|principal|essencial|define|demonstra|conclui|portanto|resultado|"
                       "evid[eê]ncia|implica|causa|consequ[eê]ncia|recomenda|prop[oõ]e|mostra|"
                       "purpose|main|essential|defines|demonstrates|concludes|therefore|result|"
                       "evidence|implies|causes|recommends|shows)\\b"),
        QRegularExpression::CaseInsensitiveOption);

    const int total = qMax(1, units.size());
    for (Unit &unit : units) {
        double lexical = 0.0;
        double context_bonus = 0.0;
        for (const QString &token : unit.tokens) {
            lexical += 1.0 + std::log1p(static_cast<double>(frequency.value(token)));
            if (context_tokens.contains(token)) {
                context_bonus += 1.4;
            }
        }
        lexical /= std::sqrt(static_cast<double>(qMax(1, unit.tokens.size())));

        const double relative_position = static_cast<double>(unit.index) / total;
        const double position_bonus = unit.index < 2 ? 1.5 : (relative_position > 0.88 ? 0.55 : 0.0);
        const double cue_bonus = discourse_cue.match(unit.text).hasMatch() ? 1.25 : 0.0;
        const double length_factor = unit.length < 45 ? 0.55
                                     : (unit.length <= 340 ? 1.0 : 0.82);
        const bool dialogue = unit.text.startsWith(QChar(0x2014))
                              || unit.text.contains(QStringLiteral(": —"));
        const double narrative_factor = dialogue && cue_bonus == 0.0 ? 0.72 : 1.0;
        const int punctuation = unit.text.count(QRegularExpression(QStringLiteral("[.!?;:]")));
        const double structure_factor = unit.length > 260 && punctuation < 2 ? 0.62 : 1.0;
        const double aside_factor = unit.text.startsWith(QLatin1Char('(')) ? 0.72 : 1.0;
        const int repetitions = sentence_repetitions.value(normalized_sentence_key(unit.text), 1);
        const double repetition_penalty = repetitions > 2 ? 1.0 / repetitions : 1.0;
        unit.relevance = (lexical + context_bonus + position_bonus + cue_bonus)
                         * length_factor * narrative_factor * structure_factor * aside_factor
                         * repetition_penalty;
    }
}

QString truncate_at_boundary(QString text, int max_chars)
{
    text = text.simplified().trimmed();
    if (text.size() <= max_chars) {
        return text;
    }
    int cut = text.lastIndexOf(QRegularExpression(QStringLiteral("[.!?]")), max_chars - 1);
    if (cut < max_chars * 2 / 3) {
        cut = text.lastIndexOf(QLatin1Char(' '), max_chars - 1);
    }
    if (cut < 1) {
        cut = max_chars - 1;
    }
    text = text.left(cut + 1).trimmed();
    if (!text.endsWith(QLatin1Char('.')) && !text.endsWith(QLatin1Char('!'))
        && !text.endsWith(QLatin1Char('?'))) {
        text.append(QChar(0x2026));
    }
    return text;
}

}  // namespace

QString gerar_resumo_contextual(const QString &texto, int max_chars, const QString &contexto)
{
    const QString cleaned = texto.simplified().trimmed();
    if (cleaned.isEmpty() || max_chars <= 0) {
        return {};
    }
    if (cleaned.size() <= max_chars) {
        return cleaned;
    }

    QVector<Unit> units = extract_units(texto);
    if (units.isEmpty()) {
        return truncate_at_boundary(cleaned, max_chars);
    }
    calculate_relevance(units, contexto);

    QVector<Unit> candidates = units;
    std::sort(candidates.begin(), candidates.end(), [](const Unit &a, const Unit &b) {
        if (qFuzzyCompare(a.relevance, b.relevance)) {
            return a.index < b.index;
        }
        return a.relevance > b.relevance;
    });

    QVector<Unit> selected;
    const int max_units = qBound(2, max_chars / 150, 8);
    int current_length = 0;
    while (!candidates.isEmpty() && selected.size() < max_units) {
        int best_index = -1;
        double best_mmr = -1e9;
        for (int i = 0; i < candidates.size(); ++i) {
            const Unit &candidate = candidates.at(i);
            const int added = candidate.length + (selected.isEmpty() ? 0 : 1);
            if (current_length + added > max_chars && !selected.isEmpty()) {
                continue;
            }
            double maximum_similarity = 0.0;
            for (const Unit &chosen : selected) {
                maximum_similarity = qMax(maximum_similarity,
                                          jaccard(candidate.tokens, chosen.tokens));
            }
            const double mmr = 0.76 * candidate.relevance - 4.2 * maximum_similarity;
            if (mmr > best_mmr) {
                best_mmr = mmr;
                best_index = i;
            }
        }
        if (best_index < 0) {
            break;
        }
        Unit chosen = candidates.takeAt(best_index);
        bool redundant = false;
        for (const Unit &existing : selected) {
            if (jaccard(chosen.tokens, existing.tokens) > 0.72) {
                redundant = true;
                break;
            }
        }
        if (redundant) {
            continue;
        }
        current_length += chosen.length + (selected.isEmpty() ? 0 : 1);
        selected.push_back(chosen);
    }

    if (selected.isEmpty()) {
        return truncate_at_boundary(cleaned, max_chars);
    }
    std::sort(selected.begin(), selected.end(), [](const Unit &a, const Unit &b) {
        return a.index < b.index;
    });

    QStringList sentences;
    sentences.reserve(selected.size());
    for (const Unit &unit : selected) {
        sentences.append(unit.text.trimmed());
    }
    return truncate_at_boundary(sentences.join(QLatin1Char(' ')), max_chars);
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
