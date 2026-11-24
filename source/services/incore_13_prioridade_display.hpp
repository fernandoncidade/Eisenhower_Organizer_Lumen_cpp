#pragma once

#include <QString>
#include <QVariant>

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore {

QString prioridade_para_texto(const QVariant &prio, source::EisenhowerMatrixApp *app = nullptr);
QString dominio_insignia(const QVariant &dominio);
QString dominio_para_texto(const QVariant &dominio, bool short_form = false);

}  // namespace source::InterfaceCore
