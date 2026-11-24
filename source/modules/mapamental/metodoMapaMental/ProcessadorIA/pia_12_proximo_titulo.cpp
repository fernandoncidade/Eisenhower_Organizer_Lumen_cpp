#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_12_proximo_titulo.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QString proximo_titulo(const QStringList &linhas, int idx)
{
    for (int i = qMax(0, idx + 1); i < linhas.size(); ++i) {
        const QString linha = linhas.at(i).trimmed();
        if (!linha.isEmpty()) {
            return linha;
        }
    }
    return {};
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
