#include "source/modules/pomodoro/Pomodoro/utils/IconUtils.hpp"

#include "source/utils/IconUtils.hpp"

namespace source::plugin::Pomodoro::utils {

QString get_icon_path(const QString &icon_name)
{
    return source::utils::get_icon_path(icon_name);
}

}  // namespace source::plugin::Pomodoro::utils
