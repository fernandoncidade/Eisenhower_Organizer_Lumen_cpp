#pragma once

#include "source/services/incore_15_ProductivityFeatures.hpp"

namespace source { class EisenhowerMatrixApp; }

namespace source::InterfaceCore::ProductivityFeatures {

void schedule_local_reminders(source::EisenhowerMatrixApp *app);

}  // namespace source::InterfaceCore::ProductivityFeatures
