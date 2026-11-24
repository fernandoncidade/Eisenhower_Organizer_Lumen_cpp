#pragma once

#include <QPair>
#include <QVariantMap>

class QListWidget;
class QListWidgetItem;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::MostrarMenuContextoModules {

QPair<QListWidget *, QListWidgetItem *> find_source_item_for_calendar(source::EisenhowerMatrixApp *app,
                                                                      const QVariantMap &task_dict);

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
