#pragma once

class QListWidget;
class QPoint;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::MostrarMenuContextoModules {

void mostrar_menu_contexto(source::EisenhowerMatrixApp *app, const QPoint &point, QListWidget *list_widget);

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
