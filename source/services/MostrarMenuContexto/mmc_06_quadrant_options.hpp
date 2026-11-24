#pragma once

#include <QStringList>

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::MostrarMenuContextoModules {

QStringList quadrant_options(source::EisenhowerMatrixApp *app, bool include_completed = false);

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
