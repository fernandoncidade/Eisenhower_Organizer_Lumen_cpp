#include "source/modules/audiolivro/metodoLeitor/EdgeTTSWithTimestamps/ettswt_04_calculate_text_offset.hpp"

namespace source::plugin::Audiolivro {

int EdgeTTSWithTimestamps::calculate_text_offset(const QString &word, const QString &segment, int last_offset) const
{
    Q_UNUSED(word)
    return qBound(0, last_offset, segment.size());
}

}  // namespace source::plugin::Audiolivro
