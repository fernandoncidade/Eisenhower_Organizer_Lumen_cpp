#pragma once

#include <QVariantList>

namespace source::plugin::MapaMental::ProcessadorIAFns {
QVariantList extrair_ideias_principais(const QString &texto);
}
