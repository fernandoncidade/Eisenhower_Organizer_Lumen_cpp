#pragma once

#include <QGroupBox>
#include <QPoint>
#include <QPointer>
#include <QString>
#include <QVector>
#include <QWidget>

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_26_FileDropLineEdit.hpp"
#include "source/modules/pomodoro/Pomodoro/pomodoro_support.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_01_Tarefa.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_04_KanbanListWidget.hpp"

class QCheckBox;
class QComboBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QTabWidget;

namespace source::plugin::Pomodoro {

class KanbanColumn : public QGroupBox {
public:
    explicit KanbanColumn(const QString &status_value, QWidget *parent = nullptr);

    _KanbanListWidget *lista = nullptr;
    QString status;
};

class PomodoroTimer;

class GerenciadorTarefas : public QWidget {
    Q_OBJECT

public:
    explicit GerenciadorTarefas(QWidget *parent = nullptr);
    GerenciadorTarefas(const QString &ui_layout_mode_key, QWidget *parent);

    QVector<Tarefa> carregar_tarefas();
    void salvar_tarefas();
    void setup_ui();
    void atualizar_traducoes();
    KanbanColumn *criar_coluna(const QString &status);
    void _on_drop_sincronizar();
    void _on_rows_moved(QListWidget *lista);
    void _sincronizar_status_pos_movimento(QListWidget *prioridade_lista = nullptr);
    void adicionar_tarefa();
    Tarefa *_adicionar_tarefa_interna(const QString &titulo,
                                      const QString &prioridade,
                                      const QString &status = QStringLiteral("Todo"),
                                      bool emitir_sinal = true,
                                      const QString &description = QString(),
                                      const QString &file_path = QString(),
                                      const QString &date = QString(),
                                      const QString &time = QString(),
                                      const QString &task_id = QString(),
                                      bool sincronizada_com_eisenhower = false,
                                      int dominio = 0,
                                      const QString &cor_texto = QString(),
                                      int pomodoros = -1,
                                      int ciclos_totais = -1,
                                      bool ciclos_customizados = false,
                                      double duracao = -1.0);
    void adicionar_tarefa_externa(const QString &titulo,
                                  const QString &prioridade = QStringLiteral("Importante e Urgente"),
                                  const QString &description = QString(),
                                  const QString &file_path = QString(),
                                  const QString &date = QString(),
                                  const QString &time = QString(),
                                  const QString &status = QStringLiteral("Todo"),
                                  const QString &task_id = QString(),
                                  bool sincronizada_com_eisenhower = true,
                                  int dominio = -1,
                                  const QString &cor_texto = QString(),
                                  int pomodoros = -1,
                                  int ciclos_totais = -1,
                                  bool ciclos_customizados = false,
                                  double duracao = -1.0);
    void mover_tarefa(QListWidgetItem *item, const QString &status_atual);
    void atualizar_listas();
    void mostrar_menu_contexto(const QPoint &pos, QListWidget *lista);
    bool _ha_tarefas_marcadas();
    void remover_tarefa_por_item(QListWidgetItem *item);
    void remover_tarefas_marcadas();
    void limpar_selecao_tarefas();
    void limpar_todas_as_tarefas();
    void resetar_contagem_botao();
    void mover_tarefas_marcadas();
    void mover_tarefa_por_item(QListWidgetItem *item);
    void on_task_duration_changed(const QString &task_id, double duration_days);
    void set_pomodoro_timer(PomodoroTimer *timer);
    void on_default_ciclos_changed(int novos_ciclos);
    int obter_ciclos_permitidos_padrao() const;
    void _ajustar_largura_combo(QComboBox *combo, int padding_extra = 42, int max_visiveis = 6);
    void _aplicar_tema_dinamico_inputs();
    bool eventFilter(QObject *obj, QEvent *event) override;

protected:
    void changeEvent(QEvent *event) override;

signals:
    void listas_atualizadas();
    void tarefa_adicionada(const QVariantMap &dados);

public:
    source::utils::LogManager *logger_ = nullptr;
    QString arquivo_tarefas;
    QVector<Tarefa> tarefas;

    compat::GerenciadorBotoesUI gerenciador_botoes_;

    FileDropLineEdit *input_tarefa = nullptr;
    QLabel *label_prioridade = nullptr;
    QComboBox *combo_prioridade = nullptr;
    QLabel *label_dominio = nullptr;
    QComboBox *combo_dominio = nullptr;
    QLabel *label_fluxo_tarefas_concluidas = nullptr;
    QPushButton *btn_adicionar = nullptr;
    QPushButton *btn_mover = nullptr;
    QPushButton *btn_remover = nullptr;
    QPushButton *btn_resetar_contagem = nullptr;
    QCheckBox *chk_fluxo_tarefas_concluidas = nullptr;
    QGroupBox *quadro_tarefas = nullptr;
    QVector<QPointer<QPushButton>> botoes_linha_principal_;
    QString pending_shared_origin_;
    QString ui_layout_mode_key_ = QStringLiteral("simple:tabs");

    KanbanColumn *col_todo = nullptr;
    KanbanColumn *col_doing = nullptr;
    KanbanColumn *col_done = nullptr;
    QTabWidget *status_tabs = nullptr;

    int input_ref_height_ = 0;
    QString dragged_file_path_;
    bool applying_theme_inputs_ = false;
    bool ajustando_linha_botoes_ = false;

private:
    void ajustar_linha_botoes();
    void atualizar_cor_rotulos_molduras();
    bool _layout_robusto() const;
    QString strip_item_title(QListWidgetItem *item) const;
    Tarefa *find_task(QListWidgetItem *item);
    const Tarefa *find_task(QListWidgetItem *item) const;
    void open_linked_file_for_item(QListWidgetItem *item);
    QVector<QListWidgetItem *> collect_marked_items() const;
    QVariantMap task_payload(const Tarefa &tarefa) const;
    QPointer<PomodoroTimer> pomodoro_timer_;
};

}  // namespace source::plugin::Pomodoro
