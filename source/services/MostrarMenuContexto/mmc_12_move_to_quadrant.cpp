#include "source/services/MostrarMenuContexto/mmc_12_move_to_quadrant.hpp"

#include <QCoreApplication>
#include <QInputDialog>
#include <QListWidget>
#include <QMessageBox>

#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/services/MostrarMenuContexto/mmc_05_is_completed_list.hpp"
#include "source/services/MostrarMenuContexto/mmc_06_quadrant_options.hpp"
#include "source/services/MostrarMenuContexto/mmc_07_target_list_for_quadrant.hpp"
#include "source/services/MostrarMenuContexto/mmc_09_base_text_from_item.hpp"
#include "source/eisenhower_app.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

}  // namespace

void move_to_quadrant(source::EisenhowerMatrixApp *app, QListWidgetItem *item, QListWidget *source_list)
{
    if (app == nullptr || item == nullptr || source_list == nullptr) {
        return;
    }

    QString task_name = base_text_from_item(item).trimmed();
    if (task_name.isEmpty()) {
        task_name = get_text(QStringLiteral("Tarefa"));
    }

    const QStringList options = quadrant_options(app, true);
    bool ok = false;
    const QString chosen = QInputDialog::getItem(app,
                                                 get_text(QStringLiteral("Mover tarefa")),
                                                 get_text(QStringLiteral("Selecione o quadrante:")),
                                                 options,
                                                 0,
                                                 false,
                                                 &ok);
    if (!ok || chosen.isEmpty()) {
        return;
    }

    const int idx = options.indexOf(chosen);
    const bool keep_completed = is_completed_list(app, source_list);
    QListWidget *target = target_list_for_quadrant(app, idx, keep_completed, true);
    if (target == nullptr || target == source_list) {
        QMessageBox::warning(
            app,
            get_text(QStringLiteral("Erro")),
            get_text(QStringLiteral("Não foi possível mover a tarefa \"{item}\"!")).replace(QStringLiteral("{item}"), task_name));
        return;
    }

    const Qt::CheckState new_state = is_completed_list(app, target) ? Qt::Checked : Qt::Unchecked;
    source_list->blockSignals(true);
    target->blockSignals(true);
    app->move_item_between_lists(item, source_list, target, new_state);
    source_list->blockSignals(false);
    target->blockSignals(false);
    if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
        app->calendar_pane->calendar_panel->update_task_list();
    }

    QMessageBox::information(
        app,
        get_text(QStringLiteral("Sucesso")),
        get_text(QStringLiteral("Tarefa \"{item}\" movida com sucesso!")).replace(QStringLiteral("{item}"), task_name));

    app->save_tasks();
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
