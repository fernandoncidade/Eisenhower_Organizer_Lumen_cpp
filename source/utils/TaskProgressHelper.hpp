#pragma once

#include <algorithm>

namespace source::utils {

/**
 * @brief Calcula o percentual de conclusão de uma tarefa com base nos ciclos concluídos e planejados.
 * 
 * Regra:
 * - Se a tarefa está marcada como concluída (is_done == true), percentual é 100.0%.
 * - Se ciclos_planejados <= 0 ou ciclos_concluidos <= 0, percentual é 0.0% (evita divisão por zero).
 * - Se ciclos_concluidos >= ciclos_planejados, percentual é 100.0%.
 * - Em andamento: (ciclos_concluidos / (double)ciclos_planejados) * 100.0, limitado a [0.0, 100.0].
 */
inline double calculate_task_completion_percentage(int ciclos_concluidos, int ciclos_planejados, bool is_done = false)
{
    if (is_done) {
        return 100.0;
    }
    if (ciclos_planejados <= 0) {
        return 0.0;
    }
    if (ciclos_concluidos <= 0) {
        return 0.0;
    }
    if (ciclos_concluidos >= ciclos_planejados) {
        return is_done ? 100.0 : 99.0;
    }
    const double percent = (static_cast<double>(ciclos_concluidos) / static_cast<double>(ciclos_planejados)) * 100.0;
    return std::clamp(percent, 0.0, is_done ? 100.0 : 99.0);
}

}  // namespace source::utils
