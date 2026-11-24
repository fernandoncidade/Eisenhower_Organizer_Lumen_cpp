#pragma once

#include <QString>

class QListWidget;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::incore_02_add_placeholder {

void add_placeholder(source::EisenhowerMatrixApp *app, QListWidget *list_widget, const QString &text);

}  // namespace source::InterfaceCore::incore_02_add_placeholder
