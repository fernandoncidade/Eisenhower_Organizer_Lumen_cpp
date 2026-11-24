#include "source/ui/CalendarDialog/caldial_08_update_badge_counts.hpp"

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/ui_10_BadgeCalendarWidget.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

void _update_badge_counts(source::GerenciamentoUI::CalendarDialog *self)
{
    try {
        if (self == nullptr || self->calendar == nullptr) {
            return;
        }

        QStringList task_dates;
        for (const QVariantMap &task : self->_collect_tasks()) {
            const QString date_str = task.value(QStringLiteral("date")).toString();
            if (!date_str.isEmpty()) {
                task_dates.append(date_str);
            }
        }

        self->calendar->set_task_counts(task_dates);
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao atualizar contadores de badges."));
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
