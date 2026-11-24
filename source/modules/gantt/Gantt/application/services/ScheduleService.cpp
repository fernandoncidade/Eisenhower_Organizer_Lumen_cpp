#include "ScheduleService.hpp"
#include <QUuid>

namespace source::gantt {

ScheduleService::ScheduleService()
{
}

Schedule ScheduleService::create_schedule(const QString &name, const QString &project_id, const QString &desc)
{
    Schedule s(name, project_id);
    s.description = desc;
    schedules_.insert(s.id, s);
    return s;
}

void ScheduleService::add_schedule(const Schedule &schedule)
{
    schedules_.insert(schedule.id, schedule);
}

Schedule *ScheduleService::get_schedule(const QString &schedule_id)
{
    auto it = schedules_.find(schedule_id);
    return it != schedules_.end() ? &(*it) : nullptr;
}

const Schedule *ScheduleService::get_schedule(const QString &schedule_id) const
{
    auto it = schedules_.find(schedule_id);
    return it != schedules_.end() ? &(*it) : nullptr;
}

QList<Schedule> ScheduleService::list_schedules() const
{
    return schedules_.values();
}

bool ScheduleService::delete_schedule(const QString &schedule_id)
{
    return schedules_.remove(schedule_id);
}

bool ScheduleService::add_activity(const QString &schedule_id, const ScheduleItem &item, QString *error_message)
{
    Schedule *s = get_schedule(schedule_id);
    if (!s) {
        if (error_message) *error_message = QStringLiteral("Cronograma não encontrado.");
        return false;
    }
    return s->add_item(item, error_message);
}

bool ScheduleService::update_activity(const QString &schedule_id, const ScheduleItem &item, QString *error_message)
{
    Schedule *s = get_schedule(schedule_id);
    if (!s) {
        if (error_message) *error_message = QStringLiteral("Cronograma não encontrado.");
        return false;
    }
    return s->update_item(item, error_message);
}

bool ScheduleService::delete_activity(const QString &schedule_id, const QString &item_id, QString *error_message)
{
    Schedule *s = get_schedule(schedule_id);
    if (!s) {
        if (error_message) *error_message = QStringLiteral("Cronograma não encontrado.");
        return false;
    }
    return s->remove_item(item_id, error_message);
}

ScheduleItem *ScheduleService::get_activity(const QString &schedule_id, const QString &item_id)
{
    Schedule *s = get_schedule(schedule_id);
    if (!s) return nullptr;
    return s->find_item(item_id);
}

bool ScheduleService::add_dependency(const QString &schedule_id, const Dependency &dep, QString *error_message)
{
    Schedule *s = get_schedule(schedule_id);
    if (!s) {
        if (error_message) *error_message = QStringLiteral("Cronograma não encontrado.");
        return false;
    }
    return s->add_dependency(dep, error_message);
}

bool ScheduleService::delete_dependency(const QString &schedule_id, const QString &dep_id, QString *error_message)
{
    Schedule *s = get_schedule(schedule_id);
    if (!s) {
        if (error_message) *error_message = QStringLiteral("Cronograma não encontrado.");
        return false;
    }
    return s->remove_dependency(dep_id, error_message);
}

Schedule ScheduleService::create_from_template(const QString &typology_name, const QString &project_id, const QString &new_schedule_name)
{
    Schedule s(new_schedule_name, project_id);
    s.description = QStringLiteral("Cronograma gerado a partir do template de tipologia: %1").arg(typology_name);

    QDateTime base_date = QDateTime::currentDateTime();
    base_date.setTime(s.calendar.morning_start);
    base_date = s.calendar.normalize_to_working_time(base_date);

    // Build standard construction WBS activities
    struct TemplateTask {
        QString wbs;
        QString name;
        double days;
        bool is_milestone;
    };

    QList<TemplateTask> t_tasks;
    if (typology_name.contains(QStringLiteral("Comercial"), Qt::CaseInsensitive)) {
        t_tasks = {
            {QStringLiteral("1.0"), QStringLiteral("Planejamento e Projetos"), 10.0, false},
            {QStringLiteral("1.1"), QStringLiteral("Aprovação Legal e Licenciamento"), 15.0, false},
            {QStringLiteral("2.0"), QStringLiteral("Mobilização e Canteiro"), 5.0, false},
            {QStringLiteral("2.1"), QStringLiteral("Fundações Profundas"), 20.0, false},
            {QStringLiteral("3.0"), QStringLiteral("Estrutura Metálica e Concreto"), 30.0, false},
            {QStringLiteral("3.1"), QStringLiteral("Fechamento Fachada Pele de Vidro"), 25.0, false},
            {QStringLiteral("4.0"), QStringLiteral("Instalações HVAC e Elétrica"), 35.0, false},
            {QStringLiteral("5.0"), QStringLiteral("Comissionamento e Habite-se"), 10.0, false},
            {QStringLiteral("5.1"), QStringLiteral("Entrega da Obra (Marco)"), 0.0, true}
        };
    } else {
        // Residential default
        t_tasks = {
            {QStringLiteral("1.0"), QStringLiteral("Serviços Preliminares e Canteiro"), 5.0, false},
            {QStringLiteral("1.1"), QStringLiteral("Terraplenagem e Contenções"), 10.0, false},
            {QStringLiteral("2.0"), QStringLiteral("Fundações (Blocos e Estacas)"), 15.0, false},
            {QStringLiteral("2.1"), QStringLiteral("Conclusão Fundação (Marco)"), 0.0, true},
            {QStringLiteral("3.0"), QStringLiteral("Superestrutura de Concreto Armado"), 40.0, false},
            {QStringLiteral("4.0"), QStringLiteral("Alvenaria e Vedações"), 20.0, false},
            {QStringLiteral("5.0"), QStringLiteral("Instalações Hidráulicas e Elétricas"), 25.0, false},
            {QStringLiteral("6.0"), QStringLiteral("Revestimentos e Acabamentos"), 30.0, false},
            {QStringLiteral("7.0"), QStringLiteral("Limpeza Final e Vistoria"), 5.0, false},
            {QStringLiteral("7.1"), QStringLiteral("Entrega das Chaves (Marco)"), 0.0, true}
        };
    }

    QList<QString> created_ids;
    QDateTime current_start = base_date;

    for (const auto &t : t_tasks) {
        ScheduleItem item(t.name);
        item.wbs_code = t.wbs;
        item.duration_hours = t.days * 8.0;
        item.is_milestone = t.is_milestone;
        item.start_date = current_start;
        item.end_date = t.is_milestone ? current_start : s.calendar.add_working_days(current_start, t.days);
        item.sanitize();

        s.add_item(item);
        created_ids.append(item.id);

        if (!t.is_milestone) {
            current_start = item.end_date;
        }
    }

    // Link sequentially FS
    for (int i = 0; i < created_ids.size() - 1; ++i) {
        Dependency dep(created_ids[i], created_ids[i + 1], DependencyType::FinishToStart, 0.0);
        s.add_dependency(dep);
    }

    schedules_.insert(s.id, s);
    return s;
}

} // namespace source::gantt
