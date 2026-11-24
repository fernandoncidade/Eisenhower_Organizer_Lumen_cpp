#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_07_extrair_texto_txt.hpp"

#include <QFile>

namespace source::plugin::MapaMental::ProcessadorIAFns {

QString extrair_texto_txt(const QString &caminho)
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

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
