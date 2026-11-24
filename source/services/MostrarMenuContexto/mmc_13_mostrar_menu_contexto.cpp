#include "source/services/MostrarMenuContexto/mmc_13_mostrar_menu_contexto.hpp"

#include <QAction>
#include <QCoreApplication>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QStringList>
#include <QVariantMap>
#include <utility>

#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/services/MostrarMenuContexto/mmc_05_is_completed_list.hpp"
#include "source/services/MostrarMenuContexto/mmc_06_quadrant_options.hpp"
#include "source/services/MostrarMenuContexto/mmc_07_target_list_for_quadrant.hpp"
#include "source/services/MostrarMenuContexto/mmc_08_find_source_item_for_calendar.hpp"
#include "source/services/MostrarMenuContexto/mmc_09_base_text_from_item.hpp"
#include "source/services/MostrarMenuContexto/mmc_11_edit_task_dialog.hpp"
#include "source/services/incore_15_ProductivityFeatures.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/eisenhower_app.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"
#include <QUuid>
#include <QInputDialog>
#include "source/modules/gantt/gantt_plugin_interface.hpp"

namespace source::InterfaceCore::MostrarMenuContextoModules {
namespace {

QString get_text(const QString &text)
{
    if (text.isEmpty()) {
        return text;
    }
    const QString translated = QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
    if (translated.isEmpty() || translated == text) {
        const QString fallback = QCoreApplication::translate("GanttPlugin", text.toUtf8().constData());
        if (!fallback.isEmpty() && fallback != text) {
            return fallback;
        }
    }
    return translated;
}

QString task_name_for_message(QListWidgetItem *item)
{
    const QString name = base_text_from_item(item).trimmed();
    return name.isEmpty() ? get_text(QStringLiteral("Tarefa")) : name;
}

QString task_name_for_payload(const QVariantMap &payload)
{
    const QString name = payload.value(QStringLiteral("text")).toString().trimmed();
    return name.isEmpty() ? get_text(QStringLiteral("Tarefa")) : name;
}

bool same_payload_task(const QVariantMap &left, const QVariantMap &right)
{
    const auto norm = [](const QVariant &value) { return value.toString().trimmed(); };
    const QString left_id = norm(left.value(QStringLiteral("id")));
    const QString right_id = norm(right.value(QStringLiteral("id")));
    if (!left_id.isEmpty() && !right_id.isEmpty()) {
        return left_id == right_id;
    }

    return norm(left.value(QStringLiteral("text"))) == norm(right.value(QStringLiteral("text")))
           && norm(left.value(QStringLiteral("date"))) == norm(right.value(QStringLiteral("date")))
           && norm(left.value(QStringLiteral("time"))) == norm(right.value(QStringLiteral("time")))
           && norm(left.value(QStringLiteral("file_path"))) == norm(right.value(QStringLiteral("file_path")))
           && norm(left.value(QStringLiteral("description"))) == norm(right.value(QStringLiteral("description")))
           && norm(left.value(QStringLiteral("priority"))) == norm(right.value(QStringLiteral("priority")))
           && norm(left.value(QStringLiteral("dominio"))) == norm(right.value(QStringLiteral("dominio")));
}

QList<QListWidget *> task_lists(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return {};
    }

    return {
        app->quadrant1_list,
        app->quadrant2_list,
        app->quadrant3_list,
        app->quadrant4_list,
        app->quadrant1_in_progress_list,
        app->quadrant2_in_progress_list,
        app->quadrant3_in_progress_list,
        app->quadrant4_in_progress_list,
        app->quadrant1_completed_list,
        app->quadrant2_completed_list,
        app->quadrant3_completed_list,
        app->quadrant4_completed_list,
    };
}

QPair<QListWidget *, QListWidgetItem *> resolve_payload(source::EisenhowerMatrixApp *app, const QVariantMap &payload)
{
    if (payload.contains(QStringLiteral("category"))) {
        const auto resolved = find_source_item_for_calendar(app, payload);
        if (resolved.first != nullptr && resolved.second != nullptr) {
            return resolved;
        }
    }

    for (QListWidget *list : task_lists(app)) {
        if (list == nullptr) {
            continue;
        }

        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *candidate = list->item(i);
            if (candidate == nullptr || !(candidate->flags() & Qt::ItemIsSelectable)) {
                continue;
            }
            if (same_payload_task(candidate->data(Qt::UserRole).toMap(), payload)) {
                return {list, candidate};
            }
        }
    }

