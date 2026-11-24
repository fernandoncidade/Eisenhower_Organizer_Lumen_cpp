#include "source/app/app_09_add_task.hpp"

#include <QColor>

#include "source/modules/feynman/feynman_plugin_interface.hpp"
#include "source/modules/pomodoro/pomodoro_plugin_interface.hpp"
#include "source/modules/gantt/gantt_plugin_interface.hpp"
#include "source/services/incore_11_atualizar_itens_tarefas.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"
#include "source/utils/TaskProgressHelper.hpp"

namespace source {
using namespace app;

void EisenhowerMatrixApp::add_placeholder(QListWidget *list_widget, const QString &text)
{
    source::InterfaceCore::add_placeholder(this, list_widget, text);
}

bool EisenhowerMatrixApp::_is_group_header(QListWidgetItem *item) const
{
    return item != nullptr && item->data(Qt::UserRole + 1).toString() == QStringLiteral("group_header");
}

QString EisenhowerMatrixApp::_time_group_label(const QVariantMap &data) const
{
    const QString date_str = data.value(QStringLiteral("date")).toString().trimmed();
    const QString time_str = data.value(QStringLiteral("time")).toString().trimmed();
    QStringList parts;

    if (!date_str.isEmpty()) {
        const QDate date = QDate::fromString(date_str, Qt::ISODate);
        const QString human_date =
            (date.isValid() && date_input != nullptr) ? date.toString(date_input->displayFormat()) : date_str;
        parts << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Data")), human_date);
    }

    if (!time_str.isEmpty()) {
        parts << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Horário")), time_str);
    }

    return parts.join(QStringLiteral("  |  "));
}

QString EisenhowerMatrixApp::_time_group_key(const QVariantMap &data) const
{
    if (!reserved_watermark_eisenhower_enabled_) {
        return {};
    }

    const QString time_str = data.value(QStringLiteral("time")).toString().trimmed();
    if (time_str.isEmpty()) {
        return {};
    }

    const QString date_str = data.value(QStringLiteral("date")).toString().trimmed();
    const QString date_key = date_str.isEmpty() ? QStringLiteral("9999-99-99") : date_str;
    return QStringLiteral("%1|%2").arg(date_key, time_str);
}

QPair<int, int> EisenhowerMatrixApp::_time_key(const QString &time_str) const
{
    if (time_str.isEmpty()) {
        return {999, 999};
    }

    bool ok_h = false;
    bool ok_m = false;
    const int hour = time_str.section(QLatin1Char(':'), 0, 0).toInt(&ok_h);
    const int minute = time_str.section(QLatin1Char(':'), 1, 1).toInt(&ok_m);
    return {(ok_h ? hour : 999), (ok_m ? minute : 999)};
}

void EisenhowerMatrixApp::_apply_task_item_appearance(QListWidgetItem *item) const
{
    if (item == nullptr) {
        return;
    }

    QVariantMap data = item->data(Qt::UserRole).toMap();
    QString base_text = data.value(QStringLiteral("text")).toString().trimmed();
    if (base_text.isEmpty()) {
        base_text = base_task_text(item);
    }
    data.insert(QStringLiteral("text"), base_text);
    item->setData(Qt::UserRole, data);

    const QString date_str = data.value(QStringLiteral("date")).toString().trimmed();
    const QString time_str = data.value(QStringLiteral("time")).toString().trimmed();
    const QString file_path = data.value(QStringLiteral("file_path")).toString().trimmed();
    const QColor text_color(data.value(QStringLiteral("cor_texto")).toString().trimmed());
    const QString description = data.value(QStringLiteral("description")).toString().trimmed();
    const QVariant priority = data.value(QStringLiteral("priority"));
    const QVariant domain = data.value(QStringLiteral("dominio"));

    QString decorated_base_text = base_text;
    if (domain.isValid() && !domain.toString().trimmed().isEmpty()) {
        const QString domain_badge = source::InterfaceCore::dominio_insignia(domain);
        if (!domain_badge.isEmpty()) {
            decorated_base_text = QStringLiteral("%1 %2").arg(domain_badge, base_text);
        }
    }

    QString display_text = compose_task_display_text(this, decorated_base_text, date_str, time_str);
    QStringList tooltip_lines;

    if (!date_str.isEmpty()) {
        const QString date_human = human_date_for_app(this, date_str);
        if (!date_human.isEmpty()) {
            tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Data")), date_human);
        }
    }

    if (!time_str.isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Horário")), time_str);
    }

    if (!file_path.isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Arquivo")), file_path);
    }

    if (!description.isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Descrição")), description);
    }

    if (priority.isValid() && !priority.toString().trimmed().isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2")
                             .arg(get_text(QStringLiteral("Prioridade")),
                                  source::InterfaceCore::prioridade_para_texto(
                                      priority, const_cast<EisenhowerMatrixApp *>(this)));
    }
    if (domain.isValid() && !domain.toString().trimmed().isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2")
                             .arg(get_text(QStringLiteral("Nível de Domínio")),
                                  source::InterfaceCore::dominio_para_texto(domain, false));
    }

    double duration = 1.0;
    if (data.contains(QStringLiteral("duracao")) && data.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        duration = data.value(QStringLiteral("duracao")).toDouble();
    } else if (data.contains(QStringLiteral("duration")) && data.value(QStringLiteral("duration")).toDouble() > 0.0) {
        duration = data.value(QStringLiteral("duration")).toDouble();
    }
    tooltip_lines << format_duration_tooltip_line(duration);

    const int pom = data.value(QStringLiteral("pomodoros_completados"), data.value(QStringLiteral("pomodoros"), 0)).toInt();
    const int def_c = default_pomodoro_cycle_count();
    const int c_tot = data.value(QStringLiteral("ciclos_totais"), data.value(QStringLiteral("ciclos_permitidos"), data.value(QStringLiteral("total_ciclos"), def_c))).toInt();
    const int safe_c_tot = c_tot > 0 ? c_tot : def_c;
    const bool is_done = data.value(QStringLiteral("concluida")).toBool() || data.value(QStringLiteral("status")).toString() == QStringLiteral("Done");
    if (pom > 0 || safe_c_tot > 0) {
        const double pom_pct = source::utils::calculate_task_completion_percentage(pom, safe_c_tot, is_done);
        const QString pom_line_fmt = get_text(QStringLiteral("Ciclos Pomodoro: %1/%2 (%3%)"));
        tooltip_lines << pom_line_fmt
                             .arg(QString::number(pom),
                                  QString::number(safe_c_tot),
                                  QString::number(pom_pct, 'f', 0));
    }



    item->setText(display_text);
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item->setToolTip(tooltip_lines.join(QLatin1Char('\n')));

    source::utils::apply_task_title_font(item, base_text, !file_path.isEmpty());
    if (text_color.isValid()) {
        item->setForeground(QBrush(text_color));
    } else {
        item->setForeground(QBrush());
    }
}

QListWidgetItem *EisenhowerMatrixApp::_create_task_item(const QVariantMap &data, Qt::CheckState check_state) const
{
    auto *item = new QListWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(check_state);
    item->setData(Qt::UserRole, data);
    _apply_task_item_appearance(item);
    return item;
}

bool EisenhowerMatrixApp::_task_sort_less(const QVariantMap &left, const QVariantMap &right) const
{
    const QLocale locale = date_input != nullptr ? date_input->locale() : QLocale::system();
    QCollator collator(locale);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);

