#pragma once

#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoAtividades/Gerenciador_Atividades.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/language/tr_01_gerenciadorTraducao.hpp"

#include <QAction>
#include <QActionGroup>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QHash>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QTimeEdit>
#include <QVariantMap>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QStringList>

#include <memory>

class QEvent;

class InterfaceGerenciadorAtividades : public QMainWindow {
    Q_OBJECT
public:
    struct CustomAgendaRow {
        QWidget *widget = nullptr;
        QCheckBox *enabled = nullptr;
        QLineEdit *title_edit = nullptr;
        QLabel *title_label = nullptr;
        QComboBox *combo = nullptr;
        bool nested = false;
        int parent_index = -1;
        QHash<QString, QStringList> nested_items;
    };

    explicit InterfaceGerenciadorAtividades(GerenciadorTraducao *gerenciador_traducao = nullptr,
                                            QWidget *parent = nullptr,
                                            bool painel_esquerdo_apenas = false,
                                            bool criar_menu_janela = true);
    InterfaceGerenciadorAtividades(GerenciadorTraducao *gerenciador_traducao,
                                   QWidget *parent,
                                   bool painel_esquerdo_apenas,
                                   bool criar_menu_janela,
                                   bool persistir_atividades_locais);
    void center_on_screen();
    void create_widgets();
    QPushButton *create_button();
    void create_menu();
    void change_language(const QString &codigo_idioma);
    void change_color_mode(const QString &codigo_modo);
    void populate_delete_item_menu(QMenu *menu);
    void populate_color_menu(QMenu *menu);
    void retranslate_ui();
    void selecionar_agenda_exemplo();
    void selecionar_agenda_customizada(const QString &titulo);
    void criar_agenda_customizada_do_menu();
    void editar_agenda_customizada(const QString &titulo);
    void remover_agenda_customizada(const QString &titulo);
    void carregar_agendas_customizadas();
    void carregar_ultima_agenda_ativa();
    void salvar_ultima_agenda_ativa(const QString &tipo, const QString &titulo = QString()) const;
    void reconstruir_menus_agendas_customizadas();
    void aplicar_listas_agenda_customizada(const QJsonArray &listas);
    void salvar_agenda_customizada_atual();
    QString caminho_arquivo_agenda_customizada(const QString &titulo) const;
    void confirmar_titulo_linha_customizada(int index);
    void editar_titulo_agenda_customizada(const QString &titulo_agenda);
    void remover_titulo_agenda_customizada(const QString &titulo_agenda);
    void atualizar_visibilidade_linhas_customizadas();
    void registrar_item_linha_agenda_customizada(int index);
    void atualizar_dependencias_agenda_customizada(int parent_index);
    void remover_item_linha_agenda_customizada(int index, const QString &texto);
    QStringList itens_linha_agenda_customizada(int index) const;
    void popular_menu_excluir_item_agenda(QMenu *menu);
    QString proxima_cor_agenda_customizada();

signals:
    void atividade_registrada(const QVariantMap &task);
    void modo_cores_alterado(const QString &codigo_modo);

protected:
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;
private slots:
    void submiter();
    void limpar_entradas();
    void limpar_ultima_entrada();
    void excluir_item();
    void editar_item();
    void exportar_para_pdf();
    void update_textbox();
    void update_ementa();
    void update_semestre();
    void update_disciplinas();
    void sync_date_time_controls();
    void limpar_definicoes_selecionadas();

public:
    GerenciamentoAtividades gerenciamento_atividades;
    std::unique_ptr<GerenciadorTraducao> gerenciador_traducao_proprio;
    GerenciadorTraducao *gerenciador_traducao = nullptr;
    QLabel *label_curso = nullptr;
    QLabel *label_ementa = nullptr;
    QLabel *label_semestre = nullptr;
    QLabel *label_disciplina = nullptr;
    QLabel *label_codigo = nullptr;
    QLabel *label_tipo = nullptr;
    QLabel *label_sequencia = nullptr;
    QLabel *label_prioridade = nullptr;
    QLabel *label_dominio = nullptr;
    QLabel *label_arquivo_vinculado = nullptr;
    QLabel *label_data = nullptr;
    QLabel *label_banco_dados = nullptr;
    QComboBox *combo_curso = nullptr;
    QComboBox *entry_ementa = nullptr;
    QComboBox *entry_semestre = nullptr;
    QComboBox *entry_disciplina = nullptr;
    QComboBox *entry_codigo = nullptr;
    QComboBox *combo_tipo = nullptr;
    QComboBox *combo_sequencia = nullptr;
    QComboBox *combo_prioridade = nullptr;
    QComboBox *combo_dominio = nullptr;
    QStackedWidget *agenda_stack = nullptr;
    QWidget *agenda_exemplo_widget = nullptr;
    QWidget *agenda_custom_widget = nullptr;
    QVBoxLayout *agenda_custom_layout = nullptr;
    QList<CustomAgendaRow> custom_agenda_rows;
    QCheckBox *check_vincular_data = nullptr;
    QCheckBox *check_vincular_horario = nullptr;
    QCalendarWidget *calendar = nullptr;
    QTimeEdit *time_edit = nullptr;
    QLineEdit *arquivo_vinculado_edit = nullptr;
    QTextBrowser *textbox = nullptr;
    QPushButton *button_clear_item = nullptr;
    QPushButton *button_clear_ultima = nullptr;
    QPushButton *button_clear = nullptr;
    QPushButton *button_editar_item = nullptr;
    QPushButton *button_submiter = nullptr;
    QPushButton *button_limpar_definicoes = nullptr;
    QPushButton *button_fechar = nullptr;
    QPushButton *button_export = nullptr;
    QPushButton *button_escolher_arquivo = nullptr;
    QPushButton *button_abrir_arquivo = nullptr;
    QPushButton *button_remover_arquivo = nullptr;
    QMenu *menu_arquivo = nullptr;
    QMenu *menu_config = nullptr;
    QMenu *menu_exemplo_agenda = nullptr;
    QMenu *menu_crie_agenda = nullptr;
    QMenu *menu_idiomas = nullptr;
    QMenu *menu_excluir_item_listas = nullptr;
    QMenu *menu_cores = nullptr;
    QMenu *menu_preto = nullptr;
    QMenu *menu_coloridas = nullptr;
    QAction *action_agenda_engenharia_civil = nullptr;
    QWidgetAction *action_novo_titulo_agenda = nullptr;
    QLineEdit *line_novo_titulo_agenda = nullptr;
    QAction *action_excluir_item = nullptr;
    QAction *action_limpar_ultima = nullptr;
    QAction *action_limpar_tudo = nullptr;
    QAction *action_editar_item = nullptr;
    QAction *action_registrar = nullptr;
    QAction *action_export_pdf = nullptr;
    QAction *action_sair = nullptr;
    QActionGroup *lang_action_group = nullptr;
    QActionGroup *color_action_group = nullptr;
    QHash<QString, QAction *> lang_actions;
    QHash<QString, QAction *> color_actions;
    QHash<QString, QMenu *> custom_agenda_menus;
    QHash<QString, QMenu *> delete_list_menus;
    QHash<QString, QString> delete_list_action_sources;
    QStringList custom_agenda_titles;
    QString current_custom_agenda_title;
    QString ultima_cor_agenda_customizada;
    bool _atualizando_ementa = false;
    bool _atualizando_semestre = false;
    bool _atualizando_disciplinas = false;
    bool custom_agenda_mode = false;
    bool updating_custom_agenda = false;
    bool painel_esquerdo_apenas = false;
    bool criar_menu_janela = true;
};
