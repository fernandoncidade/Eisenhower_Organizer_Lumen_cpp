#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_02_detectar_idioma_texto.hpp"

#include <QRegularExpression>

namespace source::plugin::MapaMental {

QString ProcessadorIA::detectar_idioma_texto(const QString &texto) const
{
    const QString folded = texto.toCaseFolded();
    int pt_score = 0;
    int en_score = 0;

    for (const QString &word : {QStringLiteral("cao"), QStringLiteral("ção"), QStringLiteral("ões"),
                                QStringLiteral("não"), QStringLiteral("para"), QStringLiteral("que")}) {
        pt_score += folded.count(word);
    }
    for (const QString &word : {QStringLiteral("the"), QStringLiteral("and"), QStringLiteral("with"),
                                QStringLiteral("that"), QStringLiteral("from"), QStringLiteral("this")}) {
        en_score += folded.count(QRegularExpression(QStringLiteral("\\b%1\\b").arg(word)));
    }

    return en_score > pt_score ? QStringLiteral("en") : QStringLiteral("pt");
}

}  // namespace source::plugin::MapaMental
