#pragma once

#include <memory>

#include <QColor>
#include <QHash>
#include <QList>
#include <QPointer>
#include <QSet>
#include <QVariantMap>
#include <QWidget>

class QHBoxLayout;
class QLabel;
class QPushButton;
class QGroupBox;
class QEvent;
class QTimer;

namespace source::plugin::MapaMental {

class LinhaConexao;
class MapaScene;
class NoConceito;
class ProcessadorIA;
class ProgressoMapaMental;
class SmoothGraphicsView;

class MapaMental final : public QWidget {
    Q_OBJECT

public:
    enum class LayoutMode {
        Vertical,
        Horizontal,
        Radial,
        Tree,
    };
    Q_ENUM(LayoutMode)

    explicit MapaMental(QWidget *parent = nullptr);
    ~MapaMental() override;

    bool modo_conexao() const { return modo_conexao_; }
    LayoutMode layout_mode() const { return layout_mode_; }
    QString layout_mode_key() const;
    void set_layout_mode(LayoutMode mode, bool persist = true);
    void set_layout_mode_key(const QString &mode_key, bool persist = true);
    QString summary_method_key() const;
    void set_summary_method_key(const QString &method_key, bool persist = true);
    void traduzir_botoes_padrao(QObject *dialogo) const;

    void setup_ui();
    void atualizar_traducoes();
    void adicionar_no();
    NoConceito *adicionar_no(const QString &texto, const QPointF &pos, const QColor &cor = QColor("#0078d4"));
    void toggle_modo_conexao();
    void clicar_no_para_conectar(NoConceito *no);
    void mudar_cor();
    void salvar_mapa();
    void carregar_mapa();
    void carregar_backup_automatico();
    void exportar_imagem();
    void limpar_mapa(bool emitir_mensagem = true);
    void atualizar_tema();
    void importar_documento_ia();
    void reorganizar_com_ia();
    void restaurar_com_ia();
    void focar_no_para_qa(NoConceito *no);
    void iniciar_visao_cinematica();
    void clicar_no_para_expandir_hierarquia(NoConceito *no);
    void remover_no(NoConceito *no);
    void remover_nos(const QList<NoConceito *> &nos);
    QList<NoConceito *> selected_nodes() const;
    const QList<NoConceito *> &nos() const { return nos_; }
    void refresh_counter();
    void refresh_fonts();
    void salvar_backup_automatico();

    Q_INVOKABLE int total_task_count() const;
    Q_INVOKABLE int selected_task_count() const;

signals:
    void task_counter_changed();

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void criar_conexao(NoConceito *inicio, NoConceito *fim);
    bool conexao_existe(NoConceito *inicio, NoConceito *fim) const;
    QVariantMap serializar_mapa() const;
    bool salvar_mapa_em_arquivo(const QString &caminho) const;
    bool carregar_mapa_de_arquivo(const QString &caminho);
    bool processar_documento_com_ia(const QString &caminho, const QString &titulo_janela);
    bool processar_texto_com_ia(const QString &texto,
                                const QString &titulo_janela,
                                ProgressoMapaMental *progresso = nullptr);
    void gerar_mapa_de_hierarquia(const QVariantMap &hierarquia);
    void gerar_mapa_de_hierarquia(const QVariantMap &hierarquia, ProgressoMapaMental *progresso);
    NoConceito *gerar_no_hierarquia(const QVariantMap &item, int nivel, int &linha, NoConceito *pai);
    NoConceito *gerar_no_hierarquia(const QVariantMap &item,
                                    int nivel,
                                    int &linha,
                                    NoConceito *pai,
                                    ProgressoMapaMental *progresso,
                                    int total_nos,
                                    int &nos_gerados);
    void aplicar_layout_arvore();
    void habilitar_navegacao_hierarquia(NoConceito *raiz_no);
    void atualizar_visibilidade_linhas();
    void resetar_navegacao_hierarquia();
    void expandir_area_se_necessario(NoConceito *no);
    QString backup_path() const;
    void carregar_preferencia_layout();
    void salvar_preferencia_layout() const;

    MapaScene *scene_ = nullptr;
    SmoothGraphicsView *view_ = nullptr;
    QGroupBox *controles_group_ = nullptr;
    QLabel *status_label_ = nullptr;
    QPushButton *btn_conectar_ = nullptr;
    QTimer *backup_timer_ = nullptr;
    QTimer *cinematic_timer_ = nullptr;
    ProcessadorIA *processador_ia_ = nullptr;

    QList<NoConceito *> nos_;
    QList<LinhaConexao *> conexoes_;
    QList<NoConceito *> nos_removidos_;
    QList<LinhaConexao *> conexoes_removidas_;
    QHash<NoConceito *, std::shared_ptr<bool>> no_animation_guards_;
    QHash<LinhaConexao *, std::shared_ptr<bool>> conexao_animation_guards_;
    bool modo_conexao_ = false;
    NoConceito *no_origem_ = nullptr;
    QColor cor_atual_ = QColor("#0078d4");
    LayoutMode layout_mode_ = LayoutMode::Vertical;
    QString summary_method_key_ = QStringLiteral("qwen");
    int cinematic_index_ = 0;
    bool navegacao_hierarquia_ativa_ = false;
    QHash<NoConceito *, QList<NoConceito *>> hierarquia_filhos_;
    QHash<NoConceito *, NoConceito *> hierarquia_pais_;
    QSet<NoConceito *> hierarquia_expandida_;
    NoConceito *hierarquia_raiz_ = nullptr;
    int layout_animation_generation_ = 0;
    bool atualizando_tema_ = false;
    bool carregando_em_lote_ = false;
};

}  // namespace source::plugin::MapaMental
