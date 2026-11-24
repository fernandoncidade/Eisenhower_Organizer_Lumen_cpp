#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_35_extrair_texto_csv.hpp"

#include <QFile>
#include <QRegularExpression>
#include <QStringList>

namespace source::plugin::MapaMental::ProcessadorIAFns {
namespace {

QString texto_arquivo(const QString &caminho)
{
    QFile file(caminho);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QByteArray data = file.readAll();
    QString texto = QString::fromUtf8(data);
    if (texto.contains(QChar::ReplacementCharacter)) {
        texto = QString::fromLocal8Bit(data);
    }
    return texto;
}

QStringList parse_csv_linha(const QString &linha)
{
    QStringList campos;
    QString atual;
    bool entre_aspas = false;

    for (int i = 0; i < linha.size(); ++i) {
        const QChar c = linha.at(i);
        if (c == QLatin1Char('"')) {
            if (entre_aspas && i + 1 < linha.size() && linha.at(i + 1) == QLatin1Char('"')) {
                atual += QLatin1Char('"');
                ++i;
            } else {
                entre_aspas = !entre_aspas;
            }
        } else if ((c == QLatin1Char(',') || c == QLatin1Char(';')) && !entre_aspas) {
            campos.append(atual.trimmed());
            atual.clear();
        } else {
            atual += c;
        }
    }

    campos.append(atual.trimmed());
    return campos;
}

}  // namespace

QString extrair_texto_csv(const QString &caminho)
{
    const QString texto = texto_arquivo(caminho);
    QString resultado;
    for (const QString &linha : texto.split(QRegularExpression(QStringLiteral("\\r?\\n")), Qt::SkipEmptyParts)) {
        resultado += parse_csv_linha(linha).join(QLatin1Char('\t'));
        resultado += QLatin1Char('\n');
    }
    return resultado.trimmed();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
