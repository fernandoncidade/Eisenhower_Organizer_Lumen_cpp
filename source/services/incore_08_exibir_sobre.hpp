#pragma once

#include <QString>

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::incore_08_exibir_sobre {

QString tr_multi(const QString &key);
QString build_cabecalho_fixo();
void exibir_sobre(source::EisenhowerMatrixApp *app);
void exibir_manual(source::EisenhowerMatrixApp *app);

}  // namespace source::InterfaceCore::incore_08_exibir_sobre
