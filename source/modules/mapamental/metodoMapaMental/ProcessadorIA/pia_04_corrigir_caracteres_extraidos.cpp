#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_04_corrigir_caracteres_extraidos.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QString corrigir_caracteres_extraidos(const QString &texto)
{
    QString result = texto;
    result.replace(QStringLiteral("\uFFFD"), QString());
    return result;
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
