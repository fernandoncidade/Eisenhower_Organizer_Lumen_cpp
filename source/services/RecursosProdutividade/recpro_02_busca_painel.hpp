#pragma once

#include "source/services/incore_15_ProductivityFeatures.hpp"

namespace source { class EisenhowerMatrixApp; }
class QListWidgetItem;

namespace source::InterfaceCore::ProductivityFeatures {

void show_global_search(source::EisenhowerMatrixApp *app);
void show_today_panel(source::EisenhowerMatrixApp *app);
void configure_recurrence_for_item(source::EisenhowerMatrixApp *app, QListWidgetItem *item);

}  // namespace source::InterfaceCore::ProductivityFeatures
