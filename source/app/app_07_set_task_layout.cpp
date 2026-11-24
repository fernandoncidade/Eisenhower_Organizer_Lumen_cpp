#include "source/app/app_07_set_task_layout.hpp"

#include <QGridLayout>
#include <QScrollArea>
#include <QVBoxLayout>

namespace source {
using namespace app;

namespace {

constexpr int kSplitTaskPanelBottomInset = 0;
constexpr int kNativeTaskPanelBottomInset = 4;
constexpr int kMatrixTaskPanelBottomInset = 4;
constexpr int kMatrixStatusPageBottomInset = 0;

QString quadrant_title(source::EisenhowerMatrixApp *app, int index)
{
    const QList<QLabel *> labels = {
        app != nullptr ? app->quadrant1_label : nullptr,
        app != nullptr ? app->quadrant2_label : nullptr,
        app != nullptr ? app->quadrant3_label : nullptr,
        app != nullptr ? app->quadrant4_label : nullptr,
    };
    const QList<QString> fallback = {
        get_text(QStringLiteral("🔴 Importante e Urgente")),
        get_text(QStringLiteral("🟠 Importante, mas Não Urgente")),
        get_text(QStringLiteral("🟡 Não Importante, mas Urgente")),
        get_text(QStringLiteral("🟢 Não Importante e Não Urgente")),
    };

    if (index >= 0 && index < labels.size() && labels.at(index) != nullptr) {
        return labels.at(index)->text();
    }
    return fallback.value(index);
}

QString pending_title(source::EisenhowerMatrixApp *app, int index)
{
    Q_UNUSED(app);
    const QList<QString> fallback = {
        get_text(QStringLiteral("🔴 Pendentes")),
        get_text(QStringLiteral("🟠 Pendentes")),
        get_text(QStringLiteral("🟡 Pendentes")),
        get_text(QStringLiteral("🟢 Pendentes")),
    };
    return fallback.value(index);
}

QString completed_title(source::EisenhowerMatrixApp *app, int index)
{
    const QList<QLabel *> labels = {
        app != nullptr ? app->quadrant1_completed_label : nullptr,
        app != nullptr ? app->quadrant2_completed_label : nullptr,
        app != nullptr ? app->quadrant3_completed_label : nullptr,
        app != nullptr ? app->quadrant4_completed_label : nullptr,
    };
    const QList<QString> fallback = {
        get_text(QStringLiteral("🔴 Concluídas")),
        get_text(QStringLiteral("🟠 Concluídas")),
        get_text(QStringLiteral("🟡 Concluídas")),
        get_text(QStringLiteral("🟢 Concluídas")),
    };

    if (index >= 0 && index < labels.size() && labels.at(index) != nullptr) {
        return labels.at(index)->text();
    }
    return fallback.value(index);
}

void set_group_layout_bottom_inset(QGroupBox *group, int bottom_inset)
{
    if (group == nullptr || group->layout() == nullptr) {
        return;
    }

    QMargins margins = group->layout()->contentsMargins();
    margins.setBottom(bottom_inset);
    group->layout()->setContentsMargins(margins);
}

void prepare_split_group(QGroupBox *group, QListWidget *list, const QString &title, bool use_native_frame, bool horizontal_layout = false)
{
    if (group == nullptr) {
        return;
    }

    group->setTitle(title);
    group->setCheckable(true);
    group->setFlat(!use_native_frame);
    set_group_layout_bottom_inset(group, use_native_frame ? kNativeTaskPanelBottomInset : kSplitTaskPanelBottomInset);
    QSizePolicy policy = group->sizePolicy();
    if (horizontal_layout) {
        policy.setHorizontalPolicy(group->isChecked() ? QSizePolicy::Expanding : QSizePolicy::Fixed);
        policy.setVerticalPolicy(QSizePolicy::Expanding);
        if (!group->isChecked()) {
            const int collapsed_w = group->minimumSizeHint().width();
            group->setMinimumWidth(collapsed_w);
            group->setMaximumWidth(collapsed_w);
            group->setCursor(Qt::PointingHandCursor);
        } else {
            group->setMinimumWidth(0);
            group->setMaximumWidth(QWIDGETSIZE_MAX);
            group->unsetCursor();
        }
    } else {
        policy.setHorizontalPolicy(QSizePolicy::Expanding);
        policy.setVerticalPolicy(group->isChecked() ? QSizePolicy::Expanding : QSizePolicy::Fixed);
        group->setMinimumWidth(0);
        group->setMaximumWidth(QWIDGETSIZE_MAX);
        group->unsetCursor();
    }
    group->setSizePolicy(policy);
    group->show();
    if (list != nullptr) {
        list->setVisible(group->isChecked());
    }
    if (group->layout() != nullptr) {
        group->layout()->invalidate();
        group->layout()->activate();
    }
    group->updateGeometry();
    group->update();
}

void prepare_matrix_group(QGroupBox *group, QListWidget *list, const QString &title)
{
    if (group == nullptr) {
        return;
    }

    group->setTitle(title);
    group->setChecked(true);
    group->setCheckable(false);
    group->setFlat(false);
    group->setMaximumWidth(QWIDGETSIZE_MAX);
    group->setMinimumWidth(0);
    group->unsetCursor();
    set_group_layout_bottom_inset(group, kMatrixTaskPanelBottomInset);
    group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    group->show();
    if (list != nullptr) {
        list->setVisible(true);
        list->show();
    }
    if (group->layout() != nullptr) {
        group->layout()->invalidate();
        group->layout()->activate();
    }
    group->updateGeometry();
    group->update();
}

bool attach_split_panel(QWidget *panel,
                        QGroupBox *pending_group,
                        QListWidget *pending_list,
                        QGroupBox *in_progress_group,
                        QListWidget *in_progress_list,
                        QGroupBox *completed_group,
                        QListWidget *completed_list,
                        const QString &pending_title,
                        const QString &in_progress_title,
                        const QString &completed_title_text,
                        bool use_native_frame,
                        bool horizontal_layout = false)
{
    auto *layout = panel != nullptr ? dynamic_cast<QBoxLayout *>(panel->layout()) : nullptr;
    if (layout == nullptr || pending_group == nullptr || in_progress_group == nullptr || completed_group == nullptr) {
        return false;
    }

    clear_layout(layout);
    layout->setDirection(horizontal_layout ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    layout->setSpacing(horizontal_layout ? 8 : 6);
    pending_group->setParent(panel);
    in_progress_group->setParent(panel);
    completed_group->setParent(panel);
    prepare_split_group(pending_group, pending_list, pending_title, use_native_frame, horizontal_layout);
    prepare_split_group(in_progress_group, in_progress_list, in_progress_title, use_native_frame, horizontal_layout);
    prepare_split_group(completed_group, completed_list, completed_title_text, use_native_frame, horizontal_layout);
    layout->addWidget(pending_group, 1);
    layout->addWidget(in_progress_group, 1);
    layout->addWidget(completed_group, 1);
    layout->addStretch(1);
    const bool pending_visible = pending_group->isChecked();
    const bool in_progress_visible = in_progress_group->isChecked();
    const bool completed_visible = completed_group->isChecked();
    const bool any_visible = pending_visible || in_progress_visible || completed_visible;
    layout->setStretch(0, pending_visible ? 1 : 0);
    layout->setStretch(1, in_progress_visible ? 1 : 0);
    layout->setStretch(2, completed_visible ? 1 : 0);
    layout->setStretch(3, any_visible ? 0 : 1);
    layout->invalidate();
    layout->activate();
    panel->show();
    panel->updateGeometry();
    panel->update();
    return true;
}

void clear_status_tabs(QTabWidget *tabs, QGroupBox *pending_group, QGroupBox *in_progress_group, QGroupBox *completed_group)
{
    if (tabs == nullptr) {
        return;
    }

    while (tabs->count() > 0) {
        QWidget *page = tabs->widget(0);
        tabs->removeTab(0);
        if (page == nullptr || page == pending_group || page == in_progress_group || page == completed_group) {
            continue;
        }
        if (pending_group != nullptr && pending_group->parentWidget() == page) {
            pending_group->setParent(nullptr);
        }
        if (in_progress_group != nullptr && in_progress_group->parentWidget() == page) {
            in_progress_group->setParent(nullptr);
        }
        if (completed_group != nullptr && completed_group->parentWidget() == page) {
            completed_group->setParent(nullptr);
        }
        page->deleteLater();
    }
}

void clear_matrix_status_tabs(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    clear_status_tabs(app->quadrant1_status_tabs, app->quadrant1_group, app->quadrant1_in_progress_group, app->quadrant1_completed_group);
    clear_status_tabs(app->quadrant2_status_tabs, app->quadrant2_group, app->quadrant2_in_progress_group, app->quadrant2_completed_group);
    clear_status_tabs(app->quadrant3_status_tabs, app->quadrant3_group, app->quadrant3_in_progress_group, app->quadrant3_completed_group);
    clear_status_tabs(app->quadrant4_status_tabs, app->quadrant4_group, app->quadrant4_in_progress_group, app->quadrant4_completed_group);
}

QWidget *matrix_status_page(QTabWidget *tabs, QGroupBox *group)
{
    if (tabs == nullptr || group == nullptr) {
        return group;
    }

    auto *page = new QWidget(tabs);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, kMatrixStatusPageBottomInset);
    layout->setSpacing(0);
    group->setParent(page);
    layout->addWidget(group, 1);
    layout->activate();
    page->updateGeometry();
    page->update();
    return page;
}

bool attach_status_tabs(QTabWidget *tabs,
                        QGroupBox *pending_group,
                        QListWidget *pending_list,
                        QGroupBox *in_progress_group,
                        QListWidget *in_progress_list,
                        QGroupBox *completed_group,
                        QListWidget *completed_list,
                        const QString &quadrant_title_text,
                        const QString &in_progress_title_text)
{
    if (tabs == nullptr || pending_group == nullptr || in_progress_group == nullptr || completed_group == nullptr) {
        return false;
    }

    clear_status_tabs(tabs, pending_group, in_progress_group, completed_group);

    prepare_matrix_group(pending_group, pending_list, quadrant_title_text);
    prepare_matrix_group(in_progress_group, in_progress_list, in_progress_title_text);
    prepare_matrix_group(completed_group, completed_list, quadrant_title_text);
    tabs->addTab(matrix_status_page(tabs, pending_group), get_text(QStringLiteral("Pendente")));
    tabs->addTab(matrix_status_page(tabs, in_progress_group), get_text(QStringLiteral("Em Progresso")));
    tabs->addTab(matrix_status_page(tabs, completed_group), get_text(QStringLiteral("Concluídas")));
    tabs->setCurrentIndex(0);
    tabs->show();
    if (tabs->layout() != nullptr) {
        tabs->layout()->invalidate();
        tabs->layout()->activate();
    }
    tabs->updateGeometry();
    tabs->update();
    return true;
}

}  // namespace

bool EisenhowerMatrixApp::set_task_layout(TaskLayoutMode mode, bool persist)
{
    const TaskLayoutMode previous_mode = task_layout_mode_;
    task_layout_mode_ = mode;
    if (!_apply_task_layout()) {
        task_layout_mode_ = previous_mode;
        _apply_task_layout();
        return false;
    }

    if (persist) {
        _save_task_layout_preference();
    }

    return true;
}

EisenhowerMatrixApp::TaskLayoutMode EisenhowerMatrixApp::current_task_layout() const
{
    return task_layout_mode_;
}

bool EisenhowerMatrixApp::module_tab_adjustment_locked() const
{
    return module_tab_adjustment_locked_;
}

void EisenhowerMatrixApp::set_module_tab_adjustment_locked(bool locked, bool persist)
{
    module_tab_adjustment_locked_ = locked;
    _sync_module_tab_movable_state();
    if (locked) {
        _capture_module_tab_order(false);
    }
    if (persist) {
        _save_task_layout_preference();
    }
}

void EisenhowerMatrixApp::restore_module_tab_order(bool persist)
{
    module_tab_order_ = default_module_tab_order();
    _apply_module_tab_order();
    _refresh_module_tabs();
    if (persist) {
        _save_task_layout_preference();
    }
}

bool EisenhowerMatrixApp::module_tab_available(const QString &key) const
{
    const QString normalized_key = normalized_module_tab_key(key);
    if (normalized_key == QStringLiteral("eisenhower")) {
        return eisenhower_module_page != nullptr;
    }
    if (normalized_key == QStringLiteral("pomodoro")) {
        return has_pomodoro_plugin();
    }
    if (normalized_key == QStringLiteral("feynman")) {
        return has_feynman_plugin();
    }
    if (normalized_key == QStringLiteral("mapamental")) {
        return has_mapamental_plugin();
    }
    if (normalized_key == QStringLiteral("audiolivro")) {
        return has_audiolivro_plugin();
    }

    return false;
}

bool EisenhowerMatrixApp::module_tab_visible(const QString &key) const
{
    const QString normalized_key = normalized_module_tab_key(key);
    if (normalized_key.isEmpty() || !module_tab_available(normalized_key)) {
        return false;
    }

    const QMap<QString, bool> visibility = normalized_module_tab_visibility(module_tab_visibility_);
    return visibility.value(normalized_key, true);
}

bool EisenhowerMatrixApp::set_module_tab_visible(const QString &key, bool visible, bool persist)
{
    const QString normalized_key = normalized_module_tab_key(key);
    if (normalized_key.isEmpty() || !module_tab_available(normalized_key)) {
        return false;
    }

    module_tab_visibility_ = normalized_module_tab_visibility(module_tab_visibility_);
    const bool previous_visible = module_tab_visibility_.value(normalized_key, true);
    if (!visible && previous_visible && _visible_available_module_tab_count() <= 1) {
        return false;
    }

    module_tab_visibility_.insert(normalized_key, visible);
    _refresh_module_tabs();
    if (persist) {
        _save_task_layout_preference();
    }

    return true;
}

bool EisenhowerMatrixApp::set_task_sort_mode(TaskSortMode mode, bool persist)
{
    if (mode == TaskSortMode::None) {
        task_sort_criteria_.clear();
    } else {
        const TaskSortCriterion criterion = task_sort_criterion_from_mode(mode);
        const Qt::SortOrder order = task_sort_order_from_mode(mode);
        const int existing_index = _task_sort_criterion_index(criterion);

        if (existing_index >= 0) {
            task_sort_criteria_[existing_index].order = order;
        } else {
            task_sort_criteria_.append({criterion, order});
        }
    }

    for (QListWidget *list : all_task_lists(this)) {
        _sort_task_list(list);
    }

    save_tasks();
    if (persist) {
        _save_task_layout_preference();
    }

    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }

