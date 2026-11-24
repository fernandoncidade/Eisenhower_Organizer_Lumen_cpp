#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_15_ha_tarefas_marcadas.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

bool GerenciadorTarefas::_ha_tarefas_marcadas()
{
    try {
        return !collect_marked_items().isEmpty();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao verificar se há tarefas marcadas: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao verificar se há tarefas marcadas"), true);
    }

    return false;
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { bool ha_tarefas_marcadas(GerenciadorTarefas &self) { return self._ha_tarefas_marcadas(); } }
