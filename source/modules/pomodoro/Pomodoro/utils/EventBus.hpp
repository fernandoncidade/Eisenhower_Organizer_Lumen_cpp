#pragma once

#include "source/modules/pomodoro/Pomodoro/pomodoro_support.hpp"

namespace source::plugin::Pomodoro::utils {

using EventBus = source::plugin::Pomodoro::compat::EventBus;

EventBus *get_event_bus();

}  // namespace source::plugin::Pomodoro::utils
