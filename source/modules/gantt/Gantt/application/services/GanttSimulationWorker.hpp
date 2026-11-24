#pragma once

#include <QDateTime>
#include <QObject>
#include "Bim4DService.hpp"
#include "../../domain/models/Schedule.hpp"

namespace source::gantt {

/**
 * @brief Worker executado em QThread dedicada para processamento de simulações 4D BIM
 * sem bloquear a thread principal de interface gráfica.
 */
class GanttSimulationWorker : public QObject {
    Q_OBJECT

public:
    explicit GanttSimulationWorker(QObject *parent = nullptr);
    ~GanttSimulationWorker() override = default;

public slots:
    void do_simulate(const Schedule &schedule, const QDateTime &reference_date);

signals:
    void simulation_finished(const Bim4DSimulationReport &report);

private:
    Bim4DService bim_svc_;
};

} // namespace source::gantt
