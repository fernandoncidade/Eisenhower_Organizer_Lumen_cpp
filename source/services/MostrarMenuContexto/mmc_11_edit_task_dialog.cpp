#include "source/services/MostrarMenuContexto/mmc_11_edit_task_dialog.hpp"

#include <QBrush>
#include <QColor>
#include <QCoreApplication>
#include <QFont>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QTimer>
#include <QUuid>
#include <QVariantMap>

#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/services/MostrarMenuContexto/mmc_05_is_completed_list.hpp"
#include "source/services/MostrarMenuContexto/mmc_07_target_list_for_quadrant.hpp"
#include "source/services/MostrarMenuContexto/mmc_08_find_source_item_for_calendar.hpp"
#include "source/services/MostrarMenuContexto/mmc_09_base_text_from_item.hpp"
#include "source/services/MostrarMenuContexto/mmc_10_build_display_and_tooltip.hpp"
#include "source/services/incore_13_prioridade_display.hpp"
#include "source/utils/TaskProgressHelper.hpp"
#include "source/services/incore_14_EditTaskDialog.hpp"
#include "source/app/app_02_get_text.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"
#include "source/modules/gantt/gantt_plugin_interface.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

int priority_for_list(source::EisenhowerMatrixApp *app, QListWidget *list)
{
    if (app == nullptr || list == nullptr) {
        return 1;
    }
    if (list == app->quadrant1_list || list == app->quadrant1_in_progress_list || list == app->quadrant1_completed_list) {
        return 1;
    }
    if (list == app->quadrant2_list || list == app->quadrant2_in_progress_list || list == app->quadrant2_completed_list) {
        return 2;
    }
    if (list == app->quadrant3_list || list == app->quadrant3_in_progress_list || list == app->quadrant3_completed_list) {
        return 3;
    }
    if (list == app->quadrant4_list || list == app->quadrant4_in_progress_list || list == app->quadrant4_completed_list) {
        return 4;
    }
    return 1;
}

QString placeholder_for_list(source::EisenhowerMatrixApp *app, QListWidget *list)
{
    if (app == nullptr || list == nullptr) {
        return get_text(QStringLiteral("Nenhuma Tarefa Concluída"));
    }
    if (list == app->quadrant1_list) {
        return get_text(QStringLiteral("1º Quadrante"));
    }
    if (list == app->quadrant2_list) {
        return get_text(QStringLiteral("2º Quadrante"));
    }
    if (list == app->quadrant3_list) {
        return get_text(QStringLiteral("3º Quadrante"));
    }
    if (list == app->quadrant4_list) {
        return get_text(QStringLiteral("4º Quadrante"));
    }
    if (app->is_in_progress_list(list)) {
        return get_text(QStringLiteral("Nenhuma Tarefa em Progresso"));
    }
    return get_text(QStringLiteral("Nenhuma Tarefa Concluída"));
}

bool list_has_selectable_items(QListWidget *list)
{
    if (list == nullptr) {
        return false;
    }
    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);
        if (item != nullptr && (item->flags() & Qt::ItemIsSelectable)) {
            return true;
        }
    }
    return false;
}

}  // namespace

