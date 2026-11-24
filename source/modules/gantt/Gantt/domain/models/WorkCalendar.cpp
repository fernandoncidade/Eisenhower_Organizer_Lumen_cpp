#include "WorkCalendar.hpp"
#include <QUuid>
#include <algorithm>

namespace source::gantt {

QJsonObject CalendarException::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("date")] = date.toString(Qt::ISODate);
    json[QStringLiteral("name")] = name;
    json[QStringLiteral("is_working_day")] = is_working_day;
    json[QStringLiteral("working_hours")] = working_hours;
    return json;
}

CalendarException CalendarException::from_json(const QJsonObject &json)
{
    CalendarException ex;
    ex.date = QDate::fromString(json[QStringLiteral("date")].toString(), Qt::ISODate);
    ex.name = json[QStringLiteral("name")].toString();
    ex.is_working_day = json[QStringLiteral("is_working_day")].toBool(false);
    ex.working_hours = json[QStringLiteral("working_hours")].toDouble(0.0);
    return ex;
}

WorkCalendar::WorkCalendar()
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      name(QStringLiteral("Calendário Padrão de Obra"))
{
    // Monday (1) to Friday (5)
    working_days = {1, 2, 3, 4, 5};
}

WorkCalendar::WorkCalendar(const QString &calendar_name)
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      name(calendar_name)
{
    working_days = {1, 2, 3, 4, 5};
}

bool WorkCalendar::is_working_day(const QDate &date) const
{
    for (const auto &ex : exceptions) {
        if (ex.date == date) {
            return ex.is_working_day;
        }
    }
    return working_days.contains(date.dayOfWeek());
}

double WorkCalendar::working_hours_in_day(const QDate &date) const
{
    for (const auto &ex : exceptions) {
        if (ex.date == date) {
            return ex.is_working_day ? (ex.working_hours > 0 ? ex.working_hours : hours_per_day) : 0.0;
        }
    }
    return working_days.contains(date.dayOfWeek()) ? hours_per_day : 0.0;
}

QDateTime WorkCalendar::normalize_to_working_time(const QDateTime &dt) const
{
    if (!dt.isValid()) return dt;

    QDateTime current = dt;
    // Step forward until we hit a working day
    while (!is_working_day(current.date())) {
        current = current.addDays(1);
    }

    // If pure date or time is invalid or 00:00:00, default to morning_start
    if (!current.time().isValid() || current.time() == QTime(0, 0)) {
        current.setTime(morning_start);
    }

    return current;
}

QDateTime WorkCalendar::add_working_hours(const QDateTime &start_dt, double hours) const
{
    if (!start_dt.isValid()) return start_dt;
    if (hours <= 0.0) return normalize_to_working_time(start_dt);

    QDateTime current = normalize_to_working_time(start_dt);
    double remaining_hours = hours;
    const double hpd = (hours_per_day > 0.0 ? hours_per_day : 8.0);

    // If start time is outside standard daytime working shifts (e.g. evening >= afternoon_end like 17:30,
    // or early morning < morning_start like 07:30, or lunch), handle working days preservation:
    const QTime t_start = current.time();
    const bool outside_shifts = (t_start >= afternoon_end || t_start < morning_start ||
                                 (t_start >= morning_end && t_start < afternoon_start));

    if (outside_shifts) {
        // Advance by full working days while preserving the user's scheduled time
        const double days = remaining_hours / hpd;
        const int full_days = static_cast<int>(days);
        const double rem_hours = remaining_hours - (full_days * hpd);

        for (int i = 0; i < full_days; ++i) {
            current = current.addDays(1);
            while (!is_working_day(current.date())) {
                current = current.addDays(1);
            }
        }

        if (rem_hours > 0.0001) {
            current = current.addSecs(static_cast<qint64>(rem_hours * 3600.0));
            while (!is_working_day(current.date())) {
                current = current.addDays(1);
            }
        }

        return current;
    }

    // Inside standard working shifts: standard shift calculation
    while (remaining_hours > 0.0001) {
        if (!is_working_day(current.date())) {
            current = current.addDays(1);
            current.setTime(morning_start);
            continue;
        }

        QTime t = current.time();
        if (t < morning_start) {
            current.setTime(morning_start);
            t = morning_start;
        }

        // Determine current period remaining hours
        if (t < morning_end) {
            double morning_available = t.secsTo(morning_end) / 3600.0;
            if (remaining_hours <= morning_available) {
                return current.addSecs(static_cast<qint64>(remaining_hours * 3600.0));
            }
            remaining_hours -= morning_available;
            current.setTime(afternoon_start);
        }

        t = current.time();
        if (t >= afternoon_start && t < afternoon_end) {
            double afternoon_available = t.secsTo(afternoon_end) / 3600.0;
            if (remaining_hours <= afternoon_available) {
                return current.addSecs(static_cast<qint64>(remaining_hours * 3600.0));
            }
            remaining_hours -= afternoon_available;
        }

        // Advance to next day morning
        current = current.addDays(1);
        current.setTime(morning_start);
    }

    return current;
}

