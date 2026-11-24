#pragma once

#include <QPoint>

namespace source::GerenciamentoUI {
class CalendarDialog;
}

namespace source::GerenciamentoUI::CalendarDialogModules {

void _on_calendar_context_menu(source::GerenciamentoUI::CalendarDialog *self, const QPoint &pos);

}  // namespace source::GerenciamentoUI::CalendarDialogModules
