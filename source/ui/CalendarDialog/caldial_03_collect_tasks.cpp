#include "source/ui/CalendarDialog/caldial_03_collect_tasks.hpp"

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/agregador_01_ui.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

QList<QVariantMap> _collect_tasks(const source::GerenciamentoUI::CalendarDialog *self)
{
    try {
        return source::GerenciamentoUI::task_utils::collect_tasks_from_app(self != nullptr ? self->app : nullptr);
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao coletar tarefas do calendário."));
        return {};
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
