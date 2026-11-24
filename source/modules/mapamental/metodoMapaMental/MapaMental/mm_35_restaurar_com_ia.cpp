#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_35_restaurar_com_ia.hpp"

#include <cmath>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"

namespace source::plugin::MapaMental {

void MapaMental::restaurar_com_ia()
{
    if (nos_.isEmpty()) {
        return;
    }

    constexpr qreal two_pi = 6.28318530717958647692;
    const qreal radius = qMax<qreal>(180.0, nos_.size() * 34.0);
    for (int i = 0; i < nos_.size(); ++i) {
        const qreal angle = (two_pi * i) / qMax(1, nos_.size());
        nos_.at(i)->setPos(std::cos(angle) * radius, std::sin(angle) * radius);
    }
    salvar_backup_automatico();
}

}  // namespace source::plugin::MapaMental
