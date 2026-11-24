#pragma once

#include <QDate>

namespace source::GerenciamentoUI {
class CalendarDialog;
}

namespace source::GerenciamentoUI::CalendarDialogModules {

void _prompt_edit_task_for_date(source::GerenciamentoUI::CalendarDialog *self, const QDate &qdate);

}  // namespace source::GerenciamentoUI::CalendarDialogModules
