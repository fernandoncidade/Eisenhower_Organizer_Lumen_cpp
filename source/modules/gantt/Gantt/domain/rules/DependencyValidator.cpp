#include "DependencyValidator.hpp"
#include <QHash>
#include <QQueue>
#include <QSet>

namespace source::gantt {

namespace {

bool find_cycle_dfs(const QString &node,
                    const QHash<QString, QList<QString>> &adj,
                    QHash<QString, int> &color, // 0 = white, 1 = grey (in stack), 2 = black (visited)
                    QStringList &path,
                    QStringList &cycle)
{
    color[node] = 1;
    path.append(node);

    if (adj.contains(node)) {
        for (const QString &next : adj[node]) {
            int c = color.value(next, 0);
            if (c == 1) {
                // Cycle found! Reconstruct cycle path
                int idx = path.indexOf(next);
                if (idx != -1) {
                    for (int i = idx; i < path.size(); ++i) {
                        cycle.append(path[i]);
                    }
                    cycle.append(next);
                }
                return true;
            }
            if (c == 0) {
                if (find_cycle_dfs(next, adj, color, path, cycle)) {
                    return true;
                }
            }
        }
    }

    path.removeLast();
    color[node] = 2;
    return false;
}

} // anonymous namespace

DependencyValidator::CycleCheckResult DependencyValidator::check_cycles(
    const QList<QString> &item_ids,
    const QList<Dependency> &dependencies)
{
    CycleCheckResult result;

    QHash<QString, QList<QString>> adj;
    QHash<QString, int> in_degree;

    for (const QString &id : item_ids) {
        in_degree[id] = 0;
        adj[id] = QList<QString>();
    }

    for (const auto &dep : dependencies) {
        if (dep.predecessor_item_id == dep.successor_item_id) {
            result.has_cycle = true;
            result.cycle_path_ids = {dep.predecessor_item_id, dep.successor_item_id};
            result.error_message = QStringLiteral("Ciclo detectado: atividade autorreferencial (%1).").arg(dep.predecessor_item_id);
            return result;
        }
        if (item_ids.contains(dep.predecessor_item_id) && item_ids.contains(dep.successor_item_id)) {
            adj[dep.predecessor_item_id].append(dep.successor_item_id);
            in_degree[dep.successor_item_id]++;
        }
    }

    // Kahn's algorithm
    QQueue<QString> queue;
    for (const QString &id : item_ids) {
        if (in_degree[id] == 0) {
            queue.enqueue(id);
        }
    }

    int visited_count = 0;
    while (!queue.isEmpty()) {
        QString u = queue.dequeue();
        visited_count++;

        for (const QString &v : adj[u]) {
            in_degree[v]--;
            if (in_degree[v] == 0) {
                queue.enqueue(v);
            }
        }
    }

    if (visited_count < item_ids.size()) {
        // Cycle detected, use DFS to locate cycle path
        result.has_cycle = true;
        QHash<QString, int> color;
        QStringList path;
        QStringList cycle;

        for (const QString &id : item_ids) {
            if (color.value(id, 0) == 0) {
                if (find_cycle_dfs(id, adj, color, path, cycle)) {
                    result.cycle_path_ids = cycle;
                    break;
                }
            }
        }

        result.error_message = QStringLiteral("Ciclo de dependências detectado: %1")
                                   .arg(result.cycle_path_ids.join(QStringLiteral(" -> ")));
    }

    return result;
}

bool DependencyValidator::can_add_dependency(
    const QList<QString> &item_ids,
    const QList<Dependency> &existing_dependencies,
    const Dependency &new_dependency,
    QString *error_message)
{
    if (!new_dependency.is_valid(error_message)) {
        return false;
    }

    QList<Dependency> combined = existing_dependencies;
    combined.append(new_dependency);

    CycleCheckResult check = check_cycles(item_ids, combined);
    if (check.has_cycle) {
        if (error_message) {
            *error_message = check.error_message;
        }
        return false;
    }

    return true;
}

bool DependencyValidator::topological_sort(
    const QList<QString> &item_ids,
    const QList<Dependency> &dependencies,
    QList<QString> &sorted_ids,
    QString *error_message)
{
    sorted_ids.clear();

    QHash<QString, QList<QString>> adj;
    QHash<QString, int> in_degree;

    for (const QString &id : item_ids) {
        in_degree[id] = 0;
        adj[id] = QList<QString>();
    }

    for (const auto &dep : dependencies) {
        if (item_ids.contains(dep.predecessor_item_id) && item_ids.contains(dep.successor_item_id)) {
            adj[dep.predecessor_item_id].append(dep.successor_item_id);
            in_degree[dep.successor_item_id]++;
        }
    }

    QQueue<QString> queue;
    for (const QString &id : item_ids) {
        if (in_degree[id] == 0) {
            queue.enqueue(id);
        }
    }

    while (!queue.isEmpty()) {
        QString u = queue.dequeue();
        sorted_ids.append(u);

        for (const QString &v : adj[u]) {
            in_degree[v]--;
            if (in_degree[v] == 0) {
                queue.enqueue(v);
            }
        }
    }

    if (sorted_ids.size() < item_ids.size()) {
        if (error_message) {
            *error_message = QStringLiteral("Não é possível ordenar topologicamente: o cronograma possui ciclos de dependência.");
        }
        sorted_ids.clear();
        return false;
    }

    return true;
}

} // namespace source::gantt
