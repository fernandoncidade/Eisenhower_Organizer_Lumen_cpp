#include "source/services/MostrarMenuContexto/mmc_07_target_list_for_quadrant.hpp"

#include <QList>

#include "source/eisenhower_app.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {

QListWidget *target_list_for_quadrant(source::EisenhowerMatrixApp *app,
                                      int quadrant_index,
                                      bool keep_completed,
                                      bool include_completed)
{
    if (app == nullptr) {
        return nullptr;
    }

    const QList<QListWidget *> pending = {app->quadrant1_list, app->quadrant2_list, app->quadrant3_list, app->quadrant4_list};
    const QList<QListWidget *> in_progress = {app->quadrant1_in_progress_list,
                                              app->quadrant2_in_progress_list,
                                              app->quadrant3_in_progress_list,
                                              app->quadrant4_in_progress_list};
    const QList<QListWidget *> done = {
        app->quadrant1_completed_list,
        app->quadrant2_completed_list,
        app->quadrant3_completed_list,
        app->quadrant4_completed_list,
    };

    if (include_completed) {
        const QList<QListWidget *> all_targets = pending + in_progress + done;
        return quadrant_index >= 0 && quadrant_index < all_targets.size() ? all_targets.at(quadrant_index) : nullptr;
    }

    return quadrant_index >= 0 && quadrant_index < 4 ? (keep_completed ? done.at(quadrant_index) : pending.at(quadrant_index))
                                                     : nullptr;
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
