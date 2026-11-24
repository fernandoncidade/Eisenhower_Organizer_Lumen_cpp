#include "source/ui/CalendarDialog/caldial_01_closeEvent.hpp"

#include <QObject>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/ui_12_CalendarDialog.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"

namespace source::GerenciamentoUI::CalendarDialogModules {

void closeEvent(source::GerenciamentoUI::CalendarDialog *self, QCloseEvent *event)
{
    Q_UNUSED(event);

    try {
        if (self != nullptr && self->app != nullptr && self->app->gerenciador_traducao != nullptr) {
            QObject::disconnect(self->app->gerenciador_traducao, nullptr, self, nullptr);
        }
    } catch (...) {
        detail::logger()->error(QStringLiteral("Erro ao fechar CalendarDialog."));
    }
}

}  // namespace source::GerenciamentoUI::CalendarDialogModules
