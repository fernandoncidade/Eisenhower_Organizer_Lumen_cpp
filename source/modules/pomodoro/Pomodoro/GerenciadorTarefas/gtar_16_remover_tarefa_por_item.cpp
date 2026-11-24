#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_16_remover_tarefa_por_item.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::remover_tarefa_por_item(QListWidgetItem *item)
{
    try {
        if (item == nullptr) {
            return;
        }

        const QString line = item->text().split(QLatin1Char('\n')).value(0).trimmed();
        const QString title = compat::strip_priority_emoji(line).trimmed();
        const QString priority = Tarefa::normalizar_prioridade(priority_from_item_text(line));
        if (!compat::MessageBox::tempo_confirm_delete_task(this, title)) {
            return;
        }

        const int before = tarefas.size();
        const QString task_id = item->data(Qt::UserRole).toString();
        const auto matches_task = [&](const Tarefa &task) {
            if (!task_id.isEmpty()) {
                return task.id == task_id;
            }
            if (task.titulo != title) {
                return false;
            }

            if (priority.isEmpty()) {
                return false;
            }

            return Tarefa::normalizar_prioridade(task.prioridade) == priority;
        };

        QVector<QVariantMap> deleted_payloads;
        for (const Tarefa &task : tarefas) {
            if (matches_task(task)) {
                deleted_payloads.append(task_payload(task));
            }
        }

        if (!deleted_payloads.isEmpty() && compat::host_bridge() != nullptr) {
            compat::host_bridge()->show_sync_operation_info(QStringLiteral("delete"), QStringLiteral("pomodoro"));
            for (const QVariantMap &payload : deleted_payloads) {
                compat::host_bridge()->publish_task_deleted(payload, QStringLiteral("pomodoro"));
            }
        }

        tarefas.erase(
            std::remove_if(
                tarefas.begin(),
                tarefas.end(),
                matches_task),
            tarefas.end());

        salvar_tarefas();
        atualizar_listas();

        if (tarefas.size() < before) {
            compat::MessageBox::tempo_delete_task_success(this);
        } else {
            compat::MessageBox::tempo_delete_task_failure(this);
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao remover tarefa pelo menu de contexto: %1").arg(QString::fromUtf8(e.what())), true);
        compat::MessageBox::tempo_delete_task_error(this);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao remover tarefa pelo menu de contexto"), true);
        compat::MessageBox::tempo_delete_task_error(this);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void remover_tarefa_por_item(GerenciadorTarefas &self, QListWidgetItem *item) { self.remover_tarefa_por_item(item); } }
