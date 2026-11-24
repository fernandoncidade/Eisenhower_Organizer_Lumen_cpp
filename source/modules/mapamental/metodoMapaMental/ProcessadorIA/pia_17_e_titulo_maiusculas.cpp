#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_17_e_titulo_maiusculas.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

bool e_titulo_maiusculas(const QString &linha)
{
    const QString trimmed = linha.trimmed();
    return !trimmed.isEmpty() && trimmed.size() <= 72 && trimmed == trimmed.toUpper();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
