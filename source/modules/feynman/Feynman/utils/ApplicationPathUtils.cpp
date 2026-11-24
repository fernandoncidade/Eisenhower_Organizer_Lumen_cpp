#include "source/modules/feynman/Feynman/utils/ApplicationPathUtils.hpp"

#include "source/utils/ApplicationPathUtils.hpp"

namespace source::plugin::Feynman::utils {

QString get_app_base_path()
{
    return source::utils::get_app_base_path();
}

QString get_text_file_path(const QString &filename, const QString &folder)
{
    return source::utils::get_text_file_path(filename, folder);
}

QString load_text_file(const QString &filename, const QString &folder, const QByteArray &encoding)
{
    return source::utils::load_text_file(filename, folder, encoding);
}

}  // namespace source::plugin::Feynman::utils
