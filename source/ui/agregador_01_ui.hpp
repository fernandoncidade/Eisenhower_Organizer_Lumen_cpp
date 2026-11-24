#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QVariant>
#include <QVariantMap>

#include "source/ui/ui_01_SobreDialog.hpp"
#include "source/ui/ui_02_OpcoesSobre.hpp"
#include "source/ui/ui_03_add_task.hpp"
#include "source/ui/ui_04_handle_item_checked.hpp"
#include "source/ui/ui_05_move_item_between_lists.hpp"
#include "source/ui/ui_06_remove_task.hpp"
#include "source/ui/ui_07_save_tasks.hpp"
#include "source/ui/ui_08_load_tasks.hpp"
#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_10_BadgeCalendarWidget.hpp"
#include "source/ui/ui_11_NativeLineEditFrame.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"
#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/ui_13_RotatedTabButton.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/ui/ui_15_edit_task.hpp"
#include "source/ui/ui_16_YearCalendarWidget.hpp"
#include "source/ui/ui_17_OllamaInstrucoesDialog.hpp"
#include "source/ui/ui_18_ModelDownloadDialog.hpp"

class QListWidget;
class QListWidgetItem;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI {

void add_task(source::EisenhowerMatrixApp *app);
void handle_item_checked(source::EisenhowerMatrixApp *app,
                         QListWidgetItem *item,
                         QListWidget *source_list,
                         QListWidget *target_list);
void move_item_between_lists(source::EisenhowerMatrixApp *app,
                             QListWidgetItem *item,
                             QListWidget *source,
                             QListWidget *target,
                             Qt::CheckState new_check_state);
bool remove_task(source::EisenhowerMatrixApp *app,
                 QListWidgetItem *item,
                 QListWidget *list_widget,
                 bool confirm = true);
void save_tasks(source::EisenhowerMatrixApp *app);
void load_tasks(source::EisenhowerMatrixApp *app);
bool load_tasks_from_path(source::EisenhowerMatrixApp *app, const QString &path);
void edit_task_datetime(source::EisenhowerMatrixApp *app, QListWidgetItem *item, QListWidget *list_widget);
void move_task_to_quadrant(source::EisenhowerMatrixApp *app,
                           QListWidgetItem *item,
                           QListWidget *source_list,
                           QListWidget *target_list);

}  // namespace source::GerenciamentoUI

namespace source::GerenciamentoUI::task_utils {

QList<QVariantMap> collect_tasks_from_app(source::EisenhowerMatrixApp *app);
QString priority_emoji(const QVariant &prio, source::EisenhowerMatrixApp *app = nullptr);
QString task_label_for_day(const QVariantMap &task,
                           bool show_priority,
                           bool show_domain,
                           source::EisenhowerMatrixApp *app = nullptr);
QPair<QListWidget *, QListWidgetItem *> find_source_item_for_calendar(source::EisenhowerMatrixApp *app,
                                                                      const QVariantMap &task_dict);
bool is_task_in_progress(const QVariantMap &task);
QString task_status_text(const QVariantMap &task);

}  // namespace source::GerenciamentoUI::task_utils
