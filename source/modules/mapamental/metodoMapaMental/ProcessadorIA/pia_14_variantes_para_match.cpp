#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_14_variantes_para_match.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QVariantMap variantes_para_match(const QString &linha)
{
    QVariantMap result;
    result.insert(QStringLiteral("original"), linha);
    result.insert(QStringLiteral("casefold"), linha.toCaseFolded());
    result.insert(QStringLiteral("simplified"), linha.simplified().toCaseFolded());
    return result;
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