    _refresh_plugin_task_views();

    return true;
}

bool EisenhowerMatrixApp::clear_task_sort_criterion(TaskSortCriterion criterion, bool persist)
{
    const int existing_index = _task_sort_criterion_index(criterion);
    if (existing_index < 0) {
        return true;
    }

    task_sort_criteria_.removeAt(existing_index);

    for (QListWidget *list : all_task_lists(this)) {
        _sort_task_list(list);
    }

    save_tasks();
    if (persist) {
        _save_task_layout_preference();
    }

    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }

    _refresh_plugin_task_views();

    return true;
}

bool EisenhowerMatrixApp::has_task_sort_criterion(TaskSortCriterion criterion) const
{
    return _task_sort_criterion_index(criterion) >= 0;
}

Qt::SortOrder EisenhowerMatrixApp::task_sort_order(TaskSortCriterion criterion) const
{
    const int index = _task_sort_criterion_index(criterion);
    return index >= 0 ? task_sort_criteria_.at(index).order : Qt::AscendingOrder;
}

int EisenhowerMatrixApp::task_sort_priority(TaskSortCriterion criterion) const
{
    return _task_sort_criterion_index(criterion);
}

bool EisenhowerMatrixApp::has_active_task_sort() const
{
    return !task_sort_criteria_.isEmpty();
}

