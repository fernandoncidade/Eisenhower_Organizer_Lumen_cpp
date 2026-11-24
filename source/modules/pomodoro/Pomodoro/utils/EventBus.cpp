#include "source/modules/pomodoro/Pomodoro/utils/EventBus.hpp"

namespace source::plugin::Pomodoro::utils {

EventBus *get_event_bus()
{
    return compat::get_event_bus();
}

}  // namespace source::plugin::Pomodoro::utils
