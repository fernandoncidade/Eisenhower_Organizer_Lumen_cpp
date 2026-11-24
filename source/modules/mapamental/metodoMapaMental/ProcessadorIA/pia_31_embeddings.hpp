#pragma once

#include <QStringList>
#include <QVariantList>

namespace source::plugin::MapaMental::ProcessadorIAFns {
QVariantList get_sentence_embeddings(const QStringList &texts);
}
