#pragma once

#include <QList>
#include <QVariantMap>

namespace source::GerenciamentoUI {
class CalendarDialog;
}

namespace source::GerenciamentoUI::CalendarDialogModules {

QList<QVariantMap> _collect_tasks(const source::GerenciamentoUI::CalendarDialog *self);

}  // namespace source::GerenciamentoUI::CalendarDialogModules