bool EisenhowerMatrixApp::task_sort_less_for_payloads(const QVariantMap &left, const QVariantMap &right) const
{
    return _task_sort_less(normalized_task_sort_payload(left), normalized_task_sort_payload(right));
}

int EisenhowerMatrixApp::_task_sort_criterion_index(TaskSortCriterion criterion) const
{
    for (int i = 0; i < task_sort_criteria_.size(); ++i) {
        if (task_sort_criteria_.at(i).criterion == criterion) {
            return i;
        }
    }
    return -1;
}

void EisenhowerMatrixApp::_ensure_task_metadata(QVariantMap &data)
{
    QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
    if (task_id.isEmpty()) {
        task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        data.insert(QStringLiteral("id"), task_id);
    }

    bool ok = false;
    const qint64 insertion_order = data.value(QStringLiteral("insertion_order")).toLongLong(&ok);
    if (!ok || insertion_order <= 0) {
        data.insert(QStringLiteral("insertion_order"), ++task_insertion_counter_);
    } else if (insertion_order > task_insertion_counter_) {
        task_insertion_counter_ = insertion_order;
    }

    const int def_c = default_pomodoro_cycle_count();
    if (!data.contains(QStringLiteral("ciclos_totais")) || data.value(QStringLiteral("ciclos_totais")).toInt() <= 0) {
        data.insert(QStringLiteral("ciclos_totais"), def_c);
        data.insert(QStringLiteral("ciclos_permitidos"), def_c);
        data.insert(QStringLiteral("total_ciclos"), def_c);
    }
    if (!data.contains(QStringLiteral("ciclos_customizados"))) {
        data.insert(QStringLiteral("ciclos_customizados"), false);
    }
}

