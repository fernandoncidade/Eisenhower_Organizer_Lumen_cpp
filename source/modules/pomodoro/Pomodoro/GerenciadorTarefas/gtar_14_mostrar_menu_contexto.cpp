#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_14_mostrar_menu_contexto.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::mostrar_menu_contexto(const QPoint &pos, QListWidget *lista)
{
    try {
        if (lista == nullptr) {
            return;
        }

        auto *item = lista->itemAt(pos);
        if (item == nullptr || is_group_header(item) || !(item->flags() & Qt::ItemIsSelectable)) {
            return;
        }

        QMenu menu(lista);

        const QVector<QListWidgetItem *> marked = collect_marked_items();
        const QVector<QListWidgetItem *> selected = marked.isEmpty() ? QVector<QListWidgetItem *>{item} : marked;

        auto *action_edit = menu.addAction(QCoreApplication::translate("PomodoroPlugin", "✏️ Editar Tarefa"));
        connect(action_edit, &QAction::triggered, this, [this, item]() {
            Tarefa *task = find_task(item);
            if (task == nullptr) {
                return;
            }

            const int default_ciclos = obter_ciclos_permitidos_padrao();
            const bool is_in_prog = (task->status == QStringLiteral("Doing"));
            const bool is_custom = task->ciclos_customizados
                                   || (task->ciclos_totais > 0 && task->ciclos_totais != default_ciclos);
            int initial_ciclos = default_ciclos;
            if (is_in_prog) {
                initial_ciclos = (is_custom && task->ciclos_totais > 0)
                                     ? task->ciclos_totais
                                     : default_ciclos;
            } else {
                initial_ciclos = (task->ciclos_totais > 0)
                                     ? task->ciclos_totais
                                     : default_ciclos;
            }
            EditTaskDialog dialog(
                this,
                task->titulo,
                task->file_path,
                task->descricao,
                task->date,
                task->time,
                task->pomodoros_completados,
                task->prioridade,
                task->dominio,
                task->duracao > 0.0 ? task->duracao : 1.0,
                initial_ciclos);

            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            const auto values = dialog.values();
            if (values.title.isEmpty()) {
                return;
            }

            task->titulo = values.title;
            task->descricao = values.description;
            task->prioridade = Tarefa::normalizar_prioridade(values.prioridade);
            task->dominio = Tarefa::normalizar_dominio(values.dominio);
            task->pomodoros_completados = values.pomodoros;
            task->ciclos_totais = values.ciclos_totais > 0 ? values.ciclos_totais : default_ciclos;
            task->ciclos_customizados = true;
            task->duracao = values.duracao;
            task->file_path = values.file_path;
            task->date = values.date_iso;
            task->time = values.time_str;

            if (task->id.trimmed().isEmpty()) {
                task->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            }

            const QString task_id = task->id;
            const QString task_title = task->titulo;
            const QString task_priority = task->prioridade;
            const int task_domain = task->dominio;
            const QString task_description = task->descricao;
            const QString task_file_path = task->file_path;
            const QString task_date = task->date;
            const QString task_time = task->time;
            const int task_pomodoros = task->pomodoros_completados;
            const int task_cycles = task->ciclos_totais > 0 ? task->ciclos_totais : default_ciclos;
            const double task_duration = task->duracao;
            const QString task_status = task->status;

            atualizar_listas();
            compat::MessageBox::tempo_edit_task_success(this);

            QTimer::singleShot(0, this, [this, task_id, task_title, task_priority, task_domain,
                                         task_description, task_file_path, task_date, task_time,
                                         task_pomodoros, task_cycles, task_duration, task_status]() {
                salvar_tarefas();

                if (compat::host_bridge() != nullptr) {
                    compat::host_bridge()->sync_task_duration(task_id, task_duration);
                    QVariantMap payload;
                    payload.insert(QStringLiteral("id"), task_id);
                    payload.insert(QStringLiteral("titulo"), task_title);
                    payload.insert(QStringLiteral("prioridade"), task_priority);
                    payload.insert(QStringLiteral("dominio"), task_domain);
                    payload.insert(QStringLiteral("description"), task_description);
                    payload.insert(QStringLiteral("file_path"), task_file_path);
                    payload.insert(QStringLiteral("date"), task_date);
                    payload.insert(QStringLiteral("time"), task_time);
                    payload.insert(QStringLiteral("pomodoros"), task_pomodoros);
                    payload.insert(QStringLiteral("pomodoros_completados"), task_pomodoros);
                    payload.insert(QStringLiteral("ciclos_totais"), task_cycles);
                    payload.insert(QStringLiteral("ciclos_permitidos"), task_cycles);
                    payload.insert(QStringLiteral("total_ciclos"), task_cycles);
                    payload.insert(QStringLiteral("ciclos_customizados"), true);
                    payload.insert(QStringLiteral("duracao"), task_duration);
                    payload.insert(QStringLiteral("duration"), task_duration);
                    payload.insert(QStringLiteral("status"), task_status);
                    payload.insert(QStringLiteral("completed"), task_status == QStringLiteral("Done"));
                    payload.insert(QStringLiteral("in_progress"), task_status == QStringLiteral("Doing"));
                    compat::host_bridge()->publish_task_to_eisenhower(payload);
                }
            });
        });

        auto *action_font = menu.addAction(QCoreApplication::translate("PomodoroPlugin", "🔤 Configurações de Fonte"));
        connect(action_font, &QAction::triggered, this, [this, selected]() {
            QList<QListWidgetItem *> target_items;
            for (QListWidgetItem *sel_item : selected) {
                if (sel_item != nullptr && !target_items.contains(sel_item)) {
                    target_items.append(sel_item);
                }
            }
            if (source::utils::open_task_font_settings_dialog(this, target_items, source::utils::PluginFontContext::Pomodoro)) {
                if (QWidget *top = this->window()) {
                    for (QListWidget *list : top->findChildren<QListWidget *>()) {
                        if (list == nullptr) continue;
                        for (int i = 0; i < list->count(); ++i) {
                            if (QListWidgetItem *sel_item = list->item(i)) {
                                if (Tarefa *task = find_task(sel_item)) {
                                    task->font_config = sel_item->data(source::utils::kCustomFontSettingsRole).toMap();
                                }
                            }
                        }
                    }
                }
                salvar_tarefas();
                atualizar_listas();
            }
        });

        auto *action_color = menu.addAction(QCoreApplication::translate("PomodoroPlugin", "🎨 Colorir tarefas"));
        connect(action_color, &QAction::triggered, this, [this, selected]() {
            QColor initial_color;
            for (QListWidgetItem *sel_item : selected) {
                if (Tarefa *task = find_task(sel_item)) {
                    if (QColor col(task->cor_texto.trimmed()); col.isValid()) {
                        initial_color = col;
                        break;
                    }
                }
            }
            const QColor selected_color = source::utils::choose_task_color(this, initial_color, source::utils::PluginFontContext::Pomodoro);
            if (!selected_color.isValid()) {
                return;
            }
            const QString color_name = selected_color.name(QColor::HexRgb);
            for (QListWidgetItem *sel_item : selected) {
                if (Tarefa *task = find_task(sel_item)) {
                    task->cor_texto = color_name;
                }
            }
            salvar_tarefas();
            atualizar_listas();
        });

        auto *action_recurrence =
            menu.addAction(QCoreApplication::translate("PomodoroPlugin", "🔁 Recorrência"));
        connect(action_recurrence, &QAction::triggered, this, [this, item]() {
            Tarefa *task = find_task(item);
            if (task == nullptr) {
                return;
            }

            QDialog dialog(this);
            dialog.setWindowTitle(QCoreApplication::translate("PomodoroPlugin", "Recorrência"));
            auto *layout = new QFormLayout(&dialog);
            auto *combo = new QComboBox(&dialog);
            combo->addItem(QCoreApplication::translate("PomodoroPlugin", "Sem recorrência"), QString());
            combo->addItem(QCoreApplication::translate("PomodoroPlugin", "Diária"), QStringLiteral("daily"));
            combo->addItem(QCoreApplication::translate("PomodoroPlugin", "Semanal"), QStringLiteral("weekly"));
            combo->addItem(QCoreApplication::translate("PomodoroPlugin", "Mensal"), QStringLiteral("monthly"));

            auto *until_enabled =
                new QCheckBox(QCoreApplication::translate("PomodoroPlugin", "Definir data final"), &dialog);
            auto *until = new QDateEdit(QDate::currentDate().addMonths(3), &dialog);
            until->setCalendarPopup(true);
            until->setEnabled(false);
            connect(until_enabled, &QCheckBox::toggled, until, &QDateEdit::setEnabled);

            QString current = task->recurrence.trimmed();
            QString until_str = task->recurrence_until.trimmed();
            const int idx = combo->findData(current);
            combo->setCurrentIndex(idx >= 0 ? idx : 0);
            const QDate current_until = QDate::fromString(until_str, Qt::ISODate);
            if (current_until.isValid()) {
                until_enabled->setChecked(true);
                until->setDate(current_until);
            }

            layout->addRow(QCoreApplication::translate("PomodoroPlugin", "Frequência"), combo);
            layout->addRow(until_enabled, until);
            auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
            if (compat::host_bridge() != nullptr) {
                compat::host_bridge()->translate_standard_buttons(buttons);
            }
            layout->addRow(buttons);
            connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
            connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

            if (dialog.exec() != QDialog::Accepted) {
                return;
            }

            task->recurrence = combo->currentData().toString();
            task->recurrence_until = until_enabled->isChecked() ? until->date().toString(Qt::ISODate) : QString();
            salvar_tarefas();
            atualizar_listas();

            if (compat::host_bridge() != nullptr) {
                QVariantMap payload = task_payload(*task);
                payload.insert(QStringLiteral("recurrence"), task->recurrence);
                payload.insert(QStringLiteral("recorrencia"), task->recurrence);
                payload.insert(QStringLiteral("recurrence_until"), task->recurrence_until);
                payload.insert(QStringLiteral("recorrencia_ate"), task->recurrence_until);
                compat::host_bridge()->publish_task_to_eisenhower(payload);
                compat::host_bridge()->notify_shared_state_changed(QStringLiteral("pomodoro"));
            }
        });

        auto *share_action =
            menu.addAction(QCoreApplication::translate("PomodoroPlugin", "🗂️ Compartilhar com Eisenhower"));
        connect(share_action, &QAction::triggered, this, [this, selected]() {
            int sent = 0;
            QSet<QString> seen;

            for (QListWidgetItem *selected_item : selected) {
                if (Tarefa *task = find_task(selected_item)) {
                    const QString signature = compat::task_identity_signature(*task);
                    if (seen.contains(signature)) {
                        continue;
                    }

                    seen.insert(signature);
                    QVariantMap payload = task_payload(*task);
                    payload.insert(QStringLiteral("integration_id"), QUuid::createUuid().toString(QUuid::WithoutBraces));
                    payload.insert(QStringLiteral("origin"), QStringLiteral("tempo"));
                    compat::get_event_bus()->send_tarefa(payload);
                    task->sincronizada_com_eisenhower = true;
                    ++sent;
                }
            }

            if (sent > 0) {
                salvar_tarefas();
            }

            if (sent > 0) {
                compat::MessageBox::tempo_share_eisenhower_success(this, sent);
            }
        });

        if (compat::host_bridge() != nullptr && compat::host_bridge()->can_publish_task_to_feynman()) {
            auto *share_feynman_action =
                menu.addAction(QCoreApplication::translate("PomodoroPlugin", "🪢 Compartilhar com Feynman"));
            connect(share_feynman_action, &QAction::triggered, this, [this, selected]() {
                int sent = 0;
                QStringList failed_names;
                QSet<QString> seen;

                for (QListWidgetItem *selected_item : selected) {
                    Tarefa *task = find_task(selected_item);
                    if (task == nullptr) {
                        failed_names << selected_item->text().split(QLatin1Char('\n')).value(0).trimmed();
                        continue;
                    }

                    const QString signature = compat::task_identity_signature(*task);
                    if (seen.contains(signature)) {
                        continue;
                    }

                    seen.insert(signature);
                    QVariantMap payload = task_payload(*task);
                    payload.insert(QStringLiteral("integration_id"), QUuid::createUuid().toString(QUuid::WithoutBraces));
                    payload.insert(QStringLiteral("origin"), QStringLiteral("pomodoro"));

                    if (compat::host_bridge() == nullptr || !compat::host_bridge()->publish_task_to_feynman(payload)) {
                        failed_names << task->titulo;
                        continue;
                    }

                    ++sent;
                }

                if (sent > 0) {
                    compat::MessageBox::tempo_share_feynman_success(this, sent);
                }

                if (!failed_names.isEmpty()) {
                    compat::MessageBox::warning(
                        this,
                        QCoreApplication::translate("PomodoroPlugin", "Erro"),
                        QCoreApplication::translate("PomodoroPlugin",
                                                    "Nao foi possivel compartilhar a tarefa \"%1\" com Feynman.")
                            .arg(failed_names.join(QStringLiteral("\", \""))));
                }
            });
        }

        if (compat::host_bridge() != nullptr && compat::host_bridge()->can_publish_task_to_gantt() && !selected.isEmpty()) {
            QListWidgetItem *selected_item = selected.first();
            Tarefa *task = find_task(selected_item);
            if (task != nullptr) {
                QString task_id = task->id.trimmed();
                if (task_id.isEmpty()) {
                    task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                    task->id = task_id;
                    salvar_tarefas();
                }
                QString task_title = task->titulo.trimmed();
                if (task_title.isEmpty()) {
                    task_title = selected_item->text().split(QLatin1Char('\n')).value(0).trimmed();
                }

                QString wbs_and_name, sched_name, act_status;
                double act_progress = 0.0;
                const bool is_linked = compat::host_bridge()->get_gantt_summary(
                    task_id, &wbs_and_name, &sched_name, &act_progress, &act_status);

                if (is_linked) {
                    // 1. Abrir no Cronograma de Gantt (...)
                    const QString open_text = !wbs_and_name.trimmed().isEmpty()
                        ? QCoreApplication::translate("PomodoroPlugin", "🏗️ Abrir no Cronograma de Gantt (%1 - %2%)")
                              .arg(wbs_and_name)
                              .arg(static_cast<int>(act_progress))
                        : QCoreApplication::translate("PomodoroPlugin", "🏗️ Abrir no Cronograma de Gantt (%1%)")
                              .arg(static_cast<int>(act_progress));
                    auto *open_gantt_action = menu.addAction(open_text);
                    connect(open_gantt_action, &QAction::triggered, this, [this, task_id]() {
                        compat::host_bridge()->open_gantt_and_highlight_activity(task_id, this);
                    });

                    // 2. Registrar Avanço de Campo no Cronograma...
                    auto *progress_action = menu.addAction(
                        QCoreApplication::translate("PomodoroPlugin", "⚡ Registrar Avanço de Campo no Cronograma..."));
                    connect(progress_action, &QAction::triggered, this, [this, task, task_id, wbs_and_name, act_progress]() {
                        bool ok = false;
                        const QString label_text = !wbs_and_name.trimmed().isEmpty()
                            ? QCoreApplication::translate("PomodoroPlugin", "Informe o percentual executado para '%1':").arg(wbs_and_name)
                            : QCoreApplication::translate("PomodoroPlugin", "Informe o percentual executado:");
                        const double val = QInputDialog::getDouble(
                            this,
                            QCoreApplication::translate("PomodoroPlugin", "Avanço Físico de Obra"),
                            label_text,
                            act_progress, 0.0, 100.0, 1, &ok);
                        if (ok) {
                            const QString note = QInputDialog::getText(
                                this,
                                QCoreApplication::translate("PomodoroPlugin", "Diário de Obra"),
                                QCoreApplication::translate("PomodoroPlugin", "Observações de campo (opcional):"),
                                QLineEdit::Normal, QString(), &ok);
                            QString err;
                            if (compat::host_bridge()->record_field_progress_for_task(task_id, val, note, QStringLiteral("Campo"), &err)) {
                                QMessageBox::information(
                                    this,
                                    QCoreApplication::translate("PomodoroPlugin", "Avanço Registrado"),
                                    QCoreApplication::translate("PomodoroPlugin", "Avanço físico de %1% registrado com sucesso no cronograma da obra!").arg(val));
                                if (val >= 100.0) {
                                    task->status = QStringLiteral("Done");
                                } else if (val > 0.0 && task->status == QStringLiteral("Todo")) {
                                    task->status = QStringLiteral("Doing");
                                }
                                salvar_tarefas();
                                atualizar_listas();
                                compat::host_bridge()->notify_shared_state_changed(QStringLiteral("pomodoro"));
                            } else {
                                QMessageBox::warning(
                                    this,
                                    QCoreApplication::translate("PomodoroPlugin", "Erro"),
                                    err);
                            }
                        }
                    });

                    // 3. Alterar Vínculo no Gantt...
                    auto *edit_link_action = menu.addAction(
                        QCoreApplication::translate("PomodoroPlugin", "🔗 Alterar Vínculo no Gantt..."));
                    connect(edit_link_action, &QAction::triggered, this, [this, task_id, task_title]() {
                        if (compat::host_bridge()->open_gantt_link_dialog(task_id, task_title, this)) {
                            QMessageBox::information(
                                this,
                                QCoreApplication::translate("PomodoroPlugin", "Cronograma"),
                                QCoreApplication::translate("PomodoroPlugin", "Vínculo com o cronograma atualizado com sucesso!"));
                            salvar_tarefas();
                            atualizar_listas();
                            compat::host_bridge()->notify_shared_state_changed(QStringLiteral("pomodoro"));
                        }
                    });

                    // 4. Desvincular do Cronograma de Gantt...
                    auto *unlink_action = menu.addAction(
                        QCoreApplication::translate("PomodoroPlugin", "🔓 Desvincular do Cronograma de Gantt..."));
                    connect(unlink_action, &QAction::triggered, this, [this, task_id, wbs_and_name]() {
                        QMessageBox confirm_unlink(this);
                        confirm_unlink.setIcon(QMessageBox::Question);
                        confirm_unlink.setWindowTitle(QCoreApplication::translate("PomodoroPlugin", "Desvincular"));
                        confirm_unlink.setText(QCoreApplication::translate("PomodoroPlugin", "Deseja desvincular esta tarefa da atividade '%1' do cronograma?").arg(wbs_and_name));
                        confirm_unlink.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                        confirm_unlink.setDefaultButton(QMessageBox::No);
                        if (compat::host_bridge() != nullptr) {
                            compat::host_bridge()->translate_standard_buttons(&confirm_unlink);
                        }
                        if (confirm_unlink.exec() == QMessageBox::Yes) {
                            compat::host_bridge()->unlink_task(task_id);
                            QMessageBox::information(
                                this,
                                QCoreApplication::translate("PomodoroPlugin", "Cronograma"),
                                QCoreApplication::translate("PomodoroPlugin", "Tarefa desvinculada com sucesso do cronograma."));
                            salvar_tarefas();
                            atualizar_listas();
                            compat::host_bridge()->notify_shared_state_changed(QStringLiteral("pomodoro"));
                        }
                    });
                } else {
                    // 1. Vincular ao Cronograma de Gantt...
                    auto *link_gantt_action = menu.addAction(
                        QCoreApplication::translate("PomodoroPlugin", "🔒 Vincular ao Cronograma de Gantt..."));
                    connect(link_gantt_action, &QAction::triggered, this, [this, task_id, task_title]() {
                        if (compat::host_bridge()->open_gantt_link_dialog(task_id, task_title, this)) {
                            QMessageBox::information(
                                this,
                                QCoreApplication::translate("PomodoroPlugin", "Cronograma"),
                                QCoreApplication::translate("PomodoroPlugin", "Vínculo com o cronograma de Gantt atualizado com sucesso!"));
                            salvar_tarefas();
                            atualizar_listas();
                            compat::host_bridge()->notify_shared_state_changed(QStringLiteral("pomodoro"));
                        }
                    });
                }
            }
        }

        if (selected.size() > 1) {
            auto *move_selected =
                menu.addAction(QCoreApplication::translate("PomodoroPlugin", "🔀 Mover Tarefa(s) Marcadas"));
            connect(move_selected, &QAction::triggered, this, &GerenciadorTarefas::mover_tarefas_marcadas);

            auto *remove_selected =
                menu.addAction(QCoreApplication::translate("PomodoroPlugin", "🗑️ Remover Tarefa(s) Marcadas"));
            connect(remove_selected, &QAction::triggered, this, &GerenciadorTarefas::remover_tarefas_marcadas);
        } else {
            auto *move_single = menu.addAction(QCoreApplication::translate("PomodoroPlugin", "🔀 Mover Tarefa"));
            connect(move_single, &QAction::triggered, this, [this, item]() { mover_tarefa_por_item(item); });

            auto *remove_single = menu.addAction(QCoreApplication::translate("PomodoroPlugin", "🗑️ Remover Tarefa"));
            connect(remove_single, &QAction::triggered, this, [this, item]() { remover_tarefa_por_item(item); });
        }

        menu.exec(lista->mapToGlobal(pos));
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao exibir menu de contexto: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao exibir menu de contexto"), true);
    }
}

void GerenciadorTarefas::on_task_duration_changed(const QString &task_id, double duration_days)
{
    const QString target_id = task_id.trimmed();
    if (target_id.isEmpty()) {
        return;
    }

    bool changed = false;
    for (Tarefa &task : tarefas) {
        if (task.id.trimmed() == target_id || task.integracao_origem.trimmed() == target_id) {
            task.duracao = duration_days;
            changed = true;
        }
    }

    if (changed) {
        atualizar_listas();
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void mostrar_menu_contexto(GerenciadorTarefas &self, const QPoint &pos, QListWidget *lista) { self.mostrar_menu_contexto(pos, lista); } }
