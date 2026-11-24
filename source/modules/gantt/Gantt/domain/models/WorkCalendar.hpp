#pragma once

#include <QDate>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QSet>
#include <QString>
#include <QTime>

namespace source::gantt {

struct CalendarException {
    QDate date;
    QString name;
    bool is_working_day = false; // false = holiday/day off, true = exceptional work day (e.g. Saturday)
    double working_hours = 0.0;

    QJsonObject to_json() const;
    static CalendarException from_json(const QJsonObject &json);
};

class WorkCalendar {
public:
    WorkCalendar();
    explicit WorkCalendar(const QString &calendar_name);

    QString id;
    QString name;
    
    // Working days of week: 1 = Monday, ..., 7 = Sunday. Default: 1..5 (Mon-Fri)
    QSet<int> working_days;
    
    // Daily shifts
    QTime morning_start = QTime(8, 0);
    QTime morning_end = QTime(12, 0);
    QTime afternoon_start = QTime(13, 0);
    QTime afternoon_end = QTime(17, 0);
    double hours_per_day = 8.0;

    QList<CalendarException> exceptions;

    bool is_working_day(const QDate &date) const;
    double working_hours_in_day(const QDate &date) const;

    // Time normalization: adjusts a given QDateTime to the next valid working moment
    QDateTime normalize_to_working_time(const QDateTime &dt) const;

    // Date math respecting working calendar
    QDateTime add_working_hours(const QDateTime &start_dt, double hours) const;
    QDateTime subtract_working_hours(const QDateTime &end_dt, double hours) const;
    QDateTime add_working_days(const QDateTime &start_dt, double days) const;
    double working_hours_between(const QDateTime &start_dt, const QDateTime &end_dt) const;
    double working_days_between(const QDateTime &start_dt, const QDateTime &end_dt) const;

    void add_holiday(const QDate &date, const QString &holiday_name);
    void remove_holiday(const QDate &date);
    bool is_holiday(const QDate &date) const;
    QString holiday_name(const QDate &date) const;
    void add_exception(const CalendarException &ex);

    QJsonObject to_json() const;
    static WorkCalendar from_json(const QJsonObject &json);
};

} // namespace source::gantt