    const QString left_text = normalized_text(left.value(QStringLiteral("text")).toString());
    const QString right_text = normalized_text(right.value(QStringLiteral("text")).toString());
    const QDate left_date = task_date_value(left);
    const QDate right_date = task_date_value(right);
    const QTime left_time = task_time_value(left);
    const QTime right_time = task_time_value(right);
    const bool left_has_date = left_date.isValid();
    const bool right_has_date = right_date.isValid();
    const bool left_has_time = left_time.isValid();
    const bool right_has_time = right_time.isValid();

    const auto compare_text = [&collator, &left_text, &right_text]() { return collator.compare(left_text, right_text); };

    const auto compare_dates = [](const QDate &lhs,
                                  bool lhs_has_date,
                                  const QDate &rhs,
                                  bool rhs_has_date,
                                  Qt::SortOrder order) {
        if (lhs_has_date != rhs_has_date) {
            return lhs_has_date ? -1 : 1;
        }
        if (!lhs_has_date || lhs == rhs) {
            return 0;
        }
        if (order == Qt::AscendingOrder) {
            return lhs < rhs ? -1 : 1;
        }
        return lhs > rhs ? -1 : 1;
    };

    const auto compare_times = [](const QTime &lhs,
                                  bool lhs_has_time,
                                  const QTime &rhs,
                                  bool rhs_has_time,
                                  Qt::SortOrder order) {
        if (lhs_has_time != rhs_has_time) {
            return lhs_has_time ? -1 : 1;
        }
        if (!lhs_has_time || lhs == rhs) {
            return 0;
        }
        if (order == Qt::AscendingOrder) {
            return lhs < rhs ? -1 : 1;
        }
        return lhs > rhs ? -1 : 1;
    };

    int result = 0;
    for (int i = 0; i < task_sort_criteria_.size(); ++i) {
        const TaskSortCriterionState &criterion_state = task_sort_criteria_.at(i);
        switch (criterion_state.criterion) {
        case TaskSortCriterion::Date:
            result = compare_dates(left_date, left_has_date, right_date, right_has_date, criterion_state.order);
            break;
        case TaskSortCriterion::Alphabetical: {
            const int text_cmp = compare_text();
            if (text_cmp == 0) {
                result = 0;
            } else if (criterion_state.order == Qt::AscendingOrder) {
                result = text_cmp < 0 ? -1 : 1;
            } else {
                result = text_cmp > 0 ? -1 : 1;
            }
            break;
        }
        case TaskSortCriterion::Time:
            if (i > 0 && left_has_date && right_has_date && left_date != right_date) {
                result = 0;
            } else {
                result = compare_times(left_time, left_has_time, right_time, right_has_time, criterion_state.order);
            }
            break;
        }

        if (result != 0) {
            return result < 0;
        }
    }

    return _task_insertion_order(left) < _task_insertion_order(right);
}

