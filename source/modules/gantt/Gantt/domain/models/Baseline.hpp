#pragma once

#include <QDateTime>
#include <QHash>
#include <QJsonObject>
#include <QString>

namespace source::gantt {

struct BaselineItemSnapshot {
    QString item_id;
    QDateTime start_date;
    QDateTime end_date;
    double duration_hours = 0.0;

    QJsonObject to_json() const;
    static BaselineItemSnapshot from_json(const QJsonObject &json);
};

class Baseline {
public:
    Baseline();
    explicit Baseline(const QString &baseline_name, const QString &sched_id);

    QString id;
    QString schedule_id;
    QString name;
    QDateTime created_at;
    bool frozen = false;
    QString created_by;
    QString notes;

    QHash<QString, BaselineItemSnapshot> items;

    bool can_modify(QString *error_message = nullptr) const;
    void freeze();
    bool unfreeze(bool has_special_permission, QString *error_message = nullptr);

    QJsonObject to_json() const;
    static Baseline from_json(const QJsonObject &json);
};

} // namespace source::gantt
