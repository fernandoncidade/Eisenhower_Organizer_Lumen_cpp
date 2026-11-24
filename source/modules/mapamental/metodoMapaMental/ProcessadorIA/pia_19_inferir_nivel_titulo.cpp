#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_19_inferir_nivel_titulo.hpp"

#include <QRegularExpression>
#include <QStringList>

#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_13_remover_acentos.hpp"

namespace source::plugin::MapaMental {
namespace {

QString chave_titulo(QString texto)
{
    texto = ProcessadorIAFns::remover_acentos(texto).toCaseFolded().simplified().trimmed();
    texto.replace(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")), QStringLiteral(" "));
    return texto.simplified().trimmed();
}

bool e_secao_principal(const QString &key)
{
    static const QStringList secoes = {
        QStringLiteral("creditos"),
        QStringLiteral("folha de rosto"),
        QStringLiteral("sumario"),
        QStringLiteral("nota do autor"),
        QStringLiteral("prefacio a edicao brasileira"),
        QStringLiteral("prefacio da edicao original"),
        QStringLiteral("epilogo"),
        QStringLiteral("perguntas frequentes"),
        QStringLiteral("um guia de recursos"),
        QStringLiteral("agradecimentos"),
        QStringLiteral("sobre os autores"),
    };
    return key == QStringLiteral("introducao")
           || key.startsWith(QStringLiteral("introducao "))
           || secoes.contains(key);
}

}  // namespace

int ProcessadorIA::inferir_nivel_titulo(const QString &linha) const
{
    static const QRegularExpression markdown_heading(QStringLiteral("^(#{1,6})\\s+"));
    static const QRegularExpression numbered_heading(QStringLiteral("^(\\d+(?:[\\.\\-]\\d+)*)(?:\\)|\\.|\\-|:)?\\s+"));
    static const QRegularExpression roman_heading(QStringLiteral("^([IVXLCDM]+)\\.?\\s+"));
    static const QRegularExpression part_heading(
        QStringLiteral("^(parte|part)\\s+([ivx]+|\\d+)\\b"));
    static const QRegularExpression chapter_heading(
        QStringLiteral("^(capitulo|cap\\.?|chapter)\\s+\\d+\\b"));
    static const QRegularExpression section_heading(
        QStringLiteral("^(secao|section|subsecao|subsection|topico|topic|anexo|appendix)\\b"));

    const QString trimmed = linha.trimmed();
    if (trimmed.size() > 160) {
        return -1;
    }

    QRegularExpressionMatch markdown = markdown_heading.match(linha);
    if (markdown.hasMatch()) {
        return markdown.captured(1).size();
    }

    QRegularExpressionMatch numbered = numbered_heading.match(trimmed);
    if (numbered.hasMatch()) {
        return numbered.captured(1).count(QLatin1Char('.')) + 1;
    }

    QRegularExpressionMatch roman = roman_heading.match(trimmed);
    if (roman.hasMatch()) {
        return 1;
    }

    const QString normalized = ProcessadorIAFns::remover_acentos(trimmed).toCaseFolded();
    if (part_heading.match(normalized).hasMatch() || e_secao_principal(chave_titulo(trimmed))) {
        return 1;
    }
    if (chapter_heading.match(normalized).hasMatch()) {
        return 2;
    }
    if (section_heading.match(normalized).hasMatch()) {
        return 3;
    }

    const bool short_line = trimmed.size() <= 72;
    const bool mostly_upper = short_line && trimmed.toUpper() == trimmed
                              && linha.contains(QRegularExpression(QStringLiteral("\\p{L}")));
    if (mostly_upper) {
        return 3;
    }

    if (short_line && trimmed.endsWith(QLatin1Char(':'))) {
        return 3;
    }

    return -1;
}

}  // namespace source::plugin::MapaMental