void EisenhowerMatrixApp::_insert_task_item_by_time_group(QListWidget *list, QListWidgetItem *item)
{
    if (list == nullptr || item == nullptr) {
        return;
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString time_str = data.value(QStringLiteral("time")).toString().trimmed();

    if (time_str.isEmpty()) {
        list->addItem(item);
        return;
    }

    if (list->count() == 1 && !(list->item(0)->flags() & Qt::ItemIsSelectable)) {
        list->clear();
    }

    const QString label = _time_group_label(data);
    const QString group_key = _time_group_key(data);
    int header_index = -1;
    int insert_header_index = list->count();
    QList<QPair<int, QString>> existing_headers;

    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *current = list->item(i);
        if (current != nullptr && _is_group_header(current)) {
            existing_headers.append({i, current->data(Qt::UserRole + 2).toString()});
        }
    }

    for (const auto &header : existing_headers) {
        if (header.second == group_key) {
            header_index = header.first;
            break;
        }
    }

    if (header_index < 0) {
        for (const auto &header : existing_headers) {
            if (group_key < header.second) {
                insert_header_index = header.first;
                break;
            }
        }

        auto *header_item = new QListWidgetItem(label);
        header_item->setFlags((header_item->flags() & ~Qt::ItemIsSelectable) & ~Qt::ItemIsEnabled);
        header_item->setData(Qt::UserRole + 1, QStringLiteral("group_header"));
        header_item->setData(Qt::UserRole + 2, group_key);
        list->insertItem(insert_header_index, header_item);
        header_index = insert_header_index;
    }

    int start = header_index + 1;
    int end = list->count();
    for (int i = start; i < list->count(); ++i) {
        QListWidgetItem *current = list->item(i);
        if (current != nullptr && _is_group_header(current)) {
            end = i;
            break;
        }
    }

    QString new_text = data.value(QStringLiteral("text")).toString().trimmed();
    if (new_text.isEmpty()) {
        new_text = base_task_text(item);
    }
    int position = end;
    if (task_sort_criteria_.isEmpty()) {
        const auto new_key = qMakePair(_time_key(time_str), new_text.toLower());
        for (int i = start; i < end; ++i) {
            QListWidgetItem *current = list->item(i);
            if (current == nullptr || _is_group_header(current)) {
                continue;
            }

            const QVariantMap current_data = current->data(Qt::UserRole).toMap();
            QString current_text = current_data.value(QStringLiteral("text")).toString().trimmed();
            if (current_text.isEmpty()) {
                current_text = base_task_text(current);
            }
            const auto current_key = qMakePair(_time_key(current_data.value(QStringLiteral("time")).toString().trimmed()),
                                               current_text.toLower());
            if (new_key < current_key) {
                position = i;
                break;
            }
        }
    }

    list->insertItem(position, item);
}

void EisenhowerMatrixApp::_sort_task_list(QListWidget *list)
{
    if (list == nullptr) {
        return;
    }

    struct TaskEntry {
        QVariantMap data;
        Qt::CheckState check_state = Qt::Unchecked;
    };

    QList<TaskEntry> entries;
    entries.reserve(list->count());

    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *current = list->item(i);
        if (current == nullptr || !(current->flags() & Qt::ItemIsSelectable) || _is_group_header(current)) {
            continue;
        }

        QVariantMap data = current->data(Qt::UserRole).toMap();
        QString base_text = data.value(QStringLiteral("text")).toString().trimmed();
        if (base_text.isEmpty()) {
            base_text = base_task_text(current);
        }
        if (base_text.isEmpty()) {
            continue;
        }

        _ensure_task_metadata(data);
        data.insert(QStringLiteral("text"), base_text);
        current->setData(Qt::UserRole, data);
        entries.append({data, current->checkState()});
    }

    const QSignalBlocker blocker(list);
    list->clear();

    if (entries.isEmpty()) {
        add_placeholder(list, placeholder_text_for_list(this, list));
        return;
    }

    std::stable_sort(entries.begin(),
                     entries.end(),
                     [this](const TaskEntry &left, const TaskEntry &right) {
                         return _task_sort_less(left.data, right.data);
                     });

    auto add_group_header = [this, list](const QVariantMap &data) {
        auto *header_item = new QListWidgetItem(_time_group_label(data));
        header_item->setFlags((header_item->flags() & ~Qt::ItemIsSelectable) & ~Qt::ItemIsEnabled);
        header_item->setData(Qt::UserRole + 1, QStringLiteral("group_header"));
        header_item->setData(Qt::UserRole + 2, _time_group_key(data));
        list->addItem(header_item);
    };

    QString last_group_key;
    for (const TaskEntry &entry : entries) {
        QListWidgetItem *new_item = _create_task_item(entry.data, entry.check_state);
        const QString group_key = _time_group_key(entry.data);
        if (!group_key.isEmpty() && group_key != last_group_key) {
            add_group_header(entry.data);
            last_group_key = group_key;
        } else if (group_key.isEmpty()) {
            last_group_key.clear();
        }

        list->addItem(new_item);
    }

    cleanup_time_groups(list);
}

void EisenhowerMatrixApp::insert_task_into_quadrant_list(QListWidget *list, QListWidgetItem *item)
{
    if (list == nullptr || item == nullptr) {
        return;
    }

    if (list->count() == 1 && !(list->item(0)->flags() & Qt::ItemIsSelectable)) {
        list->clear();
    }

    QVariantMap data = item->data(Qt::UserRole).toMap();
    _ensure_task_metadata(data);
    item->setData(Qt::UserRole, data);
    _apply_task_item_appearance(item);
    list->addItem(item);
    _sort_task_list(list);
}

void EisenhowerMatrixApp::cleanup_time_groups(QListWidget *list)
{
    int i = 0;
    while (list != nullptr && i < list->count()) {
        QListWidgetItem *item = list->item(i);
        if (item != nullptr && _is_group_header(item)) {
            const bool empty_after =
                (i + 1 >= list->count()) || _is_group_header(list->item(i + 1))
                || ((list->item(i + 1)->flags() & Qt::ItemIsSelectable) == 0);

            if (empty_after) {
                int j = i + 1;
                bool found_task = false;
                while (j < list->count()) {
                    QListWidgetItem *next = list->item(j);
                    if (_is_group_header(next)) {
                        break;
                    }
                    if (next != nullptr && (next->flags() & Qt::ItemIsSelectable)) {
                        found_task = true;
                        break;
                    }
                    ++j;
                }

                if (!found_task) {
                    delete list->takeItem(i);
                    continue;
                }
            }
        }

        ++i;
    }
}

