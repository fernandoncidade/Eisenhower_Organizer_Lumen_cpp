#pragma once

#include <QString>

namespace source::plugin::MapaMental::ProcessadorIAFns {
QString gerar_resumo_contextual(const QString &texto,
                                int max_chars = 280,
                                const QString &contexto = QString());
}
