#include "source/app/app_03_initUI.hpp"

namespace source
{
    using namespace app;

    void EisenhowerMatrixApp::criar_menu_configuracoes()
    {
        source::InterfaceCore::criar_menu_configuracoes(this);
        _attach_module_task_counter_to_menu_bar();
    }

    void EisenhowerMatrixApp::definir_idioma(const QString &codigo_idioma)
    {
        source::InterfaceCore::definir_idioma(this, codigo_idioma);
    }

    void EisenhowerMatrixApp::initUI()
    {
        source::InterfaceCore::init_ui(this);

        QWidget *old_central = centralWidget();
        auto *container = new QWidget(this);
        auto *layout = new QHBoxLayout(container);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        main_splitter = new QSplitter(Qt::Horizontal, container);
        main_splitter->setChildrenCollapsible(false);
        main_splitter->setHandleWidth(calendar_splitter_handle_width);

        calendar_pane = new source::GerenciamentoUI::Calendar(this, container);
        if (old_central != nullptr)
        {
            old_central->setMinimumWidth(qMax(120, quadrants_min_width));
            relax_module_page(old_central);
        }

        main_splitter->addWidget(calendar_pane);
        main_splitter->addWidget(old_central);
        main_splitter->setStretchFactor(0, 0);
        main_splitter->setStretchFactor(1, 1);
        connect(main_splitter, &QSplitter::splitterMoved, this, [this](int, int)
                {
        if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
            calendar_pane->calendar_panel->update_calendars_for_width();
        } });
        layout->addWidget(main_splitter, 1);

        const int configured_panel_w = qMax(220, calendar_min_width);
        calendar_pane->calendar_panel->set_calendar_content_width(configured_panel_w);
        calendar_pane->setMinimumWidth(calendar_pane->effective_min_width());

        const int calendar_w = qMax(25, calendar_pane->effective_min_width());
        const int total_w = qMax(width(), 800);
        const int quadrant_w = qMax(qMax(120, quadrants_min_width), total_w - calendar_w);
        main_splitter->setSizes({calendar_w, quadrant_w});

