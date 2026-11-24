#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_19_mover_tarefas_marcadas.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::mover_tarefas_marcadas()
{
    try {
        const QVector<QListWidgetItem *> selected = collect_marked_items();
        if (selected.isEmpty()) {
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
            QCoreApplication::translate("PomodoroPlugin", "Mover Tarefas"),
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
        const QString status = mapping.value(destination, QStringLiteral("Todo"));

        QSet<QString> ids;
        QSet<QString> titles;
        for (QListWidgetItem *item : selected) {
            const QString id = item->data(Qt::UserRole).toString();
            if (!id.isEmpty()) {
                ids.insert(id);
            } else {
                titles.insert(strip_item_title(item));
            }
        }

        for (Tarefa &task : tarefas) {
            if (ids.contains(task.id) || titles.contains(task.titulo)) {
                task.status = status;
            }
        }

        salvar_tarefas();
        atualizar_listas();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao mover tarefas marcadas: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao mover tarefas marcadas"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void mover_tarefas_marcadas(GerenciadorTarefas &self) { self.mover_tarefas_marcadas(); } }
