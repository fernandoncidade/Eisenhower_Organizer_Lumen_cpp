#include <QtTest>

#include <QMenu>
#include <QSqlDatabase>

#include "test_common.hpp"
#include "source/modules/agenda/agenda_plugin.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/BancoDeDados/Banco_Dados.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/BancoDeDados/Listas_Personalizadas.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoAtividades/Gerenciador_Atividades.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/CaminhoPersistenteUtils.hpp"

using namespace source::plugin;
using namespace eisenhower_tests;

class TestPluginAgenda : public QObject {
    Q_OBJECT

private slots:
    void test_agenda_plugin_interface();
    void test_agenda_sqlite_crud_and_color_modes();
    void test_listas_personalizadas_sqlite();
    void test_banco_dados_dictionaries_and_translations();
};

void TestPluginAgenda::test_agenda_plugin_interface()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());
    definir_caminho_persistente_override(sandbox.file_path(QStringLiteral("agenda_plugin_test")));

    AgendaPlugin plugin;
    QCOMPARE(plugin.plugin_id(), QStringLiteral("agenda"));
    QVERIFY(!plugin.display_name().isEmpty());
    QVERIFY(!plugin.open_menu_text().isEmpty());

    QMenu delete_menu;
    QMenu color_menu;
    plugin.populate_delete_item_menu(&delete_menu);
    plugin.populate_color_menu(&color_menu);
    plugin.retranslate_ui();

    limpar_caminho_persistente_override();
}

void TestPluginAgenda::test_agenda_sqlite_crud_and_color_modes()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());
    const QString db_dir = sandbox.file_path(QStringLiteral("agenda_db_crud"));
    definir_caminho_persistente_override(db_dir);

    {
        GerenciamentoAtividades manager(true);
        manager.criar_tabela();

        const Atividade created{
            QStringLiteral("2026-12-10"),
            QStringLiteral("Prova"),
            QStringLiteral("P1"),
            QStringLiteral("Cálculo III"),
            QStringLiteral("T01"),
        };
        const QString key = chave_atividade(created);
        QVERIFY(!key.isEmpty());
        QVERIFY(atividade_para_linha(created).contains(QStringLiteral("Cálculo III")));

        manager.adicionar_atividade(created);
        const auto found = manager.buscar_atividade(created);
        QVERIFY(found.has_value());
        QCOMPARE(found->nome, QStringLiteral("Cálculo III"));

        const QList<Atividade> listed = manager.listar_atividades();
        QVERIFY(!listed.isEmpty());

        const Atividade updated{
            QStringLiteral("2026-12-12"),
            QStringLiteral("Seminário"),
            QStringLiteral("S1"),
            QStringLiteral("Cálculo Numérico"),
            QStringLiteral("T02"),
        };
        QVERIFY(manager.atualizar_atividade(created, updated));
        const auto found_updated = manager.buscar_atividade(updated);
        QVERIFY(found_updated.has_value());
        QCOMPARE(found_updated->tipo, QStringLiteral("Seminário"));

        manager.remover_atividade(updated);
        QVERIFY(!manager.buscar_atividade(updated).has_value());

        QVERIFY(manager.definir_modo_cores(QStringLiteral("coloridas"), QStringLiteral("eisenhower")));
        QCOMPARE(manager.obter_modo_cores(QStringLiteral("eisenhower")), QStringLiteral("coloridas"));
        QVERIFY(manager.definir_modo_cores(QStringLiteral("preto"), QStringLiteral("pomodoro")));
        QCOMPARE(manager.obter_modo_cores(QStringLiteral("pomodoro")), QStringLiteral("preto"));
        QVERIFY(!manager.definir_modo_cores(QStringLiteral("invalido"), QStringLiteral("pomodoro")));
    }

    limpar_caminho_persistente_override();
}

void TestPluginAgenda::test_listas_personalizadas_sqlite()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());
    definir_caminho_persistente_override(sandbox.file_path(QStringLiteral("agenda_listas_db")));

    criar_tabelas_listas();
    QVERIFY(inserir_curso(QStringLiteral("pt_BR"), QStringLiteral("Engenharia da Computação Teste")));
    QVERIFY(listar_cursos(QStringLiteral("pt_BR")).contains(QStringLiteral("Engenharia da Computação Teste")));

    QVERIFY(inserir_ementa(QStringLiteral("pt_BR"), QStringLiteral("Engenharia da Computação Teste"), QStringLiteral("Matriz 2026")));
    QVERIFY(listar_ementas(QStringLiteral("pt_BR"), QStringLiteral("Engenharia da Computação Teste"))
                .contains(QStringLiteral("Matriz 2026")));

    QVERIFY(inserir_semestre(QStringLiteral("pt_BR"),
                             QStringLiteral("Engenharia da Computação Teste"),
                             QStringLiteral("Matriz 2026"),
                             QStringLiteral("5º Semestre")));
    QVERIFY(listar_semestres(QStringLiteral("pt_BR"),
                             QStringLiteral("Engenharia da Computação Teste"),
                             QStringLiteral("Matriz 2026"))
                .contains(QStringLiteral("5º Semestre")));

    QVERIFY(inserir_disciplina(QStringLiteral("pt_BR"),
                               QStringLiteral("Engenharia da Computação Teste"),
                               QStringLiteral("Matriz 2026"),
                               QStringLiteral("5º Semestre"),
                               QStringLiteral("Arquitetura de Computadores")));
    QVERIFY(listar_disciplinas(QStringLiteral("pt_BR"),
                               QStringLiteral("Engenharia da Computação Teste"),
                               QStringLiteral("Matriz 2026"),
                               QStringLiteral("5º Semestre"))
                .contains(QStringLiteral("Arquitetura de Computadores")));

    const auto items = listar_itens_personalizados_para_exclusao(QStringLiteral("curso"), QStringLiteral("pt_BR"));
    QVERIFY(!items.isEmpty());
    QVERIFY(remover_item_personalizado(QStringLiteral("curso"), items.first().id));

    limpar_caminho_persistente_override();
}

void TestPluginAgenda::test_banco_dados_dictionaries_and_translations()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());
    definir_caminho_persistente_override(sandbox.file_path(QStringLiteral("agenda_dict")));

    const QStringList cursos = obter_cursos();
    const QStringList avaliacoes = obter_avaliacoes();
    const QStringList sequencias = obter_sequencias();
    QVERIFY(!cursos.isEmpty());
    QVERIFY(!avaliacoes.isEmpty());
    QVERIFY(!sequencias.isEmpty());

    const QString cor = obter_ou_atribuir_cor_disciplina(QStringLiteral("Sistemas Operacionais"));
    QVERIFY(cor.startsWith(QLatin1Char('#')));

    limpar_caminho_persistente_override();
}

QTEST_MAIN(TestPluginAgenda)
#include "test_plugin_agenda.moc"
