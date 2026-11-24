#include "GanttSimulationWorker.hpp"

namespace source::gantt {

GanttSimulationWorker::GanttSimulationWorker(QObject *parent)
    : QObject(parent)
{
}

void GanttSimulationWorker::do_simulate(const Schedule &schedule, const QDateTime &reference_date)
{
    Bim4DSimulationReport report = bim_svc_.simulate(schedule, reference_date);
    emit simulation_finished(report);
}

} // namespace source::gantt