void EisenhowerMatrixApp::add_task()
{
    source::GerenciamentoUI::add_task(this);
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }
}

void EisenhowerMatrixApp::handle_item_checked(QListWidgetItem *item, QListWidget *source_list, QListWidget *target_list)
{
    source::GerenciamentoUI::handle_item_checked(this, item, source_list, target_list);
    if (item != nullptr && gantt_plugin_ != nullptr) {
        const QVariantMap data = item->data(Qt::UserRole).toMap();
        const QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
        if (!task_id.isEmpty()) {
            const bool completed = (item->checkState() == Qt::Checked) || is_completed_list(target_list);
            if (completed) {
                gantt_plugin_->on_eisenhower_task_completed(task_id, true);
            } else if (is_in_progress_list(target_list)) {
                gantt_plugin_->on_eisenhower_task_status_changed(task_id, QStringLiteral("IN_PROGRESS"));
            } else {
                gantt_plugin_->on_eisenhower_task_completed(task_id, false);
            }
        }
    }
}

void EisenhowerMatrixApp::move_item_between_lists(QListWidgetItem *item,
                                                  QListWidget *source,
                                                  QListWidget *target,
                                                  Qt::CheckState new_check_state)
{
    source::GerenciamentoUI::move_item_between_lists(this, item, source, target, new_check_state);
    if (item != nullptr && gantt_plugin_ != nullptr) {
        const QVariantMap data = item->data(Qt::UserRole).toMap();
        const QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
        if (!task_id.isEmpty()) {
            const bool completed = (new_check_state == Qt::Checked) || is_completed_list(target);
            if (completed) {
                gantt_plugin_->on_eisenhower_task_completed(task_id, true);
            } else if (is_in_progress_list(target)) {
                gantt_plugin_->on_eisenhower_task_status_changed(task_id, QStringLiteral("IN_PROGRESS"));
            } else {
                gantt_plugin_->on_eisenhower_task_completed(task_id, false);
            }
        }
    }
}

bool EisenhowerMatrixApp::remove_task(QListWidgetItem *item, QListWidget *list_widget, bool confirm)
{
    QString task_id;
    if (item != nullptr) {
        task_id = item->data(Qt::UserRole).toMap().value(QStringLiteral("id")).toString().trimmed();
    }
    const bool removed = source::GerenciamentoUI::remove_task(this, item, list_widget, confirm);
    cleanup_time_groups(list_widget);
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }
    if (removed && !task_id.isEmpty() && gantt_plugin_ != nullptr) {
        gantt_plugin_->on_eisenhower_task_removed(task_id);
    }
    return removed;
}

void EisenhowerMatrixApp::save_tasks()
{
    const QString origin =
        pending_shared_origin_.trimmed().isEmpty() ? QStringLiteral("eisenhower")
                                                   : pending_shared_origin_.trimmed().toCaseFolded();
    pending_shared_origin_.clear();
    pending_save_origins_.append(origin);
    if (task_save_flush_scheduled_) {
        return;
    }

    task_save_flush_scheduled_ = true;
    QTimer::singleShot(250, this, [this]() { _flush_debounced_task_save(); });
}

void EisenhowerMatrixApp::save_tasks_now()
{
    const QString origin =
        pending_shared_origin_.trimmed().isEmpty() ? QStringLiteral("eisenhower")
                                                   : pending_shared_origin_.trimmed().toCaseFolded();
    pending_shared_origin_.clear();

    QStringList origins = pending_save_origins_;
    pending_save_origins_.clear();
    task_save_flush_scheduled_ = false;
    origins.append(origin);
    _perform_task_save(origins);
}

void EisenhowerMatrixApp::_flush_debounced_task_save()
{
    task_save_flush_scheduled_ = false;
    QStringList origins = pending_save_origins_;
    pending_save_origins_.clear();
    if (origins.isEmpty()) {
        origins.append(QStringLiteral("eisenhower"));
    }
    _perform_task_save(origins);
}

void EisenhowerMatrixApp::_perform_task_save(const QStringList &origins)
{
    source::GerenciamentoUI::save_tasks(this);
    QStringList normalized_origins;
    for (const QString &origin : origins) {
        const QString normalized =
            origin.trimmed().isEmpty() ? QStringLiteral("eisenhower") : origin.trimmed().toCaseFolded();
        if (!normalized_origins.contains(normalized)) {
            normalized_origins.append(normalized);
        }
    }
    for (const QString &origin : normalized_origins) {
        _queue_runtime_sync(origin);
    }
}

void EisenhowerMatrixApp::_queue_runtime_sync(const QString &origin)
{
    const QString normalized = origin.trimmed().isEmpty() ? QStringLiteral("eisenhower")
                                                          : origin.trimmed().toCaseFolded();
    pending_runtime_sync_origins_.append(normalized);
    if (runtime_sync_flush_scheduled_) {
        return;
    }

    runtime_sync_flush_scheduled_ = true;
    QTimer::singleShot(0, this, [this]() { _flush_runtime_sync(); });
}