qint64 EisenhowerMatrixApp::_task_insertion_order(const QVariantMap &data) const
{
    bool ok = false;
    const qint64 insertion_order = data.value(QStringLiteral("insertion_order")).toLongLong(&ok);
    return (ok && insertion_order > 0) ? insertion_order : std::numeric_limits<qint64>::max();
}

void EisenhowerMatrixApp::_load_task_layout_preference()
{
    task_layout_mode_ = TaskLayoutMode::Tabs;
    task_sort_criteria_.clear();
    calendar_start_expanded = false;
    feynman_task_panel_start_expanded = false;
    pomodoro_in_progress_panel_start_expanded = false;
    sync_eisenhower_enabled_ = true;
    sync_pomodoro_enabled_ = true;
    sync_feynman_enabled_ = true;
    reserved_space_eisenhower_enabled_ = true;
    reserved_space_pomodoro_enabled_ = false;
    reserved_space_feynman_enabled_ = false;
    reserved_watermark_eisenhower_enabled_ = true;
    reserved_watermark_pomodoro_enabled_ = false;
    reserved_watermark_feynman_enabled_ = false;
    feynman_ui_layout_mode_key_ = QStringLiteral("tabs");
    pomodoro_ui_layout_mode_key_ = QStringLiteral("simple:tabs");
    module_tab_order_ = default_module_tab_order();
    module_tab_visibility_ = default_module_tab_visibility();
    module_tab_adjustment_locked_ = true;

    QFile file(layout_path);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return;
    }

    const QString layout_value = doc.object().value(QStringLiteral("layout")).toString().trimmed().toLower();
    if (layout_value == QStringLiteral("quadrants")) {
        task_layout_mode_ = TaskLayoutMode::Quadrants;
    } else if (layout_value == QStringLiteral("matrix_2x2")) {
        task_layout_mode_ = TaskLayoutMode::Matrix2x2;
    } else if (layout_value == QStringLiteral("tabs_horizontal") || layout_value == QStringLiteral("horizontal")) {
        task_layout_mode_ = TaskLayoutMode::TabsHorizontal;
    }
    pomodoro_ui_layout_mode_key_ =
        normalized_pomodoro_ui_layout_key(doc.object().value(QStringLiteral("pomodoro_ui_layout")).toString(pomodoro_ui_layout_mode_key_));
    feynman_ui_layout_mode_key_ =
        normalized_feynman_ui_layout_key(doc.object().value(QStringLiteral("feynman_ui_layout")).toString(feynman_ui_layout_mode_key_));
    const QString gantt_val = doc.object().value(QStringLiteral("gantt_table_layout")).toString(gantt_table_layout_mode_key_).trimmed();
    if (!gantt_val.isEmpty()) {
        gantt_table_layout_mode_key_ = gantt_val;
    } else {
        const QString fallback_gantt = doc.object().value(QStringLiteral("gantt_layout")).toString().trimmed();
        if (!fallback_gantt.isEmpty()) {
            gantt_table_layout_mode_key_ = fallback_gantt;
        }
    }

    module_tab_order_ =
        normalized_module_tab_order(json_string_array_to_list(doc.object().value(QStringLiteral("module_tab_order"))));
    const QJsonValue visible_tabs_value = doc.object().value(QStringLiteral("visible_module_tabs"));
    if (visible_tabs_value.isArray()) {
        QMap<QString, bool> visible_tabs;
        for (const QString &key : default_module_tab_order()) {
            visible_tabs.insert(key, false);
        }
        for (const QString &key : json_string_array_to_list(visible_tabs_value)) {
            const QString normalized_key = normalized_module_tab_key(key);
            if (!normalized_key.isEmpty()) {
                visible_tabs.insert(normalized_key, true);
            }
        }
        module_tab_visibility_ = normalized_module_tab_visibility(visible_tabs);
    }

    const QJsonValue visibility_value = doc.object().value(QStringLiteral("module_tab_visibility"));
    if (visibility_value.isObject()) {
        QMap<QString, bool> visibility = module_tab_visibility_;
        const QJsonObject visibility_object = visibility_value.toObject();
        for (const QString &key : default_module_tab_order()) {
            if (visibility_object.contains(key)) {
                visibility.insert(key, visibility_object.value(key).toBool(visibility.value(key, true)));
            }
        }
        module_tab_visibility_ = normalized_module_tab_visibility(visibility);
    }
    module_tab_adjustment_locked_ =
        doc.object().value(QStringLiteral("module_tabs_locked")).toBool(module_tab_adjustment_locked_);

    const QJsonValue criteria_value = doc.object().value(QStringLiteral("task_sort_criteria"));
    if (criteria_value.isArray()) {
        const QJsonArray criteria_array = criteria_value.toArray();
        for (const QJsonValue &value : criteria_array) {
            if (!value.isObject()) {
                continue;
            }

            const QJsonObject entry = value.toObject();
            const QString criterion_key = entry.value(QStringLiteral("criterion")).toString().trimmed().toLower();
            const QString order_key = entry.value(QStringLiteral("order")).toString().trimmed().toLower();

            TaskSortCriterion criterion;
            if (criterion_key == QStringLiteral("date")) {
                criterion = TaskSortCriterion::Date;
            } else if (criterion_key == QStringLiteral("alphabetical")) {
                criterion = TaskSortCriterion::Alphabetical;
            } else if (criterion_key == QStringLiteral("time")) {
                criterion = TaskSortCriterion::Time;
            } else {
                continue;
            }

            const Qt::SortOrder order = order_key == QStringLiteral("desc") ? Qt::DescendingOrder
                                                                            : Qt::AscendingOrder;
            const int existing_index = _task_sort_criterion_index(criterion);
            if (existing_index >= 0) {
                task_sort_criteria_[existing_index].order = order;
            } else {
                task_sort_criteria_.append({criterion, order});
            }
        }
    } else {
        const QString sort_value = doc.object().value(QStringLiteral("task_sort")).toString().trimmed().toLower();
        if (sort_value == QStringLiteral("date_asc")) {
            task_sort_criteria_.append({TaskSortCriterion::Date, Qt::AscendingOrder});
        } else if (sort_value == QStringLiteral("date_desc")) {
            task_sort_criteria_.append({TaskSortCriterion::Date, Qt::DescendingOrder});
        } else if (sort_value == QStringLiteral("time_asc")) {
            task_sort_criteria_.append({TaskSortCriterion::Time, Qt::AscendingOrder});
        } else if (sort_value == QStringLiteral("time_desc")) {
            task_sort_criteria_.append({TaskSortCriterion::Time, Qt::DescendingOrder});
        } else if (sort_value == QStringLiteral("alphabetical_asc")) {
            task_sort_criteria_.append({TaskSortCriterion::Alphabetical, Qt::AscendingOrder});
        } else if (sort_value == QStringLiteral("alphabetical_desc")) {
            task_sort_criteria_.append({TaskSortCriterion::Alphabetical, Qt::DescendingOrder});
        }
    }

    sync_eisenhower_enabled_ =
        doc.object().value(QStringLiteral("sync_eisenhower")).toBool(sync_eisenhower_enabled_);
    sync_pomodoro_enabled_ = doc.object().value(QStringLiteral("sync_pomodoro")).toBool(sync_pomodoro_enabled_);
    sync_feynman_enabled_ = doc.object().value(QStringLiteral("sync_feynman")).toBool(sync_feynman_enabled_);
    reserved_space_eisenhower_enabled_ =
        doc.object().value(QStringLiteral("reserved_space_eisenhower")).toBool(reserved_space_eisenhower_enabled_);
    reserved_space_pomodoro_enabled_ =
        doc.object().value(QStringLiteral("reserved_space_pomodoro")).toBool(reserved_space_pomodoro_enabled_);
    reserved_space_feynman_enabled_ =
        doc.object().value(QStringLiteral("reserved_space_feynman")).toBool(reserved_space_feynman_enabled_);
    reserved_watermark_eisenhower_enabled_ = doc.object()
                                                 .value(QStringLiteral("reserved_watermark_eisenhower"))
                                                 .toBool(reserved_watermark_eisenhower_enabled_);
    reserved_watermark_pomodoro_enabled_ =
        doc.object().value(QStringLiteral("reserved_watermark_pomodoro")).toBool(reserved_watermark_pomodoro_enabled_);
    reserved_watermark_feynman_enabled_ =
        doc.object().value(QStringLiteral("reserved_watermark_feynman")).toBool(reserved_watermark_feynman_enabled_);
    calendar_start_expanded = doc.object().value(QStringLiteral("show_calendar_panel")).toBool(calendar_start_expanded);
    feynman_task_panel_start_expanded =
        doc.object().value(QStringLiteral("show_feynman_task_panel")).toBool(feynman_task_panel_start_expanded);
    pomodoro_in_progress_panel_start_expanded =
        doc.object().value(QStringLiteral("show_pomodoro_in_progress_panel")).toBool(pomodoro_in_progress_panel_start_expanded);
}

