#include "source/modules/agenda/AgendaAvaliacoes/BancoDeDados/Banco_Dados.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoInterfaceGrafica/Interface_Grafica.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/ComboBoxUtils.hpp"
#include "source/utils/LogManager.hpp"

#include <QCoreApplication>
#include <QLayout>
#include <QLineEdit>
#include <QLocale>
#include <QSignalBlocker>
#include <QWidget>

void InterfaceGerenciadorAtividades::retranslate_ui() {
    auto tr = [](const char *context, const char *source) { return QCoreApplication::translate(context, source); };
    setWindowTitle(tr("InterfaceGerenciadorAtividades", "Cadastro de Atividades"));
    if (label_curso)
        label_curso->setText(tr("InterfaceGerenciadorAtividades", "Curso:"));
    if (label_ementa)
        label_ementa->setText(tr("InterfaceGerenciadorAtividades", "Ementa:"));
    if (label_semestre)
        label_semestre->setText(tr("InterfaceGerenciadorAtividades", "Semestre:"));
    if (label_disciplina)
        label_disciplina->setText(tr("InterfaceGerenciadorAtividades", "Disciplina:"));
    if (label_codigo)
        label_codigo->setText(tr("InterfaceGerenciadorAtividades", "Turma da Disciplina:"));
    if (label_tipo)
        label_tipo->setText(tr("InterfaceGerenciadorAtividades", "Tipo de Atividade Avaliativa:"));
    if (label_sequencia)
        label_sequencia->setText(tr("InterfaceGerenciadorAtividades", "Sequência da Atividade:"));
    if (label_prioridade)
        label_prioridade->setText(tr("InterfaceGerenciadorAtividades", "Prioridade:"));
    if (label_dominio)
        label_dominio->setText(tr("InterfaceGerenciadorAtividades", "Nível de Domínio:"));
    if (label_arquivo_vinculado)
        label_arquivo_vinculado->setText(tr("InterfaceGerenciadorAtividades", "Arquivo vinculado (arraste ou escolha):"));
    if (check_vincular_data)
        check_vincular_data->setText(tr("InterfaceGerenciadorAtividades", "Vincular data"));
    if (check_vincular_horario)
        check_vincular_horario->setText(tr("InterfaceGerenciadorAtividades", "Vincular horário"));
    if (label_data)
        label_data->setText(tr("InterfaceGerenciadorAtividades", "Defina a Data da Atividade:"));
    if (label_banco_dados)
        label_banco_dados->setText(tr("InterfaceGerenciadorAtividades", "Caixa de Dados:"));
    if (button_clear_item)
        button_clear_item->setText(tr("InterfaceGerenciadorAtividades", "Excluir Item(ns)"));
    if (button_clear_ultima)
        button_clear_ultima->setText(tr("InterfaceGerenciadorAtividades", "Limpar Última Entrada"));
    if (button_clear)
        button_clear->setText(tr("InterfaceGerenciadorAtividades", "Limpar Tudo"));
    if (button_editar_item)
        button_editar_item->setText(tr("InterfaceGerenciadorAtividades", "Editar Item(ns)"));
    if (button_submiter)
        button_submiter->setText(tr("InterfaceGerenciadorAtividades", "Registrar Definições"));
    if (button_limpar_definicoes)
        button_limpar_definicoes->setText(tr("InterfaceGerenciadorAtividades", "Limpar"));
    if (button_fechar)
        button_fechar->setText(tr("InterfaceGerenciadorAtividades", "Fechar"));
    if (button_export)
        button_export->setText(tr("InterfaceGerenciadorAtividades", "Exportar para PDF"));
    if (button_escolher_arquivo)
        button_escolher_arquivo->setText(tr("InterfaceGerenciadorAtividades", "Escolher..."));
    if (button_abrir_arquivo)
        button_abrir_arquivo->setText(tr("InterfaceGerenciadorAtividades", "Abrir"));
    if (button_remover_arquivo)
        button_remover_arquivo->setText(tr("InterfaceGerenciadorAtividades", "Remover"));
    if (menu_arquivo) {
        menu_arquivo->setTitle(tr("InterfaceGerenciadorAtividades", "Arquivo"));
        menu_arquivo->menuAction()->setVisible(false);
    }
    if (menu_config)
        menu_config->setTitle(tr("InterfaceGerenciadorAtividades", "Configurações"));
    if (menu_exemplo_agenda)
        menu_exemplo_agenda->setTitle(tr("InterfaceGerenciadorAtividades", "Exemplo de Agenda"));
    if (action_agenda_engenharia_civil)
        action_agenda_engenharia_civil->setText(tr("InterfaceGerenciadorAtividades", "Engenharia Civil"));
    if (menu_crie_agenda)
        menu_crie_agenda->setTitle(tr("InterfaceGerenciadorAtividades", "Crie sua agenda"));
    if (line_novo_titulo_agenda)
        line_novo_titulo_agenda->setPlaceholderText(
            tr("InterfaceGerenciadorAtividades", "Novo título para sua agenda"));
    for (const CustomAgendaRow &linha : custom_agenda_rows) {
        if (linha.title_edit)
            linha.title_edit->setPlaceholderText(
                tr("InterfaceGerenciadorAtividades", "Insira um título para a nova lista"));
        if (linha.combo && linha.combo->lineEdit())
            linha.combo->lineEdit()->setPlaceholderText(tr("InterfaceGerenciadorAtividades", "Novo item"));
    }
    reconstruir_menus_agendas_customizadas();
    if (menu_idiomas) {
        menu_idiomas->setTitle(tr("InterfaceGerenciadorAtividades", "Idiomas"));
        menu_idiomas->menuAction()->setVisible(false);
    }
    if (menu_excluir_item_listas)
        menu_excluir_item_listas->setTitle(tr("InterfaceGerenciadorAtividades", "Excluir Item"));
    for (auto it = delete_list_menus.begin(); it != delete_list_menus.end(); ++it) {
        it.value()->setTitle(
            tr("InterfaceGerenciadorAtividades", delete_list_action_sources.value(it.key()).toUtf8().constData()));
    }
    if (action_excluir_item)
        action_excluir_item->setText(tr("InterfaceGerenciadorAtividades", "Excluir Item(ns)"));
    if (action_limpar_ultima)
        action_limpar_ultima->setText(tr("InterfaceGerenciadorAtividades", "Limpar Última Entrada"));
    if (action_limpar_tudo)
        action_limpar_tudo->setText(tr("InterfaceGerenciadorAtividades", "Limpar Tudo"));
    if (action_editar_item)
        action_editar_item->setText(tr("InterfaceGerenciadorAtividades", "Editar Item(ns)"));
    if (action_registrar)
        action_registrar->setText(tr("InterfaceGerenciadorAtividades", "Registrar Definições"));
    if (action_export_pdf)
        action_export_pdf->setText(tr("InterfaceGerenciadorAtividades", "Exportar para PDF"));
    if (action_sair)
        action_sair->setText(tr("InterfaceGerenciadorAtividades", "Sair"));
    if (menu_cores) {
        menu_cores->setTitle(tr("InterfaceGerenciadorAtividades", "Cores"));
        populate_color_menu(menu_cores);
    }
    if (calendar && gerenciador_traducao && gerenciador_traducao->obter_idioma_atual().startsWith("pt")) {
        calendar->setLocale(QLocale(QLocale::Portuguese));
    } else if (calendar) {
        calendar->setLocale(QLocale(QLocale::English));
    }
    if (combo_prioridade) {
        const int indice = combo_prioridade->currentIndex();
        const QSignalBlocker blocker(combo_prioridade);
        combo_prioridade->clear();
        combo_prioridade->addItems({
            tr("InterfaceGerenciadorAtividades", "🔴 Importante e Urgente"),
            tr("InterfaceGerenciadorAtividades", "🟠 Importante, mas Não Urgente"),
            tr("InterfaceGerenciadorAtividades", "🟡 Não Importante, mas Urgente"),
            tr("InterfaceGerenciadorAtividades", "🟢 Não Importante e Não Urgente"),
        });
        combo_prioridade->setCurrentIndex(indice >= 0 ? qMin(indice, combo_prioridade->count() - 1) : 0);
    }
    if (combo_dominio) {
        const int indice = combo_dominio->currentIndex();
        const QSignalBlocker blocker(combo_dominio);
        combo_dominio->clear();
        combo_dominio->addItems({
            tr("InterfaceGerenciadorAtividades", "🥉 Iniciante (não consigo explicar)"),
            tr("InterfaceGerenciadorAtividades", "🥈 Intermediário (explico com dificuldade)"),
            tr("InterfaceGerenciadorAtividades", "🥇 Avançado (explico facilmente)"),
        });
        combo_dominio->setCurrentIndex(indice >= 0 ? qMin(indice, combo_dominio->count() - 1) : 0);
    }
    const QString cursoAtual = combo_curso ? combo_curso->currentText() : QString();
    const QString turmaAtual = entry_codigo ? entry_codigo->currentText() : QString();
    const QString tipoAtual = combo_tipo ? combo_tipo->currentText() : QString();
    const QString sequenciaAtual = combo_sequencia ? combo_sequencia->currentText() : QString();
    if (combo_curso)
        substituir_itens_combo(combo_curso, obter_cursos(), cursoAtual);
    if (entry_codigo)
        substituir_itens_combo(entry_codigo, obter_turmas(), turmaAtual);
    if (combo_tipo)
        substituir_itens_combo(combo_tipo, obter_avaliacoes(), tipoAtual);
    if (combo_sequencia)
        substituir_itens_combo(combo_sequencia, obter_sequencias(), sequenciaAtual);
    update_ementa();
    if (gerenciador_traducao) {
        for (auto it = gerenciador_traducao->idiomas_disponiveis.cbegin();
             it != gerenciador_traducao->idiomas_disponiveis.cend(); ++it) {
            if (lang_actions.contains(it.key()))
                lang_actions.value(it.key())->setText(
                    tr("InterfaceGerenciadorAtividades", it.value().toUtf8().constData()));
        }
    }
    if (painel_esquerdo_apenas) {
        if (QWidget *central = centralWidget()) {
            if (QLayout *central_layout = central->layout()) {
                central_layout->activate();
            }
        }
        setMinimumSize(0, 0);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        adjustSize();
        setFixedSize(sizeHint());
    }
}