void EisenhowerMatrixApp::_flush_runtime_sync()
{
    runtime_sync_flush_scheduled_ = false;
    const QStringList pending = pending_runtime_sync_origins_;
    pending_runtime_sync_origins_.clear();
    if (pending.isEmpty()) {
        return;
    }

    QStringList origins;
    for (const QString &origin : pending) {
        const QString normalized = origin.trimmed().toCaseFolded();
        if (!normalized.isEmpty() && !origins.contains(normalized)) {
            origins.append(normalized);
        }
    }

    const auto canonical_sync_module = [](const QString &module) {
        const QString normalized = module.trimmed().toCaseFolded();
        if (normalized == QStringLiteral("tempo")) {
            return QStringLiteral("pomodoro");
        }
        if (normalized == QStringLiteral("eisenhower") || normalized == QStringLiteral("pomodoro")
            || normalized == QStringLiteral("feynman")) {
            return normalized;
        }
        return QString();
    };

    const auto sync_enabled_for_origin = [this, &canonical_sync_module](const QString &origin) {
        const QString module = canonical_sync_module(origin);
        if (module.isEmpty()) {
            return true;
        }
        if (module == QStringLiteral("eisenhower")) {
            return sync_eisenhower_enabled_;
        }
        if (module == QStringLiteral("pomodoro")) {
            return sync_pomodoro_enabled_;
        }
        if (module == QStringLiteral("feynman")) {
            return sync_feynman_enabled_;
        }
        return false;
    };

    const auto should_reload = [&origins, &canonical_sync_module, &sync_enabled_for_origin](const QString &module) {
        for (const QString &origin : origins) {
            const QString origin_module = canonical_sync_module(origin);
            const QString comparable_origin = origin_module.isEmpty() ? origin : origin_module;
            if (comparable_origin != module && sync_enabled_for_origin(origin)) {
                return true;
            }
        }
        return false;
    };

    if (sync_eisenhower_enabled_ && should_reload(QStringLiteral("eisenhower"))) {
        _reload_eisenhower_from_shared_store();
    }

    if (sync_pomodoro_enabled_ && has_pomodoro_plugin() && should_reload(QStringLiteral("pomodoro"))) {
        _reload_pomodoro_from_shared_store();
    }

    if (sync_feynman_enabled_ && has_feynman_plugin() && should_reload(QStringLiteral("feynman"))) {
        _reload_feynman_from_shared_store();
    }

    if (has_gantt_plugin() && gantt_plugin_ != nullptr) {
        gantt_plugin_->sync_all_tasks();
    }
}

void EisenhowerMatrixApp::_reload_eisenhower_from_shared_store()
{
    load_tasks();
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }
}

void EisenhowerMatrixApp::_reload_pomodoro_from_shared_store()
{
    if (pomodoro_plugin_ != nullptr) {
        pomodoro_plugin_->reload_shared_state();
    }
}

void EisenhowerMatrixApp::_reload_feynman_from_shared_store()
{
    if (feynman_plugin_ != nullptr) {
        feynman_plugin_->reload_shared_state();
    }
}

void EisenhowerMatrixApp::_refresh_plugin_task_views()
{
    if (pomodoro_plugin_ != nullptr) {
        pomodoro_plugin_->refresh_task_views();
    }

    if (feynman_plugin_ != nullptr) {
        feynman_plugin_->refresh_task_views();
    }

    if (mapamental_plugin_ != nullptr) {
        mapamental_plugin_->refresh_task_views();
    }

    if (gantt_plugin_ != nullptr) {
        gantt_plugin_->sync_all_tasks();
    }
}

void EisenhowerMatrixApp::refresh_task_title_styles()
{
    source::InterfaceCore::incore_11_atualizar_itens_tarefas::atualizar_itens_tarefas(this);
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }
    _refresh_plugin_task_views();
}

