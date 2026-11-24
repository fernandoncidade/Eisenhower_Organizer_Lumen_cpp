#pragma once

#include <QDateTime>
#include <QString>
#include "../models/Schedule.hpp"

namespace source::gantt {

class CpmEngine {
public:
    struct CpmResult {
        bool success = false;
        QString error_message;
        QDateTime project_early_start;
        QDateTime project_early_finish;
        double total_duration_hours = 0.0;
        int critical_activities_count = 0;
        QList<QString> critical_path_ids;
    };

    // Recalculates CPM (Early/Late Dates, Total Float, Free Float, Critical Path)
    // and automatically updates start_date and end_date on schedule items.
    static CpmResult calculate(Schedule &schedule, const QDateTime &forced_start_date = QDateTime());

    // Propagates changes after a single activity's dates or duration have been edited
    static CpmResult reschedule_from_item(Schedule &schedule, const QString &changed_item_id);
};

} // namespace source::gantt
