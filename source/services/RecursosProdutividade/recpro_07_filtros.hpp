#pragma once

#include "source/services/incore_15_ProductivityFeatures.hpp"

namespace source { class EisenhowerMatrixApp; }

namespace source::InterfaceCore::ProductivityFeatures {

void show_filters_dialog(source::EisenhowerMatrixApp *app);
void apply_active_filters(source::EisenhowerMatrixApp *app);

}  // namespace source::InterfaceCore::ProductivityFeatures
