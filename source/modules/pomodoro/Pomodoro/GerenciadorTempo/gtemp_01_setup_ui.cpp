#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_01_setup_ui.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

void GerenciadorTempo::setup_ui()
{
    try {
        reset_layout_widgets();
        if (_layout_robusto()) {
            setup_ui_robusta();
        } else {
            setup_ui_simples();
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao configurar interface do GerenciadorTempo: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao configurar interface do GerenciadorTempo"), true);
    }
}

void GerenciadorTempo::reset_layout_widgets()
{
    if (pomodoro != nullptr && tempo_worker_ != nullptr) {
        disconnect(pomodoro, &PomodoroTimer::ciclo_completado, tempo_worker_, &_TimeWorker::on_pomodoro);
    }

    if (QLayout *old_layout = layout()) {
        clear_layout(old_layout);
        delete old_layout;
    }

    pomodoro = nullptr;
    tarefas = nullptr;
    tarefas_group = nullptr;
    col_doing_timer_group = nullptr;
    col_doing_timer_list = nullptr;
    left_widget = nullptr;
    main_splitter = nullptr;
    btn_toggle_painel_em_progresso_ = nullptr;
    ajustando_geometria_ = false;
    largura_inicial_painel_em_progresso_capturada_ = false;
    divisao_inicial_splitter_aplicada_ = false;
    usuario_ajustou_splitter_ = false;
}

bool GerenciadorTempo::_layout_robusto() const
{
    return is_robust_ui_layout(ui_layout_mode_key_);
}

void GerenciadorTempo::setup_ui_simples()
{
    auto *layout = new QVBoxLayout();
    layout->setContentsMargins(8, 6, 8, 8);
    layout->setSpacing(8);

    left_widget = new QWidget(this);
    left_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *left_col = new QVBoxLayout(left_widget);
    left_col->setContentsMargins(0, 0, 0, 0);
    left_col->setSpacing(8);
    left_col->setSizeConstraint(QLayout::SetMinimumSize);

    pomodoro = new PomodoroTimer(ui_layout_mode_key_, this);
    pomodoro->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    {
        const int timer_min_height = qMax(pomodoro->minimumHeight(), pomodoro->minimumSizeHint().height());
        const int timer_max_height = qBound(260, pomodoro->sizeHint().height(), qMax(360, available_window_size(this).height() / 2));
        pomodoro->setMaximumHeight(qMax(timer_max_height, timer_min_height));
    }
    connect(pomodoro, &PomodoroTimer::ciclo_completado, tempo_worker_, &_TimeWorker::on_pomodoro);
    left_col->addWidget(pomodoro, 0, Qt::AlignTop);

    tarefas = new GerenciadorTarefas(ui_layout_mode_key_, this);
    tarefas->set_pomodoro_timer(pomodoro);
    pomodoro->set_gerenciador_tarefas(tarefas);
    connect(pomodoro, &PomodoroTimer::ciclos_permitidos_alterados, tarefas, &GerenciadorTarefas::on_default_ciclos_changed);
    tarefas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tarefas->setMinimumHeight(0);
    tarefas_group = tarefas->quadro_tarefas;
    if (tarefas_group != nullptr) {
        tarefas_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        left_col->addSpacing(12);
        left_col->addWidget(tarefas_group, 0, Qt::AlignTop);
    }

    btn_toggle_painel_em_progresso_ =
        new source::GerenciamentoUI::RotatedTabButton(collapse_in_progress_panel_text(this), this);
    btn_toggle_painel_em_progresso_->setRotationDirection(source::GerenciamentoUI::RotatedTabButton::RotationDirection::None);
    btn_toggle_painel_em_progresso_->setSizeReferenceTexts(
        {expand_in_progress_panel_text(this), collapse_in_progress_panel_text(this)});
    btn_toggle_painel_em_progresso_->setClickHandler([this]() { _alternar_painel_em_progresso(); });

    auto *in_progress_header = new QHBoxLayout();
    in_progress_header->setContentsMargins(0, 0, 0, 0);
    in_progress_header->setSpacing(6);
    in_progress_header->addStretch(1);
    in_progress_header->addWidget(btn_toggle_painel_em_progresso_, 0, Qt::AlignRight);
    left_col->addLayout(in_progress_header);

    col_doing_timer_group = new QGroupBox(this);
    col_doing_timer_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *doing_layout = new QVBoxLayout(col_doing_timer_group);
    doing_layout->setContentsMargins(8, 8, 8, 8);
    col_doing_timer_list = new QListWidget(col_doing_timer_group);
    col_doing_timer_list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    col_doing_timer_list->setMinimumHeight(0);
    col_doing_timer_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    col_doing_timer_list->setSelectionBehavior(QAbstractItemView::SelectItems);
    col_doing_timer_list->setSelectionRectVisible(true);
    col_doing_timer_list->setContextMenuPolicy(Qt::CustomContextMenu);
    col_doing_timer_list->setDragDropMode(QListWidget::NoDragDrop);
    source::utils::install_linked_title_underline_delegate(col_doing_timer_list);
    connect(col_doing_timer_list, &QListWidget::itemDoubleClicked, this, &GerenciadorTempo::_abrir_arquivo_vinculado_coluna_doing_timer);
    connect(col_doing_timer_list,
            &QListWidget::customContextMenuRequested,
            this,
            &GerenciadorTempo::_mostrar_menu_contexto_coluna_doing_timer);
    connect(col_doing_timer_list, &QListWidget::itemSelectionChanged, this, [this]() {
        if (tarefas != nullptr && col_doing_timer_list != nullptr && !col_doing_timer_list->selectedItems().isEmpty()) {
            tarefas->limpar_selecao_tarefas();
        }
        emit task_counter_changed();
    });
    doing_layout->addWidget(col_doing_timer_list);

    col_doing_timer_group->setVisible(painel_em_progresso_expandido_);
    left_col->addWidget(col_doing_timer_group, 1);

    auto *spacer = new QWidget(this);
    spacer->setObjectName(QStringLiteral("simple_layout_spacer"));
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    spacer->setVisible(!painel_em_progresso_expandido_);
    left_col->addWidget(spacer, 1);

    connect(tarefas, &GerenciadorTarefas::listas_atualizadas, this, &GerenciadorTempo::atualizar_coluna_doing_timer);
    connect(tarefas, &GerenciadorTarefas::listas_atualizadas, this, &GerenciadorTempo::task_counter_changed);
    connect(tarefas, &GerenciadorTarefas::tarefa_adicionada, this, &GerenciadorTempo::_on_tarefa_adicionada_gestao_tempo);
    if (tarefas->col_todo != nullptr && tarefas->col_todo->lista != nullptr) {
        connect(tarefas->col_todo->lista, &QListWidget::itemSelectionChanged, this, &GerenciadorTempo::task_counter_changed);
    }
    if (tarefas->col_doing != nullptr && tarefas->col_doing->lista != nullptr) {
        connect(tarefas->col_doing->lista, &QListWidget::itemSelectionChanged, this, &GerenciadorTempo::task_counter_changed);
    }
    if (tarefas->col_done != nullptr && tarefas->col_done->lista != nullptr) {
        connect(tarefas->col_done->lista, &QListWidget::itemSelectionChanged, this, &GerenciadorTempo::task_counter_changed);
    }
    main_splitter = new QSplitter(Qt::Horizontal, this);
    main_splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    main_splitter->setChildrenCollapsible(false);
    main_splitter->addWidget(left_widget);
    main_splitter->addWidget(tarefas);
    main_splitter->setCollapsible(0, false);
    main_splitter->setCollapsible(1, false);
    main_splitter->setStretchFactor(0, 1);
    main_splitter->setStretchFactor(1, 1);
    main_splitter->setHandleWidth(kTempoSplitterExpandedHandleWidth);
    main_splitter->setOpaqueResize(true);
    connect(main_splitter, &QSplitter::splitterMoved, this, [this](int, int) {
        if (ajustando_geometria_) {
            return;
        }
        usuario_ajustou_splitter_ = true;
        QScopedValueRollback<bool> guard(ajustando_geometria_, true);
        _ajustar_largura_minima_coluna_esquerda();
        _ajustar_limites_splitter_principal();
    });

    _ajustar_largura_minima_coluna_esquerda();
    tarefas->setMinimumWidth(_largura_minima_coluna_direita_efetiva());
    {
        QSignalBlocker blocker(main_splitter);
        main_splitter->setSizes({1, 1});
    }
    _ajustar_limites_splitter_principal();

    auto *content_row = new QHBoxLayout();
    content_row->setContentsMargins(0, 0, 0, 0);
    content_row->setSpacing(0);
    content_row->addWidget(main_splitter, 1);

    layout->addLayout(content_row, 1);
    setLayout(layout);
    setMinimumHeight(bounded_module_min_height(this));

    atualizar_traducoes();
    _ajustar_limites_verticais();
    _atualizar_interacao_painel_em_progresso();
    QTimer::singleShot(0, this, [this]() { _aplicar_divisao_inicial_splitter(); });
    QTimer::singleShot(50, this, [this]() { _aplicar_divisao_inicial_splitter(); });
    QTimer::singleShot(150, this, [this]() { _aplicar_divisao_inicial_splitter(); });
    atualizar_coluna_doing_timer();
}

void GerenciadorTempo::setup_ui_robusta()
{
    auto *layout = new QVBoxLayout();
    layout->setContentsMargins(8, 6, 8, 8);
    layout->setSpacing(8);

    left_widget = new QWidget(this);
    left_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *left_col = new QVBoxLayout(left_widget);
    left_col->setContentsMargins(0, 0, 0, 0);
    left_col->setSpacing(8);
    left_col->setSizeConstraint(QLayout::SetMinimumSize);

    pomodoro = new PomodoroTimer(ui_layout_mode_key_, this);
    pomodoro->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    {
        const int timer_min_height = qMax(pomodoro->minimumHeight(), pomodoro->minimumSizeHint().height());
        const int timer_max_height = qBound(200, pomodoro->sizeHint().height(), qMax(220, available_window_size(this).height() / 3));
        pomodoro->setMaximumHeight(qMax(timer_max_height, timer_min_height));
    }
    connect(pomodoro, &PomodoroTimer::ciclo_completado, tempo_worker_, &_TimeWorker::on_pomodoro);
    left_col->addWidget(pomodoro, 0, Qt::AlignTop);

    tarefas = new GerenciadorTarefas(ui_layout_mode_key_, this);
    tarefas->set_pomodoro_timer(pomodoro);
    pomodoro->set_gerenciador_tarefas(tarefas);
    connect(pomodoro, &PomodoroTimer::ciclos_permitidos_alterados, tarefas, &GerenciadorTarefas::on_default_ciclos_changed);
    tarefas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tarefas->setMinimumHeight(0);
    tarefas_group = tarefas->quadro_tarefas;
    if (tarefas_group != nullptr) {
        tarefas_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        left_col->addSpacing(12);
        left_col->addWidget(tarefas_group, 0, Qt::AlignTop);
    }
    left_col->addWidget(tarefas, 1);

    connect(tarefas, &GerenciadorTarefas::listas_atualizadas, this, &GerenciadorTempo::atualizar_coluna_doing_timer);
    connect(tarefas, &GerenciadorTarefas::listas_atualizadas, this, &GerenciadorTempo::task_counter_changed);
    connect(tarefas, &GerenciadorTarefas::tarefa_adicionada, this, &GerenciadorTempo::_on_tarefa_adicionada_gestao_tempo);
    if (tarefas->col_todo != nullptr && tarefas->col_todo->lista != nullptr) {
        connect(tarefas->col_todo->lista, &QListWidget::itemSelectionChanged, this, &GerenciadorTempo::task_counter_changed);
    }
    if (tarefas->col_doing != nullptr && tarefas->col_doing->lista != nullptr) {
        connect(tarefas->col_doing->lista, &QListWidget::itemSelectionChanged, this, &GerenciadorTempo::task_counter_changed);
    }
    if (tarefas->col_done != nullptr && tarefas->col_done->lista != nullptr) {
        connect(tarefas->col_done->lista, &QListWidget::itemSelectionChanged, this, &GerenciadorTempo::task_counter_changed);
    }

    col_doing_timer_group = new QGroupBox(this);
    col_doing_timer_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *doing_layout = new QVBoxLayout(col_doing_timer_group);
    doing_layout->setContentsMargins(8, 8, 8, 8);
    col_doing_timer_list = new QListWidget(col_doing_timer_group);
    col_doing_timer_list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    col_doing_timer_list->setMinimumHeight(0);
    col_doing_timer_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    col_doing_timer_list->setSelectionBehavior(QAbstractItemView::SelectItems);
    col_doing_timer_list->setSelectionRectVisible(true);
    col_doing_timer_list->setContextMenuPolicy(Qt::CustomContextMenu);
    col_doing_timer_list->setDragDropMode(QListWidget::NoDragDrop);
    source::utils::install_linked_title_underline_delegate(col_doing_timer_list);
    connect(col_doing_timer_list, &QListWidget::itemDoubleClicked, this, &GerenciadorTempo::_abrir_arquivo_vinculado_coluna_doing_timer);
    connect(col_doing_timer_list,
            &QListWidget::customContextMenuRequested,
            this,
            &GerenciadorTempo::_mostrar_menu_contexto_coluna_doing_timer);
    connect(col_doing_timer_list, &QListWidget::itemSelectionChanged, this, [this]() {
        if (tarefas != nullptr && col_doing_timer_list != nullptr && !col_doing_timer_list->selectedItems().isEmpty()) {
            tarefas->limpar_selecao_tarefas();
        }
        emit task_counter_changed();
    });
    doing_layout->addWidget(col_doing_timer_list);

    main_splitter = new QSplitter(Qt::Horizontal, this);
    main_splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    main_splitter->setChildrenCollapsible(false);
    main_splitter->addWidget(left_widget);
    main_splitter->addWidget(col_doing_timer_group);
    main_splitter->setCollapsible(0, false);
    main_splitter->setCollapsible(1, false);
    main_splitter->setStretchFactor(0, 3);
    main_splitter->setStretchFactor(1, 1);
    main_splitter->setHandleWidth(kTempoSplitterExpandedHandleWidth);
    main_splitter->setOpaqueResize(true);
    connect(main_splitter, &QSplitter::splitterMoved, this, [this](int, int) {
        if (ajustando_geometria_) {
            return;
        }
        QScopedValueRollback<bool> guard(ajustando_geometria_, true);
        _ajustar_largura_minima_coluna_esquerda();
        _ajustar_limites_splitter_principal();
        _registrar_largura_atual_painel_em_progresso();
    });

    _ajustar_largura_minima_coluna_esquerda();
    col_doing_timer_group->setMinimumWidth(240);
    main_splitter->setSizes({1080, kTempoDefaultRightPanelWidth});
    col_doing_timer_group->setVisible(painel_em_progresso_expandido_);
    _ajustar_limites_splitter_principal();

    btn_toggle_painel_em_progresso_ =
        new source::GerenciamentoUI::RotatedTabButton(collapse_in_progress_panel_text(this), this);
    btn_toggle_painel_em_progresso_->setMinimumWidth(25);
    btn_toggle_painel_em_progresso_->setSizeReferenceTexts(
        {expand_in_progress_panel_text(this), collapse_in_progress_panel_text(this)});
    btn_toggle_painel_em_progresso_->setRotationDirection(
        source::GerenciamentoUI::RotatedTabButton::RotationDirection::CounterClockwise90);
    btn_toggle_painel_em_progresso_->setClickHandler([this]() { _alternar_painel_em_progresso(); });

    auto *content_row = new QHBoxLayout();
    content_row->setContentsMargins(0, 0, 0, 0);
    content_row->setSpacing(0);
    content_row->addWidget(main_splitter, 1);
    content_row->addWidget(btn_toggle_painel_em_progresso_, 0, Qt::AlignRight | Qt::AlignTop);

    layout->addLayout(content_row, 1);
    setLayout(layout);
    setMinimumHeight(bounded_module_min_height(this));

    atualizar_traducoes();
    _ajustar_limites_verticais();
    _atualizar_interacao_painel_em_progresso();
    QTimer::singleShot(0, this, [this]() { _estabilizar_largura_painel_em_progresso(); });
    QTimer::singleShot(50, this, [this]() { _estabilizar_largura_painel_em_progresso(); });
    atualizar_coluna_doing_timer();
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns { void setup_ui(GerenciadorTempo &self) { self.setup_ui(); } }
