#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_31_embeddings.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {

QVariantList get_sentence_embeddings(const QStringList &texts)
{
    QVariantList result;
    for (const QString &text : texts) {
        QVariantList vector;
        vector << text.size();
        result << QVariant(vector);
    }
    return result;
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
