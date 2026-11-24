#pragma once

#include "source/services/incore_15_ProductivityFeatures.hpp"

namespace source { class EisenhowerMatrixApp; }

namespace source::InterfaceCore::ProductivityFeatures {

void create_spaced_reviews(source::EisenhowerMatrixApp *app);
void show_plugin_manager(source::EisenhowerMatrixApp *app);
void show_log_manager(source::EisenhowerMatrixApp *app);

}  // namespace source::InterfaceCore::ProductivityFeatures
