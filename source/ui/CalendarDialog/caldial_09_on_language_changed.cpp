#include "source/ui/CalendarDialog/caldial_09_on_language_changed.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QGroupBox>
#include <QLabel>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"
#include "source/eisenhower_app.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

void _on_language_changed(source::GerenciamentoUI::CalendarDialog *self)
{
    try {
        if (self == nullptr) {
            return;
        }

        self->setWindowTitle(detail::get_text(QStringLiteral("Calendário de Tarefas")));
        if (self->calendar_group != nullptr) {
            self->calendar_group->setTitle(detail::get_text(QStringLiteral("Calendário")));
        }
        if (self->filter_label != nullptr) {
            self->filter_label->setText(detail::get_text(QStringLiteral("Exibir por")));
        }
        if (self->filter_combo != nullptr) {
            self->filter_combo->setItemText(0, detail::get_text(QStringLiteral("Dia")));
            self->filter_combo->setItemText(1, detail::get_text(QStringLiteral("Semana")));
            self->filter_combo->setItemText(2, detail::get_text(QStringLiteral("Mês")));
        }
        if (self->tasks_label != nullptr) {
            self->tasks_label->setText(detail::get_text(QStringLiteral("Lista Global de Tarefas:")));
        }
        if (self->tasks_group != nullptr) {
            self->tasks_group->setTitle(detail::get_text(QStringLiteral("Lista Global de Tarefas:")));
        }
        if (self->show_label != nullptr) {
            self->show_label->setText(detail::get_text(QStringLiteral("Mostrar:")));
        }
        if (self->show_priority_checkbox != nullptr) {
            self->show_priority_checkbox->setText(detail::get_text(QStringLiteral("Prioridade")));
        }
        if (self->show_domain_checkbox != nullptr) {
            self->show_domain_checkbox->setText(detail::get_text(QStringLiteral("Domínio")));
        }

        if (self->app != nullptr && self->app->date_input != nullptr) {
            self->_date_format = self->app->date_input->displayFormat();
        }

        self->_apply_locale_to_calendar();
        self->update_task_list();
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao atualizar idioma do calendário."));
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
