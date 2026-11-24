#pragma once

#include <QJsonObject>
#include <QString>
#include "GanttEnums.hpp"

namespace source::gantt {

class Dependency {
public:
    Dependency();
    Dependency(const QString &pred_id, const QString &succ_id, DependencyType dep_type = DependencyType::FinishToStart, double lag = 0.0);

    QString id;
    QString schedule_id;
    QString predecessor_item_id;
    QString successor_item_id;
    DependencyType type = DependencyType::FinishToStart;
    double lag_hours = 0.0;

    bool is_valid(QString *error_message = nullptr) const;

    QJsonObject to_json() const;
    static Dependency from_json(const QJsonObject &json);
};

} // namespace source::gantt
