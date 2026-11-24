#include "Baseline.hpp"
#include <QJsonArray>
#include <QUuid>

namespace source::gantt {

QJsonObject BaselineItemSnapshot::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("item_id")] = item_id;
    json[QStringLiteral("start_date")] = start_date.isValid() ? start_date.toString(Qt::ISODate) : QString();
    json[QStringLiteral("end_date")] = end_date.isValid() ? end_date.toString(Qt::ISODate) : QString();
    json[QStringLiteral("duration_hours")] = duration_hours;
    return json;
}

BaselineItemSnapshot BaselineItemSnapshot::from_json(const QJsonObject &json)
{
    BaselineItemSnapshot snapshot;
    snapshot.item_id = json[QStringLiteral("item_id")].toString();
    snapshot.start_date = QDateTime::fromString(json[QStringLiteral("start_date")].toString(), Qt::ISODate);
    snapshot.end_date = QDateTime::fromString(json[QStringLiteral("end_date")].toString(), Qt::ISODate);
    snapshot.duration_hours = json[QStringLiteral("duration_hours")].toDouble();
    return snapshot;
}

Baseline::Baseline()
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      created_at(QDateTime::currentDateTime())
{
}

Baseline::Baseline(const QString &baseline_name, const QString &sched_id)
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      schedule_id(sched_id),
      name(baseline_name),
      created_at(QDateTime::currentDateTime())
{
}

bool Baseline::can_modify(QString *error_message) const
{
    if (frozen) {
        if (error_message) {
            *error_message = QStringLiteral("Baseline congelada não pode ser alterada sem permissão especial.");
        }
        return false;
    }
    return true;
}

void Baseline::freeze()
{
    frozen = true;
}

bool Baseline::unfreeze(bool has_special_permission, QString *error_message)
{
    if (!has_special_permission) {
        if (error_message) {
            *error_message = QStringLiteral("Permissão negada: apenas administradores podem descongelar uma baseline.");
        }
        return false;
    }
    frozen = false;
    return true;
}

QJsonObject Baseline::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("id")] = id;
    json[QStringLiteral("schedule_id")] = schedule_id;
    json[QStringLiteral("name")] = name;
    json[QStringLiteral("created_at")] = created_at.toString(Qt::ISODate);
    json[QStringLiteral("frozen")] = frozen;
    json[QStringLiteral("created_by")] = created_by;
    json[QStringLiteral("notes")] = notes;

    QJsonArray items_array;
    for (const auto &item : items) {
        items_array.append(item.to_json());
    }
    json[QStringLiteral("items")] = items_array;

    return json;
}

Baseline Baseline::from_json(const QJsonObject &json)
{
    Baseline base;
    if (json.contains(QStringLiteral("id"))) base.id = json[QStringLiteral("id")].toString();
    if (json.contains(QStringLiteral("schedule_id"))) base.schedule_id = json[QStringLiteral("schedule_id")].toString();
    if (json.contains(QStringLiteral("name"))) base.name = json[QStringLiteral("name")].toString();
    if (json.contains(QStringLiteral("created_at"))) base.created_at = QDateTime::fromString(json[QStringLiteral("created_at")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("frozen"))) base.frozen = json[QStringLiteral("frozen")].toBool(false);
    if (json.contains(QStringLiteral("created_by"))) base.created_by = json[QStringLiteral("created_by")].toString();
    if (json.contains(QStringLiteral("notes"))) base.notes = json[QStringLiteral("notes")].toString();

    if (json.contains(QStringLiteral("items"))) {
        for (const auto &val : json[QStringLiteral("items")].toArray()) {
            BaselineItemSnapshot snap = BaselineItemSnapshot::from_json(val.toObject());
            base.items.insert(snap.item_id, snap);
        }
    }

    return base;
}

} // namespace source::gantt
