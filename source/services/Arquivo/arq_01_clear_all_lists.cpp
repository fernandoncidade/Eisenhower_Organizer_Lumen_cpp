#include "source/services/Arquivo/arq_01_clear_all_lists.hpp"

#include <QListWidget>

#include "source/services/incore_09_Arquivo.hpp"
#include "source/eisenhower_app.hpp"

namespace source::InterfaceCore::ArquivoModules {

void clear_all_lists(Arquivo *self)
{
    if (self == nullptr || self->app() == nullptr) {
        return;
    }

    source::EisenhowerMatrixApp *app = self->app();
    const QList<QListWidget *> lists = {
        app->quadrant1_list,
        app->quadrant2_list,
        app->quadrant3_list,
        app->quadrant4_list,
        app->quadrant1_completed_list,
        app->quadrant2_completed_list,
        app->quadrant3_completed_list,
        app->quadrant4_completed_list,
    };

    for (QListWidget *list : lists) {
        if (list != nullptr) {
            list->clear();
        }
    }
}

}  // namespace source::InterfaceCore::ArquivoModules
