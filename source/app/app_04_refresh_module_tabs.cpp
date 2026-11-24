#include "source/app/app_04_refresh_module_tabs.hpp"

namespace source {
using namespace app;

void EisenhowerMatrixApp::_sync_pomodoro_host_bridge()
{
    if (pomodoro_host_bridge_ == nullptr || gerenciador_traducao == nullptr) {
        return;
    }

    pomodoro_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
}

void EisenhowerMatrixApp::_sync_feynman_host_bridge()
{
    if (feynman_host_bridge_ == nullptr || gerenciador_traducao == nullptr) {
        return;
    }

    feynman_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
}

void EisenhowerMatrixApp::_sync_mapamental_host_bridge()
{
    if (mapamental_host_bridge_ == nullptr || gerenciador_traducao == nullptr) {
        return;
    }

    mapamental_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
}

void EisenhowerMatrixApp::_sync_audiolivro_host_bridge()
{
    if (audiolivro_host_bridge_ == nullptr || gerenciador_traducao == nullptr) {
        return;
    }

    audiolivro_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
}

void EisenhowerMatrixApp::_sync_agenda_host_bridge()
{
    if (agenda_host_bridge_ == nullptr || gerenciador_traducao == nullptr) {
        return;
    }

    agenda_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
    agenda_host_bridge_->set_persistent_dir(source::utils::obter_caminho_persistente());
}

void EisenhowerMatrixApp::_sync_gantt_host_bridge()
{
    if (gantt_host_bridge_ == nullptr || gerenciador_traducao == nullptr) {
        return;
    }

    gantt_host_bridge_->set_eisenhower_host(this);
    gantt_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
    gantt_host_bridge_->set_persistent_dir(source::utils::obter_caminho_persistente());
}

void EisenhowerMatrixApp::_refresh_module_tabs()
{
    if (module_tabs == nullptr) {
        return;
    }

    _sync_module_tab_visibility();
    _apply_module_tab_order();

    const int eisenhower_index = eisenhower_module_page != nullptr ? module_tabs->indexOf(eisenhower_module_page) : -1;
    if (eisenhower_index >= 0) {
        module_tabs->setTabText(eisenhower_index, eisenhower_module_tab_text());
        source::utils::AccentTabStyle::set_tab_accent(module_tabs, eisenhower_index, source::utils::AccentTabStyle::accent_green());
    }

    const int feynman_index = feynman_module_page != nullptr ? module_tabs->indexOf(feynman_module_page) : -1;
    if (feynman_index >= 0) {
        module_tabs->setTabText(feynman_index, feynman_module_tab_text());
        source::utils::AccentTabStyle::set_tab_accent(module_tabs, feynman_index, source::utils::AccentTabStyle::accent_blue());
    }

    const int pomodoro_index = pomodoro_module_page != nullptr ? module_tabs->indexOf(pomodoro_module_page) : -1;
    if (pomodoro_index >= 0) {
        module_tabs->setTabText(pomodoro_index, pomodoro_module_tab_text());
        source::utils::AccentTabStyle::set_tab_accent(module_tabs, pomodoro_index, source::utils::AccentTabStyle::accent_red());
    }

    const int mapamental_index = mapamental_module_page != nullptr ? module_tabs->indexOf(mapamental_module_page) : -1;
    if (mapamental_index >= 0) {
        module_tabs->setTabText(mapamental_index, mapamental_module_tab_text(mapamental_display_name()));
        source::utils::AccentTabStyle::set_tab_accent(module_tabs,
                                                      mapamental_index,
                                                      source::utils::AccentTabStyle::accent_yellow());
    }

    const int audiolivro_index = audiolivro_module_page != nullptr ? module_tabs->indexOf(audiolivro_module_page) : -1;
    if (audiolivro_index >= 0) {
        module_tabs->setTabText(audiolivro_index, audiolivro_module_tab_text(audiolivro_display_name()));
        source::utils::AccentTabStyle::set_tab_accent(module_tabs,
                                                      audiolivro_index,
                                                      source::utils::AccentTabStyle::accent_blue());
    }

    refresh_module_task_counter();
}

QString EisenhowerMatrixApp::_module_tab_key(QWidget *page) const
{
    if (page == nullptr) {
        return {};
    }
    if (page == eisenhower_module_page) {
        return QStringLiteral("eisenhower");
    }
    if (page == pomodoro_module_page) {
        return QStringLiteral("pomodoro");
    }
    if (page == feynman_module_page) {
        return QStringLiteral("feynman");
    }
    if (page == mapamental_module_page) {
        return QStringLiteral("mapamental");
    }
    if (page == audiolivro_module_page) {
        return QStringLiteral("audiolivro");
    }

    return {};
}

QWidget *EisenhowerMatrixApp::_module_page_for_tab_key(const QString &key) const
{
    const QString normalized_key = key.trimmed().toLower();
    if (normalized_key == QStringLiteral("eisenhower")) {
        return eisenhower_module_page;
    }
    if (normalized_key == QStringLiteral("pomodoro")) {
        return pomodoro_module_page;
    }
    if (normalized_key == QStringLiteral("feynman")) {
        return feynman_module_page;
    }
    if (normalized_key == QStringLiteral("mapamental")) {
        return mapamental_module_page;
    }
    if (normalized_key == QStringLiteral("audiolivro")) {
        return audiolivro_module_page;
    }

    return nullptr;
}

QStringList EisenhowerMatrixApp::_current_module_tab_order() const
{
    QStringList order;
    if (module_tabs == nullptr) {
        return order;
    }

    order.reserve(module_tabs->count());
    for (int i = 0; i < module_tabs->count(); ++i) {
        const QString key = _module_tab_key(module_tabs->widget(i));
        if (!key.isEmpty() && !order.contains(key)) {
            order.append(key);
        }
    }

    return order;
}

void EisenhowerMatrixApp::_capture_module_tab_order(bool persist)
{
    QStringList captured = _current_module_tab_order();
    for (const QString &key : module_tab_order_) {
        if (!captured.contains(key)) {
            captured.append(key);
        }
    }

    module_tab_order_ = normalized_module_tab_order(captured);
    if (persist) {
        _save_task_layout_preference();
    }
}

int EisenhowerMatrixApp::_visible_available_module_tab_count() const
{
    int count = 0;
    const QMap<QString, bool> visibility = normalized_module_tab_visibility(module_tab_visibility_);
    for (const QString &key : default_module_tab_order()) {
        if (module_tab_available(key) && visibility.value(key, true)) {
            ++count;
        }
    }
    return count;
}

void EisenhowerMatrixApp::_sync_module_tab_visibility()
{
    if (module_tabs == nullptr) {
        return;
    }

    module_tab_visibility_ = normalized_module_tab_visibility(module_tab_visibility_);
    QWidget *current_page = module_tabs->currentWidget();

    const auto sync_page = [this](const QString &key, QWidget *page, const QString &title) {
        if (page == nullptr) {
            return;
        }

        const bool should_show = module_tab_available(key) && module_tab_visibility_.value(key, true);
        const int current_index = module_tabs->indexOf(page);
        if (should_show && current_index < 0) {
            module_tabs->addTab(page, title);
        } else if (!should_show && current_index >= 0) {
            module_tabs->removeTab(current_index);
        }
    };

    sync_page(QStringLiteral("eisenhower"), eisenhower_module_page, eisenhower_module_tab_text());
    sync_page(QStringLiteral("pomodoro"), pomodoro_module_page, pomodoro_module_tab_text());
    sync_page(QStringLiteral("feynman"), feynman_module_page, feynman_module_tab_text());
    sync_page(QStringLiteral("mapamental"), mapamental_module_page, mapamental_module_tab_text(mapamental_display_name()));
    sync_page(QStringLiteral("audiolivro"), audiolivro_module_page, audiolivro_module_tab_text(audiolivro_display_name()));

    if (module_tabs->count() == 0 && eisenhower_module_page != nullptr) {
        module_tabs->addTab(eisenhower_module_page, eisenhower_module_tab_text());
        current_page = eisenhower_module_page;
    }

    if (current_page != nullptr && module_tabs->indexOf(current_page) >= 0) {
        module_tabs->setCurrentWidget(current_page);
    } else if (module_tabs->count() > 0) {
        module_tabs->setCurrentIndex(0);
    }
}

void EisenhowerMatrixApp::_apply_module_tab_order()
{
    if (module_tabs == nullptr || module_tabs->tabBar() == nullptr) {
        return;
    }

    const QStringList order = normalized_module_tab_order(module_tab_order_);
    module_tab_order_ = order;

    QWidget *current_page = module_tabs->currentWidget();
    applying_module_tab_order_ = true;
    int target_index = 0;
    for (const QString &key : order) {
        QWidget *page = _module_page_for_tab_key(key);
        if (page == nullptr) {
            continue;
        }

        const int current_index = module_tabs->indexOf(page);
        if (current_index < 0) {
            continue;
        }

        if (current_index != target_index) {
            module_tabs->tabBar()->moveTab(current_index, target_index);
        }
        ++target_index;
    }
    applying_module_tab_order_ = false;

    if (current_page != nullptr && module_tabs->indexOf(current_page) >= 0) {
        module_tabs->setCurrentWidget(current_page);
    }
    _sync_module_tab_movable_state();
}

void EisenhowerMatrixApp::_sync_module_tab_movable_state()
{
    if (module_tabs != nullptr) {
        module_tabs->setMovable(!module_tab_adjustment_locked_);
    }
}

void EisenhowerMatrixApp::_connect_task_counter_list(QListWidget *list)
{
    if (list == nullptr) {
        return;
    }

    QObject::connect(list, &QListWidget::itemSelectionChanged, this, &EisenhowerMatrixApp::refresh_module_task_counter);
    if (QAbstractItemModel *model = list->model(); model != nullptr) {
        QObject::connect(model, &QAbstractItemModel::modelReset, this, &EisenhowerMatrixApp::refresh_module_task_counter);
        QObject::connect(model,
                         &QAbstractItemModel::rowsInserted,
                         this,
                         [this](const QModelIndex &, int, int) { refresh_module_task_counter(); });
        QObject::connect(model,
                         &QAbstractItemModel::rowsRemoved,
                         this,
                         [this](const QModelIndex &, int, int) { refresh_module_task_counter(); });
    }
}

void EisenhowerMatrixApp::_connect_feynman_task_counter_sources()
{
    if (feynman_module_page == nullptr) {
        return;
    }

    QObject::connect(feynman_module_page, SIGNAL(task_counter_changed()), this, SLOT(refresh_module_task_counter()));
}

void EisenhowerMatrixApp::_connect_pomodoro_task_counter_sources()
{
    if (pomodoro_module_page == nullptr) {
        return;
    }

    QObject::connect(pomodoro_module_page, SIGNAL(task_counter_changed()), this, SLOT(refresh_module_task_counter()));
}

void EisenhowerMatrixApp::_connect_mapamental_task_counter_sources()
{
    if (mapamental_module_page == nullptr) {
        return;
    }

    QObject::connect(mapamental_module_page, SIGNAL(task_counter_changed()), this, SLOT(refresh_module_task_counter()));
}

void EisenhowerMatrixApp::_connect_audiolivro_task_counter_sources()
{
    if (audiolivro_module_page == nullptr) {
        return;
    }

    QObject::connect(audiolivro_module_page, SIGNAL(task_counter_changed()), this, SLOT(refresh_module_task_counter()));
}

QPair<int, int> EisenhowerMatrixApp::_eisenhower_task_counts() const
{
    QSet<QString> selected_keys;
    int total_count = 0;

    for (const QListWidget *list : {static_cast<const QListWidget *>(quadrant1_list),
                                    static_cast<const QListWidget *>(quadrant2_list),
                                    static_cast<const QListWidget *>(quadrant3_list),
                                    static_cast<const QListWidget *>(quadrant4_list),
                                    static_cast<const QListWidget *>(quadrant1_in_progress_list),
                                    static_cast<const QListWidget *>(quadrant2_in_progress_list),
                                    static_cast<const QListWidget *>(quadrant3_in_progress_list),
                                    static_cast<const QListWidget *>(quadrant4_in_progress_list),
                                    static_cast<const QListWidget *>(quadrant1_completed_list),
                                    static_cast<const QListWidget *>(quadrant2_completed_list),
                                    static_cast<const QListWidget *>(quadrant3_completed_list),
                                    static_cast<const QListWidget *>(quadrant4_completed_list)}) {
        collect_selected_task_keys(list, selected_keys);
        total_count += count_selectable_items(list);
    }

    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        collect_selected_task_keys(calendar_pane->calendar_panel->tasks_list, selected_keys);
    }