void EisenhowerMatrixApp::_save_task_layout_preference() const
{
    if (layout_path.isEmpty()) {
        return;
    }

    const QFileInfo info(layout_path);
    QDir().mkpath(info.absolutePath());

    QJsonObject config;
    {
        QFile existing_file(layout_path);
        if (existing_file.open(QIODevice::ReadOnly)) {
            const QJsonDocument existing_doc = QJsonDocument::fromJson(existing_file.readAll());
            if (existing_doc.isObject()) {
                config = existing_doc.object();
            }
        }
    }

    QSaveFile file(layout_path);
    if (!file.open(QIODevice::WriteOnly)) {
        logger()->error(QStringLiteral("Erro ao salvar configuração de layout: %1").arg(layout_path));
        return;
    }
    config.insert(QStringLiteral("layout"), task_layout_key(task_layout_mode_));
    config.insert(QStringLiteral("pomodoro_ui_layout"), normalized_pomodoro_ui_layout_key(pomodoro_ui_layout_mode_key_));
    config.insert(QStringLiteral("feynman_ui_layout"), normalized_feynman_ui_layout_key(feynman_ui_layout_mode_key_));
    config.insert(QStringLiteral("module_tabs_locked"), module_tab_adjustment_locked_);
    QJsonArray module_tab_order;
    for (const QString &key : normalized_module_tab_order(module_tab_order_)) {
        module_tab_order.append(key);
    }
    config.insert(QStringLiteral("module_tab_order"), module_tab_order);
    QJsonObject module_tab_visibility;
    QJsonArray visible_module_tabs;
    const QMap<QString, bool> visibility = normalized_module_tab_visibility(module_tab_visibility_);
    for (const QString &key : default_module_tab_order()) {
        const bool visible = visibility.value(key, true);
        module_tab_visibility.insert(key, visible);
        if (visible) {
            visible_module_tabs.append(key);
        }
    }
    config.insert(QStringLiteral("module_tab_visibility"), module_tab_visibility);
    config.insert(QStringLiteral("visible_module_tabs"), visible_module_tabs);
    QJsonArray sort_criteria;
    for (const TaskSortCriterionState &entry : task_sort_criteria_) {
        QJsonObject criterion_entry;
        criterion_entry.insert(QStringLiteral("criterion"), task_sort_criterion_key(entry.criterion));
        criterion_entry.insert(QStringLiteral("order"), task_sort_order_key(entry.order));
        sort_criteria.append(criterion_entry);
    }
    config.insert(QStringLiteral("task_sort_criteria"), sort_criteria);
    config.insert(QStringLiteral("sync_eisenhower"), sync_eisenhower_enabled_);
    config.insert(QStringLiteral("sync_pomodoro"), sync_pomodoro_enabled_);
    config.insert(QStringLiteral("sync_feynman"), sync_feynman_enabled_);
    config.insert(QStringLiteral("reserved_space_eisenhower"), reserved_space_eisenhower_enabled_);
    config.insert(QStringLiteral("reserved_space_pomodoro"), reserved_space_pomodoro_enabled_);
    config.insert(QStringLiteral("reserved_space_feynman"), reserved_space_feynman_enabled_);
    config.insert(QStringLiteral("reserved_watermark_eisenhower"), reserved_watermark_eisenhower_enabled_);
    config.insert(QStringLiteral("reserved_watermark_pomodoro"), reserved_watermark_pomodoro_enabled_);
    config.insert(QStringLiteral("reserved_watermark_feynman"), reserved_watermark_feynman_enabled_);
    config.insert(QStringLiteral("show_calendar_panel"), calendar_panel_visible());
    config.insert(QStringLiteral("show_feynman_task_panel"), feynman_task_panel_visible());
    config.insert(QStringLiteral("show_pomodoro_in_progress_panel"), pomodoro_in_progress_panel_visible());
    if (!gantt_table_layout_mode_key_.isEmpty()) {
        config.insert(QStringLiteral("gantt_table_layout"), gantt_table_layout_mode_key_);
        config.insert(QStringLiteral("gantt_layout"), gantt_table_layout_mode_key_);
    }
    const QByteArray payload = QJsonDocument(config).toJson(QJsonDocument::Indented);
    if (file.write(payload) != payload.size() || !file.commit()) {
        logger()->error(QStringLiteral("Erro ao confirmar configuração de layout: %1").arg(layout_path));
    }
}

