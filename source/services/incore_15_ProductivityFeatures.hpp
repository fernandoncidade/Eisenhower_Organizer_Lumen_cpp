#pragma once

#include <QListWidgetItem>

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::ProductivityFeatures {

void show_global_search(source::EisenhowerMatrixApp *app);
void show_today_panel(source::EisenhowerMatrixApp *app);
void configure_recurrence_for_item(source::EisenhowerMatrixApp *app, QListWidgetItem *item);
void materialize_recurring_tasks(source::EisenhowerMatrixApp *app);
void schedule_local_reminders(source::EisenhowerMatrixApp *app);
void export_calendar_ics(source::EisenhowerMatrixApp *app);
void show_productivity_history(source::EisenhowerMatrixApp *app);
void run_study_flow(source::EisenhowerMatrixApp *app);
void show_templates_dialog(source::EisenhowerMatrixApp *app);
void create_spaced_reviews(source::EisenhowerMatrixApp *app);
void show_plugin_manager(source::EisenhowerMatrixApp *app);
void show_log_manager(source::EisenhowerMatrixApp *app);
void show_filters_dialog(source::EisenhowerMatrixApp *app);
void apply_active_filters(source::EisenhowerMatrixApp *app);

}  // namespace source::InterfaceCore::ProductivityFeatures
