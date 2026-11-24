#pragma once

#include <QSet>
#include <QString>

namespace source::GerenciamentoUI {
class CalendarDialog;
}

namespace source::GerenciamentoUI::CalendarDialogModules {

QSet<QString> _get_task_dates(const source::GerenciamentoUI::CalendarDialog *self);

}  // namespace source::GerenciamentoUI::CalendarDialogModules
