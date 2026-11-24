#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_18_resetar_contagem_botao.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::resetar_contagem_botao()
{
    try {
        const QStringList scopes = {
            QCoreApplication::translate("PomodoroPlugin", "🔹 Uma tarefa específica"),
            QCoreApplication::translate("PomodoroPlugin", "📂 Todas as tarefas de uma coluna"),
            QCoreApplication::translate("PomodoroPlugin", "🌐 Todas as tarefas (global)"),
        };

        bool ok = false;
        const QString scope = QInputDialog::getItem(
            this,
            QCoreApplication::translate("PomodoroPlugin", "🔄 Resetar Contagem"),
            QCoreApplication::translate("PomodoroPlugin", "O que deseja restaurar?"),
            scopes,
            0,
            false,
            &ok);

        if (!ok || scope.isEmpty()) {
            return;
        }

        const QStringList columns = {
            QCoreApplication::translate("PomodoroPlugin", "📋 A Fazer"),
            QCoreApplication::translate("PomodoroPlugin", "⚙️ Em Progresso"),
            QCoreApplication::translate("PomodoroPlugin", "✅ Concluído"),
        };
        const QMap<QString, QString> statuses = {
            {columns[0], QStringLiteral("Todo")},
            {columns[1], QStringLiteral("Doing")},
            {columns[2], QStringLiteral("Done")},
        };

        if (scope == scopes[0] || scope == scopes[1]) {
            const QString column = QInputDialog::getItem(
                this,
                QCoreApplication::translate("PomodoroPlugin", "🔄 Resetar Contagem"),
                QCoreApplication::translate("PomodoroPlugin", "Selecione a coluna:"),
                columns,
                0,
                false,
                &ok);
            if (!ok || column.isEmpty()) {
                return;
            }

            QVector<Tarefa *> column_tasks;
            for (Tarefa &task : tarefas) {
                if (task.status == statuses.value(column)) {
                    column_tasks.append(&task);
                }
            }

            if (column_tasks.isEmpty()) {
                compat::MessageBox::tempo_no_tasks_in_column(this);
                return;
            }

            if (scope == scopes[0]) {
                QStringList titles;
                for (const Tarefa *task : column_tasks) {
                    titles.append(task->titulo);
                }

                const QString chosen = QInputDialog::getItem(
                    this,
                    QCoreApplication::translate("PomodoroPlugin", "🔄 Resetar Contagem"),
                    QCoreApplication::translate("PomodoroPlugin", "Selecione a tarefa:"),
                    titles,
                    0,
                    false,
                    &ok);
                if (!ok || chosen.isEmpty()) {
                    return;
                }

                for (Tarefa *task : column_tasks) {
                    if (task->titulo == chosen) {
                        task->pomodoros_completados = 0;
                        break;
                    }
                }
            } else {
                for (Tarefa *task : column_tasks) {
                    task->pomodoros_completados = 0;
                }
            }
        } else {
            if (tarefas.isEmpty()) {
                compat::MessageBox::tempo_no_tasks_anywhere(this);
                return;
            }

            for (Tarefa &task : tarefas) {
                task.pomodoros_completados = 0;
            }
        }

        salvar_tarefas();
        atualizar_listas();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao resetar contagem: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao resetar contagem"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void resetar_contagem_botao(GerenciadorTarefas &self) { self.resetar_contagem_botao(); } }