        _main_content = old_central;
        eisenhower_module_page = container;
        _setup_module_tabs(container);
        _hide_legacy_calendar_button();
        _apply_task_layout();
    }

    void EisenhowerMatrixApp::_setup_module_tabs(QWidget *eisenhower_content)
    {
        module_tabs = new QTabWidget(this);
        module_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        module_tabs->setDocumentMode(true);
        module_tabs->setMovable(!module_tab_adjustment_locked_);
        module_tabs->setTabsClosable(false);
        source::utils::AccentTabStyle::install(module_tabs);
        source::utils::AccentTabStyle::set_alignment(module_tabs, Qt::AlignLeft);
        if (module_tabs->tabBar() != nullptr)
        {
            module_tabs->tabBar()->setExpanding(false);
            module_tabs->tabBar()->setContentsMargins(10, 0, 0, 0);
            QObject::connect(module_tabs->tabBar(), &QTabBar::tabMoved, this, [this](int, int)
                             {
            if (!applying_module_tab_order_) {
                _capture_module_tab_order(true);
                refresh_module_task_counter();
            } });
        }

        if (eisenhower_content != nullptr)
        {
            relax_module_page(eisenhower_content);
            module_tabs->addTab(eisenhower_content, eisenhower_module_tab_text());
            source::utils::AccentTabStyle::set_tab_accent(module_tabs, eisenhower_content, source::utils::AccentTabStyle::accent_green());
        }

        module_task_counter_container = new QWidget(this);
        module_task_counter_container->setMinimumWidth(420);

        auto *counter_layout = new QHBoxLayout(module_task_counter_container);
        counter_layout->setContentsMargins(16, 0, 12, 0);
        counter_layout->setSpacing(0);
        counter_layout->addStretch(1);

        module_task_counter_label = new QLabel(module_task_counter_container);
        module_task_counter_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        module_task_counter_label->setMinimumWidth(392);
        counter_layout->addWidget(module_task_counter_label, 0, Qt::AlignRight | Qt::AlignVCenter);
        _attach_module_task_counter_to_menu_bar();

        QObject::connect(module_tabs, &QTabWidget::currentChanged, this, &EisenhowerMatrixApp::refresh_module_task_counter);

        for (QListWidget *list : {quadrant1_list,
                                  quadrant2_list,
                                  quadrant3_list,
                                  quadrant4_list,
                                  quadrant1_in_progress_list,
                                  quadrant2_in_progress_list,
                                  quadrant3_in_progress_list,
                                  quadrant4_in_progress_list,
                                  quadrant1_completed_list,
                                  quadrant2_completed_list,
                                  quadrant3_completed_list,
                                  quadrant4_completed_list})
        {
            _connect_task_counter_list(list);
        }
        if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr)
        {
            _connect_task_counter_list(calendar_pane->calendar_panel->tasks_list);
        }

        setCentralWidget(module_tabs);
        _root_container = module_tabs;
        _refresh_module_tabs();
        refresh_module_task_counter();
    }

    void EisenhowerMatrixApp::_attach_module_task_counter_to_menu_bar()
    {
        if (module_task_counter_container == nullptr)
        {
            return;
        }

        if (menu_bar == nullptr)
        {
            menu_bar = new QMenuBar(this);
            menu_bar->setNativeMenuBar(false);
            menu_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        }

        if (menu_bar_host == nullptr)
        {
            menu_bar_host = new QWidget(this);
            auto *host_layout = new QHBoxLayout(menu_bar_host);
            host_layout->setContentsMargins(0, 0, 8, 0);
            host_layout->setSpacing(0);
        }

        auto *host_layout = qobject_cast<QHBoxLayout *>(menu_bar_host->layout());
        if (host_layout == nullptr)
        {
            return;
        }

        if (host_layout->indexOf(menu_bar) < 0)
        {
            host_layout->insertWidget(0, menu_bar, 1, Qt::AlignLeft | Qt::AlignVCenter);
        }

        module_task_counter_container->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        if (host_layout->indexOf(module_task_counter_container) < 0)
        {
            host_layout->addWidget(module_task_counter_container, 0, Qt::AlignRight | Qt::AlignVCenter);
        }

        setMenuWidget(menu_bar_host);
        _sync_module_task_counter_geometry();
    }

    void EisenhowerMatrixApp::_sync_module_task_counter_geometry()
    {
        if (menu_bar == nullptr || menu_bar_host == nullptr || module_task_counter_container == nullptr || module_task_counter_label == nullptr)
        {
            return;
        }

        const int menu_bar_height =
            qMax(qMax(menu_bar->height(), menu_bar->sizeHint().height()), module_task_counter_label->sizeHint().height());
        if (menu_bar_height <= 0)
        {
            return;
        }

        module_task_counter_container->setFixedHeight(menu_bar_height);
        module_task_counter_label->setFixedHeight(menu_bar_height);
        menu_bar_host->setMinimumHeight(menu_bar_height);
    }

    void EisenhowerMatrixApp::clear_eisenhower_task_selections(QListWidget *exclude_list)
    {
        const QList<QListWidget *> lists = {
            quadrant1_list,
            quadrant2_list,
            quadrant3_list,
            quadrant4_list,
            quadrant1_in_progress_list,
            quadrant2_in_progress_list,
            quadrant3_in_progress_list,
            quadrant4_in_progress_list,
            quadrant1_completed_list,
            quadrant2_completed_list,
            quadrant3_completed_list,
            quadrant4_completed_list,
        };

        for (QListWidget *list : lists)
        {
            if (list != nullptr && list != exclude_list)
            {
                if (!list->selectedItems().isEmpty() || list->currentItem() != nullptr)
                {
                    if (QItemSelectionModel *sm = list->selectionModel(); sm != nullptr)
                    {
                        sm->clear();
                    }
                    else
                    {
                        list->clearSelection();
                    }
                }
            }
        }

        if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr)
        {
            QListWidget *cal_list = calendar_pane->calendar_panel->tasks_list;
            if (cal_list != nullptr && cal_list != exclude_list)
            {
                if (!cal_list->selectedItems().isEmpty() || cal_list->currentItem() != nullptr)
                {
                    if (QItemSelectionModel *sm = cal_list->selectionModel(); sm != nullptr)
                    {
                        sm->clear();
                    }
                    else
                    {
                        cal_list->clearSelection();
                    }
                }
            }
        }
    }

} // namespace source
