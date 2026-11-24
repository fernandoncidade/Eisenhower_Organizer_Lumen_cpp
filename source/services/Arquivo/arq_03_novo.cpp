#include "source/services/Arquivo/arq_03_novo.hpp"

#include <QCoreApplication>
#include <QComboBox>
#include <QMessageBox>

#include "source/services/Arquivo/arq_01_clear_all_lists.hpp"
#include "source/services/Arquivo/arq_02_add_placeholders_after_clear.hpp"
#include "source/services/incore_09_Arquivo.hpp"
#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/eisenhower_app.hpp"
#include <QLineEdit>

namespace source::InterfaceCore::ArquivoModules {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

}  // namespace

void novo(Arquivo *self)
{
    if (self == nullptr || self->app() == nullptr) {
        return;
    }

    source::EisenhowerMatrixApp *app = self->app();
    clear_all_lists(self);
    add_placeholders_after_clear(self);
    if (app->task_input != nullptr) {
        app->task_input->clear();
    }
    if (app->domain_selector != nullptr) {
        app->domain_selector->setCurrentIndex(0);
    }

    app->save_tasks();
    if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
        app->calendar_pane->calendar_panel->update_task_list();
    }

    QMessageBox::information(app, get_text(QStringLiteral("Novo")), get_text(QStringLiteral("Nova sessão iniciada.")));
}

}  // namespace source::InterfaceCore::ArquivoModules
