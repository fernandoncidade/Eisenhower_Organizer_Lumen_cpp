#include "source/app/app_06_send_task_to_feynman.hpp"

namespace source {
using namespace app;

namespace {

QString canonical_sync_module(const QString &module)
{
    const QString normalized = module.trimmed().toCaseFolded();
    if (normalized == QStringLiteral("tempo")) {
        return QStringLiteral("pomodoro");
    }
    if (normalized == QStringLiteral("eisenhower") || normalized == QStringLiteral("pomodoro")
        || normalized == QStringLiteral("feynman")) {
        return normalized;
    }
    return {};
}

QStringList enabled_sync_modules(const EisenhowerMatrixApp *app)
{
    QStringList modules;
    if (app != nullptr && app->sync_eisenhower_enabled()) {
        modules.append(QStringLiteral("eisenhower"));
    }
    if (app != nullptr && app->sync_pomodoro_enabled() && app->has_pomodoro_plugin()) {
        modules.append(QStringLiteral("pomodoro"));
    }
    if (app != nullptr && app->sync_feynman_enabled() && app->has_feynman_plugin()) {
        modules.append(QStringLiteral("feynman"));
    }
    return modules;
}

QStringList active_sync_modules_for_origin(const EisenhowerMatrixApp *app, const QString &origin)
{
    QStringList modules = enabled_sync_modules(app);
    const QString origin_module = canonical_sync_module(origin);
    if (!origin_module.isEmpty() && !modules.contains(origin_module)) {
        return {};
    }
    return modules;
}

QString sync_module_display_name(const QString &module)
{
    const QString normalized = canonical_sync_module(module);
    if (normalized == QStringLiteral("eisenhower")) {
        return get_text(QStringLiteral("Eisenhower"));
    }
    if (normalized == QStringLiteral("pomodoro")) {
        return get_text(QStringLiteral("Pomodoro"));
    }
    if (normalized == QStringLiteral("feynman")) {
        return get_text(QStringLiteral("Feynman"));
    }
    return module;
}

QString sync_module_display_list(const QStringList &modules)
{
    QStringList labels;
    for (const QString &module : modules) {
        labels.append(sync_module_display_name(module));
    }
    return labels.join(QStringLiteral(", "));
}

}  // namespace

void EisenhowerMatrixApp::refresh_module_task_counter()
{
    _attach_module_task_counter_to_menu_bar();

    if (module_task_counter_label == nullptr || module_tabs == nullptr) {
        return;
    }

    QString module_name;
    QPair<int, int> counts = {0, 0};
    QWidget *current_page = module_tabs->currentWidget();
    if (current_page == eisenhower_module_page) {
        module_name = QStringLiteral("Eisenhower");
        counts = _eisenhower_task_counts();
    } else if (current_page == feynman_module_page) {
        module_name = QStringLiteral("Feynman");
        counts = _feynman_task_counts();
    } else if (current_page == pomodoro_module_page) {
        module_name = QStringLiteral("Pomodoro");
        counts = _pomodoro_task_counts();
    } else if (current_page == mapamental_module_page) {
        module_name = mapamental_display_name();
        counts = _mapamental_task_counts();
    } else if (current_page == audiolivro_module_page) {
        module_name = audiolivro_display_name();
        counts = _audiolivro_task_counts();
    }

    module_task_counter_label->setText(module_name.isEmpty()
                                           ? QString()
                                           : module_task_counter_text(module_name, counts.first, counts.second));
}

bool EisenhowerMatrixApp::clear_pomodoro_tasks_if_available()
{
    if (!has_pomodoro_plugin() || pomodoro_module_page == nullptr) {
        return false;
    }

    bool cleared = false;
    const bool invoked = QMetaObject::invokeMethod(pomodoro_module_page,
                                                   "limpar_todas_as_tarefas",
                                                   Qt::DirectConnection,
                                                   Q_RETURN_ARG(bool, cleared));
    if (!invoked) {
        logger()->warning(QStringLiteral("Plugin Pomodoro carregado, mas não expôs limpeza total de tarefas."));
        return false;
    }

    return cleared;
}

bool EisenhowerMatrixApp::clear_feynman_tasks_if_available()
{
    if (!has_feynman_plugin() || feynman_module_page == nullptr) {
        return false;
    }

    bool cleared = false;
    const bool invoked = QMetaObject::invokeMethod(feynman_module_page,
                                                   "limpar_todas_as_tarefas",
                                                   Qt::DirectConnection,
                                                   Q_RETURN_ARG(bool, cleared));
    if (!invoked) {
        logger()->warning(QStringLiteral("Plugin Feynman carregado, mas não expôs limpeza total de tarefas."));
        return false;
    }

    return cleared;
}

void EisenhowerMatrixApp::reload_all_modules_from_shared_store()
{
    pending_runtime_sync_origins_.clear();
    _reload_eisenhower_from_shared_store();
    _reload_pomodoro_from_shared_store();
    _reload_feynman_from_shared_store();
    _refresh_plugin_task_views();
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }
}

