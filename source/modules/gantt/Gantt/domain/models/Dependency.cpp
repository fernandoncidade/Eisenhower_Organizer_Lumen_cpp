#include "Dependency.hpp"
#include <QUuid>

namespace source::gantt {

Dependency::Dependency()
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

Dependency::Dependency(const QString &pred_id, const QString &succ_id, DependencyType dep_type, double lag)
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      predecessor_item_id(pred_id),
      successor_item_id(succ_id),
      type(dep_type),
      lag_hours(lag)
{
}

bool Dependency::is_valid(QString *error_message) const
{
    if (predecessor_item_id.trimmed().isEmpty()) {
        if (error_message) *error_message = QStringLiteral("ID da atividade predecessora é obrigatório.");
        return false;
    }
    if (successor_item_id.trimmed().isEmpty()) {
        if (error_message) *error_message = QStringLiteral("ID da atividade sucessora é obrigatório.");
        return false;
    }
    if (predecessor_item_id == successor_item_id) {
        if (error_message) *error_message = QStringLiteral("Uma atividade não pode depender de si mesma.");
        return false;
    }
    return true;
}

QJsonObject Dependency::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("id")] = id;
    json[QStringLiteral("schedule_id")] = schedule_id;
    json[QStringLiteral("predecessor_item_id")] = predecessor_item_id;
    json[QStringLiteral("successor_item_id")] = successor_item_id;
    json[QStringLiteral("dependency_type")] = dependency_type_to_string(type);
    json[QStringLiteral("lag_hours")] = lag_hours;
    return json;
}

Dependency Dependency::from_json(const QJsonObject &json)
{
    Dependency dep;
    if (json.contains(QStringLiteral("id"))) dep.id = json[QStringLiteral("id")].toString();
    if (json.contains(QStringLiteral("schedule_id"))) dep.schedule_id = json[QStringLiteral("schedule_id")].toString();
    if (json.contains(QStringLiteral("predecessor_item_id"))) dep.predecessor_item_id = json[QStringLiteral("predecessor_item_id")].toString();
    if (json.contains(QStringLiteral("successor_item_id"))) dep.successor_item_id = json[QStringLiteral("successor_item_id")].toString();
    if (json.contains(QStringLiteral("dependency_type"))) dep.type = string_to_dependency_type(json[QStringLiteral("dependency_type")].toString());
    if (json.contains(QStringLiteral("lag_hours"))) dep.lag_hours = json[QStringLiteral("lag_hours")].toDouble();
    return dep;
}

} // namespace source::gantt
