#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_13_remover_acentos.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QString remover_acentos(QString texto)
{
    texto = texto.normalized(QString::NormalizationForm_D);
    QString result;
    for (const QChar c : texto) {
        if (c.category() != QChar::Mark_NonSpacing) {
            result.append(c);
        }
    }
    return result;
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
