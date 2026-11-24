#pragma once

#include <QList>
#include <QString>
#include <QVariantMap>

#include "source/utils/LogManager.hpp"

#include "source/ui/CalendarDialog/caldial_01_closeEvent.hpp"
#include "source/ui/CalendarDialog/caldial_02_open_linked_file.hpp"
#include "source/ui/CalendarDialog/caldial_03_collect_tasks.hpp"
#include "source/ui/CalendarDialog/caldial_04_update_task_list.hpp"
#include "source/ui/CalendarDialog/caldial_05_on_calendar_context_menu.hpp"
#include "source/ui/CalendarDialog/caldial_06_prompt_edit_task_for_date.hpp"
#include "source/ui/CalendarDialog/caldial_07_prompt_remove_task_for_date.hpp"
#include "source/ui/CalendarDialog/caldial_08_update_badge_counts.hpp"
#include "source/ui/CalendarDialog/caldial_09_on_language_changed.hpp"
#include "source/ui/CalendarDialog/caldial_10_apply_locale_to_calendar.hpp"
#include "source/ui/CalendarDialog/caldial_11_get_task_dates.hpp"
#include "source/ui/CalendarDialog/caldial_12_apply_highlighted_dates.hpp"

class QDate;

namespace source::GerenciamentoUI {
class CalendarDialog;
}

namespace source::GerenciamentoUI::CalendarDialogModules::detail {

source::utils::LogManager *logger();
QString get_text(const QString &text);
QList<QVariantMap> tasks_for_date(const source::GerenciamentoUI::CalendarDialog *self, const QDate &qdate);
QString tooltip_for_task(const source::GerenciamentoUI::CalendarDialog *self, const QVariantMap &task);

}  // namespace source::GerenciamentoUI::CalendarDialogModules::detail
