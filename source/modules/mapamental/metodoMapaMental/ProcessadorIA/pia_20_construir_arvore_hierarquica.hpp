#pragma once

#include <QVariantMap>

namespace source::plugin::MapaMental::ProcessadorIAFns {
QVariantMap construir_arvore_hierarquica(const QString &texto);
}
