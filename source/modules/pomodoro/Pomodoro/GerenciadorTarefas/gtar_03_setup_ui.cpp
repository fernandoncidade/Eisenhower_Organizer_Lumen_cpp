#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_03_setup_ui.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

bool GerenciadorTarefas::_layout_robusto() const
{
    return is_robust_ui_layout(ui_layout_mode_key_);
}

void GerenciadorTarefas::setup_ui()
{
    try {
        auto *layout = new QVBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(6);
        layout->setSizeConstraint(QLayout::SetDefaultConstraint);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setMinimumHeight(0);

        quadro_tarefas = new QGroupBox(this);
        quadro_tarefas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        auto *quadro_layout = new QVBoxLayout(quadro_tarefas);
        quadro_layout->setContentsMargins(8, 8, 8, 8);
        quadro_layout->setSpacing(6);

        auto *linha_entrada = new QHBoxLayout();
        linha_entrada->setSpacing(6);
        input_tarefa = new FileDropLineEdit(this, quadro_tarefas);
        connect(input_tarefa, &QLineEdit::returnPressed, this, &GerenciadorTarefas::adicionar_tarefa);
        connect(input_tarefa, &FileDropLineEdit::fileDropped, this, [this](const QString &path) {
            dragged_file_path_ = path;
            input_tarefa->setToolTip(path);
        });
        input_tarefa->setMinimumHeight(qMax(24, input_tarefa->fontMetrics().height() + 10));
        linha_entrada->addWidget(input_tarefa);
        quadro_layout->addLayout(linha_entrada);

        const bool robust_layout = _layout_robusto();
        btn_adicionar = gerenciador_botoes_.create_button();
        connect(btn_adicionar, &QPushButton::clicked, this, &GerenciadorTarefas::adicionar_tarefa);

        label_prioridade = new QLabel(quadro_tarefas);
        combo_prioridade = new QComboBox(quadro_tarefas);
        label_dominio = new QLabel(quadro_tarefas);
        combo_dominio = new QComboBox(quadro_tarefas);
        label_fluxo_tarefas_concluidas = robust_layout ? new QLabel(quadro_tarefas) : nullptr;

        input_ref_height_ = qMax(1, btn_adicionar->sizeHint().height());
        input_tarefa->setFixedHeight(input_ref_height_);
        combo_prioridade->setFixedHeight(input_ref_height_);
        combo_dominio->setFixedHeight(input_ref_height_);

        btn_mover = gerenciador_botoes_.create_button();
        connect(btn_mover, &QPushButton::clicked, this, &GerenciadorTarefas::mover_tarefas_marcadas);

        btn_remover = gerenciador_botoes_.create_button();
        connect(btn_remover, &QPushButton::clicked, this, &GerenciadorTarefas::remover_tarefas_marcadas);

        btn_resetar_contagem = gerenciador_botoes_.create_button();
        connect(btn_resetar_contagem, &QPushButton::clicked, this, &GerenciadorTarefas::resetar_contagem_botao);

        chk_fluxo_tarefas_concluidas = new IndicatorOnlyCheckBox(quadro_tarefas);
        chk_fluxo_tarefas_concluidas->setChecked(true);
        chk_fluxo_tarefas_concluidas->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(chk_fluxo_tarefas_concluidas, &QCheckBox::toggled, this, [this](bool) { ajustar_linha_botoes(); });

        botoes_linha_principal_ = {btn_adicionar, btn_mover, btn_remover, btn_resetar_contagem};

        if (robust_layout) {
            auto *settings_row = new QHBoxLayout();
            settings_row->setContentsMargins(0, 0, 0, 0);
            settings_row->setSpacing(6);
            settings_row->addWidget(label_prioridade, 0, Qt::AlignVCenter);
            settings_row->addWidget(combo_prioridade, 0, Qt::AlignVCenter);
            settings_row->addSpacing(6);
            settings_row->addWidget(label_dominio, 0, Qt::AlignVCenter);
            settings_row->addWidget(combo_dominio, 0, Qt::AlignVCenter);
            settings_row->addSpacing(6);
            settings_row->addWidget(chk_fluxo_tarefas_concluidas, 0, Qt::AlignVCenter);
            settings_row->addWidget(label_fluxo_tarefas_concluidas, 0, Qt::AlignVCenter);
            settings_row->addStretch(1);
            quadro_layout->addLayout(settings_row);

            auto *buttons_row = new QHBoxLayout();
            buttons_row->setContentsMargins(0, 0, 0, 0);
            buttons_row->setSpacing(6);
            buttons_row->addWidget(btn_adicionar);
            buttons_row->addWidget(btn_mover);
            buttons_row->addWidget(btn_remover);
            buttons_row->addWidget(btn_resetar_contagem);
            buttons_row->addStretch(1);
            quadro_layout->addLayout(buttons_row);
        } else {
            auto *controls_row = new QHBoxLayout();
            controls_row->setContentsMargins(0, 0, 0, 0);
            controls_row->setSpacing(12);

            auto *buttons_column = new QVBoxLayout();
            buttons_column->setContentsMargins(0, 0, 0, 0);
            buttons_column->setSpacing(6);
            buttons_column->addWidget(btn_adicionar);
            buttons_column->addWidget(btn_mover);
            buttons_column->addWidget(btn_remover);
            buttons_column->addWidget(btn_resetar_contagem);
            buttons_column->addStretch(1);
            controls_row->addLayout(buttons_column, 0);

            auto *settings_column = new QVBoxLayout();
            settings_column->setContentsMargins(0, 0, 0, 0);
            settings_column->setSpacing(6);
            settings_column->addWidget(chk_fluxo_tarefas_concluidas, 0, Qt::AlignLeft);
            settings_column->addWidget(label_prioridade);
            settings_column->addWidget(combo_prioridade);
            settings_column->addWidget(label_dominio);
            settings_column->addWidget(combo_dominio);
            settings_column->addStretch(1);
            controls_row->addStretch(1);
            controls_row->addLayout(settings_column, 0);
            quadro_layout->addLayout(controls_row);
        }

        col_todo = criar_coluna(QStringLiteral("Todo"));
        col_doing = criar_coluna(QStringLiteral("Doing"));
        col_done = criar_coluna(QStringLiteral("Done"));

        const QString panel_layout = normalized_panel_layout_key(ui_layout_mode_key_);
        if (panel_layout == QStringLiteral("tabs")) {
            status_tabs = new AutoOpenTabWidget(this, this);
            status_tabs->setObjectName(QStringLiteral("pomodoroTaskStatusTabs"));
            status_tabs->setStyleSheet(QStringLiteral("QTabWidget#pomodoroTaskStatusTabs::pane { border: 0px; }"));
            source::utils::AccentTabStyle::install(status_tabs);
            status_tabs->addTab(col_todo, QStringLiteral("📋 A Fazer"));
            status_tabs->addTab(col_doing, QStringLiteral("⚙️ Em Progresso"));
            status_tabs->addTab(col_done, QStringLiteral("✅ Concluído"));
            source::utils::AccentTabStyle::set_tab_accent(status_tabs, 0, source::utils::AccentTabStyle::accent_blue());
            source::utils::AccentTabStyle::set_tab_accent(status_tabs, 1, source::utils::AccentTabStyle::accent_yellow());
            source::utils::AccentTabStyle::set_tab_accent(status_tabs, 2, source::utils::AccentTabStyle::accent_green());
            status_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            status_tabs->setMinimumHeight(0);
            layout->addWidget(status_tabs, 1);
        } else {
            status_tabs = nullptr;
            auto *status_scroll = new QScrollArea(this);
            status_scroll->setWidgetResizable(true);
            status_scroll->setFrameShape(QFrame::NoFrame);
            status_scroll->setHorizontalScrollBarPolicy(panel_layout == QStringLiteral("horizontal")
                                                            ? Qt::ScrollBarAsNeeded
                                                            : Qt::ScrollBarAlwaysOff);
            status_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            status_scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            status_scroll->setMinimumHeight(0);

            auto *content = new QWidget(status_scroll);
            content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            if (panel_layout == QStringLiteral("horizontal")) {
                auto *status_layout = new QHBoxLayout(content);
                status_layout->setContentsMargins(0, 0, 0, 0);
                status_layout->setSpacing(6);

                const QList<KanbanColumn *> columns = {col_todo, col_doing, col_done};
                for (KanbanColumn *column : columns) {
                    if (column != nullptr) {
                        column->setMinimumWidth(260);
                        status_layout->addWidget(column, 1);
                    }
                }
                content->setMinimumWidth(3 * 260 + 2 * status_layout->spacing());
            } else {
                auto *status_layout = new QVBoxLayout(content);
                status_layout->setContentsMargins(0, 0, 0, 0);
                status_layout->setSpacing(6);

                const QList<KanbanColumn *> columns = {col_todo, col_doing, col_done};
                for (KanbanColumn *column : columns) {
                    if (column != nullptr) {
                        column->setMinimumHeight(180);
                        status_layout->addWidget(column, 1);
                    }
                }
            }

            status_scroll->setWidget(content);
            layout->addWidget(status_scroll, 1);
        }

        setLayout(layout);
        atualizar_traducoes();
        ajustar_linha_botoes();
        atualizar_listas();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao configurar interface do GerenciadorTarefas: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao configurar interface do GerenciadorTarefas"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void setup_ui(GerenciadorTarefas &self) { self.setup_ui(); } }
