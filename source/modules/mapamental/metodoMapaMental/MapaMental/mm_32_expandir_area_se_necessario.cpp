#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_32_expandir_area_se_necessario.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"

namespace source::plugin::MapaMental {

void MapaMental::expandir_area_se_necessario(NoConceito *no)
{
    if (scene_ == nullptr || no == nullptr) {
        return;
    }
    scene_->setSceneRect(scene_->sceneRect().united(no->sceneBoundingRect().adjusted(-400, -300, 400, 300)));
}

}  // namespace source::plugin::MapaMental
