#include "ProgressUpdate.hpp"
#include <QUuid>

namespace source::gantt {

ProgressUpdate::ProgressUpdate()
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      timestamp(QDateTime::currentDateTime())
{
}

ProgressUpdate::ProgressUpdate(const QString &item_id, double percent, const QString &author)
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      schedule_item_id(item_id),
      timestamp(QDateTime::currentDateTime()),
      percent_complete(percent),
      reported_by(author)
{
}

QJsonObject ProgressUpdate::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("id")] = id;
    json[QStringLiteral("schedule_item_id")] = schedule_item_id;
    json[QStringLiteral("timestamp")] = timestamp.toString(Qt::ISODate);
    json[QStringLiteral("percent_complete")] = percent_complete;
    json[QStringLiteral("previous_percent")] = previous_percent;
    json[QStringLiteral("actual_start")] = actual_start.isValid() ? actual_start.toString(Qt::ISODate) : QString();
    json[QStringLiteral("actual_end")] = actual_end.isValid() ? actual_end.toString(Qt::ISODate) : QString();
    json[QStringLiteral("notes")] = notes;
    json[QStringLiteral("reported_by")] = reported_by;
    json[QStringLiteral("variance_hours")] = variance_hours;
    return json;
}

ProgressUpdate ProgressUpdate::from_json(const QJsonObject &json)
{
    ProgressUpdate update;
    if (json.contains(QStringLiteral("id"))) update.id = json[QStringLiteral("id")].toString();
    if (json.contains(QStringLiteral("schedule_item_id"))) update.schedule_item_id = json[QStringLiteral("schedule_item_id")].toString();
    if (json.contains(QStringLiteral("timestamp"))) update.timestamp = QDateTime::fromString(json[QStringLiteral("timestamp")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("percent_complete"))) update.percent_complete = json[QStringLiteral("percent_complete")].toDouble();
    if (json.contains(QStringLiteral("previous_percent"))) update.previous_percent = json[QStringLiteral("previous_percent")].toDouble();
    if (json.contains(QStringLiteral("actual_start"))) update.actual_start = QDateTime::fromString(json[QStringLiteral("actual_start")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("actual_end"))) update.actual_end = QDateTime::fromString(json[QStringLiteral("actual_end")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("notes"))) update.notes = json[QStringLiteral("notes")].toString();
    if (json.contains(QStringLiteral("reported_by"))) update.reported_by = json[QStringLiteral("reported_by")].toString();
    if (json.contains(QStringLiteral("variance_hours"))) update.variance_hours = json[QStringLiteral("variance_hours")].toDouble();
    return update;
}

} // namespace source::gantt