void EisenhowerMatrixApp::_apply_agenda_color_mode(const QString &mode)
{
    QString normalized_mode;
    QString target_module;

    const QString lower_code = mode.trimmed().toCaseFolded();
    if (lower_code.startsWith(QStringLiteral("preto_"))) {
        normalized_mode = QStringLiteral("preto");
        target_module = lower_code.mid(6);
    } else if (lower_code.startsWith(QStringLiteral("coloridas_"))) {
        normalized_mode = QStringLiteral("coloridas");
        target_module = lower_code.mid(10);
    } else if (lower_code == QStringLiteral("preto") || lower_code == QStringLiteral("coloridas")) {
        normalized_mode = lower_code;
        target_module = QStringLiteral("geral");
    } else {
        return;
    }

    const bool update_eisenhower = (target_module == QStringLiteral("eisenhower") || target_module == QStringLiteral("geral"));
    const bool update_pomodoro = (target_module == QStringLiteral("pomodoro") || target_module == QStringLiteral("geral"));
    const bool update_feynman = (target_module == QStringLiteral("feynman") || target_module == QStringLiteral("geral"));

    if (update_eisenhower) {
        agenda_color_mode_ = normalized_mode;
    }

    const QColor window_color = palette().color(QPalette::Window);
    const double luminance =
        (0.2126 * window_color.red() + 0.7152 * window_color.green() + 0.0722 * window_color.blue()) / 255.0;
    const QString system_text_color =
        (luminance >= 0.5 ? QColor(Qt::black) : QColor(Qt::white)).name(QColor::HexRgb);

    const auto normalized_color_name = [](const QString &value) {
        const QColor color(value.trimmed());
        return color.isValid() ? color.name(QColor::HexRgb).toLower() : QString();
    };

    const auto agenda_description_values_for_color = [](const QVariantMap &data) {
        QStringList values;
        const QString description = data.value(QStringLiteral("description")).toString();
        const QStringList labels = {
            QStringLiteral("disciplina:"),
            QStringLiteral("subject:"),
        };

        for (const QString &line : description.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            const QString trimmed = line.trimmed();
            const QString folded = trimmed.toCaseFolded();
            for (const QString &label : labels) {
                if (folded.startsWith(label)) {
                    const int colon_index = trimmed.indexOf(QLatin1Char(':'));
                    if (colon_index >= 0) {
                        const QString value = trimmed.mid(colon_index + 1).trimmed();
                        if (!value.isEmpty()) {
                            values << value;
                        }
                    }
                    break;
                }
            }
        }

        return values;
    };

    const auto agenda_defined_color = [this, &normalized_color_name, &agenda_description_values_for_color](
                                          const QVariantMap &data) {
        const auto color_for_candidate = [this, &normalized_color_name](const QString &candidate) {
            if (agenda_host_bridge_ == nullptr) {
                return QString();
            }
            const QString color = agenda_host_bridge_->resolve_color_for_title(candidate.trimmed());
            return normalized_color_name(color);
        };

        const QStringList title_values = {
            data.value(QStringLiteral("titulo")).toString(),
            data.value(QStringLiteral("text")).toString(),
        };
        QStringList candidates;
        candidates << agenda_description_values_for_color(data) << title_values;

        for (const QString &title : title_values) {
            const QStringList parts = title.split(QStringLiteral(" - "), Qt::SkipEmptyParts);
            for (const QString &part : parts) {
                candidates << part;
            }
        }

        for (const QString &candidate : candidates) {
            const QString full_title_color = color_for_candidate(candidate);
            if (!full_title_color.isEmpty()) {
                return full_title_color;
            }
        }

        return QString();
    };

    const auto has_legacy_agenda_marker = [](const QVariantMap &data) {
        const QString description = data.value(QStringLiteral("description")).toString().toCaseFolded();
        if (description.trimmed().isEmpty()) {
            return false;
        }
        if (description.contains(QStringLiteral("agenda:"))) {
            return true;
        }

        int marker_count = 0;
        const QStringList markers = {
            QStringLiteral("disciplina:"),
            QStringLiteral("subject:"),
            QStringLiteral("turma da disciplina:"),
            QStringLiteral("tipo de atividade avaliativa:"),
            QStringLiteral("type of assessment activity:"),
            QStringLiteral("sequência da atividade:"),
            QStringLiteral("activity sequence:"),
            QStringLiteral("ementa:"),
            QStringLiteral("syllabus:"),
            QStringLiteral("semestre:"),
            QStringLiteral("semester:"),
        };
        for (const QString &marker : markers) {
            if (description.contains(marker)) {
                ++marker_count;
            }
        }

        return marker_count >= 2
               || description.contains(QStringLiteral("tipo de atividade avaliativa:"))
               || description.contains(QStringLiteral("type of assessment activity:"))
               || description.contains(QStringLiteral("sequência da atividade:"))
               || description.contains(QStringLiteral("activity sequence:"));
    };

    const auto legacy_agenda_fallback_color = [&normalized_color_name, &agenda_description_values_for_color](const QVariantMap &data) {
        static const QStringList palette = {
            QStringLiteral("#E53935"),
            QStringLiteral("#8E24AA"),
            QStringLiteral("#3949AB"),
            QStringLiteral("#039BE5"),
            QStringLiteral("#00897B"),
            QStringLiteral("#43A047"),
            QStringLiteral("#FDD835"),
            QStringLiteral("#FB8C00"),
            QStringLiteral("#6D4C41"),
            QStringLiteral("#546E7A"),
        };

        QString seed;
        const QStringList discipline_candidates = agenda_description_values_for_color(data);
        if (!discipline_candidates.isEmpty()) {
            seed = discipline_candidates.first().trimmed().toCaseFolded();
        } else {
            const QString title = data.value(QStringLiteral("titulo"), data.value(QStringLiteral("text"))).toString().trimmed();
            const QStringList parts = title.split(QStringLiteral(" - "), Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                seed = parts.first().trimmed().toCaseFolded();
            } else {
                seed = title.toCaseFolded();
            }
        }

        quint32 hash = 2166136261u;
        for (const QChar ch : seed) {
            hash ^= static_cast<quint32>(ch.unicode());
            hash *= 16777619u;
        }

        return normalized_color_name(palette.at(static_cast<int>(hash % static_cast<quint32>(palette.size()))));
    };

    const auto is_system_text_color = [&normalized_color_name](const QString &value) {
        const QString color = normalized_color_name(value);
        return color == QStringLiteral("#000000") || color == QStringLiteral("#ffffff");
    };

    bool changed = false;

    if (update_eisenhower) {
        for (QListWidget *list : all_task_lists(this)) {
            if (list == nullptr) {
                continue;
            }

            for (int row = 0; row < list->count(); ++row) {
                QListWidgetItem *item = list->item(row);
                if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || _is_group_header(item)) {
                    continue;
                }

                QVariantMap data = item->data(Qt::UserRole).toMap();
                const QString origin =
                    data.value(QStringLiteral("integracao_origem"), data.value(QStringLiteral("origin")))
                        .toString()
                        .trimmed()
                        .toCaseFolded();
                const bool has_agenda_color_metadata =
                    !data.value(QStringLiteral("cor_texto_original"), data.value(QStringLiteral("agenda_cor_texto")))
                         .toString()
                         .trimmed()
                         .isEmpty();
                const bool has_legacy_marker = has_legacy_agenda_marker(data);
                if (origin == QStringLiteral("eisenhower") && !has_legacy_marker) {
                    continue;
                }
                QString agenda_color_from_definition;
                const bool is_likely_agenda = (origin == QStringLiteral("agenda")) || has_legacy_marker || has_agenda_color_metadata;
                if (is_likely_agenda || origin.isEmpty()) {
                    agenda_color_from_definition = agenda_defined_color(data);
                }
                const bool is_legacy_agenda_task =
                    (origin.isEmpty()
                     && (!agenda_color_from_definition.isEmpty() || has_legacy_marker || has_agenda_color_metadata))
                    || (origin == QStringLiteral("eisenhower") && has_legacy_marker);
                if (origin != QStringLiteral("agenda") && !is_legacy_agenda_task) {
                    continue;
                }

                const QVariantMap before = data;
                if (origin != QStringLiteral("agenda")) {
                    data.insert(QStringLiteral("integracao_origem"), QStringLiteral("agenda"));
                }

                QString original_color =
                    data.value(QStringLiteral("cor_texto_original"), data.value(QStringLiteral("agenda_cor_texto")))
                        .toString()
                        .trimmed();
                const QString current_color = data.value(QStringLiteral("cor_texto")).toString().trimmed();

                if (original_color.isEmpty()) {
                    original_color = agenda_color_from_definition;
                }
                if (original_color.isEmpty() && !current_color.isEmpty() && !is_system_text_color(current_color)) {
                    original_color = normalized_color_name(current_color);
                }
                if (original_color.isEmpty()) {
                    original_color = legacy_agenda_fallback_color(data);
                }

                QString custom_color = data.value(QStringLiteral("cor_texto_customizada")).toString().trimmed();
                if (custom_color.isEmpty() && !current_color.isEmpty() && !is_system_text_color(current_color)) {
                    const QString norm_current = normalized_color_name(current_color);
                    const QString norm_original = normalized_color_name(original_color);
                    if (!norm_current.isEmpty() && !norm_original.isEmpty() && norm_current != norm_original) {
                        custom_color = norm_current;
                    }
                }

                const QString normalized_original = normalized_color_name(original_color);
                const QString normalized_custom = normalized_color_name(custom_color);

                if (normalized_mode == QStringLiteral("preto")) {
                    if (!normalized_original.isEmpty()) {
                        data.insert(QStringLiteral("cor_texto_original"), normalized_original);
                        data.insert(QStringLiteral("agenda_cor_texto"), normalized_original);
                    }
                    if (!normalized_custom.isEmpty()) {
                        data.insert(QStringLiteral("cor_texto_customizada"), normalized_custom);
                    }
                    data.insert(QStringLiteral("cor_texto"), system_text_color);
                    data.insert(QStringLiteral("task_color"), system_text_color);
                } else {
                    if (!normalized_original.isEmpty()) {
                        data.insert(QStringLiteral("cor_texto_original"), normalized_original);
                        data.insert(QStringLiteral("agenda_cor_texto"), normalized_original);
                    }
                    const QString color_to_apply = !normalized_custom.isEmpty() ? normalized_custom : normalized_original;
                    if (!color_to_apply.isEmpty()) {
                        data.insert(QStringLiteral("cor_texto"), color_to_apply);
                        data.insert(QStringLiteral("task_color"), color_to_apply);
                        if (!normalized_custom.isEmpty()) {
                            data.insert(QStringLiteral("cor_texto_customizada"), normalized_custom);
                        }
                    } else {
                        data.remove(QStringLiteral("cor_texto"));
                        data.remove(QStringLiteral("task_color"));
                    }
                }

                if (data != before) {
                    item->setData(Qt::UserRole, data);
                    _apply_task_item_appearance(item);
                    changed = true;
                }
            }
        }
    }

    if (update_pomodoro || update_feynman) {
        QVariantList records = source::utils::UnifiedTaskStore::load_records(tasks_path);
        bool store_changed = false;
        for (int i = 0; i < records.size(); ++i) {
            QVariantMap rec = records.at(i).toMap();
            const QString mod = rec.value(QStringLiteral("modulo")).toString().trimmed().toLower();
            if ((mod == QStringLiteral("pomodoro") && update_pomodoro) ||
                (mod == QStringLiteral("feynman") && update_feynman)) {
                const QString origin = rec.value(QStringLiteral("integracao_origem"), rec.value(QStringLiteral("origin"))).toString().trimmed().toLower();
                const bool has_agenda_color_meta = !rec.value(QStringLiteral("cor_texto_original"), rec.value(QStringLiteral("agenda_cor_texto"))).toString().trimmed().isEmpty();
                const bool has_legacy_mkr = has_legacy_agenda_marker(rec);
                if (origin == mod && !has_legacy_mkr) {
                    continue;
                }
                QString agenda_color_from_def;
                const bool is_likely_agenda = (origin == QStringLiteral("agenda")) || has_legacy_mkr || has_agenda_color_meta;
                if (is_likely_agenda || origin.isEmpty()) {
                    agenda_color_from_def = agenda_defined_color(rec);
                }
                const bool is_legacy = (origin.isEmpty() && (!agenda_color_from_def.isEmpty() || has_legacy_mkr || has_agenda_color_meta))
                                       || (origin == mod && has_legacy_mkr);
                if (origin != QStringLiteral("agenda") && !is_legacy) {
                    continue;
                }

                const QVariantMap before = rec;

                QString original_color = rec.value(QStringLiteral("cor_texto_original"), rec.value(QStringLiteral("agenda_cor_texto"))).toString().trimmed();
                const QString current_color = rec.value(QStringLiteral("cor_texto")).toString().trimmed();
                if (original_color.isEmpty()) original_color = agenda_color_from_def;
                if (original_color.isEmpty() && !current_color.isEmpty() && !is_system_text_color(current_color)) original_color = normalized_color_name(current_color);
                if (original_color.isEmpty()) original_color = legacy_agenda_fallback_color(rec);

                QString custom_color = rec.value(QStringLiteral("cor_texto_customizada")).toString().trimmed();
                if (custom_color.isEmpty() && !current_color.isEmpty() && !is_system_text_color(current_color)) {
                    const QString norm_curr = normalized_color_name(current_color);
                    const QString norm_orig = normalized_color_name(original_color);
                    if (!norm_curr.isEmpty() && !norm_orig.isEmpty() && norm_curr != norm_orig) {
                        custom_color = norm_curr;
                    }
                }

                const QString norm_orig = normalized_color_name(original_color);
                const QString norm_cust = normalized_color_name(custom_color);

                if (normalized_mode == QStringLiteral("preto")) {
                    if (!norm_orig.isEmpty()) {
                        rec.insert(QStringLiteral("cor_texto_original"), norm_orig);
                        rec.insert(QStringLiteral("agenda_cor_texto"), norm_orig);
                    }
                    if (!norm_cust.isEmpty()) {
                        rec.insert(QStringLiteral("cor_texto_customizada"), norm_cust);
                    }
                    rec.insert(QStringLiteral("cor_texto"), system_text_color);
                    rec.insert(QStringLiteral("task_color"), system_text_color);
                } else {
                    if (!norm_orig.isEmpty()) {
                        rec.insert(QStringLiteral("cor_texto_original"), norm_orig);
                        rec.insert(QStringLiteral("agenda_cor_texto"), norm_orig);
                    }
                    const QString color_to_apply = !norm_cust.isEmpty() ? norm_cust : norm_orig;
                    if (!color_to_apply.isEmpty()) {
                        rec.insert(QStringLiteral("cor_texto"), color_to_apply);
                        rec.insert(QStringLiteral("task_color"), color_to_apply);
                        if (!norm_cust.isEmpty()) {
                            rec.insert(QStringLiteral("cor_texto_customizada"), norm_cust);
                        }
                    } else {
                        rec.remove(QStringLiteral("cor_texto"));
                        rec.remove(QStringLiteral("task_color"));
                    }
                }
                if (rec != before) {
                    records[i] = rec;
                    store_changed = true;
                }
            }
        }
        if (store_changed) {
            source::utils::UnifiedTaskStore::save_records(tasks_path, records);
            if (update_pomodoro && pomodoro_plugin_ != nullptr) {
                pomodoro_plugin_->reload_shared_state();
            }
            if (update_feynman && feynman_plugin_ != nullptr) {
                feynman_plugin_->reload_shared_state();
            }
        }
    }

    if (changed) {
        pending_shared_origin_ = QStringLiteral("agenda");
        save_tasks_now();
        _flush_runtime_sync();

        if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
            calendar_pane->calendar_panel->update_task_list();
        }
    }
}