void edit_task_dialog(source::EisenhowerMatrixApp *app, QListWidgetItem *item)
{
    if (app == nullptr || item == nullptr) {
        return;
    }

    QVariantMap data = item->data(Qt::UserRole).toMap();
    QListWidget *src_list = item->listWidget();
    QListWidgetItem *src_item = item;

    if (data.contains(QStringLiteral("category"))) {
        const auto resolved = find_source_item_for_calendar(app, data);
        if (resolved.first != nullptr && resolved.second != nullptr) {
            src_list = resolved.first;
            src_item = resolved.second;
            data = src_item->data(Qt::UserRole).toMap();
        }
    }

    const QString initial_text = data.value(QStringLiteral("text")).toString().isEmpty() ? base_text_from_item(src_item)
                                                                                          : data.value(QStringLiteral("text")).toString();
    const QString initial_file = data.value(QStringLiteral("file_path")).toString();
    const QString initial_desc = data.value(QStringLiteral("description")).toString();
    const QString initial_date_iso = data.value(QStringLiteral("date")).toString();
    const QString initial_time_str = data.value(QStringLiteral("time")).toString();
    const int list_prio = priority_for_list(app, src_list);
    QVariant initial_priority;
    if (list_prio >= 1 && list_prio <= 4) {
        initial_priority = list_prio;
    } else {
        initial_priority = data.value(QStringLiteral("priority"));
        bool ok = false;
        const int p = initial_priority.toInt(&ok);
        if (!ok || p < 1 || p > 4) {
            initial_priority = 1;
        }
    }
    const QVariant initial_domain = data.value(QStringLiteral("dominio"));

    double initial_duration = 1.0;
    if (data.contains(QStringLiteral("duracao")) && data.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        initial_duration = data.value(QStringLiteral("duracao")).toDouble();
    } else if (data.contains(QStringLiteral("duration")) && data.value(QStringLiteral("duration")).toDouble() > 0.0) {
        initial_duration = data.value(QStringLiteral("duration")).toDouble();
    }
    const int initial_pomodoros = data.value(QStringLiteral("pomodoros_completados"), data.value(QStringLiteral("pomodoros"), 0)).toInt();
    const int default_ciclos = app != nullptr ? app->default_pomodoro_cycle_count() : 4;
    const bool is_in_prog = app != nullptr && app->is_in_progress_list(src_list);
    const bool is_custom = data.value(QStringLiteral("ciclos_customizados"), false).toBool();
    int initial_ciclos_totais = default_ciclos;
    if (is_in_prog) {
        initial_ciclos_totais = is_custom
                                    ? data.value(QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("total_ciclos"), default_ciclos))).toInt()
                                    : default_ciclos;
    } else {
        initial_ciclos_totais = data.value(QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("total_ciclos"), default_ciclos))).toInt();
    }
    if (initial_ciclos_totais <= 0) {
        initial_ciclos_totais = default_ciclos;
    }

    source::InterfaceCore::incore_14_EditTaskDialog::EditTaskDialog dialog(app,
                                                                           app,
                                                                           initial_text,
                                                                           initial_file,
                                                                           initial_desc,
                                                                           initial_date_iso,
                                                                           initial_time_str,
                                                                           initial_priority,
                                                                           initial_domain,
                                                                           initial_pomodoros,
                                                                           initial_duration,
                                                                           initial_ciclos_totais);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const auto values = dialog.get_values();
    if (values.text.isEmpty()) {
        return;
    }

    QVariantMap new_data = data;
    new_data.insert(QStringLiteral("text"), values.text);
    new_data.insert(QStringLiteral("date"), values.date_iso);
    new_data.insert(QStringLiteral("time"), values.time_str);
    new_data.insert(QStringLiteral("priority"), values.priority);
    new_data.insert(QStringLiteral("prioridade"), values.priority);
    new_data.insert(QStringLiteral("quadrante"), values.priority);
    new_data.insert(QStringLiteral("dominio"), values.domain);
    new_data.insert(QStringLiteral("pomodoros"), values.pomodoros);
    new_data.insert(QStringLiteral("pomodoros_completados"), values.pomodoros);
    new_data.insert(QStringLiteral("duracao"), values.duration);
    new_data.insert(QStringLiteral("duration"), values.duration);
    new_data.insert(QStringLiteral("duracao_dias"), values.duration);
    new_data.insert(QStringLiteral("ciclos_totais"), values.ciclos_totais);
    new_data.insert(QStringLiteral("ciclos_permitidos"), values.ciclos_totais);
    new_data.insert(QStringLiteral("total_ciclos"), values.ciclos_totais);
    new_data.insert(QStringLiteral("ciclos_customizados"), true);
    if (!values.file_path.isEmpty()) {
        new_data.insert(QStringLiteral("file_path"), values.file_path);
    } else {
        new_data.remove(QStringLiteral("file_path"));
    }
    if (!values.description.isEmpty()) {
        new_data.insert(QStringLiteral("description"), values.description);
    } else {
        new_data.remove(QStringLiteral("description"));
    }

    const QString domain_badge = source::InterfaceCore::dominio_insignia(values.domain);
    const QString decorated_text =
        domain_badge.isEmpty() ? values.text : QStringLiteral("%1 %2").arg(domain_badge, values.text);
    const auto display_and_tooltip = build_display_and_tooltip(app, decorated_text, values.date_iso, values.time_str);
    QStringList tooltip_lines;
    if (!display_and_tooltip.second.isEmpty()) {
        tooltip_lines << display_and_tooltip.second;
    }
    if (!values.file_path.isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Arquivo")), values.file_path);
    }
    if (!values.description.isEmpty()) {
        QStringList lines;
        for (const QString &line : values.description.split(QLatin1Char('\n'))) {
            if (!line.trimmed().isEmpty()) {
                lines << line.trimmed();
            }
            if (lines.size() == 3) {
                break;
            }
        }
        if (!lines.isEmpty()) {
            tooltip_lines << QStringLiteral("%1:").arg(get_text(QStringLiteral("Descrição")));
            tooltip_lines << lines.join(QLatin1Char('\n'));
        }
    }
    tooltip_lines << QStringLiteral("%1: %2")
                         .arg(get_text(QStringLiteral("Prioridade")),
                              source::InterfaceCore::prioridade_para_texto(values.priority, app));
    tooltip_lines << QStringLiteral("%1: %2")
                         .arg(get_text(QStringLiteral("Nível de Domínio")),
                              source::InterfaceCore::dominio_para_texto(values.domain, false));
    tooltip_lines << source::app::format_duration_tooltip_line(values.duration);
    const int task_ciclos_totais = values.ciclos_totais > 0 ? values.ciclos_totais : default_ciclos;
    const bool is_done = data.value(QStringLiteral("concluida")).toBool() || data.value(QStringLiteral("status")).toString() == QStringLiteral("Done");
    if (values.pomodoros > 0 || values.ciclos_totais > 0) {
        const double pom_pct = source::utils::calculate_task_completion_percentage(values.pomodoros, task_ciclos_totais, is_done);
        const QString pom_line_fmt = get_text(QStringLiteral("Ciclos Pomodoro: %1/%2 (%3%)"));
        tooltip_lines << pom_line_fmt
                             .arg(QString::number(values.pomodoros),
                                  QString::number(task_ciclos_totais),
                                  QString::number(pom_pct, 'f', 0));
    }

    QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
    if (task_id.isEmpty()) {
        task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        new_data.insert(QStringLiteral("id"), task_id);
    }

    const bool is_comp = is_completed_list(app, src_list);

    QListWidget *target_list = src_list;
    if (values.priority >= 1 && values.priority <= 4) {
        if (is_in_prog) {
            const QList<QListWidget *> in_progress = {app->quadrant1_in_progress_list,
                                                      app->quadrant2_in_progress_list,
                                                      app->quadrant3_in_progress_list,
                                                      app->quadrant4_in_progress_list};
            target_list = in_progress.at(values.priority - 1);
        } else if (is_comp) {
            const QList<QListWidget *> done = {app->quadrant1_completed_list,
                                               app->quadrant2_completed_list,
                                               app->quadrant3_completed_list,
                                               app->quadrant4_completed_list};
            target_list = done.at(values.priority - 1);
        } else {
            const QList<QListWidget *> pending = {app->quadrant1_list,
                                                  app->quadrant2_list,
                                                  app->quadrant3_list,
                                                  app->quadrant4_list};
            target_list = pending.at(values.priority - 1);
        }
        if (target_list == nullptr) {
            target_list = src_list;
        }
    }

    if (app->is_in_progress_list(target_list)) {
        new_data.insert(QStringLiteral("status"), QStringLiteral("Doing"));
        new_data.insert(QStringLiteral("in_progress"), true);
        new_data.insert(QStringLiteral("em_progresso"), true);
        new_data.insert(QStringLiteral("em_andamento"), true);
        new_data.remove(QStringLiteral("completed_at"));
    } else if (is_completed_list(app, target_list)) {
        new_data.insert(QStringLiteral("status"), QStringLiteral("Done"));
        new_data.insert(QStringLiteral("concluida"), true);
        new_data.insert(QStringLiteral("completed"), true);
        new_data.remove(QStringLiteral("in_progress"));
        new_data.remove(QStringLiteral("em_progresso"));
        new_data.remove(QStringLiteral("em_andamento"));
    } else {
        new_data.insert(QStringLiteral("status"), QStringLiteral("Todo"));
        new_data.insert(QStringLiteral("concluida"), false);
        new_data.insert(QStringLiteral("completed"), false);
        new_data.remove(QStringLiteral("in_progress"));
        new_data.remove(QStringLiteral("em_progresso"));
        new_data.remove(QStringLiteral("em_andamento"));
        new_data.remove(QStringLiteral("completed_at"));
    }

    const int row = src_list != nullptr && src_item != nullptr ? src_list->row(src_item) : -1;
    const Qt::CheckState check_state = src_item != nullptr ? src_item->checkState() : Qt::Unchecked;
    if (row >= 0) {
        delete src_list->takeItem(row);
    }
    app->cleanup_time_groups(src_list);
    if (!list_has_selectable_items(src_list)) {
        src_list->clear();
        app->add_placeholder(src_list, placeholder_for_list(app, src_list));
    }

    auto *new_item = new QListWidgetItem(display_and_tooltip.first);
    new_item->setFlags(new_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    new_item->setCheckState(check_state);
    new_item->setData(Qt::UserRole, new_data);
    source::utils::apply_task_title_font(new_item, values.text, !values.file_path.isEmpty());
    if (!tooltip_lines.isEmpty()) {
        new_item->setToolTip(tooltip_lines.join(QLatin1Char('\n')));
    }
    if (!values.file_path.isEmpty()) {
        const QColor text_color(new_data.value(QStringLiteral("cor_texto")).toString().trimmed());
        if (text_color.isValid()) {
            new_item->setForeground(QBrush(text_color));
        } else {
            new_item->setForeground(QBrush());
        }
    } else {
        const QColor text_color(new_data.value(QStringLiteral("cor_texto")).toString().trimmed());
        if (text_color.isValid()) {
            new_item->setForeground(QBrush(text_color));
        }
    }

    if (target_list->count() == 1 && !(target_list->item(0)->flags() & Qt::ItemIsSelectable)) {
        target_list->clear();
    }
    app->insert_task_into_quadrant_list(target_list, new_item);
    app->cleanup_time_groups(target_list);

    QMessageBox message_box(app);
    message_box.setIcon(QMessageBox::Information);
    message_box.setWindowTitle(get_text(QStringLiteral("✏️ Editar Tarefa")));
    message_box.setText(get_text(QStringLiteral("Alterações aplicadas com sucesso.")));
    message_box.setStandardButtons(QMessageBox::Ok);
    message_box.setDefaultButton(QMessageBox::Ok);
    if (app->gerenciador_traducao != nullptr) {
        app->gerenciador_traducao->traduzir_botoes_padrao(&message_box);
    }
    message_box.exec();

    app->save_tasks();
    if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
        app->calendar_pane->calendar_panel->update_task_list();
    }
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
