#pragma once

#include <QString>

class QWidget;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::MostrarMenuContextoModules {

int compute_widget_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name = QString());

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
