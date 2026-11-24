#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_08_sincronizar_status_pos_movimento.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::_sincronizar_status_pos_movimento(QListWidget *prioridade_lista)
{
    try {
        const QList<QListWidget *> lists = {col_todo->lista, col_doing->lista, col_done->lista};
        bool changed = false;

        QList<QListWidget *> ordered = lists;
        if (prioridade_lista != nullptr && ordered.removeOne(prioridade_lista)) {
            ordered.append(prioridade_lista);
        }

        for (QListWidget *list : ordered) {
            const QString status = task_status_for_list(this, list);
            for (int i = 0; i < list->count(); ++i) {
                if (auto *item = list->item(i)) {
                    if (Tarefa *task = find_task(item)) {
                        if (task->status != status) {
                            task->status = status;
                            changed = true;
                        }
                    }
                }
            }
        }

        if (changed) {
            salvar_tarefas();
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao sincronizar status após movimento: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao sincronizar status após movimento"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void sincronizar_status_pos_movimento(GerenciadorTarefas &self, QListWidget *prioridade_lista) { self._sincronizar_status_pos_movimento(prioridade_lista); } }
