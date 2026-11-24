#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_20_mover_tarefa_por_item.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::mover_tarefa_por_item(QListWidgetItem *item)
{
    try {
        Tarefa *task = find_task(item);
        if (task == nullptr) {
            return;
        }

        const QStringList options = {
            QCoreApplication::translate("PomodoroPlugin", "📋 A Fazer"),
            QCoreApplication::translate("PomodoroPlugin", "⚙️ Em Progresso"),
            QCoreApplication::translate("PomodoroPlugin", "✅ Concluído"),
        };
        bool ok = false;
        const QString destination = QInputDialog::getItem(
            this,
            QCoreApplication::translate("PomodoroPlugin", "Mover Tarefa"),
            QCoreApplication::translate("PomodoroPlugin", "Selecione a coluna de destino:"),
            options,
            0,
            false,
            &ok);

        if (!ok || destination.isEmpty()) {
            return;
        }

        const QMap<QString, QString> mapping = {
            {options[0], QStringLiteral("Todo")},
            {options[1], QStringLiteral("Doing")},
            {options[2], QStringLiteral("Done")},
        };
        task->status = mapping.value(destination, QStringLiteral("Todo"));
        salvar_tarefas();
        atualizar_listas();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao mover tarefa por item (menu): %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao mover tarefa por item (menu)"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void mover_tarefa_por_item(GerenciadorTarefas &self, QListWidgetItem *item) { self.mover_tarefa_por_item(item); } }
