#pragma once

#include <QString>

class QWidget;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::MostrarMenuContextoModules {

void apply_widget_min_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name = QString());

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
