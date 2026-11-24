#pragma once

#include <QPoint>
#include <QPointer>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QWidget>

#include "source/modules/feynman/Feynman/feynman_support.hpp"

class QFrame;
class QGroupBox;
class QHBoxLayout;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QTextEdit;
class QComboBox;
class QTabWidget;
class QThread;
class QSplitter;
class QResizeEvent;

namespace source::GerenciamentoUI {
class RotatedTabButton;
}

namespace source::plugin::Feynman {

class FeynmanWorker;

class MetodoFeynman final : public QWidget {
    Q_OBJECT

public:
    explicit MetodoFeynman(QWidget *parent = nullptr);
    ~MetodoFeynman() override;

    void carregar_conceitos();
    void setup_ui();
    void atualizar_traducoes();
    void novo_conceito();
    void limpar_campos();
    void salvar_conceito_atual();
    void selecionar_conceito(QListWidgetItem *item);
    void deletar_conceito();
    void atualizar_lista();
    void atualizar_ciclos_em_progresso();
    void salvar_conceitos();
    void mostrar_menu_contexto(const QPoint &position);
    void editar_tarefa(QListWidgetItem *item);
    void redefinir_dominio(QListWidgetItem *item, int novo_nivel);
    void redefinir_prioridade(QListWidgetItem *item, int nova_prioridade);
    void vincular_arquivo_conceito(QListWidgetItem *item);
    void remover_arquivo_vinculado(QListWidgetItem *item);
    void receber_conceito_mapa_mental(const QVariantMap &dados);
    void receber_tarefa_compartilhada(const QVariantMap &dados);
    void on_task_duration_changed(const QString &task_id, double duration_days);
    void reset_in_progress_task_cycles(int new_cycles, const QVariantList &affected_tasks);
    void _on_conceitos_loaded(const QVariantList &conceitos);
    void _stop_feynman_thread();
    void cleanup();
    void limpar_abas();
    Q_INVOKABLE bool limpar_todas_as_tarefas();
    Q_INVOKABLE void limpar_selecao_tarefas();
    Q_INVOKABLE int total_task_count() const;
    Q_INVOKABLE int selected_task_count() const;
    Q_INVOKABLE QString selected_task_color() const;
    Q_INVOKABLE int apply_color_to_selected_tasks(const QString &color);
    Q_INVOKABLE QString ui_layout_mode_key() const;
    Q_INVOKABLE bool set_ui_layout_mode_key(const QString &mode_key);
    Q_INVOKABLE bool is_task_panel_expanded() const;
    Q_INVOKABLE void set_task_panel_expanded(bool expanded);
    void adicionar_botao_limpar();
    void abrir_arquivo_conceito(QListWidgetItem *item);
    void on_lista_conceitos_double_click(QListWidgetItem *item);
    void mostrar_tooltip_arquivo_vinculado(QListWidgetItem *item);
    void ocultar_tooltip_arquivo_vinculado();
    void configurar_tooltip_arquivo_vinculado();
    bool eventFilter(QObject *obj, QEvent *event) override;
    void atualizar_aba_descricao();
    QSize minimumSizeHint() const override;

    void ajustar_largura_combo_dominio();
    void ajustar_largura_combo_prioridade();
    void ajustar_altura_combos_footer();
    void atualizar_largura_minima_painel_esquerdo();
    void atualizar_largura_minima_painel_direito();
    void atualizar_dimensoes_minimas_iniciais();

signals:
    void task_counter_changed();

public:
    source::utils::LogManager *logger_ = nullptr;
    QString arquivo_conceitos_;
    QVariantList conceitos_;
    QVariantMap conceito_atual_;
    bool has_conceito_atual_ = false;
    compat::GerenciadorBotoesUI gerenciador_botoes_;
    FeynmanWorker *feynman_worker_ = nullptr;
    QThread *feynman_thread_ = nullptr;

    QLabel *label_meus_conceitos = nullptr;
    QGroupBox *frame_meus_conceitos = nullptr;
    QListWidget *lista_conceitos = nullptr;
    QPushButton *btn_novo = nullptr;
    QPushButton *btn_deletar = nullptr;
    QLabel *label_conceito = nullptr;
    QGroupBox *frame_conceito = nullptr;
    QTextEdit *input_titulo = nullptr;
    QLabel *label_passo1 = nullptr;
    QGroupBox *frame_explique = nullptr;
    QTextEdit *texto_explicacao = nullptr;
    QLabel *label_passo2 = nullptr;
    QGroupBox *frame_lacunas = nullptr;
    QTextEdit *texto_lacunas = nullptr;
    QLabel *label_passo3 = nullptr;
    QGroupBox *frame_revisao = nullptr;
    QTextEdit *texto_revisao = nullptr;
    QLabel *label_descricao = nullptr;
    QGroupBox *frame_descricao = nullptr;
    QTextEdit *texto_descricao = nullptr;
    QLabel *label_prioridade = nullptr;
    QComboBox *combo_prioridade = nullptr;
    QLabel *label_nivel = nullptr;
    QComboBox *combo_dominio = nullptr;
    QPushButton *btn_salvar = nullptr;
    QPushButton *btn_limpar = nullptr;
    QTabWidget *tab_widget = nullptr;
    bool sincronizando_descricao_ = false;
    bool descricao_editada_manualmente_ = false;
    QString descricao_gerada_atual_;
    QString pending_shared_origin_;
    QStringList pending_save_origins_;
    QString ui_layout_mode_key_ = QStringLiteral("tabs");

    QWidget *painel_esquerdo_ = nullptr;
    QWidget *painel_direito_ = nullptr;
    QSplitter *splitter_principal_ = nullptr;
    source::GerenciamentoUI::RotatedTabButton *btn_toggle_painel_tarefas_ = nullptr;
    QWidget *footer_container_ = nullptr;
    QHBoxLayout *footer_layout_ = nullptr;
    QHBoxLayout *footer_row_layout_ = nullptr;
    QPointer<QFrame> feynman_tooltip_widget_;
    bool painel_tarefas_expandido_ = false;
    int largura_painel_tarefas_ = 0;
    int painel_esquerdo_min_base_ = 0;
    int painel_direito_min_base_ = 0;

private:
    QVariantMap obter_conceito_do_item(QListWidgetItem *item) const;
    int obter_indice_conceito_do_item(QListWidgetItem *item) const;
    int encontrar_indice_conceito(const QString &titulo, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive) const;
    int encontrar_indice_conceito_integrado(const QString &origem, const QString &task_id) const;
    void atualizar_arquivo_vinculado(QListWidgetItem *item, const QString &caminho);
    QVariantMap construir_payload_tarefa_compartilhada(QListWidgetItem *item, bool *metadata_changed = nullptr);
    void reset_layout_widgets();
    int largura_minima_abas() const;
    int largura_minima_efetiva_painel_tarefas() const;
    void alternar_painel_tarefas(bool abrir_se_oculto = false);
    void sincronizar_splitter_painel_tarefas(bool preservar_atual = true);
    void atualizar_interacao_splitter_painel_tarefas();
    void aplicar_larguras_minimas_paineis();
    void atualizar_placeholders_dinamicos();


protected:
    void resizeEvent(QResizeEvent *event) override;
};

}  // namespace source::plugin::Feynman
