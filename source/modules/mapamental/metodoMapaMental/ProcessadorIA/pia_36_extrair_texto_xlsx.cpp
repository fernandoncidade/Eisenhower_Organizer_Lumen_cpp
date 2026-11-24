#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_36_extrair_texto_xlsx.hpp"

#include <QXmlStreamReader>

#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_33_zip_utils.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {
namespace {

QStringList ler_shared_strings(const QString &caminho)
{
    QXmlStreamReader xml(ler_entrada_zip(caminho, QStringLiteral("xl/sharedStrings.xml")));
    QStringList strings;
    QString atual;
    bool em_si = false;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("si")) {
                atual.clear();
                em_si = true;
            } else if (em_si && xml.name() == QLatin1String("t")) {
                atual += xml.readElementText();
            }
        } else if (xml.isEndElement() && xml.name() == QLatin1String("si")) {
            strings.append(atual);
            em_si = false;
        }
    }

    return strings;
}

QString texto_celula(const QString &valor, const QString &tipo, const QStringList &shared_strings)
{
    if (tipo == QLatin1String("s")) {
        bool ok = false;
        const int idx = valor.toInt(&ok);
        if (ok && idx >= 0 && idx < shared_strings.size()) {
            return shared_strings.at(idx);
        }
    }
    if (tipo == QLatin1String("b")) {
        return valor == QLatin1String("1") ? QStringLiteral("true") : QStringLiteral("false");
    }
    return valor;
}

QString ler_planilha(const QString &caminho, const QString &entrada, const QStringList &shared_strings)
{
    QXmlStreamReader xml(ler_entrada_zip(caminho, entrada));
    QString texto;
    QStringList linha;
    QString valor;
    QString tipo;
    bool em_celula = false;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("row")) {
                linha.clear();
            } else if (xml.name() == QLatin1String("c")) {
                em_celula = true;
                valor.clear();
                tipo = xml.attributes().value(QStringLiteral("t")).toString();
            } else if (em_celula && xml.name() == QLatin1String("v")) {
                valor = xml.readElementText();
            } else if (em_celula && xml.name() == QLatin1String("t")) {
                valor += xml.readElementText();
            }
        } else if (xml.isEndElement()) {
            if (xml.name() == QLatin1String("c")) {
                linha.append(texto_celula(valor.trimmed(), tipo, shared_strings));
                em_celula = false;
            } else if (xml.name() == QLatin1String("row")) {
                const QString conteudo = linha.join(QLatin1Char('\t')).trimmed();
                if (!conteudo.isEmpty()) {
                    texto += conteudo;
                    texto += QLatin1Char('\n');
                }
            }
        }
    }

    return texto;
}

}  // namespace

QString extrair_texto_xlsx(const QString &caminho)
{
    const QStringList shared_strings = ler_shared_strings(caminho);
    QString texto;
    const QStringList entradas = listar_entradas_zip(caminho);
    for (const QString &entrada : entradas) {
        if (entrada.startsWith(QStringLiteral("xl/worksheets/sheet"), Qt::CaseInsensitive)
            && entrada.endsWith(QStringLiteral(".xml"), Qt::CaseInsensitive)) {
            texto += QStringLiteral("\n[%1]\n").arg(entrada);
            texto += ler_planilha(caminho, entrada, shared_strings);
        }
    }
    return texto.trimmed();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
