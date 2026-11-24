#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_12_mover_tarefa.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::mover_tarefa(QListWidgetItem *item, const QString &status_atual)
{
    try {
        if (Tarefa *task = find_task(item)) {
            if (status_atual == QStringLiteral("Todo")) {
                task->status = QStringLiteral("Doing");
            } else if (status_atual == QStringLiteral("Doing")) {
                task->status = QStringLiteral("Done");
            } else {
                task->status = QStringLiteral("Todo");
            }

            salvar_tarefas();
            atualizar_listas();
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao mover tarefa: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao mover tarefa"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void mover_tarefa(GerenciadorTarefas &self, QListWidgetItem *item, const QString &status_atual) { self.mover_tarefa(item, status_atual); } }