bool EisenhowerMatrixApp::_attach_quadrant_panels_to_tabs(bool horizontal_layout)
{
    if (quadrant_tabs == nullptr || quadrant1_tab == nullptr || quadrant2_tab == nullptr || quadrant3_tab == nullptr
        || quadrant4_tab == nullptr) {
        return false;
    }

    clear_layout(quadrants_layout);
    clear_layout(matrix2x2_layout);
    clear_matrix_status_tabs(this);

    if (!attach_split_panel(quadrant1_tab,
                            quadrant1_group,
                            quadrant1_list,
                            quadrant1_in_progress_group,
                            quadrant1_in_progress_list,
                            quadrant1_completed_group,
                            quadrant1_completed_list,
                            pending_title(this, 0),
                            quadrant1_in_progress_label->text(),
                            completed_title(this, 0),
                            true,
                            horizontal_layout)
        || !attach_split_panel(quadrant2_tab,
                               quadrant2_group,
                               quadrant2_list,
                               quadrant2_in_progress_group,
                               quadrant2_in_progress_list,
                               quadrant2_completed_group,
                               quadrant2_completed_list,
                               pending_title(this, 1),
                               quadrant2_in_progress_label->text(),
                               completed_title(this, 1),
                               true,
                               horizontal_layout)
        || !attach_split_panel(quadrant3_tab,
                               quadrant3_group,
                               quadrant3_list,
                               quadrant3_in_progress_group,
                               quadrant3_in_progress_list,
                               quadrant3_completed_group,
                               quadrant3_completed_list,
                               pending_title(this, 2),
                               quadrant3_in_progress_label->text(),
                               completed_title(this, 2),
                               true,
                               horizontal_layout)
        || !attach_split_panel(quadrant4_tab,
                               quadrant4_group,
                               quadrant4_list,
                               quadrant4_in_progress_group,
                               quadrant4_in_progress_list,
                               quadrant4_completed_group,
                               quadrant4_completed_list,
                               pending_title(this, 3),
                               quadrant4_in_progress_label->text(),
                               completed_title(this, 3),
                               true,
                               horizontal_layout)) {
        return false;
    }

    while (quadrant_tabs->count() > 0) {
        quadrant_tabs->removeTab(0);
    }

    const QList<QPair<QWidget *, QString>> panels = {
        {quadrant1_tab, quadrant1_label != nullptr ? quadrant1_label->text()
                                                   : get_text(QStringLiteral("🔴 Importante e Urgente"))},
        {quadrant2_tab, quadrant2_label != nullptr ? quadrant2_label->text()
                                                   : get_text(QStringLiteral("🟠 Importante, mas Não Urgente"))},
        {quadrant3_tab, quadrant3_label != nullptr ? quadrant3_label->text()
                                                   : get_text(QStringLiteral("🟡 Não Importante, mas Urgente"))},
        {quadrant4_tab, quadrant4_label != nullptr ? quadrant4_label->text()
                                                   : get_text(QStringLiteral("🟢 Não Importante e Não Urgente"))},
    };

    for (const auto &panel : panels) {
        if (panel.first != nullptr) {
            panel.first->setMinimumWidth(horizontal_layout ? 680 : 0);
            quadrant_tabs->addTab(panel.first, panel.second);
        }
    }

    if (quadrant_tabs->count() > 0) {
        quadrant_tabs->setCurrentIndex(qBound(0, last_quadrant_tab_index_, quadrant_tabs->count() - 1));
    }

    return true;
}

