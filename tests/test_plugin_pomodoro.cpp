#include <QtTest>

#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QSignalSpy>
#include <QTimer>

#include "test_common.hpp"
#include "source/modules/pomodoro/pomodoro_plugin.hpp"
#include "source/modules/pomodoro/Pomodoro/pomodoro_support.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_01_Tarefa.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_02_PomodoroTimer.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_03_GerenciadorTarefas.hpp"

using namespace source::plugin;
using namespace source::plugin::Pomodoro;
using namespace eisenhower_tests;

class TestPluginPomodoro : public QObject {
    Q_OBJECT

private slots:
    void test_pomodoro_plugin_interface();
    void test_tarefa_serialization_and_normalization();
    void test_pomodoro_timer_state_and_cycles();
    void test_gerenciador_tarefas_kanban_and_persistence();
    void test_event_bus_messaging();
};

void TestPluginPomodoro::test_pomodoro_plugin_interface()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    PomodoroPlugin plugin;
    QCOMPARE(plugin.plugin_id(), QStringLiteral("pomodoro"));
    QVERIFY(!plugin.display_name().isEmpty());
    QVERIFY(!plugin.open_menu_text().isEmpty());
    QVERIFY(!plugin.alarm_menu_text().isEmpty());
    QVERIFY(!plugin.automatic_mode_menu_text().isEmpty());
    QVERIFY(plugin.default_pomodoro_cycle_count() >= 1);

    QWidget *widget = plugin.create_widget();
    QVERIFY(widget != nullptr);
    plugin.retranslate_ui();
    plugin.receive_eisenhower_task(QVariantMap{
        {QStringLiteral("id"), QStringLiteral("pomo-eisen-1")},
        {QStringLiteral("titulo"), QStringLiteral("Refatorar CMake")},
        {QStringLiteral("prioridade"), 1},
    });
    plugin.refresh_task_views();

    QCoreApplication::processEvents();
    delete widget;
}

void TestPluginPomodoro::test_tarefa_serialization_and_normalization()
{
    QCOMPARE(Tarefa::normalizar_dominio(QVariant(-2)), 0);
    QCOMPARE(Tarefa::normalizar_dominio(QVariant(9)), 2);
    QVERIFY(!Tarefa::normalizar_prioridade(QStringLiteral("Importante e Urgente")).isEmpty());

    Tarefa t(QStringLiteral("Implementar Cache"),
             QStringLiteral("Detalhes técnicos"),
             {QStringLiteral("Etapa 1"), QStringLiteral("Etapa 2")},
             QStringLiteral("Importante e Urgente"),
             QStringLiteral("Doing"),
             QStringLiteral("task-p-100"),
             QStringLiteral("C:/temp/doc.pdf"),
             QStringLiteral("2026-12-20"),
             QStringLiteral("10:00"),
             1,
             QStringLiteral("#2E7D32"),
             QStringLiteral("eisenhower"));
    t.pomodoros_completados = 2;
    t.ciclos_totais = 4;
    QCOMPARE(t.percentual_conclusao(), 50.0);

    const QVariantMap dict = t.to_dict();
    const auto restored = Tarefa::from_dict(dict);
    QVERIFY(restored.has_value());
    QCOMPARE(restored->titulo, QStringLiteral("Implementar Cache"));
    QCOMPARE(restored->status, QStringLiteral("Doing"));
    QCOMPARE(restored->pomodoros_completados, 2);
    QCOMPARE(restored->ciclos_totais, 4);
    QCOMPARE(restored->id, QStringLiteral("task-p-100"));
}

void TestPluginPomodoro::test_pomodoro_timer_state_and_cycles()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    PomodoroTimer timer;
    QVERIFY(timer.em_foco);
    QCOMPARE(timer.ciclos_completados, 0);
    QVERIFY(!timer.timer_ativo);

    timer.toggle_timer();
    QVERIFY(timer.timer_ativo);
    timer.toggle_timer();
    QVERIFY(!timer.timer_ativo);

    // Dismiss modal alert dialog automatically if _executar_alerta spawns QMessageBox::exec().
    QSignalSpy cycle_spy(&timer, &PomodoroTimer::ciclo_completado);
    QTimer dismiss_timer;
    dismiss_timer.setInterval(20);
    connect(&dismiss_timer, &QTimer::timeout, &dismiss_timer, []() {
        if (QWidget *modal = QApplication::activeModalWidget()) {
            if (auto *dlg = qobject_cast<QDialog *>(modal)) {
                dlg->accept();
            } else {
                modal->close();
            }
        }
    });
    dismiss_timer.start();
    timer.pular_ciclo();
    dismiss_timer.stop();

    QCOMPARE(cycle_spy.count(), 1);
    QCOMPARE(timer.ciclos_completados, 1);
    QVERIFY(!timer.em_foco);

    timer.resetar_timer();
    QVERIFY(!timer.timer_ativo);
    QVERIFY(!timer.em_foco);

    timer.resetar_ciclo();
    QVERIFY(timer.em_foco);
    QCOMPARE(timer.ciclos_completados, 0);
}

void TestPluginPomodoro::test_gerenciador_tarefas_kanban_and_persistence()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    GerenciadorTarefas gt;
    gt.limpar_todas_as_tarefas();

    Tarefa *added = gt._adicionar_tarefa_interna(
        QStringLiteral("Escrever Relatório"),
        QStringLiteral("Importante e Urgente"),
        QStringLiteral("Todo"),
        false,
        QStringLiteral("Resumo semanal"),
        QString(),
        QStringLiteral("2026-11-30"),
        QStringLiteral("15:00"),
        QStringLiteral("kanban-1"),
        false,
        1);
    QVERIFY(added != nullptr);
    QCOMPARE(gt.tarefas.size(), 1);

    gt.on_task_duration_changed(QStringLiteral("kanban-1"), 4.5);
    QCOMPARE(gt.tarefas.first().duracao, 4.5);

    gt.salvar_tarefas();
    const QVector<Tarefa> reloaded = gt.carregar_tarefas();
    QVERIFY(!reloaded.isEmpty());
}

void TestPluginPomodoro::test_event_bus_messaging()
{
    auto *bus = compat::get_event_bus();
    QVERIFY(bus != nullptr);
    bus->clear_pending();

    QSignalSpy conceito_spy(bus, &compat::EventBus::conceito_atualizado);
    QSignalSpy tarefa_spy(bus, &compat::EventBus::tarefa_integracao);

    bus->send_conceito(QVariantMap{{QStringLiteral("titulo"), QStringLiteral("Grafos")}});
    bus->send_tarefa(QVariantMap{{QStringLiteral("titulo"), QStringLiteral("Implementar Dijkstra")}});

    QCOMPARE(conceito_spy.count(), 1);
    QCOMPARE(tarefa_spy.count(), 1);
    bus->clear_pending();
}

QTEST_MAIN(TestPluginPomodoro)
#include "test_plugin_pomodoro.moc"
