#include "Schedule.hpp"
#include <QJsonArray>
#include <QUuid>
#include <algorithm>

namespace source::gantt {

Schedule::Schedule()
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      created_at(QDateTime::currentDateTime()),
      updated_at(QDateTime::currentDateTime())
{
}

Schedule::Schedule(const QString &schedule_name, const QString &proj_id)
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      project_id(proj_id),
      name(schedule_name),
      created_at(QDateTime::currentDateTime()),
      updated_at(QDateTime::currentDateTime())
{
}

ScheduleItem *Schedule::find_item(const QString &item_id)
{
    auto it = items.find(item_id);
    return it != items.end() ? &(*it) : nullptr;
}

const ScheduleItem *Schedule::find_item(const QString &item_id) const
{
    auto it = items.find(item_id);
    return it != items.end() ? &(*it) : nullptr;
}

ScheduleItem *Schedule::find_item_by_task_id(const QString &eisenhower_task_id)
{
    if (eisenhower_task_id.trimmed().isEmpty()) return nullptr;
    for (auto &item : items) {
        if (item.task_id == eisenhower_task_id) {
            return &item;
        }
    }
    return nullptr;
}

QList<Dependency> Schedule::get_predecessors_of(const QString &item_id) const
{
    QList<Dependency> result;
    for (const auto &dep : dependencies) {
        if (dep.successor_item_id == item_id) {
            result.append(dep);
        }
    }
    return result;
}

QList<Dependency> Schedule::get_successors_of(const QString &item_id) const
{
    QList<Dependency> result;
    for (const auto &dep : dependencies) {
        if (dep.predecessor_item_id == item_id) {
            result.append(dep);
        }
    }
    return result;
}

QList<ScheduleItem> Schedule::get_sorted_by_wbs() const
{
    QList<ScheduleItem> list = items.values();
    std::sort(list.begin(), list.end(), [](const ScheduleItem &a, const ScheduleItem &b) {
        if (a.wbs_code != b.wbs_code) {
            return a.wbs_code < b.wbs_code;
        }
        return a.start_date < b.start_date;
    });
    return list;
}

QList<ScheduleItem> Schedule::get_critical_path_items() const
{
    QList<ScheduleItem> list;
    for (const auto &item : items) {
        if (item.critical) {
            list.append(item);
        }
    }
    std::sort(list.begin(), list.end(), [](const ScheduleItem &a, const ScheduleItem &b) {
        return a.early_start < b.early_start;
    });
    return list;
}

bool Schedule::add_item(const ScheduleItem &item, QString *error_message)
{
    if (!item.is_valid(error_message)) {
        return false;
    }
    ScheduleItem copy = item;
    copy.schedule_id = id;
    copy.sanitize();
    items.insert(copy.id, copy);
    updated_at = QDateTime::currentDateTime();
    return true;
}

bool Schedule::update_item(const ScheduleItem &item, QString *error_message)
{
    if (!items.contains(item.id)) {
        if (error_message) *error_message = QStringLiteral("Atividade não encontrada no cronograma.");
        return false;
    }
    if (!item.is_valid(error_message)) {
        return false;
    }
    ScheduleItem copy = item;
    copy.schedule_id = id;
    copy.sanitize();
    copy.updated_at = QDateTime::currentDateTime();
    items.insert(copy.id, copy);
    updated_at = QDateTime::currentDateTime();
    return true;
}

bool Schedule::remove_item(const QString &item_id, QString *error_message)
{
    if (!items.contains(item_id)) {
        if (error_message) *error_message = QStringLiteral("Atividade não encontrada no cronograma.");
        return false;
    }
    items.remove(item_id);

    // Remove connected dependencies
    dependencies.removeIf([&item_id](const Dependency &d) {
        return d.predecessor_item_id == item_id || d.successor_item_id == item_id;
    });

    // Remove connected BIM links
    bim_links.removeIf([&item_id](const BimLink &b) {
        return b.schedule_item_id == item_id;
    });

    updated_at = QDateTime::currentDateTime();
    return true;
}

bool Schedule::add_dependency(const Dependency &dep, QString *error_message)
{
    if (!dep.is_valid(error_message)) {
        return false;
    }
    if (!items.contains(dep.predecessor_item_id)) {
        if (error_message) *error_message = QStringLiteral("Atividade predecessora não existe no cronograma.");
        return false;
    }
    if (!items.contains(dep.successor_item_id)) {
        if (error_message) *error_message = QStringLiteral("Atividade sucessora não existe no cronograma.");
        return false;
    }

    // Check for duplicate dependency
    for (const auto &existing : dependencies) {
        if (existing.predecessor_item_id == dep.predecessor_item_id &&
            existing.successor_item_id == dep.successor_item_id) {
            if (error_message) *error_message = QStringLiteral("Dependência entre estas atividades já existe.");
            return false;
        }
    }

    Dependency copy = dep;
    copy.schedule_id = id;
    dependencies.append(copy);
    updated_at = QDateTime::currentDateTime();
    return true;
}

bool Schedule::remove_dependency(const QString &dep_id, QString *error_message)
{
    qsizetype removed = dependencies.removeIf([&dep_id](const Dependency &d) {
        return d.id == dep_id;
    });
    if (removed == 0) {
        if (error_message) *error_message = QStringLiteral("Dependência não encontrada.");
        return false;
    }
    updated_at = QDateTime::currentDateTime();
    return true;
}

void Schedule::add_bim_link(const BimLink &link)
{
    BimLink copy = link;
    bim_links.append(copy);
    updated_at = QDateTime::currentDateTime();
}

bool Schedule::remove_bim_link(const QString &link_id)
{
    qsizetype removed = bim_links.removeIf([&link_id](const BimLink &b) {
        return b.id == link_id;
    });
    if (removed > 0) {
        updated_at = QDateTime::currentDateTime();
        return true;
    }
    return false;
}

QList<BimLink> Schedule::get_bim_links_for_item(const QString &item_id) const
{
    QList<BimLink> list;
    for (const auto &link : bim_links) {
        if (link.schedule_item_id == item_id) {
            list.append(link);
        }
    }
    return list;
}

QDateTime Schedule::project_start_date() const
{
    QDateTime earliest;
    for (const auto &item : items) {
        if (item.start_date.isValid()) {
            if (!earliest.isValid() || item.start_date < earliest) {
                earliest = item.start_date;
            }
        }
    }
    return earliest;
}

QDateTime Schedule::project_finish_date() const
{
    QDateTime latest;
    for (const auto &item : items) {
        if (item.end_date.isValid()) {
            if (!latest.isValid() || item.end_date > latest) {
                latest = item.end_date;
            }
        }
    }
    return latest;
}

double Schedule::total_duration_hours() const
{
    QDateTime s = project_start_date();
    QDateTime f = project_finish_date();
    if (!s.isValid() || !f.isValid() || f <= s) {
        return 0.0;
    }
    return calendar.working_hours_between(s, f);
}

double Schedule::overall_percent_complete() const
{
    if (items.isEmpty()) return 0.0;
    double weighted_progress = 0.0;
    double total_weight = 0.0;

    for (const auto &item : items) {
        double weight = item.duration_hours > 0 ? item.duration_hours : 1.0;
        weighted_progress += item.percent_complete * weight;
        total_weight += weight;
    }

    return total_weight > 0 ? (weighted_progress / total_weight) : 0.0;
}

int Schedule::critical_items_count() const
{
    int count = 0;
    for (const auto &item : items) {
        if (item.critical) count++;
    }
    return count;
}

int Schedule::delayed_items_count() const
{
    int count = 0;
    for (const auto &item : items) {
        if (item.is_delayed()) count++;
    }
    return count;
}

QJsonObject Schedule::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("id")] = id;
    json[QStringLiteral("project_id")] = project_id;
    json[QStringLiteral("name")] = name;
    json[QStringLiteral("description")] = description;
    json[QStringLiteral("active_baseline_id")] = active_baseline_id;
    json[QStringLiteral("created_at")] = created_at.toString(Qt::ISODate);
    json[QStringLiteral("updated_at")] = updated_at.toString(Qt::ISODate);
    json[QStringLiteral("calendar")] = calendar.to_json();

    QJsonArray items_array;
    for (const auto &item : items) {
        items_array.append(item.to_json());
    }
    json[QStringLiteral("items")] = items_array;

    QJsonArray deps_array;
    for (const auto &dep : dependencies) {
        deps_array.append(dep.to_json());
    }
    json[QStringLiteral("dependencies")] = deps_array;

    QJsonArray base_array;
    for (const auto &b : baselines) {
        base_array.append(b.to_json());
    }
    json[QStringLiteral("baselines")] = base_array;

    QJsonArray bim_array;
    for (const auto &link : bim_links) {
        bim_array.append(link.to_json());
    }
    json[QStringLiteral("bim_links")] = bim_array;

    QJsonArray prog_array;
    for (const auto &prog : progress_updates) {
        prog_array.append(prog.to_json());
    }
    json[QStringLiteral("progress_updates")] = prog_array;

    return json;
}

