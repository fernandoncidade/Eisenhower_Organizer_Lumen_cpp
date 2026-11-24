#include "source/modules/feynman/Feynman/utils/IconUtils.hpp"

#include "source/utils/IconUtils.hpp"

namespace source::plugin::Feynman::utils {

QString get_icon_path(const QString &icon_name)
{
    return source::utils::get_icon_path(icon_name);
}

}  // namespace source::plugin::Feynman::utils