QDateTime WorkCalendar::subtract_working_hours(const QDateTime &end_dt, double hours) const
{
    if (!end_dt.isValid()) return end_dt;
    if (hours <= 0.0) return end_dt;

    QDateTime current = end_dt;
    while (!is_working_day(current.date())) {
        current = current.addDays(-1);
    }

    const double hpd = (hours_per_day > 0.0 ? hours_per_day : 8.0);
    const QTime t_end = current.time();
    const bool outside_shifts = (t_end >= afternoon_end || t_end < morning_start ||
                                 (t_end >= morning_end && t_end < afternoon_start));

    if (outside_shifts) {
        const double days = hours / hpd;
        const int full_days = static_cast<int>(days);
        const double rem_hours = hours - (full_days * hpd);

        for (int i = 0; i < full_days; ++i) {
            current = current.addDays(-1);
            while (!is_working_day(current.date())) {
                current = current.addDays(-1);
            }
        }

        if (rem_hours > 0.0001) {
            current = current.addSecs(-static_cast<qint64>(rem_hours * 3600.0));
            while (!is_working_day(current.date())) {
                current = current.addDays(-1);
            }
        }

        return current;
    }

    QTime t = current.time();
    if (t > afternoon_end) {
        current.setTime(afternoon_end);
    } else if (t > morning_end && t < afternoon_start) {
        current.setTime(morning_end);
    } else if (t < morning_start) {
        current = current.addDays(-1);
        current.setTime(afternoon_end);
        while (!is_working_day(current.date())) {
            current = current.addDays(-1);
            current.setTime(afternoon_end);
        }
    }

    double remaining_hours = hours;
    while (remaining_hours > 0.0001) {
        if (!is_working_day(current.date())) {
            current = current.addDays(-1);
            current.setTime(afternoon_end);
            continue;
        }

        t = current.time();
        if (t > afternoon_start && t <= afternoon_end) {
            double afternoon_avail = afternoon_start.secsTo(t) / 3600.0;
            if (remaining_hours <= afternoon_avail) {
                return current.addSecs(-static_cast<qint64>(remaining_hours * 3600.0));
            }
            remaining_hours -= afternoon_avail;
            current.setTime(morning_end);
        }

        t = current.time();
        if (t > morning_start && t <= morning_end) {
            double morning_avail = morning_start.secsTo(t) / 3600.0;
            if (remaining_hours <= morning_avail) {
                return current.addSecs(-static_cast<qint64>(remaining_hours * 3600.0));
            }
            remaining_hours -= morning_avail;
            current = current.addDays(-1);
            current.setTime(afternoon_end);
        } else if (t <= morning_start) {
            current = current.addDays(-1);
            current.setTime(afternoon_end);
        }
    }

    return current;
}

QDateTime WorkCalendar::add_working_days(const QDateTime &start_dt, double days) const
{
    return add_working_hours(start_dt, days * hours_per_day);
}

double WorkCalendar::working_hours_between(const QDateTime &start_dt, const QDateTime &end_dt) const
{
    if (!start_dt.isValid() || !end_dt.isValid() || end_dt <= start_dt) {
        return 0.0;
    }

    QDateTime current = normalize_to_working_time(start_dt);
    double total_hours = 0.0;

    while (current < end_dt) {
        if (is_working_day(current.date())) {
            QTime t = current.time();
            if (t < morning_end) {
                QDateTime m_end(current.date(), morning_end);
                QDateTime limit = std::min(m_end, end_dt);
                if (current < limit) {
                    total_hours += current.secsTo(limit) / 3600.0;
                }
            }
            if (current.date() == end_dt.date() && end_dt.time() <= afternoon_start) {
                break;
            }
            QDateTime a_start(current.date(), afternoon_start);
            QDateTime a_end(current.date(), afternoon_end);
            QDateTime from = std::max(current, a_start);
            QDateTime to = std::min(a_end, end_dt);
            if (from < to) {
                total_hours += from.secsTo(to) / 3600.0;
            }
        }

        current = current.addDays(1);
        current.setTime(morning_start);
    }

    if (total_hours <= 0.0001 && is_working_day(start_dt.date()) && start_dt.date() == end_dt.date()) {
        total_hours = start_dt.secsTo(end_dt) / 3600.0;
    }

    return total_hours;
}

