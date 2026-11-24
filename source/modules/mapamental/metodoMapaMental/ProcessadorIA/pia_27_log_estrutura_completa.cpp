#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_27_log_estrutura_completa.hpp"

#include <QJsonDocument>
#include <QJsonObject>

namespace source::plugin::MapaMental::ProcessadorIAFns {

QString log_estrutura_completa(const QVariantMap &estrutura)
{
    return QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(estrutura)).toJson(QJsonDocument::Compact));
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
