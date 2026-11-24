#include "source/ui/CalendarDialog/caldial_05_on_calendar_context_menu.hpp"

#include <QAction>
#include <QCheckBox>
#include <QDate>
#include <QMenu>
#include <QMessageBox>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/agregador_01_ui.hpp"
#include "source/ui/ui_10_BadgeCalendarWidget.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"
#include "source/eisenhower_app.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

void _on_calendar_context_menu(source::GerenciamentoUI::CalendarDialog *self, const QPoint &pos)
{
    try {
        if (self == nullptr || self->calendar == nullptr) {
            return;
        }

        const QDate qdate = self->calendar->_date_at_pos(pos);
        if (!qdate.isValid()) {
            return;
        }

        const QList<QVariantMap> tasks = detail::tasks_for_date(self, qdate);
        const bool show_priority =
            self->show_priority_checkbox == nullptr || self->show_priority_checkbox->isChecked();
        const bool show_domain =
            self->show_domain_checkbox == nullptr || self->show_domain_checkbox->isChecked();

        QMenu menu(self->calendar);
        QMenu *edit_menu = menu.addMenu(detail::get_text(QStringLiteral("✏️ Editar Tarefa")));
        QMenu *remove_menu = menu.addMenu(detail::get_text(QStringLiteral("🗑️ Remover Tarefa")));

        if (tasks.isEmpty()) {
            edit_menu->setEnabled(false);
            remove_menu->setEnabled(false);
        } else {
            for (const QVariantMap &task : tasks) {
                QAction *action = edit_menu->addAction(
                    source::GerenciamentoUI::task_utils::task_label_for_day(task, show_priority, show_domain, self->app));
                QObject::connect(action, &QAction::triggered, self, [self, task]() {
                    const auto found = source::GerenciamentoUI::task_utils::find_source_item_for_calendar(self->app, task);
                    if (found.first == nullptr || found.second == nullptr) {
                        QMessageBox::warning(self,
                                             detail::get_text(QStringLiteral("✏️ Editar Tarefa")),
                                             detail::get_text(QStringLiteral("Tarefa não encontrada.")));
                        return;
                    }

                    self->app->edit_task_datetime(found.second, found.first);
                    self->update_task_list();
                });
            }

            for (const QVariantMap &task : tasks) {
                QAction *action = remove_menu->addAction(
                    source::GerenciamentoUI::task_utils::task_label_for_day(task, show_priority, show_domain, self->app));
                QObject::connect(action, &QAction::triggered, self, [self, task]() {
                    const auto found = source::GerenciamentoUI::task_utils::find_source_item_for_calendar(self->app, task);
                    if (found.first == nullptr || found.second == nullptr) {
                        QMessageBox::warning(self,
                                             detail::get_text(QStringLiteral("🗑️ Remover Tarefa")),
                                             detail::get_text(QStringLiteral("Tarefa não encontrada.")));
                        return;
                    }

                    if (self->app->remove_task(found.second, found.first, true)) {
                        self->update_task_list();
                    }
                });
            }

            remove_menu->addSeparator();
            QAction *remove_all = remove_menu->addAction(detail::get_text(QStringLiteral("Remover Todas")));
            QObject::connect(remove_all, &QAction::triggered, self, [self, tasks]() {
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

                self->update_task_list();

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
            });
        }

        menu.exec(self->calendar->mapToGlobal(pos));
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao abrir menu de contexto do calendário."));
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
