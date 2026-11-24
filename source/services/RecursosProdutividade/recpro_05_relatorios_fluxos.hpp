#pragma once

#include "source/services/incore_15_ProductivityFeatures.hpp"

namespace source { class EisenhowerMatrixApp; }

namespace source::InterfaceCore::ProductivityFeatures {

void export_calendar_ics(source::EisenhowerMatrixApp *app);
void show_productivity_history(source::EisenhowerMatrixApp *app);
void run_study_flow(source::EisenhowerMatrixApp *app);
void show_templates_dialog(source::EisenhowerMatrixApp *app);

}  // namespace source::InterfaceCore::ProductivityFeatures