double WorkCalendar::working_days_between(const QDateTime &start_dt, const QDateTime &end_dt) const
{
    return working_hours_between(start_dt, end_dt) / (hours_per_day > 0 ? hours_per_day : 8.0);
}

void WorkCalendar::add_holiday(const QDate &date, const QString &holiday_name)
{
    for (auto &ex : exceptions) {
        if (ex.date == date && !ex.is_working_day) {
            ex.name = holiday_name;
            return;
        }
    }
    CalendarException ex;
    ex.date = date;
    ex.name = holiday_name;
    ex.is_working_day = false;
    ex.working_hours = 0.0;
    exceptions.append(ex);
}

void WorkCalendar::remove_holiday(const QDate &date)
{
    exceptions.removeIf([&date](const CalendarException &ex) {
        return ex.date == date && !ex.is_working_day;
    });
}

bool WorkCalendar::is_holiday(const QDate &date) const
{
    for (const auto &ex : exceptions) {
        if (ex.date == date && !ex.is_working_day) {
            return true;
        }
    }
    return false;
}

QString WorkCalendar::holiday_name(const QDate &date) const
{
    for (const auto &ex : exceptions) {
        if (ex.date == date && !ex.is_working_day) {
            return ex.name;
        }
    }
    return QString();
}

void WorkCalendar::add_exception(const CalendarException &ex)
{
    exceptions.append(ex);
}

QJsonObject WorkCalendar::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("id")] = id;
    json[QStringLiteral("name")] = name;
    json[QStringLiteral("hours_per_day")] = hours_per_day;
    json[QStringLiteral("morning_start")] = morning_start.toString(QStringLiteral("HH:mm"));
    json[QStringLiteral("morning_end")] = morning_end.toString(QStringLiteral("HH:mm"));
    json[QStringLiteral("afternoon_start")] = afternoon_start.toString(QStringLiteral("HH:mm"));
    json[QStringLiteral("afternoon_end")] = afternoon_end.toString(QStringLiteral("HH:mm"));

    QJsonArray days_array;
    for (int day : working_days) {
        days_array.append(day);
    }
    json[QStringLiteral("working_days")] = days_array;

    QJsonArray ex_array;
    for (const auto &ex : exceptions) {
        ex_array.append(ex.to_json());
    }
    json[QStringLiteral("exceptions")] = ex_array;

    return json;
}

WorkCalendar WorkCalendar::from_json(const QJsonObject &json)
{
    WorkCalendar cal;
    if (json.contains(QStringLiteral("id"))) cal.id = json[QStringLiteral("id")].toString();
    if (json.contains(QStringLiteral("name"))) cal.name = json[QStringLiteral("name")].toString();
    if (json.contains(QStringLiteral("hours_per_day"))) cal.hours_per_day = json[QStringLiteral("hours_per_day")].toDouble(8.0);
    if (json.contains(QStringLiteral("morning_start"))) cal.morning_start = QTime::fromString(json[QStringLiteral("morning_start")].toString(), QStringLiteral("HH:mm"));
    if (json.contains(QStringLiteral("morning_end"))) cal.morning_end = QTime::fromString(json[QStringLiteral("morning_end")].toString(), QStringLiteral("HH:mm"));
    if (json.contains(QStringLiteral("afternoon_start"))) cal.afternoon_start = QTime::fromString(json[QStringLiteral("afternoon_start")].toString(), QStringLiteral("HH:mm"));
    if (json.contains(QStringLiteral("afternoon_end"))) cal.afternoon_end = QTime::fromString(json[QStringLiteral("afternoon_end")].toString(), QStringLiteral("HH:mm"));

    if (json.contains(QStringLiteral("working_days"))) {
        cal.working_days.clear();
        for (const auto &val : json[QStringLiteral("working_days")].toArray()) {
            cal.working_days.insert(val.toInt());
        }
    }

    if (json.contains(QStringLiteral("exceptions"))) {
        cal.exceptions.clear();
        for (const auto &val : json[QStringLiteral("exceptions")].toArray()) {
            cal.exceptions.append(CalendarException::from_json(val.toObject()));
        }
    }

    return cal;
}

} // namespace source::gantt
