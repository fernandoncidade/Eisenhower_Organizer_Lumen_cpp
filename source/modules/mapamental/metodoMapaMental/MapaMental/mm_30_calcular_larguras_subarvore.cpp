#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_30_calcular_larguras_subarvore.hpp"

#include <QtGlobal>

namespace source::plugin::MapaMental::MapaMentalFns {

double calcular_largura_subarvore(int filhos_count, double espacamento_base)
{
    return qMax(espacamento_base, filhos_count * espacamento_base);
}

}  // namespace source::plugin::MapaMental::MapaMentalFns