Schedule Schedule::from_json(const QJsonObject &json)
{
    Schedule sched;
    if (json.contains(QStringLiteral("id"))) sched.id = json[QStringLiteral("id")].toString();
    if (json.contains(QStringLiteral("project_id"))) sched.project_id = json[QStringLiteral("project_id")].toString();
    if (json.contains(QStringLiteral("name"))) sched.name = json[QStringLiteral("name")].toString();
    if (json.contains(QStringLiteral("description"))) sched.description = json[QStringLiteral("description")].toString();
    if (json.contains(QStringLiteral("active_baseline_id"))) sched.active_baseline_id = json[QStringLiteral("active_baseline_id")].toString();
    if (json.contains(QStringLiteral("created_at"))) sched.created_at = QDateTime::fromString(json[QStringLiteral("created_at")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("updated_at"))) sched.updated_at = QDateTime::fromString(json[QStringLiteral("updated_at")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("calendar"))) sched.calendar = WorkCalendar::from_json(json[QStringLiteral("calendar")].toObject());

    if (json.contains(QStringLiteral("items"))) {
        for (const auto &val : json[QStringLiteral("items")].toArray()) {
            ScheduleItem item = ScheduleItem::from_json(val.toObject());
            sched.items.insert(item.id, item);
        }
    }

    if (json.contains(QStringLiteral("dependencies"))) {
        for (const auto &val : json[QStringLiteral("dependencies")].toArray()) {
            sched.dependencies.append(Dependency::from_json(val.toObject()));
        }
    }

    if (json.contains(QStringLiteral("baselines"))) {
        for (const auto &val : json[QStringLiteral("baselines")].toArray()) {
            Baseline b = Baseline::from_json(val.toObject());
            sched.baselines.insert(b.id, b);
        }
    }

    if (json.contains(QStringLiteral("bim_links"))) {
        for (const auto &val : json[QStringLiteral("bim_links")].toArray()) {
            sched.bim_links.append(BimLink::from_json(val.toObject()));
        }
    }

    if (json.contains(QStringLiteral("progress_updates"))) {
        for (const auto &val : json[QStringLiteral("progress_updates")].toArray()) {
            sched.progress_updates.append(ProgressUpdate::from_json(val.toObject()));
        }
    }

    return sched;
}

} // namespace source::gantt