bool EisenhowerMatrixApp::_attach_quadrant_panels_to_columns()
{
    if (quadrants_layout == nullptr || quadrant1_tab == nullptr || quadrant2_tab == nullptr || quadrant3_tab == nullptr
        || quadrant4_tab == nullptr || quadrants_page == nullptr) {
        return false;
    }

    if (quadrant_tabs != nullptr && quadrant_tabs->count() > 0) {
        last_quadrant_tab_index_ = qMax(0, quadrant_tabs->currentIndex());
    }

    clear_layout(quadrants_layout);
    clear_layout(matrix2x2_layout);
    if (quadrant_tabs != nullptr) {
        while (quadrant_tabs->count() > 0) {
            quadrant_tabs->removeTab(0);
        }
    }
    clear_matrix_status_tabs(this);

    if (!attach_split_panel(quadrant1_tab,
                            quadrant1_group,
                            quadrant1_list,
                            quadrant1_in_progress_group,
                            quadrant1_in_progress_list,
                            quadrant1_completed_group,
                            quadrant1_completed_list,
                            quadrant_title(this, 0),
                            quadrant1_in_progress_label->text(),
                            completed_title(this, 0),
                            true)
        || !attach_split_panel(quadrant2_tab,
                               quadrant2_group,
                               quadrant2_list,
                               quadrant2_in_progress_group,
                               quadrant2_in_progress_list,
                               quadrant2_completed_group,
                               quadrant2_completed_list,
                               quadrant_title(this, 1),
                               quadrant2_in_progress_label->text(),
                               completed_title(this, 1),
                               true)
        || !attach_split_panel(quadrant3_tab,
                               quadrant3_group,
                               quadrant3_list,
                               quadrant3_in_progress_group,
                               quadrant3_in_progress_list,
                               quadrant3_completed_group,
                               quadrant3_completed_list,
                               quadrant_title(this, 2),
                               quadrant3_in_progress_label->text(),
                               completed_title(this, 2),
                               true)
        || !attach_split_panel(quadrant4_tab,
                               quadrant4_group,
                               quadrant4_list,
                               quadrant4_in_progress_group,
                               quadrant4_in_progress_list,
                               quadrant4_completed_group,
                               quadrant4_completed_list,
                               quadrant_title(this, 3),
                               quadrant4_in_progress_label->text(),
                               completed_title(this, 3),
                               true)) {
        return false;
    }

    for (QWidget *panel : {quadrant1_tab, quadrant2_tab, quadrant3_tab, quadrant4_tab}) {
        if (panel != nullptr) {
            panel->setMinimumWidth(260);
            quadrants_layout->addWidget(panel, 1);
            panel->show();
        }
    }

    return true;
}

