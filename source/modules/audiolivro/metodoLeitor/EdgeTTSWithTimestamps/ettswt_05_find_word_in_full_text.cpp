#include "source/modules/audiolivro/metodoLeitor/EdgeTTSWithTimestamps/ettswt_05_find_word_in_full_text.hpp"

namespace source::plugin::Audiolivro {

WordTimestamp EdgeTTSWithTimestamps::find_word_in_full_text(const QString &word,
                                                            const QString &full_text,
                                                            int start) const
{
    WordTimestamp timestamp;
    timestamp.text = word;
    timestamp.text_offset = qBound(0, start, full_text.size());
    return timestamp;
}

}  // namespace source::plugin::Audiolivro