int EisenhowerMatrixApp::selected_plugin_task_count() const
{
    int count = 0;
    if (pomodoro_plugin_ != nullptr) {
        count += pomodoro_plugin_->selected_task_count();
    }
    if (feynman_plugin_ != nullptr) {
        count += feynman_plugin_->selected_task_count();
    }
    return count;
}

QString EisenhowerMatrixApp::selected_plugin_task_color() const
{
    if (pomodoro_plugin_ != nullptr) {
        const QString color = pomodoro_plugin_->selected_task_color().trimmed();
        if (QColor(color).isValid()) {
            return color;
        }
    }

    if (feynman_plugin_ != nullptr) {
        const QString color = feynman_plugin_->selected_task_color().trimmed();
        if (QColor(color).isValid()) {
            return color;
        }
    }

    return {};
}

int EisenhowerMatrixApp::apply_color_to_selected_plugin_tasks(const QString &color)
{
    int updated = 0;
    if (pomodoro_plugin_ != nullptr) {
        updated += pomodoro_plugin_->apply_color_to_selected_tasks(color);
    }
    if (feynman_plugin_ != nullptr) {
        updated += feynman_plugin_->apply_color_to_selected_tasks(color);
    }
    return updated;
}

void EisenhowerMatrixApp::clear_pomodoro_custom_font_settings(bool target_linked)
{
    if (pomodoro_plugin_ != nullptr) {
        pomodoro_plugin_->reset_task_font_overrides(target_linked);
    }
}

