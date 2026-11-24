#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_31_posicionar_no_arvore.hpp"

namespace source::plugin::MapaMental::MapaMentalFns {

QPointF posicionar_no_arvore(int profundidade, int ordem, double espacamento_h, double espacamento_v)
{
    return QPointF(profundidade * espacamento_h, ordem * espacamento_v);
}

}  // namespace source::plugin::MapaMental::MapaMentalFns
