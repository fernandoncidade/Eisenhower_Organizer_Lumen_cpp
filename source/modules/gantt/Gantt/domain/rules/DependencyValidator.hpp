#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include "../models/Dependency.hpp"
#include "../models/ScheduleItem.hpp"

namespace source::gantt {

class DependencyValidator {
public:
    struct CycleCheckResult {
        bool has_cycle = false;
        QStringList cycle_path_ids;
        QString error_message;
    };

    // Validates whether candidate_dependencies form a Directed Acyclic Graph (DAG).
    static CycleCheckResult check_cycles(const QList<QString> &item_ids,
                                         const QList<Dependency> &dependencies);

    // Tests if adding a new dependency to the existing schedule creates a cycle
    static bool can_add_dependency(const QList<QString> &item_ids,
                                  const QList<Dependency> &existing_dependencies,
                                  const Dependency &new_dependency,
                                  QString *error_message = nullptr);

    // Returns topologically sorted item IDs if acyclic, or empty if cyclic
    static bool topological_sort(const QList<QString> &item_ids,
                                 const QList<Dependency> &dependencies,
                                 QList<QString> &sorted_ids,
                                 QString *error_message = nullptr);
};

} // namespace source::gantt
