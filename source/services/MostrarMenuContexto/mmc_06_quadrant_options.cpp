#include "source/services/MostrarMenuContexto/mmc_06_quadrant_options.hpp"

#include <QLabel>

#include "source/eisenhower_app.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {

QStringList quadrant_options(source::EisenhowerMatrixApp *app, bool include_completed)
{
    if (app == nullptr) {
        return {};
    }

    QStringList pending = {
        app->quadrant1_label != nullptr ? app->quadrant1_label->text() : QString(),
        app->quadrant2_label != nullptr ? app->quadrant2_label->text() : QString(),
        app->quadrant3_label != nullptr ? app->quadrant3_label->text() : QString(),
        app->quadrant4_label != nullptr ? app->quadrant4_label->text() : QString(),
    };

    if (!include_completed) {
        return pending;
    }

    pending << (app->quadrant1_in_progress_label != nullptr ? app->quadrant1_in_progress_label->text() : QString())
            << (app->quadrant2_in_progress_label != nullptr ? app->quadrant2_in_progress_label->text() : QString())
            << (app->quadrant3_in_progress_label != nullptr ? app->quadrant3_in_progress_label->text() : QString())
            << (app->quadrant4_in_progress_label != nullptr ? app->quadrant4_in_progress_label->text() : QString());

    pending << (app->quadrant1_completed_label != nullptr ? app->quadrant1_completed_label->text() : QString())
            << (app->quadrant2_completed_label != nullptr ? app->quadrant2_completed_label->text() : QString())
            << (app->quadrant3_completed_label != nullptr ? app->quadrant3_completed_label->text() : QString())
            << (app->quadrant4_completed_label != nullptr ? app->quadrant4_completed_label->text() : QString());
    return pending;
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
