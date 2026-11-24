#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>

namespace source::gantt {

class ProgressUpdate {
public:
    ProgressUpdate();
    explicit ProgressUpdate(const QString &item_id, double percent, const QString &author = QString());

    QString id;
    QString schedule_item_id;
    QDateTime timestamp;
    double percent_complete = 0.0;
    double previous_percent = 0.0;
    QDateTime actual_start;
    QDateTime actual_end;
    QString notes;
    QString reported_by;
    double variance_hours = 0.0;

    QJsonObject to_json() const;
    static ProgressUpdate from_json(const QJsonObject &json);
};

} // namespace source::gantt
