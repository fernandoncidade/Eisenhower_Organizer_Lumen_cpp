#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_10_preprocessar_linhas.hpp"

#include <QRegularExpression>

#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_13_remover_acentos.hpp"

namespace source::plugin::MapaMental {
namespace {

QString normalizar_linha(QString texto)
{
    texto = texto.trimmed();
    texto.remove(QRegularExpression(QStringLiteral("^#{1,6}\\s+")));
    return ProcessadorIAFns::remover_acentos(texto).toCaseFolded().simplified().trimmed();
}

bool e_marcador_capitulo_sem_titulo(const QString &linha)
{
    static const QRegularExpression chapter_only(
        QStringLiteral("^(capitulo|cap\\.?|chapter)\\s+\\d+\\s*$"));
    return chapter_only.match(normalizar_linha(linha)).hasMatch();
}

bool e_marcador_parte_sem_titulo(const QString &linha)
{
    static const QRegularExpression part_only(
        QStringLiteral("^(parte|part)\\s+([ivx]+|\\d+)\\s*$"));
    return part_only.match(normalizar_linha(linha)).hasMatch();
}

bool e_inicio_bloco_estrutural(const QString &linha)
{
    static const QRegularExpression structural(
        QStringLiteral("^(parte|part)\\s+([ivx]+|\\d+)\\b|^(capitulo|cap\\.?|chapter)\\s+\\d+\\s*$"));
    return structural.match(normalizar_linha(linha)).hasMatch();
}

bool e_linha_caixa_alta(const QString &linha)
{
    const QString trimmed = linha.trimmed();
    if (trimmed.size() < 4 || trimmed.size() > 140) {
        return false;
    }
    if (!trimmed.contains(QRegularExpression(QStringLiteral("\\p{L}")))) {
        return false;
    }
    return trimmed.toUpper() == trimmed;
}

}  // namespace

QStringList ProcessadorIA::preprocessar_linhas(const QString &texto) const
{
    QStringList result;
    static const QRegularExpression short_heading(QStringLiteral("^(\\d+|[IVXLCDM]+)[\\.)]?$"));
    static const QRegularExpression list_prefix(QStringLiteral("^(?:[-*]\\s+|\\d+[\\.)]\\s+)"));
    static const QRegularExpression sentence_end(QStringLiteral("[\\.!\\?]"));

    QString pending;
    bool pending_is_heading = false;

    auto flush_pending = [&]() {
        if (!pending.isEmpty()) {
            result.append(pending);
            pending.clear();
            pending_is_heading = false;
        }
    };

    auto is_heading_continuation = [&](const QString &line) {
        if (!pending_is_heading) {
            return false;
        }
        if (pending.trimmed().startsWith(QLatin1Char('#'))) {
            return false;
        }
        if ((e_marcador_capitulo_sem_titulo(pending) || e_marcador_parte_sem_titulo(pending))
            && !e_inicio_bloco_estrutural(line) && !list_prefix.match(line).hasMatch()
            && line.size() <= 120) {
            return true;
        }
        if (e_linha_caixa_alta(pending) && e_linha_caixa_alta(line)
            && !e_inicio_bloco_estrutural(pending) && !e_inicio_bloco_estrutural(line)
            && pending.size() + 1 + line.size() <= 190) {
            return true;
        }
        if (e_linha_caixa_alta(pending) && !e_linha_caixa_alta(line)) {
            return false;
        }
        if (inferir_nivel_titulo(line) > 0) {
            return false;
        }
        if (list_prefix.match(line).hasMatch()) {
            return false;
        }
        if (line.size() > 72) {
            return false;
        }
        if (sentence_end.match(line).hasMatch()) {
            return false;
        }
        if (pending.size() + 1 + line.size() > 140) {
            return false;
        }
        return true;
    };

    auto append_continuation = [&](const QString &line) {
        if (e_marcador_capitulo_sem_titulo(pending)) {
            pending += QLatin1Char('/');
            pending += line;
            return;
        }
        if (e_marcador_parte_sem_titulo(pending)) {
            pending += QStringLiteral(": ");
            pending += line;
            return;
        }
        if (pending.endsWith(QLatin1Char('-')) && !line.isEmpty() && line.at(0).isLower()) {
            pending.chop(1);
            pending += line;
        } else {
            pending += QLatin1Char(' ');
            pending += line;
        }
    };

    for (const QString &line : texto.split(QRegularExpression(QStringLiteral("[\\r\\n]+")))) {
        const QString trimmed = line.simplified().trimmed();
        if (trimmed.isEmpty()) {
            flush_pending();
            continue;
        }
        if (trimmed.size() < 3 && !short_heading.match(trimmed).hasMatch()) {
            continue;
        }
        if (is_heading_continuation(trimmed)) {
            append_continuation(trimmed);
            continue;
        }

        flush_pending();
        pending = trimmed;
        pending_is_heading = inferir_nivel_titulo(trimmed) > 0;
    }
    flush_pending();
    return result;
}

}  // namespace source::plugin::MapaMental
