#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_27_obter_conexoes_existentes.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

namespace source::plugin::MapaMental::MapaMentalFns {

QVariantMap obter_conexoes_existentes(const MapaMental &self)
{
    QVariantMap result;
    result.insert(QStringLiteral("total"), self.total_task_count());
    return result;
}

}  // namespace source::plugin::MapaMental::MapaMentalFns
