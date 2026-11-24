#pragma once

class QCloseEvent;

namespace source::GerenciamentoUI {
class CalendarDialog;
}

namespace source::GerenciamentoUI::CalendarDialogModules {

void closeEvent(source::GerenciamentoUI::CalendarDialog *self, QCloseEvent *event);

}  // namespace source::GerenciamentoUI::CalendarDialogModules
