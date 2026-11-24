#include "ScheduleItem.hpp"
#include "../../application/services/GanttTaskIntegrationBridge.hpp"
#include <QUuid>

namespace source::gantt {

ScheduleItem::ScheduleItem()
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      created_at(QDateTime::currentDateTime()),
      updated_at(QDateTime::currentDateTime())
{
}

ScheduleItem::ScheduleItem(const QString &item_name)
    : id(QUuid::createUuid().toString(QUuid::WithoutBraces)),
      name(item_name),
      created_at(QDateTime::currentDateTime()),
      updated_at(QDateTime::currentDateTime())
{
}

bool ScheduleItem::is_valid(QString *error_message) const
{
    if (name.trimmed().isEmpty()) {
        if (error_message) *error_message = QStringLiteral("Nome da atividade não pode ser vazio.");
        return false;
    }
    if (percent_complete < 0.0 || percent_complete > 100.0) {
        if (error_message) *error_message = QStringLiteral("Percentual de conclusão deve estar entre 0 e 100.");
        return false;
    }
    if (is_milestone && duration_hours != 0.0) {
        if (error_message) *error_message = QStringLiteral("Marcos (milestones) devem ter duração zero.");
        return false;
    }
    if (!is_milestone && duration_hours < 0.0) {
        if (error_message) *error_message = QStringLiteral("Duração não pode ser negativa.");
        return false;
    }
    if (start_date.isValid() && end_date.isValid() && end_date < start_date) {
        if (error_message) *error_message = QStringLiteral("Data de fim não pode ser anterior à data de início.");
        return false;
    }
    return true;
}

void ScheduleItem::sanitize()
{
    if (percent_complete < 0.0) percent_complete = 0.0;
    if (percent_complete > 100.0) percent_complete = 100.0;

    if (is_milestone) {
        duration_hours = 0.0;
        if (start_date.isValid()) {
            end_date = start_date;
        }
    }

    if (percent_complete >= 100.0) {
        status = ActivityStatus::Done;
    } else if (percent_complete > 0.0 && (status == ActivityStatus::NotStarted || status == ActivityStatus::Blocked)) {
        status = ActivityStatus::InProgress;
    }
}

double ScheduleItem::variance_start_hours() const
{
    if (!baseline_start_date.isValid() || !start_date.isValid()) {
        return 0.0;
    }
    // Positive means delayed (start is later than baseline)
    return baseline_start_date.secsTo(start_date) / 3600.0;
}

double ScheduleItem::variance_end_hours() const
{
    if (!baseline_end_date.isValid() || !end_date.isValid()) {
        return 0.0;
    }
    // Positive means delayed (finish is later than baseline)
    return baseline_end_date.secsTo(end_date) / 3600.0;
}

bool ScheduleItem::is_delayed() const
{
    // O atraso deve ser calculado e reportado apenas para tarefas/atividades em progresso/andamento
    // nos módulos Eisenhower e Pomodoro, considerando a data e horário presentes/atuais do SO.
    bool is_in_progress = false;

    if (!task_id.trimmed().isEmpty()) {
        const QVariantMap task = GanttTaskIntegrationBridge::instance().get_task_by_id(task_id);
        if (!task.isEmpty()) {
            bool done = false;
            if (task.contains(QStringLiteral("concluida"))) {
                done = task.value(QStringLiteral("concluida")).toBool();
            } else if (task.contains(QStringLiteral("completed"))) {
                done = task.value(QStringLiteral("completed")).toBool();
            } else {
                const QString st = task.value(QStringLiteral("status")).toString().toLower();
                done = (st == QStringLiteral("done") || st == QStringLiteral("concluida") ||
                        st == QStringLiteral("completed") || st == QStringLiteral("executado"));
            }

            if (!done) {
                const QString st = task.value(QStringLiteral("status")).toString().toLower();
                const QString cat = task.value(QStringLiteral("category")).toString().toLower();
                if (st == QStringLiteral("doing") ||
                    st.contains(QStringLiteral("progress")) ||
                    st.contains(QStringLiteral("andamento")) ||
                    cat.contains(QStringLiteral("doing")) ||
                    cat.contains(QStringLiteral("progress")) ||
                    cat.contains(QStringLiteral("andamento")) ||
                    task.value(QStringLiteral("em_progresso")).toBool() ||
                    task.value(QStringLiteral("in_progress")).toBool() ||
                    task.value(QStringLiteral("em_andamento")).toBool()) {
                    is_in_progress = true;
                }
            }
        }
    }

    // Se a tarefa não estiver em progresso/andamento, não é calculada como atrasada
    if (!is_in_progress) {
        return false;
    }

    const QDateTime now = QDateTime::currentDateTime();

    if (end_date.isValid()) {
        return end_date < now;
    }

    if (!task_id.trimmed().isEmpty()) {
        const QVariantMap task = GanttTaskIntegrationBridge::instance().get_task_by_id(task_id);
        const QString d_str = task.value(QStringLiteral("date"), task.value(QStringLiteral("data"))).toString().trimmed();
        const QString t_str = task.value(QStringLiteral("time"), task.value(QStringLiteral("hora"))).toString().trimmed();
        if (!d_str.isEmpty()) {
            QDate d = QDate::fromString(d_str, Qt::ISODate);
            if (!d.isValid()) d = QDate::fromString(d_str, QStringLiteral("yyyy-MM-dd"));
            if (!d.isValid()) d = QDate::fromString(d_str, QStringLiteral("dd/MM/yyyy"));
            if (d.isValid()) {
                QTime t = QTime::fromString(t_str, QStringLiteral("HH:mm"));
                if (!t.isValid()) t = QTime(23, 59, 59);
                const QDateTime dt(d, t);
                return dt < now;
            }
        }
    }

    if (start_date.isValid() && duration_hours > 0.0) {
        return start_date.addSecs(static_cast<qint64>(duration_hours * 3600.0)) < now;
    }

    return false;
}

QJsonObject ScheduleItem::to_json() const
{
    QJsonObject json;
    json[QStringLiteral("id")] = id;
    json[QStringLiteral("schedule_id")] = schedule_id;
    json[QStringLiteral("task_id")] = task_id;
    json[QStringLiteral("wbs_code")] = wbs_code;
    json[QStringLiteral("name")] = name;
    json[QStringLiteral("description")] = description;
    json[QStringLiteral("start_date")] = start_date.isValid() ? start_date.toString(Qt::ISODate) : QString();
    json[QStringLiteral("end_date")] = end_date.isValid() ? end_date.toString(Qt::ISODate) : QString();
    json[QStringLiteral("duration_hours")] = duration_hours;
    json[QStringLiteral("percent_complete")] = percent_complete;
    json[QStringLiteral("status")] = activity_status_to_string(status);
    json[QStringLiteral("is_milestone")] = is_milestone;
    json[QStringLiteral("critical")] = critical;
    json[QStringLiteral("early_start")] = early_start.isValid() ? early_start.toString(Qt::ISODate) : QString();
    json[QStringLiteral("early_finish")] = early_finish.isValid() ? early_finish.toString(Qt::ISODate) : QString();
    json[QStringLiteral("late_start")] = late_start.isValid() ? late_start.toString(Qt::ISODate) : QString();
    json[QStringLiteral("late_finish")] = late_finish.isValid() ? late_finish.toString(Qt::ISODate) : QString();
    json[QStringLiteral("total_float_hours")] = total_float_hours;
    json[QStringLiteral("free_float_hours")] = free_float_hours;
    json[QStringLiteral("baseline_start_date")] = baseline_start_date.isValid() ? baseline_start_date.toString(Qt::ISODate) : QString();
    json[QStringLiteral("baseline_end_date")] = baseline_end_date.isValid() ? baseline_end_date.toString(Qt::ISODate) : QString();
    json[QStringLiteral("baseline_duration_hours")] = baseline_duration_hours;
    json[QStringLiteral("external_system")] = external_system;
    json[QStringLiteral("external_id")] = external_id;
    json[QStringLiteral("version")] = version;
    json[QStringLiteral("created_at")] = created_at.toString(Qt::ISODate);
    json[QStringLiteral("updated_at")] = updated_at.toString(Qt::ISODate);
    return json;
}

ScheduleItem ScheduleItem::from_json(const QJsonObject &json)
{
    ScheduleItem item;
    if (json.contains(QStringLiteral("id"))) item.id = json[QStringLiteral("id")].toString();
    if (json.contains(QStringLiteral("schedule_id"))) item.schedule_id = json[QStringLiteral("schedule_id")].toString();
    if (json.contains(QStringLiteral("task_id"))) item.task_id = json[QStringLiteral("task_id")].toString();
    if (json.contains(QStringLiteral("wbs_code"))) item.wbs_code = json[QStringLiteral("wbs_code")].toString();
    if (json.contains(QStringLiteral("name"))) item.name = json[QStringLiteral("name")].toString();
    if (json.contains(QStringLiteral("description"))) item.description = json[QStringLiteral("description")].toString();
    if (json.contains(QStringLiteral("start_date"))) item.start_date = QDateTime::fromString(json[QStringLiteral("start_date")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("end_date"))) item.end_date = QDateTime::fromString(json[QStringLiteral("end_date")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("duration_hours"))) item.duration_hours = json[QStringLiteral("duration_hours")].toDouble();
    if (json.contains(QStringLiteral("percent_complete"))) item.percent_complete = json[QStringLiteral("percent_complete")].toDouble();
    if (json.contains(QStringLiteral("status"))) item.status = string_to_activity_status(json[QStringLiteral("status")].toString());
    if (json.contains(QStringLiteral("is_milestone"))) item.is_milestone = json[QStringLiteral("is_milestone")].toBool();
    if (json.contains(QStringLiteral("critical"))) item.critical = json[QStringLiteral("critical")].toBool();
    if (json.contains(QStringLiteral("early_start"))) item.early_start = QDateTime::fromString(json[QStringLiteral("early_start")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("early_finish"))) item.early_finish = QDateTime::fromString(json[QStringLiteral("early_finish")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("late_start"))) item.late_start = QDateTime::fromString(json[QStringLiteral("late_start")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("late_finish"))) item.late_finish = QDateTime::fromString(json[QStringLiteral("late_finish")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("total_float_hours"))) item.total_float_hours = json[QStringLiteral("total_float_hours")].toDouble();
    if (json.contains(QStringLiteral("free_float_hours"))) item.free_float_hours = json[QStringLiteral("free_float_hours")].toDouble();
    if (json.contains(QStringLiteral("baseline_start_date"))) item.baseline_start_date = QDateTime::fromString(json[QStringLiteral("baseline_start_date")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("baseline_end_date"))) item.baseline_end_date = QDateTime::fromString(json[QStringLiteral("baseline_end_date")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("baseline_duration_hours"))) item.baseline_duration_hours = json[QStringLiteral("baseline_duration_hours")].toDouble();
    if (json.contains(QStringLiteral("external_system"))) item.external_system = json[QStringLiteral("external_system")].toString();
    if (json.contains(QStringLiteral("external_id"))) item.external_id = json[QStringLiteral("external_id")].toString();
    if (json.contains(QStringLiteral("version"))) item.version = json[QStringLiteral("version")].toInt(1);
    if (json.contains(QStringLiteral("created_at"))) item.created_at = QDateTime::fromString(json[QStringLiteral("created_at")].toString(), Qt::ISODate);
    if (json.contains(QStringLiteral("updated_at"))) item.updated_at = QDateTime::fromString(json[QStringLiteral("updated_at")].toString(), Qt::ISODate);
    item.sanitize();
    return item;
}

} // namespace source::gantt
