#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_39_extrair_texto_xml.hpp"

#include <QFile>
#include <QXmlStreamReader>

namespace source::plugin::MapaMental::ProcessadorIAFns {

QString extrair_texto_xml(const QString &caminho)
{
    QFile file(caminho);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QXmlStreamReader xml(&file);
    QStringList pilha;
    QStringList linhas;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            pilha.append(xml.name().toString());
        } else if (xml.isCharacters()) {
            const QString texto = xml.text().toString().simplified();
            if (!texto.isEmpty()) {
                linhas.append(QStringLiteral("%1: %2").arg(pilha.join(QLatin1Char('/')), texto));
            }
        } else if (xml.isEndElement() && !pilha.isEmpty()) {
            pilha.removeLast();
        }
    }

    if (linhas.isEmpty()) {
        file.seek(0);
        return QString::fromUtf8(file.readAll());
    }
    return linhas.join(QLatin1Char('\n')).trimmed();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
