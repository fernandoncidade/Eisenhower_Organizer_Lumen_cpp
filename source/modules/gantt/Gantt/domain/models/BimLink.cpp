#include "BimLink.hpp"
#include <QUuid>

namespace source::gantt {

BimLink::BimLink()
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

BimLink::BimLink(const QString &item_id, const QString &element_id, BimSource src)
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      schedule_item_id(item_id),
      bim_source(src),
      bim_element_id(element_id)
{
}

bool BimLink::is_valid(QString *error_message) const
{
    if (schedule_item_id.trimmed().isEmpty()) {
        if (error_message) *error_message = QStringLiteral("ID da atividade do cronograma é obrigatório.");
        return false;
    }
    if (bim_element_id.trimmed().isEmpty()) {
        if (error_message) *error_message = QStringLiteral("ID do elemento BIM (IFC GUID ou UniqueId) é obrigatório.");
        return false;
    }
    return true;
}

QJsonObject BimLink::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("id")] = id;
    json[QStringLiteral("schedule_item_id")] = schedule_item_id;
    json[QStringLiteral("bim_source")] = bim_source_to_string(bim_source);
    json[QStringLiteral("bim_element_id")] = bim_element_id;
    json[QStringLiteral("category")] = category;
    json[QStringLiteral("level")] = level;
    json[QStringLiteral("discipline")] = discipline;
    json[QStringLiteral("metadata")] = metadata;
    return json;
}

BimLink BimLink::from_json(const QJsonObject &json)
{
    BimLink link;
    if (json.contains(QStringLiteral("id"))) link.id = json[QStringLiteral("id")].toString();
    if (json.contains(QStringLiteral("schedule_item_id"))) link.schedule_item_id = json[QStringLiteral("schedule_item_id")].toString();
    if (json.contains(QStringLiteral("bim_source"))) link.bim_source = string_to_bim_source(json[QStringLiteral("bim_source")].toString());
    if (json.contains(QStringLiteral("bim_element_id"))) link.bim_element_id = json[QStringLiteral("bim_element_id")].toString();
    if (json.contains(QStringLiteral("category"))) link.category = json[QStringLiteral("category")].toString();
    if (json.contains(QStringLiteral("level"))) link.level = json[QStringLiteral("level")].toString();
    if (json.contains(QStringLiteral("discipline"))) link.discipline = json[QStringLiteral("discipline")].toString();
    if (json.contains(QStringLiteral("metadata"))) link.metadata = json[QStringLiteral("metadata")].toObject();
    return link;
}

} // namespace source::gantt
