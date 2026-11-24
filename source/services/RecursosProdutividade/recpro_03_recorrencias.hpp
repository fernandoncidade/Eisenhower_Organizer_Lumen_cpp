#pragma once

#include "source/services/incore_15_ProductivityFeatures.hpp"

namespace source { class EisenhowerMatrixApp; }

namespace source::InterfaceCore::ProductivityFeatures {

void materialize_recurring_tasks(source::EisenhowerMatrixApp *app);

}  // namespace source::InterfaceCore::ProductivityFeatures