    return {nullptr, nullptr};
}

QPair<QListWidget *, QListWidgetItem *> resolve_item(source::EisenhowerMatrixApp *app,
                                                     QListWidget *fallback_list,
                                                     QListWidgetItem *item)
{
    if (item == nullptr) {
        return {fallback_list, nullptr};
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    if (data.contains(QStringLiteral("category"))) {
        const auto resolved = find_source_item_for_calendar(app, data);
        if (resolved.first != nullptr && resolved.second != nullptr) {
            return resolved;
        }
    }

    return {fallback_list, item};
}

}  // namespace

void mostrar_menu_contexto(source::EisenhowerMatrixApp *app, const QPoint &point, QListWidget *list_widget)
{
    if (app == nullptr || list_widget == nullptr) {
        return;
    }

    QListWidgetItem *item = list_widget->itemAt(point);
    if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
        return;
    }

    QList<QListWidgetItem *> selected_items = list_widget->selectedItems();
    if (!selected_items.contains(item)) {
        selected_items = {item};
    }

    QMenu menu(list_widget);
    auto *edit_action = menu.addAction(get_text(QStringLiteral("✏️ Editar Tarefa")));
    QObject::connect(edit_action, &QAction::triggered, app, [app, item]() { edit_task_dialog(app, item); });

    auto *font_action = menu.addAction(get_text(QStringLiteral("🔤 Configurações de Fonte")));
    QObject::connect(font_action, &QAction::triggered, app, [app, list_widget, selected_items]() {
        QList<QListWidgetItem *> target_items;
        for (QListWidgetItem *selected : selected_items) {
            if (selected != nullptr && !target_items.contains(selected)) {
                target_items.append(selected);
            }
            const auto resolved = resolve_item(app, list_widget, selected);
            if (resolved.second != nullptr && !target_items.contains(resolved.second)) {
                target_items.append(resolved.second);
            }
        }
        if (source::utils::open_task_font_settings_dialog(app, target_items)) {
            app->save_tasks_now();
            app->refresh_task_title_styles();
        }
    });

    auto *color_action = menu.addAction(get_text(QStringLiteral("🎨 Colorir tarefas")));
    QObject::connect(color_action, &QAction::triggered, app, [app, list_widget, selected_items]() {
        QList<QListWidgetItem *> target_items;
        for (QListWidgetItem *selected : selected_items) {
            if (selected != nullptr && !target_items.contains(selected)) {
                target_items.append(selected);
            }
            const auto resolved = resolve_item(app, list_widget, selected);
            if (resolved.second != nullptr && !target_items.contains(resolved.second)) {
                target_items.append(resolved.second);
            }
        }
        if (target_items.isEmpty()) {
            return;
        }
        QColor initial_color;
        for (QListWidgetItem *target_item : target_items) {
            if (target_item != nullptr) {
                const QVariantMap data = target_item->data(Qt::UserRole).toMap();
                const QString col_str = data.value(QStringLiteral("cor_texto")).toString().trimmed();
                if (QColor(col_str).isValid()) {
                    initial_color = QColor(col_str);
                    break;
                }
            }
        }
        const QColor selected_color = source::utils::choose_task_color(app, initial_color, source::utils::PluginFontContext::Global);
        if (!selected_color.isValid()) {
            return;
        }
        const QString color_name = selected_color.name(QColor::HexRgb);
        for (QListWidgetItem *target_item : target_items) {
            if (target_item == nullptr) {
                continue;
            }
            QVariantMap data = target_item->data(Qt::UserRole).toMap();
            const QString current = data.value(QStringLiteral("cor_texto")).toString().trimmed();
            if (data.value(QStringLiteral("cor_texto_original")).toString().trimmed().isEmpty()
                && !current.isEmpty() && QColor(current).isValid()
                && current.compare(QStringLiteral("#000000"), Qt::CaseInsensitive) != 0
                && current.compare(QStringLiteral("#ffffff"), Qt::CaseInsensitive) != 0) {
                data.insert(QStringLiteral("cor_texto_original"), QColor(current).name(QColor::HexRgb));
            }
            data.insert(QStringLiteral("cor_texto"), color_name);
            data.insert(QStringLiteral("task_color"), color_name);
            data.insert(QStringLiteral("cor_texto_customizada"), color_name);
            target_item->setData(Qt::UserRole, data);
            target_item->setForeground(QBrush(selected_color));
        }
        app->save_tasks_now();
        if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
            app->calendar_pane->calendar_panel->update_task_list();
        }
    });

    auto *recurrence_action = menu.addAction(get_text(QStringLiteral("🔁 Recorrência")));
    QObject::connect(recurrence_action, &QAction::triggered, app, [app, list_widget, item]() {
        const auto resolved = resolve_item(app, list_widget, item);
        source::InterfaceCore::ProductivityFeatures::configure_recurrence_for_item(app, resolved.second);
    });

    QMenu *move_menu = menu.addMenu(get_text(QStringLiteral("🔀 Mover para outro quadrante")));
    const QStringList options = quadrant_options(app, true);
    for (int idx = 0; idx < options.size(); ++idx) {
        if (idx == 4 || idx == 8) {
            move_menu->addSeparator();
        }

        QAction *action = move_menu->addAction(options.at(idx));
        QObject::connect(action, &QAction::triggered, app, [app, list_widget, selected_items, idx]() {
            QStringList moved_names;
            QStringList failed_names;

            for (QListWidgetItem *selected : selected_items) {
                const auto resolved = resolve_item(app, list_widget, selected);
                QListWidget *source_list = resolved.first;
                QListWidgetItem *source_item = resolved.second;
                if (source_list == nullptr || source_item == nullptr) {
                    failed_names << task_name_for_message(selected);
                    continue;
                }

                const QString task_name = task_name_for_message(source_item);
                QListWidget *target = target_list_for_quadrant(app, idx, is_completed_list(app, source_list), true);
                if (target == nullptr || target == source_list) {
                    failed_names << task_name;
                    continue;
                }

                const Qt::CheckState new_state = is_completed_list(app, target) ? Qt::Checked : Qt::Unchecked;
                source_list->blockSignals(true);
                target->blockSignals(true);
                app->move_item_between_lists(source_item, source_list, target, new_state);
                source_list->blockSignals(false);
                target->blockSignals(false);
                moved_names << task_name;
            }

            if (!moved_names.isEmpty()) {
                app->save_tasks();
                if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
                    app->calendar_pane->calendar_panel->update_task_list();
                }
            }

            if (!failed_names.isEmpty()) {
                QMessageBox::warning(
                    app,
                    get_text(QStringLiteral("Erro")),
                    get_text(QStringLiteral("Não foi possível mover a tarefa \"{items}\"!"))
                        .replace(QStringLiteral("{items}"), failed_names.join(QStringLiteral("\", \""))));
            } else if (!moved_names.isEmpty()) {
                QMessageBox::information(
                    app,
                    get_text(QStringLiteral("Sucesso")),
                    get_text(QStringLiteral("Tarefa \"{items}\" movida com sucesso!"))
                        .replace(QStringLiteral("{items}"), moved_names.join(QStringLiteral("\", \""))));
            }
        });
    }

    if (app->has_pomodoro_plugin()) {
        auto *share_action = menu.addAction(get_text(QStringLiteral("🍅 Compartilhar com Pomodoro")));
        QObject::connect(share_action, &QAction::triggered, app, [app, list_widget, selected_items]() {
            int shared = 0;
            QStringList failed_names;

            for (QListWidgetItem *selected : selected_items) {
                const auto resolved = resolve_item(app, list_widget, selected);
                QListWidget *source_list = resolved.first;
                QListWidgetItem *source_item = resolved.second;
                if (source_list == nullptr || source_item == nullptr || !app->send_task_to_pomodoro(source_item, source_list)) {
                    failed_names << task_name_for_message(selected);
                    continue;
                }

                ++shared;
            }

            if (shared > 0) {
                QMessageBox::information(
                    app,
                    get_text(QStringLiteral("Sucesso")),
                    shared == 1 ? get_text(QStringLiteral("Tarefa compartilhada com Pomodoro."))
                                : get_text(QStringLiteral("Tarefas compartilhadas com Pomodoro.")));
            }

            if (!failed_names.isEmpty()) {
                QMessageBox::warning(
                    app,
                    get_text(QStringLiteral("Erro")),
                    get_text(QStringLiteral("Não foi possível compartilhar a tarefa \"{items}\" com Pomodoro."))
                        .replace(QStringLiteral("{items}"), failed_names.join(QStringLiteral("\", \""))));
            }
        });
    }

    if (app->has_feynman_plugin()) {
        auto *share_action = menu.addAction(get_text(QStringLiteral("🪢 Compartilhar com Feynman")));
        QObject::connect(share_action, &QAction::triggered, app, [app, list_widget, selected_items]() {
            int shared = 0;
            QStringList failed_names;

            for (QListWidgetItem *selected : selected_items) {
                const auto resolved = resolve_item(app, list_widget, selected);
                QListWidget *source_list = resolved.first;
                QListWidgetItem *source_item = resolved.second;
                if (source_list == nullptr || source_item == nullptr || !app->send_task_to_feynman(source_item, source_list)) {
                    failed_names << task_name_for_message(selected);
                    continue;
                }

                ++shared;
            }

            if (shared > 0) {
                QMessageBox::information(
                    app,
                    get_text(QStringLiteral("Sucesso")),
                    shared == 1 ? get_text(QStringLiteral("Tarefa compartilhada com Feynman."))
                                : get_text(QStringLiteral("Tarefas compartilhadas com Feynman.")));
            }

            if (!failed_names.isEmpty()) {
                QMessageBox::warning(
                    app,
                    get_text(QStringLiteral("Erro")),
                    get_text(QStringLiteral("Não foi possível compartilhar a tarefa \"{items}\" com Feynman."))
                        .replace(QStringLiteral("{items}"), failed_names.join(QStringLiteral("\", \""))));
            }
        });
    }

    if (app->has_gantt_plugin() && !selected_items.isEmpty()) {
        QListWidgetItem *first_selected = selected_items.first();
        const auto resolved = resolve_item(app, list_widget, first_selected);
        QListWidgetItem *target_item = (resolved.second != nullptr) ? resolved.second : first_selected;
        if (target_item != nullptr) {
            QVariantMap data = target_item->data(Qt::UserRole).toMap();
            QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
            if (task_id.isEmpty()) {
                task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                data.insert(QStringLiteral("id"), task_id);
                target_item->setData(Qt::UserRole, data);
                app->save_tasks_now();
            }

            QString task_title = data.value(QStringLiteral("text")).toString().trimmed();
            if (task_title.isEmpty()) task_title = data.value(QStringLiteral("titulo")).toString().trimmed();
            if (task_title.isEmpty()) task_title = base_text_from_item(target_item);

            auto *gantt = app->gantt_plugin();
            if (gantt != nullptr) {
                QString wbs_and_name, sched_name, act_status;
                double act_progress = 0.0;
                const bool is_linked = gantt->get_task_gantt_summary(
                    task_id, &wbs_and_name, &sched_name, &act_progress, &act_status);

                if (is_linked) {
                    // 1. Abrir no Gantt
                    const QString open_text = !wbs_and_name.trimmed().isEmpty()
                        ? get_text(QStringLiteral("🏗️ Abrir no Cronograma de Gantt (%1 - %2%)"))
                              .arg(wbs_and_name)
                              .arg(static_cast<int>(act_progress))
                        : get_text(QStringLiteral("🏗️ Abrir no Cronograma de Gantt (%1%)"))
                              .arg(static_cast<int>(act_progress));
                    auto *open_gantt_action = menu.addAction(open_text);
                    QObject::connect(open_gantt_action, &QAction::triggered, app, [gantt, task_id, app]() {
                        gantt->open_gantt_and_highlight_activity(task_id, app);
                    });

                    // 2. Registrar avanço de campo
                    auto *progress_action = menu.addAction(get_text(QStringLiteral("⚡ Registrar Avanço de Campo no Cronograma...")));
                    QObject::connect(progress_action, &QAction::triggered, app, [gantt, task_id, wbs_and_name, act_progress, app]() {
                        bool ok = false;
                        const QString label_text = !wbs_and_name.trimmed().isEmpty()
                            ? get_text(QStringLiteral("Informe o percentual executado para '%1':")).arg(wbs_and_name)
                            : get_text(QStringLiteral("Informe o percentual executado:"));
                        const double val = QInputDialog::getDouble(
                            app,
                            get_text(QStringLiteral("Avanço Físico de Obra")),
                            label_text,
                            act_progress, 0.0, 100.0, 1, &ok);
                        if (ok) {
                            const QString note = QInputDialog::getText(
                                app,
                                get_text(QStringLiteral("Diário de Obra")),
                                get_text(QStringLiteral("Observações de campo (opcional):")),
                                QLineEdit::Normal, QString(), &ok);
                            QString err;
                            if (gantt->record_field_progress_for_task(task_id, val, note, QStringLiteral("Campo"), &err)) {
                                QMessageBox::information(app, get_text(QStringLiteral("Avanço Registrado")),
                                                         get_text(QStringLiteral("Avanço físico de %1% registrado com sucesso no cronograma da obra!")).arg(val));
                            } else {
                                QMessageBox::warning(app, get_text(QStringLiteral("Erro")), err);
                            }
                        }
                    });

                    // 3. Alterar vínculo
                    auto *edit_link_action = menu.addAction(get_text(QStringLiteral("🔗 Alterar Vínculo no Gantt...")));
                    QObject::connect(edit_link_action, &QAction::triggered, app, [gantt, app, task_id, task_title]() {
                        if (gantt->open_task_link_dialog(task_id, task_title, app)) {
                            QMessageBox::information(app, get_text(QStringLiteral("Cronograma")),
                                                     get_text(QStringLiteral("Vínculo com o cronograma atualizado com sucesso!")));
                        }
                    });

                    // 4. Desvincular
                    auto *unlink_action = menu.addAction(get_text(QStringLiteral("🔓 Desvincular do Cronograma de Gantt...")));
                    QObject::connect(unlink_action, &QAction::triggered, app, [gantt, app, task_id, wbs_and_name]() {
                        QMessageBox confirm_unlink(app);
                        confirm_unlink.setIcon(QMessageBox::Question);
                        confirm_unlink.setWindowTitle(get_text(QStringLiteral("Desvincular")));
                        confirm_unlink.setText(get_text(QStringLiteral("Deseja desvincular esta tarefa da atividade '%1' do cronograma?")).arg(wbs_and_name));
                        confirm_unlink.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                        confirm_unlink.setDefaultButton(QMessageBox::No);
                        if (app != nullptr && app->gerenciador_traducao != nullptr) {
                            app->gerenciador_traducao->traduzir_botoes_padrao(&confirm_unlink);
                        }
                        if (confirm_unlink.exec() == QMessageBox::Yes) {
                            gantt->unlink_task(task_id);
                            QMessageBox::information(app, get_text(QStringLiteral("Cronograma")),
                                                     get_text(QStringLiteral("Tarefa desvinculada com sucesso do cronograma.")));
                        }
                    });
                } else {
                    // 1. Criar Atividade no Cronograma
                    auto *create_act_action = menu.addAction(get_text(QStringLiteral("🏗️ Criar Atividade no Cronograma de Gantt")));
                    QObject::connect(create_act_action, &QAction::triggered, app, [gantt, app, data]() {
                        QString new_item_id;
                        QString err;
                        if (gantt->convert_eisenhower_task_to_gantt_activity(data, &new_item_id, &err)) {
                            QMessageBox confirm_open(app);
                            confirm_open.setIcon(QMessageBox::Question);
                            confirm_open.setWindowTitle(get_text(QStringLiteral("Atividade Criada")));
                            confirm_open.setText(get_text(QStringLiteral("Atividade criada e vinculada com sucesso no Cronograma de Gantt!\n\nDeseja abrir o cronograma agora?")));
                            confirm_open.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                            confirm_open.setDefaultButton(QMessageBox::Yes);
                            if (app != nullptr && app->gerenciador_traducao != nullptr) {
                                app->gerenciador_traducao->traduzir_botoes_padrao(&confirm_open);
                            }
                            if (confirm_open.exec() == QMessageBox::Yes) {
                                gantt->open_gantt_and_highlight_activity(new_item_id, app);
                            }
                        } else {
                            QMessageBox::warning(app, get_text(QStringLiteral("Erro")), err);
                        }
                    });

                    // 2. Vincular ao Cronograma de Gantt...
                    auto *link_gantt_action = menu.addAction(get_text(QStringLiteral("🔒 Vincular ao Cronograma de Gantt...")));
                    QObject::connect(link_gantt_action, &QAction::triggered, app, [gantt, app, task_id, task_title]() {
                        if (gantt->open_task_link_dialog(task_id, task_title, app)) {
                            QMessageBox::information(
                                app,
                                get_text(QStringLiteral("Cronograma")),
                                get_text(QStringLiteral("Vínculo com o cronograma de Gantt atualizado com sucesso!")));
                        }
                    });
                }
            }
        }
    }

    menu.addSeparator();
    auto *remove_action = menu.addAction(get_text(QStringLiteral("🗑️ Remover Tarefa")));
    QObject::connect(remove_action, &QAction::triggered, app, [app, list_widget, selected_items]() {
        QList<QVariantMap> selected_payloads;
        QStringList selected_names;
        selected_payloads.reserve(selected_items.size());
        selected_names.reserve(selected_items.size());
        for (QListWidgetItem *selected : selected_items) {
            if (selected == nullptr) {
                continue;
            }
            const QVariantMap payload = selected->data(Qt::UserRole).toMap();
            if (payload.isEmpty()) {
                continue;
            }
            selected_payloads.append(payload);
            selected_names << task_name_for_message(selected);
        }

        if (selected_payloads.isEmpty()) {
            return;
        }

        const QString prompt = selected_payloads.size() == 1
                                   ? get_text(QStringLiteral("Deseja remover a tarefa '{item}'?"))
                                         .replace(QStringLiteral("{item}"), selected_names.value(0, task_name_for_payload(selected_payloads.first())))
                                   : get_text(QStringLiteral("Deseja remover as tarefas selecionadas?"));

        QMessageBox confirmation(app);
        confirmation.setIcon(QMessageBox::Question);
        confirmation.setWindowTitle(get_text(QStringLiteral("🗑️ Remover Tarefa")));
        confirmation.setText(prompt);
        confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        confirmation.setDefaultButton(QMessageBox::No);
        if (app->gerenciador_traducao != nullptr) {
            app->gerenciador_traducao->traduzir_botoes_padrao(&confirmation);
        }

        if (confirmation.exec() != QMessageBox::Yes) {
            return;
        }

        app->show_task_sync_delete_info(QStringLiteral("eisenhower"));

        int removed = 0;
        for (const QVariantMap &payload : std::as_const(selected_payloads)) {
            const auto resolved = resolve_payload(app, payload);
            if (resolved.first != nullptr && resolved.second != nullptr) {
                if (app->remove_task(resolved.second, resolved.first, false)) {
                    ++removed;
                }
            }
        }

        if (removed > 0) {
            app->save_tasks();
            if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
                app->calendar_pane->calendar_panel->update_task_list();
            }
            QMessageBox::information(
                app,
                get_text(QStringLiteral("🗑️ Remover Tarefa")),
                get_text(QStringLiteral("Tarefa(s) removida(s) com sucesso.")));
        } else {
            QMessageBox::warning(
                app,
                get_text(QStringLiteral("🗑️ Remover Tarefa")),
                get_text(QStringLiteral("Não foi possível remover as tarefas selecionadas.")));
        }
    });

    menu.exec(list_widget->viewport()->mapToGlobal(point));
}

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
