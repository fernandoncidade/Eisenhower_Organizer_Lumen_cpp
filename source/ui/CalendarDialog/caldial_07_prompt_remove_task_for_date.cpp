#include "source/ui/CalendarDialog/caldial_07_prompt_remove_task_for_date.hpp"

#include <QInputDialog>
#include <QCheckBox>
#include <QMessageBox>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/agregador_01_ui.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"
#include "source/eisenhower_app.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

void _prompt_remove_task_for_date(source::GerenciamentoUI::CalendarDialog *self, const QDate &qdate)
{
    try {
        if (self == nullptr) {
            return;
        }

        const QList<QVariantMap> tasks = detail::tasks_for_date(self, qdate);
        if (tasks.isEmpty()) {
            QMessageBox::information(self,
                                     detail::get_text(QStringLiteral("🗑️ Remover Tarefa")),
                                     detail::get_text(QStringLiteral("Nenhuma tarefa para este dia.")));
            return;
        }

        const bool show_priority =
            self->show_priority_checkbox == nullptr || self->show_priority_checkbox->isChecked();
        const bool show_domain =
            self->show_domain_checkbox == nullptr || self->show_domain_checkbox->isChecked();

        QStringList items;
        for (int i = 0; i < tasks.size(); ++i) {
            items << QStringLiteral("%1. %2")
                         .arg(i + 1)
                         .arg(source::GerenciamentoUI::task_utils::task_label_for_day(tasks.at(i),
                                                                                     show_priority,
                                                                                     show_domain,
                                                                                     self->app));
        }
        items << detail::get_text(QStringLiteral("Remover Todas"));

        bool ok = false;
        const QString choice = QInputDialog::getItem(self,
                                                     detail::get_text(QStringLiteral("🗑️ Remover Tarefa")),
                                                     detail::get_text(QStringLiteral("Selecione a tarefa")),
                                                     items,
                                                     0,
                                                     false,
                                                     &ok);
        if (!ok || choice.isEmpty()) {
            return;
        }

        if (choice == items.constLast()) {
            const auto confirm = QMessageBox::question(
                self,
                detail::get_text(QStringLiteral("Remover Todas")),
                detail::get_text(QStringLiteral("Deseja remover todas as tarefas deste dia?")),
                QMessageBox::Yes | QMessageBox::No);
            if (confirm != QMessageBox::Yes) {
                return;
            }

            int removed_count = 0;
            int failed_count = 0;
            for (const QVariantMap &task : tasks) {
                const auto found = source::GerenciamentoUI::task_utils::find_source_item_for_calendar(self->app, task);
                if (found.first == nullptr || found.second == nullptr) {
                    ++failed_count;
                    continue;
                }

                if (self->app->remove_task(found.second, found.first, false)) {
                    ++removed_count;
                } else {
                    ++failed_count;
                }
            }

            if (removed_count > 0 && failed_count == 0) {
                QMessageBox::information(
                    self,
                    detail::get_text(QStringLiteral("🗑️ Remover Tarefa")),
                    detail::get_text(QStringLiteral("Tarefa(s) removida(s) com sucesso.")));
            } else if (removed_count > 0) {
                QMessageBox::warning(
                    self,
                    detail::get_text(QStringLiteral("🗑️ Remover Tarefa")),
                    detail::get_text(QStringLiteral("Algumas tarefas nao puderam ser removidas.")));
            } else {
                QMessageBox::warning(
                    self,
                    detail::get_text(QStringLiteral("🗑️ Remover Tarefa")),
                    detail::get_text(QStringLiteral("Nao foi possivel remover as tarefas deste dia.")));
            }
        } else {
            const int idx = items.indexOf(choice);
            const auto found = source::GerenciamentoUI::task_utils::find_source_item_for_calendar(self->app, tasks.at(idx));
            if (found.first == nullptr || found.second == nullptr) {
                QMessageBox::warning(self,
                                     detail::get_text(QStringLiteral("🗑️ Remover Tarefa")),
                                     detail::get_text(QStringLiteral("Tarefa não encontrada.")));
                return;
            }

            if (self->app->remove_task(found.second, found.first, true)) {
                self->update_task_list();
                return;
            }
        }

        self->update_task_list();
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao remover tarefa do calendário."));
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
