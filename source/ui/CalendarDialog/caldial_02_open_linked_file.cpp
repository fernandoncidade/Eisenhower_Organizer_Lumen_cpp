#include "source/ui/CalendarDialog/caldial_02_open_linked_file.hpp"

#include <QFileInfo>
#include <QListWidgetItem>
#include <QMessageBox>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/agregador_01_ui.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"
#include "source/eisenhower_app.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

void _open_linked_file(source::GerenciamentoUI::CalendarDialog *self, QListWidgetItem *item)
{
    try {
        if (self == nullptr || item == nullptr) {
            return;
        }

        const QVariantMap data = item->data(Qt::UserRole).toMap();
        const QString path = data.value(QStringLiteral("file_path")).toString();
        if (path.isEmpty()) {
            return;
        }

        const auto source_entry = source::GerenciamentoUI::task_utils::find_source_item_for_calendar(self->app, data);
        if (source_entry.second != nullptr) {
            const bool changed = self->app->handle_linked_file_open_request(source_entry.second, self);
            if (changed) {
                self->update_task_list();
            }
            return;
        }

        QMessageBox::warning(self, detail::get_text(QStringLiteral("Erro")), detail::get_text(QStringLiteral("Tarefa não encontrada.")));
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao abrir arquivo vinculado (Calendar)."));
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
