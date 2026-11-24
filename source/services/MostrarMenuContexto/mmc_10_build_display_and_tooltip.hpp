#pragma once

#include <QPair>
#include <QString>

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::MostrarMenuContextoModules {

QPair<QString, QString> build_display_and_tooltip(source::EisenhowerMatrixApp *app,
                                                  const QString &base_text,
                                                  const QString &date_iso,
                                                  const QString &time_str);

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