bool EisenhowerMatrixApp::_attach_quadrant_panels_to_matrix_2x2()
{
    if (matrix2x2_layout == nullptr || matrix2x2_page == nullptr || quadrant1_status_tabs == nullptr
        || quadrant2_status_tabs == nullptr || quadrant3_status_tabs == nullptr || quadrant4_status_tabs == nullptr) {
        return false;
    }

    if (quadrant_tabs != nullptr && quadrant_tabs->count() > 0) {
        last_quadrant_tab_index_ = qMax(0, quadrant_tabs->currentIndex());
    }

    clear_layout(quadrants_layout);
    clear_layout(matrix2x2_layout);
    if (quadrant_tabs != nullptr) {
        while (quadrant_tabs->count() > 0) {
            quadrant_tabs->removeTab(0);
        }
    }

    if (!attach_status_tabs(quadrant1_status_tabs,
                            quadrant1_group,
                            quadrant1_list,
                            quadrant1_in_progress_group,
                            quadrant1_in_progress_list,
                            quadrant1_completed_group,
                            quadrant1_completed_list,
                            quadrant_title(this, 0),
                            quadrant_title(this, 0))
        || !attach_status_tabs(quadrant2_status_tabs,
                               quadrant2_group,
                               quadrant2_list,
                               quadrant2_in_progress_group,
                               quadrant2_in_progress_list,
                               quadrant2_completed_group,
                               quadrant2_completed_list,
                               quadrant_title(this, 1),
                               quadrant_title(this, 1))
        || !attach_status_tabs(quadrant3_status_tabs,
                               quadrant3_group,
                               quadrant3_list,
                               quadrant3_in_progress_group,
                               quadrant3_in_progress_list,
                               quadrant3_completed_group,
                               quadrant3_completed_list,
                               quadrant_title(this, 2),
                               quadrant_title(this, 2))
        || !attach_status_tabs(quadrant4_status_tabs,
                               quadrant4_group,
                               quadrant4_list,
                               quadrant4_in_progress_group,
                               quadrant4_in_progress_list,
                               quadrant4_completed_group,
                               quadrant4_completed_list,
                               quadrant_title(this, 3),
                               quadrant_title(this, 3))) {
        return false;
    }

    for (QTabWidget *tabs : {quadrant1_status_tabs, quadrant2_status_tabs, quadrant3_status_tabs, quadrant4_status_tabs}) {
        if (tabs != nullptr) {
            tabs->setMinimumWidth(260);
        }
    }

    matrix2x2_layout->addWidget(quadrant1_status_tabs, 0, 0);
    matrix2x2_layout->addWidget(quadrant2_status_tabs, 0, 1);
    matrix2x2_layout->addWidget(quadrant3_status_tabs, 1, 0);
    matrix2x2_layout->addWidget(quadrant4_status_tabs, 1, 1);
    matrix2x2_layout->invalidate();
    matrix2x2_layout->activate();
    matrix2x2_page->updateGeometry();
    matrix2x2_page->update();
    return true;
}

bool EisenhowerMatrixApp::_apply_task_layout()
{
    if (task_layout_stack == nullptr || quadrant_tabs_page == nullptr || quadrants_page == nullptr || matrix2x2_page == nullptr) {
        return false;
    }

    bool applied = false;
    if (task_layout_mode_ == TaskLayoutMode::Quadrants) {
        applied = _attach_quadrant_panels_to_columns();
        if (applied) {
            task_layout_stack->setCurrentWidget(quadrants_scroll != nullptr ? static_cast<QWidget *>(quadrants_scroll) : quadrants_page);
        }
    } else if (task_layout_mode_ == TaskLayoutMode::Matrix2x2) {
        applied = _attach_quadrant_panels_to_matrix_2x2();
        if (applied) {
            task_layout_stack->setCurrentWidget(matrix2x2_scroll != nullptr ? static_cast<QWidget *>(matrix2x2_scroll) : matrix2x2_page);
        }
    } else if (task_layout_mode_ == TaskLayoutMode::TabsHorizontal) {
        applied = _attach_quadrant_panels_to_tabs(true);
        if (applied) {
            task_layout_stack->setCurrentWidget(quadrant_tabs_scroll != nullptr ? static_cast<QWidget *>(quadrant_tabs_scroll) : quadrant_tabs_page);
        }
    } else {
        applied = _attach_quadrant_panels_to_tabs(false);
        if (applied) {
            task_layout_stack->setCurrentWidget(quadrant_tabs_scroll != nullptr ? static_cast<QWidget *>(quadrant_tabs_scroll) : quadrant_tabs_page);
        }
    }

    if (!applied) {
        return false;
    }

    _apply_theme_palette();
    return true;
}

}  // namespace source
