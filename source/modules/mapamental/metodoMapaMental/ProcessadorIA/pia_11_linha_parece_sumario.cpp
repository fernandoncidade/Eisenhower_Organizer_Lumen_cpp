#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_11_linha_parece_sumario.hpp"

#include <QRegularExpression>

namespace source::plugin::MapaMental::ProcessadorIAFns {

bool linha_parece_sumario(const QString &linha)
{
    return linha.contains(QRegularExpression(QStringLiteral("\\.{3,}\\s*\\d+$")));
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
