#pragma once

#include <QColor>
#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>
#include "../../domain/models/BimLink.hpp"
#include "../../domain/models/Schedule.hpp"

namespace source::gantt {

struct Bim4DSimulationElement {
    QString bim_element_id;
    QString bim_source;
    QString schedule_item_id;
    QString activity_name;
    QString wbs_code;
    QString category;
    QString level;
    Bim4DStatus status = Bim4DStatus::Unstarted;
    QString status_string;
    QString color_hex; // e.g. "#4CAF50" (Green), "#2196F3" (Blue), "#F44336" (Red), "#9E9E9E" (Gray)
    double progress_percent = 0.0;
    QDateTime planned_start;
    QDateTime planned_end;

    QJsonObject to_json() const;
};

struct Bim4DSimulationReport {
    QString schedule_id;
    QDateTime reference_date;
    int total_elements = 0;
    int unstarted_count = 0;
    int planned_in_progress_count = 0;
    int executed_count = 0;
    int delayed_start_count = 0;
    int delayed_overdue_count = 0;

    QList<Bim4DSimulationElement> elements;

    QJsonObject to_json() const;
};

class Bim4DService {
public:
    Bim4DService();

    // Link management
    bool add_link(Schedule &schedule, const BimLink &link, QString *error_message = nullptr);
    bool remove_link(Schedule &schedule, const QString &link_id, QString *error_message = nullptr);
    QList<BimLink> list_links(const Schedule &schedule) const;

    // Temporal simulation engine
    Bim4DSimulationReport simulate(const Schedule &schedule, const QDateTime &reference_date) const;

    // Helper for color coding
    static QString status_to_color(Bim4DStatus status);
};

} // namespace source::gantt
