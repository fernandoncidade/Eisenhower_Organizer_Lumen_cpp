#pragma once

class QListWidget;
class QListWidgetItem;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::MostrarMenuContextoModules {

void move_to_quadrant(source::EisenhowerMatrixApp *app, QListWidgetItem *item, QListWidget *source_list);

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
