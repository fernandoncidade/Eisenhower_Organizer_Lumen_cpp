#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_40_extrair_texto_doc.hpp"

#include <QFile>
#include <QStringList>

namespace source::plugin::MapaMental::ProcessadorIAFns {
namespace {

bool printable_ansi(uchar c)
{
    return c == '\n' || c == '\r' || c == '\t' || (c >= 32 && c < 127) || c >= 160;
}

bool printable_unicode(ushort c)
{
    return c == '\n' || c == '\r' || c == '\t' || (c >= 32 && c < 0xD800);
}

QStringList extrair_runs_ansi(const QByteArray &data)
{
    QStringList runs;
    QByteArray atual;
    for (uchar c : data) {
        if (printable_ansi(c)) {
            atual.append(static_cast<char>(c));
        } else {
            if (atual.size() >= 6) {
                runs.append(QString::fromLocal8Bit(atual).simplified());
            }
            atual.clear();
        }
    }
    if (atual.size() >= 6) {
        runs.append(QString::fromLocal8Bit(atual).simplified());
    }
    return runs;
}

QStringList extrair_runs_utf16le(const QByteArray &data)
{
    QStringList runs;
    QString atual;
    for (int i = 0; i + 1 < data.size(); i += 2) {
        const ushort code = static_cast<uchar>(data.at(i)) | (static_cast<uchar>(data.at(i + 1)) << 8);
        if (printable_unicode(code)) {
            atual += QChar(code);
        } else {
            if (atual.size() >= 4) {
                runs.append(atual.simplified());
            }
            atual.clear();
        }
    }
    if (atual.size() >= 4) {
        runs.append(atual.simplified());
    }
    return runs;
}

}  // namespace

QString extrair_texto_doc(const QString &caminho)
{
    QFile file(caminho);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QByteArray data = file.readAll();
    QStringList runs = extrair_runs_utf16le(data);
    runs.append(extrair_runs_ansi(data));
    runs.removeDuplicates();
    return runs.join(QLatin1Char('\n')).trimmed();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
