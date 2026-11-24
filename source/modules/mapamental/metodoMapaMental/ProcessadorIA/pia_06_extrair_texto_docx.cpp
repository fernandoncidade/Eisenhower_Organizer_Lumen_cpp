#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_06_extrair_texto_docx.hpp"

#include <QXmlStreamReader>

#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_33_zip_utils.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {
namespace {

QString extrair_wordprocessing_xml(const QByteArray &xml_data)
{
    QXmlStreamReader xml(xml_data);
    QString texto;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            const QStringView nome = xml.name();
            if (nome == QLatin1String("t")) {
                texto += xml.readElementText();
            } else if (nome == QLatin1String("tab")) {
                texto += QLatin1Char('\t');
            } else if (nome == QLatin1String("br") || nome == QLatin1String("cr")) {
                texto += QLatin1Char('\n');
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("p")) {
            if (!texto.endsWith(QLatin1Char('\n'))) {
                texto += QLatin1Char('\n');
            }
        }
    }

    return texto;
}

}  // namespace

QString extrair_texto_docx(const QString &caminho)
{
    QString texto;
    const QByteArray document_xml = ler_entrada_zip(caminho, QStringLiteral("word/document.xml"));
    texto += extrair_wordprocessing_xml(document_xml);

    const QStringList entradas = listar_entradas_zip(caminho);
    for (const QString &entrada : entradas) {
        const bool complemento =
            (entrada.startsWith(QStringLiteral("word/header"), Qt::CaseInsensitive)
             || entrada.startsWith(QStringLiteral("word/footer"), Qt::CaseInsensitive))
            && entrada.endsWith(QStringLiteral(".xml"), Qt::CaseInsensitive);
        if (complemento) {
            texto += QLatin1Char('\n');
            texto += extrair_wordprocessing_xml(ler_entrada_zip(caminho, entrada));
        }
    }

    return texto.trimmed();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