void EisenhowerMatrixApp::clear_feynman_custom_font_settings(bool target_linked)
{
    if (feynman_plugin_ != nullptr) {
        feynman_plugin_->reset_task_font_overrides(target_linked);
    }
}

void EisenhowerMatrixApp::clear_mapamental_custom_font_settings(bool target_linked)
{
    if (mapamental_plugin_ != nullptr) {
        mapamental_plugin_->reset_task_font_overrides(target_linked);
    }
}

QVariantList EisenhowerMatrixApp::get_pomodoro_task_font_overrides(bool target_linked) const
{
    return pomodoro_plugin_ != nullptr ? pomodoro_plugin_->get_task_font_overrides(target_linked) : QVariantList();
}

QVariantList EisenhowerMatrixApp::get_feynman_task_font_overrides(bool target_linked) const
{
    return feynman_plugin_ != nullptr ? feynman_plugin_->get_task_font_overrides(target_linked) : QVariantList();
}

QVariantList EisenhowerMatrixApp::get_mapamental_task_font_overrides(bool target_linked) const
{
    return mapamental_plugin_ != nullptr ? mapamental_plugin_->get_task_font_overrides(target_linked) : QVariantList();
}

void EisenhowerMatrixApp::load_tasks()
{
    source::GerenciamentoUI::load_tasks(this);
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }
}

}  // namespace source
