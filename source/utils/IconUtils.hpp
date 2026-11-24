#pragma once

#include <QString>

namespace source::utils {

QString get_default_window_icon_name();
QString get_default_window_icon_path();
QString get_icon_path(const QString &icon_name);

}  // namespace source::utils
