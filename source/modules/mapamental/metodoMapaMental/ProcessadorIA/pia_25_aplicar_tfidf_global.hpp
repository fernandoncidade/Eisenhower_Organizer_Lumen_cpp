#pragma once

#include <QVariantList>

namespace source::plugin::MapaMental::ProcessadorIAFns {
QVariantList aplicar_tfidf_global(const QString &texto, int max_conceitos = 15);
}
