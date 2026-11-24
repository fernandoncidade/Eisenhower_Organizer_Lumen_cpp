#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_02_salvar_tarefas.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::salvar_tarefas()
{
    try {
        const QString origin = pending_shared_origin_.trimmed().isEmpty() ? QStringLiteral("pomodoro")
                                                                          : pending_shared_origin_.trimmed().toCaseFolded();
        pending_shared_origin_.clear();
        if (source::utils::UnifiedTaskStore::replace_module_records(arquivo_tarefas,
                                                                   QStringLiteral("pomodoro"),
                                                                   records_from_tasks(tarefas))
            && compat::host_bridge() != nullptr) {
            compat::host_bridge()->notify_shared_state_changed(origin);
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao salvar tarefas: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao salvar tarefas"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void salvar_tarefas(GerenciadorTarefas &self) { self.salvar_tarefas(); } }
