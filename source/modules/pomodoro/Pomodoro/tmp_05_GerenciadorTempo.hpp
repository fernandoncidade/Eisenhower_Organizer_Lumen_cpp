#pragma once

#include <QPoint>
#include <QSet>
#include <QString>
#include <QWidget>

#include "source/modules/pomodoro/Pomodoro/pomodoro_support.hpp"

class QAbstractButton;
class QGroupBox;
class QListWidget;
class QListWidgetItem;
class QResizeEvent;
class QShowEvent;
class QSplitter;
class QThread;

namespace source::GerenciamentoUI {
class RotatedTabButton;
}

namespace source::plugin::Pomodoro {

class PomodoroTimer;
class GerenciadorTarefas;
class _TimeWorker;

class GerenciadorTempo : public QWidget {
    Q_OBJECT

public:
    explicit GerenciadorTempo(QWidget *parent = nullptr);
    ~GerenciadorTempo() override;

    void setup_ui();
    void _atualizar_rotulo_botao_eisenhower();
    void _setup_integracao_eisenhower();
    void _on_tarefa_adicionada_gestao_tempo(const QVariantMap &dados);
    void _on_event_bus_tarefa(const QVariantMap &dados);
    void _aplicar_tema_dinamico_inputs();
    bool eventFilter(QObject *obj, QEvent *event) override;
    void atualizar_traducoes();

protected:
    void changeEvent(QEvent *event) override;

public:
    void registrar_pomodoro(const QString &tipo);
    void cleanup();
    QSize minimumSizeHint() const override;
    Q_INVOKABLE QString ui_layout_mode_key() const;
    Q_INVOKABLE bool set_ui_layout_mode_key(const QString &mode_key);
    Q_INVOKABLE bool limpar_todas_as_tarefas();
    Q_INVOKABLE void limpar_selecao_tarefas();
    Q_INVOKABLE int total_task_count() const;
    Q_INVOKABLE int selected_task_count() const;
    Q_INVOKABLE QString selected_task_color() const;
    Q_INVOKABLE int apply_color_to_selected_tasks(const QString &color);
    Q_INVOKABLE bool is_in_progress_panel_expanded() const;
    Q_INVOKABLE void set_in_progress_panel_expanded(bool expanded);
    void atualizar_coluna_doing_timer();
    void setup_ui_simples();
    void setup_ui_robusta();
    void reset_layout_widgets();
    bool _layout_robusto() const;
    int _largura_minima_coluna_esquerda_efetiva() const;
    int _largura_minima_coluna_direita_efetiva() const;
    bool _aplicar_divisao_inicial_splitter();
    void _ajustar_largura_minima_coluna_esquerda();
    void _ajustar_limites_splitter_principal();
    int _largura_minima_efetiva_painel_em_progresso() const;
    bool _capturar_largura_inicial_painel_em_progresso();
    void _registrar_largura_atual_painel_em_progresso();
    void _estabilizar_largura_painel_em_progresso();
    void _alternar_painel_em_progresso(bool abrir_se_oculto = false);
    void _sincronizar_splitter_painel_em_progresso(bool preserve_current = true);
    void _atualizar_interacao_painel_em_progresso();
    void _ajustar_limites_verticais();
    void _abrir_arquivo_vinculado_coluna_doing_timer(QListWidgetItem *item);
    void _sincronizar_estado_coluna_doing_timer();
    void _mostrar_menu_contexto_coluna_doing_timer(const QPoint &pos);

signals:
    void task_counter_changed();

public:
    source::utils::LogManager *logger_ = nullptr;
    QSet<QString> integracao_ids_recebidos_;
    QThread *tempo_thread_ = nullptr;
    _TimeWorker *tempo_worker_ = nullptr;
    compat::EventBus *event_bus_ = nullptr;

    PomodoroTimer *pomodoro = nullptr;
    GerenciadorTarefas *tarefas = nullptr;
    QGroupBox *tarefas_group = nullptr;
    QGroupBox *col_doing_timer_group = nullptr;
    QListWidget *col_doing_timer_list = nullptr;
    QWidget *left_widget = nullptr;
    QSplitter *main_splitter = nullptr;
    source::GerenciamentoUI::RotatedTabButton *btn_toggle_painel_em_progresso_ = nullptr;
    QAbstractButton *btn_eisenhower_integrado = nullptr;
    bool ajustando_geometria_ = false;
    bool painel_em_progresso_expandido_ = false;
    bool largura_inicial_painel_em_progresso_capturada_ = false;
    bool divisao_inicial_splitter_aplicada_ = false;
    bool usuario_ajustou_splitter_ = false;
    int largura_painel_em_progresso_ = 0;
    QString ui_layout_mode_key_ = QStringLiteral("simple:tabs");

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
};

}  // namespace source::plugin::Pomodoro
