#include "ginter_12_excluir_item_lista_personalizada.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoInterfaceGrafica/Interface_Grafica.hpp"
#include "source/utils/LogManager.hpp"

#include <QAction>
#include <QActionGroup>
#include <QCoreApplication>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QWidgetAction>

void InterfaceGerenciadorAtividades::create_menu() {
    auto tr = [](const char *context, const char *source) { return QCoreApplication::translate(context, source); };
    QMenuBar *bar = menuBar();
    menu_arquivo = bar->addMenu(tr("InterfaceGerenciadorAtividades", "Arquivo"));
    menu_arquivo->setEnabled(false);
    menu_arquivo->menuAction()->setVisible(false);
    action_excluir_item = new QAction(tr("InterfaceGerenciadorAtividades", "Excluir Item(ns)"), this);
    connect(action_excluir_item, &QAction::triggered, this, &InterfaceGerenciadorAtividades::excluir_item);
    menu_arquivo->addAction(action_excluir_item);
    action_limpar_ultima = new QAction(tr("InterfaceGerenciadorAtividades", "Limpar Última Entrada"), this);
    connect(action_limpar_ultima, &QAction::triggered, this, &InterfaceGerenciadorAtividades::limpar_ultima_entrada);
    menu_arquivo->addAction(action_limpar_ultima);
    action_limpar_tudo = new QAction(tr("InterfaceGerenciadorAtividades", "Limpar Tudo"), this);
    connect(action_limpar_tudo, &QAction::triggered, this, &InterfaceGerenciadorAtividades::limpar_entradas);
    menu_arquivo->addAction(action_limpar_tudo);
    action_editar_item = new QAction(tr("InterfaceGerenciadorAtividades", "Editar Item(ns)"), this);
    connect(action_editar_item, &QAction::triggered, this, &InterfaceGerenciadorAtividades::editar_item);
    menu_arquivo->addAction(action_editar_item);
    action_registrar = new QAction(tr("InterfaceGerenciadorAtividades", "Registrar Definições"), this);
    connect(action_registrar, &QAction::triggered, this, &InterfaceGerenciadorAtividades::submiter);
    menu_arquivo->addAction(action_registrar);
    action_export_pdf = new QAction(tr("InterfaceGerenciadorAtividades", "Exportar para PDF"), this);
    connect(action_export_pdf, &QAction::triggered, this, &InterfaceGerenciadorAtividades::exportar_para_pdf);
    menu_arquivo->addAction(action_export_pdf);
    menu_arquivo->addSeparator();
    action_sair = new QAction(tr("InterfaceGerenciadorAtividades", "Sair"), this);
    connect(action_sair, &QAction::triggered, qApp, &QCoreApplication::quit);
    menu_arquivo->addAction(action_sair);
    menu_config = bar->addMenu(tr("InterfaceGerenciadorAtividades", "Configurações"));

    menu_exemplo_agenda = menu_config->addMenu(tr("InterfaceGerenciadorAtividades", "Exemplo de Agenda"));
    action_agenda_engenharia_civil =
        new QAction(tr("InterfaceGerenciadorAtividades", "Engenharia Civil"), this);
    connect(action_agenda_engenharia_civil,
            &QAction::triggered,
            this,
            &InterfaceGerenciadorAtividades::selecionar_agenda_exemplo);
    menu_exemplo_agenda->addAction(action_agenda_engenharia_civil);

    menu_crie_agenda = menu_config->addMenu(tr("InterfaceGerenciadorAtividades", "Crie sua agenda"));
    line_novo_titulo_agenda = new QLineEdit(menu_crie_agenda);
    line_novo_titulo_agenda->setClearButtonEnabled(true);
    line_novo_titulo_agenda->setMinimumWidth(260);
    line_novo_titulo_agenda->setPlaceholderText(
        tr("InterfaceGerenciadorAtividades", "Novo título para sua agenda"));
    action_novo_titulo_agenda = new QWidgetAction(this);
    action_novo_titulo_agenda->setDefaultWidget(line_novo_titulo_agenda);
    menu_crie_agenda->addAction(action_novo_titulo_agenda);
    connect(line_novo_titulo_agenda,
            &QLineEdit::returnPressed,
            this,
            &InterfaceGerenciadorAtividades::criar_agenda_customizada_do_menu);

    carregar_agendas_customizadas();
    reconstruir_menus_agendas_customizadas();

    menu_idiomas = menu_config->addMenu(tr("InterfaceGerenciadorAtividades", "Idiomas"));
    menu_idiomas->menuAction()->setVisible(false);
    lang_action_group = new QActionGroup(this);
    lang_action_group->setExclusive(true);
    for (auto it = gerenciador_traducao->idiomas_disponiveis.cbegin();
         it != gerenciador_traducao->idiomas_disponiveis.cend(); ++it) {
        QAction *action = new QAction(tr("InterfaceGerenciadorAtividades", it.value().toUtf8().constData()), this);
        action->setCheckable(true);
        action->setData(it.key());
        connect(action, &QAction::triggered, this, [this, codigo = it.key()] { change_language(codigo); });
        menu_idiomas->addAction(action);
        lang_action_group->addAction(action);
        lang_actions.insert(it.key(), action);
    }
    if (lang_actions.contains(gerenciador_traducao->obter_idioma_atual()))
        lang_actions.value(gerenciador_traducao->obter_idioma_atual())->setChecked(true);
    menu_excluir_item_listas = menu_config->addMenu(tr("InterfaceGerenciadorAtividades", "Excluir Item"));
    connect(menu_excluir_item_listas, &QMenu::aboutToShow, this, [this] {
        popular_menu_excluir_item_agenda(menu_excluir_item_listas);
    });
    menu_cores = menu_config->addMenu(tr("InterfaceGerenciadorAtividades", "Cores"));
    populate_color_menu(menu_cores);
}