bool EisenhowerMatrixApp::has_active_task_sync(const QString &origin) const
{
    return active_sync_modules_for_origin(this, origin).size() > 1;
}

void EisenhowerMatrixApp::show_task_sync_add_info(const QString &origin)
{
    const QStringList modules = active_sync_modules_for_origin(this, origin);
    if (modules.size() <= 1) {
        return;
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle(get_text(QStringLiteral("Sincronização de Tarefas")));
    box.setText(get_text(QStringLiteral("A tarefa será adicionada aos métodos sincronizados: %1."))
                    .arg(sync_module_display_list(modules)));
    box.setStandardButtons(QMessageBox::Ok);
    if (gerenciador_traducao != nullptr) {
        gerenciador_traducao->traduzir_botoes_padrao(&box);
    }
    box.exec();
}

void EisenhowerMatrixApp::show_task_sync_delete_info(const QString &origin, QWidget *parent)
{
    const QStringList modules = active_sync_modules_for_origin(this, origin);
    if (modules.size() <= 1) {
        return;
    }

    QMessageBox box(parent != nullptr ? parent : this);
    box.setIcon(QMessageBox::Information);
    box.setWindowTitle(get_text(QStringLiteral("Sincronização de Tarefas")));
    box.setText(get_text(QStringLiteral("A tarefa será excluída dos métodos sincronizados: %1."))
                    .arg(sync_module_display_list(modules)));
    box.setStandardButtons(QMessageBox::Ok);
    if (gerenciador_traducao != nullptr) {
        gerenciador_traducao->traduzir_botoes_padrao(&box);
    }
    box.exec();
}

void EisenhowerMatrixApp::remove_task_from_synced_modules(const QVariantMap &task, const QString &origin)
{
    const QStringList modules = active_sync_modules_for_origin(this, origin);
    if (modules.size() <= 1 || tasks_path.trimmed().isEmpty()) {
        return;
    }

    const QVariantList records = source::utils::UnifiedTaskStore::load_records(tasks_path);
    const QVariantList updated = source::utils::UnifiedTaskStore::remove_linked_records(records, task, modules);
    if (updated.size() != records.size()) {
        source::utils::UnifiedTaskStore::save_records(tasks_path, updated);
    }
}

bool EisenhowerMatrixApp::_send_task_payload_to_feynman(const QVariantMap &task, bool activate_tab)
{
    if (!has_feynman_plugin() || feynman_plugin_ == nullptr) {
        return false;
    }

    QVariantMap payload = task;
    if (payload.value(QStringLiteral("origin")).toString().trimmed().isEmpty()) {
        payload.insert(QStringLiteral("origin"), QStringLiteral("eisenhower"));
    }

    feynman_plugin_->receive_task(payload);

    if (activate_tab) {
        open_feynman();
    }

    return true;
}

bool EisenhowerMatrixApp::send_task_to_feynman(QListWidgetItem *item, QListWidget *source_list, bool activate_tab)
{
    if (!has_feynman_plugin() || item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
        return false;
    }

    QVariantMap data = item->data(Qt::UserRole).toMap();
    QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
    if (task_id.isEmpty()) {
        task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        data.insert(QStringLiteral("id"), task_id);
        item->setData(Qt::UserRole, data);
    }

    QVariantMap payload;
    payload.insert(QStringLiteral("integration_id"), QUuid::createUuid().toString(QUuid::WithoutBraces));
    payload.insert(QStringLiteral("origin"), QStringLiteral("eisenhower"));
    payload.insert(QStringLiteral("id"), task_id);
    payload.insert(QStringLiteral("titulo"), base_task_text(item));
    payload.insert(QStringLiteral("prioridade"), priority_label_without_emoji(data.value(QStringLiteral("priority")), this));
    if (data.contains(QStringLiteral("dominio"))) {
        payload.insert(QStringLiteral("dominio"), normalized_domain_value(data.value(QStringLiteral("dominio"))));
    }
    payload.insert(QStringLiteral("description"), data.value(QStringLiteral("description")).toString());
    payload.insert(QStringLiteral("file_path"), data.value(QStringLiteral("file_path")).toString());
    payload.insert(QStringLiteral("date"), data.value(QStringLiteral("date")).toString());
    payload.insert(QStringLiteral("time"), data.value(QStringLiteral("time")).toString());
    payload.insert(QStringLiteral("cor_texto"), data.value(QStringLiteral("cor_texto")).toString());
    payload.insert(QStringLiteral("cor_texto_original"), data.value(QStringLiteral("cor_texto_original")).toString());
    payload.insert(QStringLiteral("cor_texto_customizada"), data.value(QStringLiteral("cor_texto_customizada")).toString());
    payload.insert(QStringLiteral("completed"), item->checkState() == Qt::Checked);
    payload.insert(QStringLiteral("status"),
                   (source_list != nullptr ? is_completed_list(source_list) : item->checkState() == Qt::Checked)
                       ? QStringLiteral("Done")
                       : QStringLiteral("Todo"));

    const bool sent = _send_task_payload_to_feynman(payload, activate_tab);
    if (sent) {
        save_tasks();
    }

    return sent;
}

bool EisenhowerMatrixApp::send_task_to_pomodoro(QListWidgetItem *item, QListWidget *source_list, bool activate_tab)
{
    if (!has_pomodoro_plugin() || item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
        return false;
    }

    QVariantMap data = item->data(Qt::UserRole).toMap();
    QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
    if (task_id.isEmpty()) {
        task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        data.insert(QStringLiteral("id"), task_id);
        item->setData(Qt::UserRole, data);
    }

    QVariantMap payload;
    payload.insert(QStringLiteral("integration_id"), QUuid::createUuid().toString(QUuid::WithoutBraces));
    payload.insert(QStringLiteral("origin"), QStringLiteral("eisenhower"));
    payload.insert(QStringLiteral("id"), task_id);
    payload.insert(QStringLiteral("titulo"), base_task_text(item));
    payload.insert(QStringLiteral("prioridade"), priority_label_without_emoji(data.value(QStringLiteral("priority")), this));
    if (data.contains(QStringLiteral("dominio"))) {
        payload.insert(QStringLiteral("dominio"), normalized_domain_value(data.value(QStringLiteral("dominio"))));
    }
    payload.insert(QStringLiteral("description"), data.value(QStringLiteral("description")).toString());
    payload.insert(QStringLiteral("file_path"), data.value(QStringLiteral("file_path")).toString());
    payload.insert(QStringLiteral("date"), data.value(QStringLiteral("date")).toString());
    payload.insert(QStringLiteral("time"), data.value(QStringLiteral("time")).toString());
    payload.insert(QStringLiteral("cor_texto"), data.value(QStringLiteral("cor_texto")).toString());
    payload.insert(QStringLiteral("cor_texto_original"), data.value(QStringLiteral("cor_texto_original")).toString());
    payload.insert(QStringLiteral("cor_texto_customizada"), data.value(QStringLiteral("cor_texto_customizada")).toString());
    payload.insert(QStringLiteral("completed"), item->checkState() == Qt::Checked);
    payload.insert(QStringLiteral("status"),
                   (source_list != nullptr ? is_completed_list(source_list) : item->checkState() == Qt::Checked)
                       ? QStringLiteral("Done")
                       : QStringLiteral("Todo"));

    pomodoro_plugin_->receive_eisenhower_task(payload);
    save_tasks();

    if (activate_tab) {
        open_pomodoro();
    }

    return true;
}

bool EisenhowerMatrixApp::_upsert_task_from_external(const QVariantMap &task)
{
    const QString origin = task.value(QStringLiteral("origin")).toString().trimmed().toCaseFolded();
    if (origin.isEmpty() || origin == QStringLiteral("eisenhower")) {
        return false;
    }

    const QString title = task.value(QStringLiteral("titulo")).toString().trimmed();
    if (title.isEmpty()) {
        return false;
    }

    QListWidget *existing_list = nullptr;
    QListWidgetItem *existing_item = nullptr;
    const bool sync_only = task.value(QStringLiteral("sync_only")).toBool();
    for (QListWidget *list : all_task_lists(this)) {
        if (list == nullptr) {
            continue;
        }

        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *candidate = list->item(i);
            if (candidate == nullptr || !(candidate->flags() & Qt::ItemIsSelectable)) {
                continue;
            }

            const bool matched =
                sync_only ? task_matches_sync_payload(candidate, task) : task_matches_payload(candidate, task);
            if (matched) {
                existing_list = list;
                existing_item = candidate;
                break;
            }
        }

        if (existing_item != nullptr) {
            break;
        }
    }

    if (existing_item == nullptr && sync_only) {
        return false;
    }

    const int quadrant = quadrant_from_priority(task.value(QStringLiteral("prioridade")), this);
    const bool completed = payload_completed(task);
    const bool was_in_progress = (existing_list != nullptr && (existing_list == quadrant1_in_progress_list
                                                              || existing_list == quadrant2_in_progress_list
                                                              || existing_list == quadrant3_in_progress_list
                                                              || existing_list == quadrant4_in_progress_list));
    const QString task_status = task.value(QStringLiteral("status")).toString().trimmed();
    const bool is_in_prog = !completed && (was_in_progress
                                           || task.value(QStringLiteral("in_progress")).toBool()
                                           || task.value(QStringLiteral("em_progresso")).toBool()
                                           || task.value(QStringLiteral("em_andamento")).toBool()
                                           || task_status.compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0
                                           || task_status.compare(QStringLiteral("In Progress"), Qt::CaseInsensitive) == 0
                                           || task_status.compare(QStringLiteral("Em Progresso"), Qt::CaseInsensitive) == 0);
    QListWidget *target_list = completed
                                   ? quadrant_list(quadrant, true)
                                   : (is_in_prog ? quadrant_in_progress_list(quadrant)
                                                 : quadrant_list(quadrant, false));
    if (target_list == nullptr) {
        return false;
    }

    QVariant preserved_insertion_order;
    QVariant preserved_domain;
    if (existing_item != nullptr) {
        const QVariantMap existing_data = existing_item->data(Qt::UserRole).toMap();
        preserved_insertion_order = existing_data.value(QStringLiteral("insertion_order"));
        preserved_domain = existing_data.value(QStringLiteral("dominio"));
    }

    if (existing_list != nullptr && existing_item != nullptr) {
        const int row = existing_list->row(existing_item);
        if (row >= 0) {
            delete existing_list->takeItem(row);
            cleanup_time_groups(existing_list);
        }
    }

    if (target_list->count() == 1 && !(target_list->item(0)->flags() & Qt::ItemIsSelectable)) {
        target_list->clear();
    }

    QVariantMap normalized_task = task;
    if (!normalized_task.contains(QStringLiteral("dominio")) && preserved_domain.isValid()
        && !preserved_domain.toString().trimmed().isEmpty()) {
        normalized_task.insert(QStringLiteral("dominio"), preserved_domain);
    }

    const auto [display_text, tooltip] = display_and_tooltip_for_task(this, normalized_task, quadrant);
    auto *new_item = new QListWidgetItem(display_text);
    new_item->setFlags(new_item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    new_item->setCheckState(completed ? Qt::Checked : Qt::Unchecked);

    QVariantMap data;
    data.insert(QStringLiteral("id"), task.value(QStringLiteral("id")).toString().trimmed());
    data.insert(QStringLiteral("text"), title);
    data.insert(QStringLiteral("date"), task.value(QStringLiteral("date")).toString().trimmed());
    data.insert(QStringLiteral("time"), task.value(QStringLiteral("time")).toString().trimmed());
    data.insert(QStringLiteral("priority"), quadrant);
    data.insert(QStringLiteral("integracao_origem"), origin);
    if (normalized_task.contains(QStringLiteral("dominio"))) {
        data.insert(QStringLiteral("dominio"), normalized_domain_value(normalized_task.value(QStringLiteral("dominio"))));
    }
    if (preserved_insertion_order.isValid()) {
        data.insert(QStringLiteral("insertion_order"), preserved_insertion_order);
    }
    if (!task.value(QStringLiteral("file_path")).toString().trimmed().isEmpty()) {
        data.insert(QStringLiteral("file_path"), task.value(QStringLiteral("file_path")).toString().trimmed());
    }
    if (!task.value(QStringLiteral("description")).toString().trimmed().isEmpty()) {
        data.insert(QStringLiteral("description"), task.value(QStringLiteral("description")).toString().trimmed());
    }
    if (!task.value(QStringLiteral("cor_texto"), task.value(QStringLiteral("task_color"))).toString().trimmed().isEmpty()) {
        data.insert(QStringLiteral("cor_texto"),
                    task.value(QStringLiteral("cor_texto"), task.value(QStringLiteral("task_color"))).toString().trimmed());
    }
    if (!task.value(QStringLiteral("cor_texto_original"), task.value(QStringLiteral("agenda_cor_texto")))
             .toString()
             .trimmed()
             .isEmpty()) {
        data.insert(QStringLiteral("cor_texto_original"),
                    task.value(QStringLiteral("cor_texto_original"), task.value(QStringLiteral("agenda_cor_texto")))
                        .toString()
                        .trimmed());
    }
    if (!task.value(QStringLiteral("cor_texto_customizada")).toString().trimmed().isEmpty()) {
        data.insert(QStringLiteral("cor_texto_customizada"), task.value(QStringLiteral("cor_texto_customizada")).toString().trimmed());
    }
    double dur = 1.0;
    if (task.contains(QStringLiteral("duracao")) && task.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        dur = task.value(QStringLiteral("duracao")).toDouble();
    } else if (task.contains(QStringLiteral("duration")) && task.value(QStringLiteral("duration")).toDouble() > 0.0) {
        dur = task.value(QStringLiteral("duration")).toDouble();
    } else if (task.contains(QStringLiteral("duracao_dias")) && task.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
        dur = task.value(QStringLiteral("duracao_dias")).toDouble();
    }
    data.insert(QStringLiteral("duracao"), dur);
    data.insert(QStringLiteral("duration"), dur);
    data.insert(QStringLiteral("duracao_dias"), dur);

    const int pom = task.value(QStringLiteral("pomodoros"), task.value(QStringLiteral("pomodoros_completados"), 0)).toInt();
    data.insert(QStringLiteral("pomodoros"), pom);
    data.insert(QStringLiteral("pomodoros_completados"), pom);

    const int default_ciclos = default_pomodoro_cycle_count();
    int c_tot = task.value(QStringLiteral("ciclos_totais"),
                task.value(QStringLiteral("ciclos_permitidos"),
                task.value(QStringLiteral("total_ciclos"), 0))).toInt();
    if (c_tot <= 0 && existing_item != nullptr) {
        const QVariantMap existing_data = existing_item->data(Qt::UserRole).toMap();
        c_tot = existing_data.value(QStringLiteral("ciclos_totais"),
                existing_data.value(QStringLiteral("ciclos_permitidos"),
                existing_data.value(QStringLiteral("total_ciclos"), default_ciclos))).toInt();
    }
    if (c_tot <= 0) {
        c_tot = default_ciclos;
    }
    data.insert(QStringLiteral("ciclos_totais"), c_tot);
    data.insert(QStringLiteral("ciclos_permitidos"), c_tot);
    data.insert(QStringLiteral("total_ciclos"), c_tot);

    bool c_custom = task.value(QStringLiteral("ciclos_customizados"), false).toBool();
    if (!c_custom && existing_item != nullptr) {
        const QVariantMap existing_data = existing_item->data(Qt::UserRole).toMap();
        c_custom = existing_data.value(QStringLiteral("ciclos_customizados"), false).toBool();
    }
    if (!c_custom && c_tot != default_ciclos) {
        c_custom = true;
    }
    data.insert(QStringLiteral("ciclos_customizados"), c_custom);
    data.insert(QStringLiteral("status"), completed ? QStringLiteral("Done") : (is_in_prog ? QStringLiteral("Doing") : QStringLiteral("Todo")));
    data.insert(QStringLiteral("in_progress"), is_in_prog);
    data.insert(QStringLiteral("em_progresso"), is_in_prog);
    data.insert(QStringLiteral("em_andamento"), is_in_prog);

    new_item->setData(Qt::UserRole, data);
    new_item->setToolTip(tooltip);
    _apply_task_item_appearance(new_item);

    insert_task_into_quadrant_list(target_list, new_item);
    cleanup_time_groups(target_list);
    pending_shared_origin_ = origin;
    save_tasks();
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }
    return true;
}

}  // namespace source