    return {selected_keys.size(), total_count};
}

QPair<int, int> EisenhowerMatrixApp::_feynman_task_counts() const
{
    if (feynman_module_page == nullptr) {
        return {0, 0};
    }

    int selected_count = 0;
    int total_count = 0;
    QMetaObject::invokeMethod(feynman_module_page,
                              "selected_task_count",
                              Qt::DirectConnection,
                              Q_RETURN_ARG(int, selected_count));
    QMetaObject::invokeMethod(feynman_module_page, "total_task_count", Qt::DirectConnection, Q_RETURN_ARG(int, total_count));
    return {selected_count, total_count};
}

QPair<int, int> EisenhowerMatrixApp::_pomodoro_task_counts() const
{
    if (pomodoro_module_page == nullptr) {
        return {0, 0};
    }

    int selected_count = 0;
    int total_count = 0;
    QMetaObject::invokeMethod(pomodoro_module_page,
                              "selected_task_count",
                              Qt::DirectConnection,
                              Q_RETURN_ARG(int, selected_count));
    QMetaObject::invokeMethod(pomodoro_module_page, "total_task_count", Qt::DirectConnection, Q_RETURN_ARG(int, total_count));
    return {selected_count, total_count};
}

QPair<int, int> EisenhowerMatrixApp::_mapamental_task_counts() const
{
    if (mapamental_module_page == nullptr) {
        return {0, 0};
    }

    int selected_count = 0;
    int total_count = 0;
    QMetaObject::invokeMethod(mapamental_module_page,
                              "selected_task_count",
                              Qt::DirectConnection,
                              Q_RETURN_ARG(int, selected_count));
    QMetaObject::invokeMethod(mapamental_module_page,
                              "total_task_count",
                              Qt::DirectConnection,
                              Q_RETURN_ARG(int, total_count));
    return {selected_count, total_count};
}

QPair<int, int> EisenhowerMatrixApp::_audiolivro_task_counts() const
{
    if (audiolivro_module_page == nullptr) {
        return {0, 0};
    }

    int selected_count = 0;
    int total_count = 0;
    QMetaObject::invokeMethod(audiolivro_module_page,
                              "selected_task_count",
                              Qt::DirectConnection,
                              Q_RETURN_ARG(int, selected_count));
    QMetaObject::invokeMethod(audiolivro_module_page,
                              "total_task_count",
                              Qt::DirectConnection,
                              Q_RETURN_ARG(int, total_count));
    return {selected_count, total_count};
}

}  // namespace source
