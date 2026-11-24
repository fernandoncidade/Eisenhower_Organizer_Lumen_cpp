#include "source/ui/CalendarDialog/caldial_06_prompt_edit_task_for_date.hpp"

#include <QInputDialog>
#include <QCheckBox>
#include <QMessageBox>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/agregador_01_ui.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"
#include "source/eisenhower_app.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

void _prompt_edit_task_for_date(source::GerenciamentoUI::CalendarDialog *self, const QDate &qdate)
{
    try {
        if (self == nullptr) {
            return;
        }

        const QList<QVariantMap> tasks = detail::tasks_for_date(self, qdate);
        if (tasks.isEmpty()) {
            QMessageBox::information(self,
                                     detail::get_text(QStringLiteral("✏️ Editar Tarefa")),
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

        bool ok = false;
        const QString choice = QInputDialog::getItem(self,
                                                     detail::get_text(QStringLiteral("✏️ Editar Tarefa")),
                                                     detail::get_text(QStringLiteral("Selecione a tarefa")),
                                                     items,
                                                     0,
                                                     false,
                                                     &ok);
        if (!ok || choice.isEmpty()) {
            return;
        }

        const int idx = items.indexOf(choice);
        const auto found = source::GerenciamentoUI::task_utils::find_source_item_for_calendar(self->app, tasks.at(idx));
        if (found.first == nullptr || found.second == nullptr) {
            QMessageBox::warning(self,
                                 detail::get_text(QStringLiteral("✏️ Editar Tarefa")),
                                 detail::get_text(QStringLiteral("Tarefa não encontrada.")));
            return;
        }

        self->app->edit_task_datetime(found.second, found.first);
        self->update_task_list();
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao editar tarefa do calendário."));
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
