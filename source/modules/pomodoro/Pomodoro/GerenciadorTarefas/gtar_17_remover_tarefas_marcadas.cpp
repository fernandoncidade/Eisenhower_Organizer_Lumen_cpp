#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_17_remover_tarefas_marcadas.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::remover_tarefas_marcadas()
{
    try {
        const QVector<QListWidgetItem *> selected = collect_marked_items();
        if (selected.isEmpty()) {
            return;
        }

        if (!compat::MessageBox::tempo_confirm_delete_tasks(this, selected.size())) {
            return;
        }

        const int before = tarefas.size();
        QSet<QString> ids;
        QSet<QString> title_priority_keys;
        for (QListWidgetItem *item : selected) {
            const QString id = item->data(Qt::UserRole).toString();
            if (!id.isEmpty()) {
                ids.insert(id);
            } else {
                const QString line = item->text().split(QLatin1Char('\n')).value(0).trimmed();
                const QString title = compat::strip_priority_emoji(line).trimmed();
                const QString priority = Tarefa::normalizar_prioridade(priority_from_item_text(line));
                title_priority_keys.insert(QStringLiteral("%1|%2").arg(title, priority));
            }
        }

        const auto matches_marked_task = [&](const Tarefa &task) {
            return ids.contains(task.id)
                || title_priority_keys.contains(
                    QStringLiteral("%1|%2").arg(task.titulo, Tarefa::normalizar_prioridade(task.prioridade)));
        };

        QVector<QVariantMap> deleted_payloads;
        for (const Tarefa &task : tarefas) {
            if (matches_marked_task(task)) {
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
                matches_marked_task),
            tarefas.end());

        salvar_tarefas();
        atualizar_listas();

        const int removed = qMax(before - tarefas.size(), 0);
        if (removed > 0) {
            compat::MessageBox::tempo_delete_tasks_success(this, removed);
        } else {
            compat::MessageBox::tempo_delete_tasks_failure(this);
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao remover tarefas marcadas: %1").arg(QString::fromUtf8(e.what())), true);
        compat::MessageBox::tempo_delete_tasks_error(this);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao remover tarefas marcadas"), true);
        compat::MessageBox::tempo_delete_tasks_error(this);
    }
}

void GerenciadorTarefas::limpar_todas_as_tarefas()
{
    try {
        if (auto *event_bus = compat::get_event_bus(); event_bus != nullptr) {
            event_bus->clear_pending();
        }

        tarefas.clear();
        dragged_file_path_.clear();

        if (input_tarefa != nullptr) {
            input_tarefa->clear();
            input_tarefa->setToolTip(QString());
        }

        salvar_tarefas();
        atualizar_listas();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao limpar todas as tarefas do Pomodoro: %1").arg(QString::fromUtf8(e.what())),
                       true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao limpar todas as tarefas do Pomodoro"), true);
    }
}

void GerenciadorTarefas::limpar_selecao_tarefas()
{
    for (KanbanColumn *col : {col_todo, col_doing, col_done}) {
        if (col != nullptr && col->lista != nullptr) {
            col->lista->clearSelection();
            if (QItemSelectionModel *sm = col->lista->selectionModel(); sm != nullptr) {
                sm->clear();
            }
        }
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void remover_tarefas_marcadas(GerenciadorTarefas &self) { self.remover_tarefas_marcadas(); } }
