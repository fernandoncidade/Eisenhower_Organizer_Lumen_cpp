#pragma once

#include <QVariantList>

namespace source::plugin::MapaMental::ProcessadorIAFns {
QVariantList extrair_conceitos_semanticos(const QString &texto);
}
