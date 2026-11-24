#pragma once

class QListWidget;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::MostrarMenuContextoModules {

QListWidget *target_list_for_quadrant(source::EisenhowerMatrixApp *app,
                                      int quadrant_index,
                                      bool keep_completed,
                                      bool include_completed = false);

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
