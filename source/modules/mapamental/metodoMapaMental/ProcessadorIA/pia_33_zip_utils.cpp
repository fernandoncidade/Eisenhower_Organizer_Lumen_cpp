#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_33_zip_utils.hpp"

#include <QDir>
#include <private/qzipreader_p.h>

namespace source::plugin::MapaMental::ProcessadorIAFns {

QByteArray ler_entrada_zip(const QString &arquivo_zip, const QString &entrada)
{
    QZipReader zip(arquivo_zip);
    if (!zip.exists() || !zip.isReadable()) {
        return {};
    }
    return zip.fileData(entrada);
}

QStringList listar_entradas_zip(const QString &arquivo_zip)
{
    QZipReader zip(arquivo_zip);
    if (!zip.exists() || !zip.isReadable()) {
        return {};
    }

    QStringList entradas;
    for (const QZipReader::FileInfo &info : zip.fileInfoList()) {
        if (info.isFile && !info.filePath.trimmed().isEmpty()) {
            entradas.append(info.filePath);
        }
    }
    entradas.sort(Qt::CaseInsensitive);
    return entradas;
}

QString resolver_caminho_zip(const QString &entrada_base, const QString &caminho_relativo)
{
    if (caminho_relativo.startsWith(QLatin1Char('/'))) {
        return QDir::cleanPath(caminho_relativo.mid(1)).replace(QLatin1Char('\\'), QLatin1Char('/'));
    }

    const int slash = entrada_base.lastIndexOf(QLatin1Char('/'));
    const QString base = slash >= 0 ? entrada_base.left(slash + 1) : QString();
    return QDir::cleanPath(base + caminho_relativo).replace(QLatin1Char('\\'), QLatin1Char('/'));
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
