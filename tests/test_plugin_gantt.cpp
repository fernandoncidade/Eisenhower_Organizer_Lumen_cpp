#include <QtTest>

#include <QDateTime>
#include <QDir>

#include "test_common.hpp"
#include "source/modules/gantt/gantt_plugin.hpp"
#include "source/modules/gantt/Gantt/application/services/Bim4DService.hpp"
#include "source/modules/gantt/Gantt/application/services/CsvExchangeService.hpp"
#include "source/modules/gantt/Gantt/application/services/FieldProgressService.hpp"
#include "source/modules/gantt/Gantt/application/services/ScheduleService.hpp"
#include "source/modules/gantt/Gantt/application/services/SecurityService.hpp"
#include "source/modules/gantt/Gantt/domain/models/AuditLog.hpp"
#include "source/modules/gantt/Gantt/domain/models/BimLink.hpp"
#include "source/modules/gantt/Gantt/domain/models/Dependency.hpp"
#include "source/modules/gantt/Gantt/domain/models/GanttEnums.hpp"
#include "source/modules/gantt/Gantt/domain/models/Schedule.hpp"
#include "source/modules/gantt/Gantt/domain/models/ScheduleItem.hpp"
#include "source/modules/gantt/Gantt/domain/rules/CpmEngine.hpp"
#include "source/modules/gantt/Gantt/domain/rules/DependencyValidator.hpp"
#include "source/modules/gantt/Gantt/infrastructure/connectors/MsProjectXmlConnector.hpp"
#include "source/modules/gantt/Gantt/infrastructure/persistence/GanttDatabase.hpp"

using namespace source::gantt;
using namespace source::plugin;
using namespace eisenhower_tests;

class TestPluginGantt : public QObject {
    Q_OBJECT

private slots:
    void test_dependency_validator_dag_and_cycles();
    void test_cpm_engine_critical_path();
    void test_schedule_models_and_json_roundtrip();
    void test_gantt_database_sqlite_persistence();
    void test_schedule_service_and_templates();
    void test_csv_exchange_roundtrip();
    void test_ms_project_xml_roundtrip();
    void test_field_progress_and_bim4d();
    void test_security_service_rbac_and_ratelimit();
    void test_gantt_plugin_interface();
};

void TestPluginGantt::test_dependency_validator_dag_and_cycles()
{
    const QList<QString> nodes{QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C"), QStringLiteral("D")};
    const QList<Dependency> acyclic{
        Dependency(QStringLiteral("A"), QStringLiteral("B"), DependencyType::FinishToStart, 0.0),
        Dependency(QStringLiteral("A"), QStringLiteral("C"), DependencyType::FinishToStart, 0.0),
        Dependency(QStringLiteral("B"), QStringLiteral("D"), DependencyType::FinishToStart, 0.0),
        Dependency(QStringLiteral("C"), QStringLiteral("D"), DependencyType::FinishToStart, 0.0),
    };

    const auto dag_check = DependencyValidator::check_cycles(nodes, acyclic);
    QVERIFY(!dag_check.has_cycle);

    QList<QString> sorted;
    QVERIFY(DependencyValidator::topological_sort(nodes, acyclic, sorted));
    QCOMPARE(sorted.size(), 4);
    QVERIFY(sorted.indexOf(QStringLiteral("A")) < sorted.indexOf(QStringLiteral("B")));
    QVERIFY(sorted.indexOf(QStringLiteral("B")) < sorted.indexOf(QStringLiteral("D")));

    QString err;
    QVERIFY(!DependencyValidator::can_add_dependency(nodes, acyclic,
        Dependency(QStringLiteral("D"), QStringLiteral("A")), &err));
    QVERIFY(!err.isEmpty());

    QVERIFY(!DependencyValidator::can_add_dependency(nodes, acyclic,
        Dependency(QStringLiteral("B"), QStringLiteral("B")), &err));

    QList<Dependency> cyclic = acyclic;
    cyclic.append(Dependency(QStringLiteral("D"), QStringLiteral("A")));
    const auto cycle_res = DependencyValidator::check_cycles(nodes, cyclic);
    QVERIFY(cycle_res.has_cycle);
}

void TestPluginGantt::test_cpm_engine_critical_path()
{
    Schedule schedule(QStringLiteral("CPM Benchmark"), QStringLiteral("PROJ-CPM"));

    ScheduleItem a(QStringLiteral("Fundação"));
    a.id = QStringLiteral("A");
    a.wbs_code = QStringLiteral("1.1");
    a.duration_hours = 16.0;

    ScheduleItem b(QStringLiteral("Estrutura"));
    b.id = QStringLiteral("B");
    b.wbs_code = QStringLiteral("1.2");
    b.duration_hours = 24.0;

    ScheduleItem c(QStringLiteral("Instalações"));
    c.id = QStringLiteral("C");
    c.wbs_code = QStringLiteral("1.3");
    c.duration_hours = 8.0;

    ScheduleItem d(QStringLiteral("Acabamento"));
    d.id = QStringLiteral("D");
    d.wbs_code = QStringLiteral("1.4");
    d.duration_hours = 8.0;

    QVERIFY(schedule.add_item(a));
    QVERIFY(schedule.add_item(b));
    QVERIFY(schedule.add_item(c));
    QVERIFY(schedule.add_item(d));

    QVERIFY(schedule.add_dependency(Dependency(QStringLiteral("A"), QStringLiteral("B"))));
    QVERIFY(schedule.add_dependency(Dependency(QStringLiteral("A"), QStringLiteral("C"))));
    QVERIFY(schedule.add_dependency(Dependency(QStringLiteral("B"), QStringLiteral("D"))));
    QVERIFY(schedule.add_dependency(Dependency(QStringLiteral("C"), QStringLiteral("D"))));

    const QDateTime start(QDate(2026, 6, 1), QTime(8, 0), QTimeZone::utc());
    const auto result = CpmEngine::calculate(schedule, start);
    QVERIFY2(result.success, qPrintable(result.error_message));

    const ScheduleItem *res_a = schedule.find_item(QStringLiteral("A"));
    const ScheduleItem *res_b = schedule.find_item(QStringLiteral("B"));
    const ScheduleItem *res_c = schedule.find_item(QStringLiteral("C"));
    const ScheduleItem *res_d = schedule.find_item(QStringLiteral("D"));
    QVERIFY(res_a && res_b && res_c && res_d);

    QVERIFY(res_a->critical);
    QVERIFY(res_b->critical);
    QVERIFY(!res_c->critical);
    QVERIFY(res_d->critical);
    QVERIFY(res_c->total_float_hours > 0.0);
    QVERIFY(qFuzzyCompare(res_a->total_float_hours, 0.0));
}

void TestPluginGantt::test_schedule_models_and_json_roundtrip()
{
    Schedule s(QStringLiteral("Obra Teste"), QStringLiteral("PRJ-1"));
    ScheduleItem item(QStringLiteral("Escavação"));
    item.wbs_code = QStringLiteral("1.1");
    item.duration_hours = 16.0;
    item.percent_complete = 50.0;
    item.status = ActivityStatus::InProgress;
    QVERIFY(s.add_item(item));

    const QJsonObject json = s.to_json();
    const Schedule restored = Schedule::from_json(json);
    QCOMPARE(restored.name, s.name);
    QCOMPARE(restored.items.size(), 1);
    QCOMPARE(restored.items.value(item.id).percent_complete, 50.0);
}

void TestPluginGantt::test_gantt_database_sqlite_persistence()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    GanttDatabase db;
    const QString db_path = sandbox.file_path(QStringLiteral("gantt_test.sqlite"));
    QString err;
    QVERIFY2(db.initialize(db_path, &err), qPrintable(err));
    QVERIFY(db.is_open());

    Schedule sched(QStringLiteral("Cronograma SQLite"), QStringLiteral("SQL-1"));
    ScheduleItem it(QStringLiteral("Terraplanagem"));
    it.wbs_code = QStringLiteral("1.1");
    it.duration_hours = 16.0;
    QVERIFY(sched.add_item(it));

    QVERIFY2(db.save_schedule(sched, &err), qPrintable(err));
    const QList<Schedule> all = db.list_schedules(&err);
    QVERIFY(!all.isEmpty());

    Schedule loaded;
    QVERIFY2(db.load_schedule(sched.id, loaded, &err), qPrintable(err));
    QCOMPARE(loaded.id, sched.id);
    QCOMPARE(loaded.items.size(), 1);

    AuditLog log(QStringLiteral("Schedule"), sched.id, QStringLiteral("CREATE"), QStringLiteral("engineer1"), UserRole::Engineer);
    QVERIFY2(db.save_audit_log(log, &err), qPrintable(err));
    const QList<AuditLog> logs = db.load_audit_logs(QStringLiteral("Schedule"), sched.id, 10);
    QVERIFY(!logs.isEmpty());

    QVERIFY2(db.delete_schedule(sched.id, &err), qPrintable(err));
    db.close();
}

void TestPluginGantt::test_schedule_service_and_templates()
{
    ScheduleService service;
    Schedule created = service.create_schedule(QStringLiteral("Residencial Alpha"), QStringLiteral("RES-1"), QStringLiteral("Desc"));
    QVERIFY(!created.id.isEmpty());
    QVERIFY(service.get_schedule(created.id) != nullptr);

    ScheduleItem i1(QStringLiteral("Tarefa 1"));
    i1.wbs_code = QStringLiteral("1");
    ScheduleItem i2(QStringLiteral("Tarefa 2"));
    i2.wbs_code = QStringLiteral("2");
    QString err;
    QVERIFY2(service.add_activity(created.id, i1, &err), qPrintable(err));
    QVERIFY2(service.add_activity(created.id, i2, &err), qPrintable(err));
    QVERIFY2(service.add_dependency(created.id, Dependency(i1.id, i2.id), &err), qPrintable(err));

    QVERIFY(service.delete_schedule(created.id));
    QCOMPARE(service.get_schedule(created.id), nullptr);
}

void TestPluginGantt::test_csv_exchange_roundtrip()
{
    QTemporaryDir temp_dir;
    QVERIFY(temp_dir.isValid());

    Schedule orig(QStringLiteral("CSV Export"));
    ScheduleItem a(QStringLiteral("Planejamento"));
    a.id = QStringLiteral("item-1");
    a.wbs_code = QStringLiteral("1.1");
    a.duration_hours = 8.0;
    a.start_date = QDateTime(QDate(2026, 7, 1), QTime(8, 0), QTimeZone::utc());
    a.end_date = QDateTime(QDate(2026, 7, 1), QTime(16, 0), QTimeZone::utc());
    ScheduleItem b(QStringLiteral("Execução"));
    b.id = QStringLiteral("item-2");
    b.wbs_code = QStringLiteral("1.2");
    b.duration_hours = 16.0;
    b.start_date = QDateTime(QDate(2026, 7, 2), QTime(8, 0), QTimeZone::utc());
    b.end_date = QDateTime(QDate(2026, 7, 3), QTime(16, 0), QTimeZone::utc());
    orig.add_item(a);
    orig.add_item(b);
    orig.add_dependency(Dependency(a.id, b.id));

    CsvExchangeService csv;
    const QString csv_file = QDir(temp_dir.path()).filePath(QStringLiteral("schedule.csv"));
    QString err;
    QVERIFY2(csv.export_to_file(orig, csv_file, &err), qPrintable(err));

    Schedule target(QStringLiteral("CSV Import"));
    const CsvImportResult res = csv.import_from_file(target, csv_file);
    QVERIFY(res.success);
    QCOMPARE(res.items_imported, 2);
}

void TestPluginGantt::test_ms_project_xml_roundtrip()
{
    QTemporaryDir temp_dir;
    QVERIFY(temp_dir.isValid());

    Schedule orig(QStringLiteral("MSP XML"));
    ScheduleItem a(QStringLiteral("Projeto Executivo"));
    a.wbs_code = QStringLiteral("1.1");
    a.duration_hours = 24.0;
    a.start_date = QDateTime(QDate(2026, 8, 1), QTime(8, 0), QTimeZone::utc());
    a.end_date = QDateTime(QDate(2026, 8, 3), QTime(17, 0), QTimeZone::utc());
    orig.add_item(a);

    MsProjectXmlConnector xml;
    const QString xml_path = QDir(temp_dir.path()).filePath(QStringLiteral("schedule.xml"));
    QString err;
    QVERIFY2(xml.export_to_file(orig, xml_path, &err), qPrintable(err));

    Schedule imported(QStringLiteral("MSP Imported"));
    QVERIFY2(xml.import_from_file(imported, xml_path, &err), qPrintable(err));
    QCOMPARE(imported.items.size(), 1);
}

void TestPluginGantt::test_field_progress_and_bim4d()
{
    Schedule sched(QStringLiteral("BIM 4D"));
    ScheduleItem wall(QStringLiteral("Alvenaria"));
    wall.id = QStringLiteral("wall-1");
    wall.wbs_code = QStringLiteral("2.1");
    wall.start_date = QDateTime(QDate(2026, 5, 1), QTime(8, 0), QTimeZone::utc());
    wall.end_date = QDateTime(QDate(2026, 5, 10), QTime(17, 0), QTimeZone::utc());
    wall.duration_hours = 80.0;
    sched.add_item(wall);

    Bim4DService bim;
    BimLink link(wall.id, QStringLiteral("2O2Fr$t4X7Zf8NOew3FLOH"), BimSource::IFC);
    link.category = QStringLiteral("IfcWall");
    link.level = QStringLiteral("Térreo");
    QVERIFY(bim.add_link(sched, link));

    const auto report_before = bim.simulate(sched, QDateTime(QDate(2026, 4, 15), QTime(12, 0), QTimeZone::utc()));
    QCOMPARE(report_before.total_elements, 1);
    QCOMPARE(report_before.unstarted_count, 1);

    FieldProgressService progress;
    bool callback_called = false;
    QVERIFY(progress.link_eisenhower_task(sched, wall.id, QStringLiteral("eisen-99")));
    progress.set_host_sync_callback([&](const QString &task_id, double pct, bool is_done) {
        callback_called = (task_id == QStringLiteral("eisen-99") && qFuzzyCompare(pct, 50.0) && !is_done);
    });
    QVERIFY(progress.record_progress(sched, wall.id, 50.0, QStringLiteral("Meio concluído")));
    QVERIFY(callback_called);

    const auto report_mid = bim.simulate(sched, QDateTime(QDate(2026, 5, 5), QTime(12, 0), QTimeZone::utc()));
    QCOMPARE(report_mid.planned_in_progress_count, 1);

    ScheduleItem *wall_ptr = sched.find_item(wall.id);
    QVERIFY(wall_ptr != nullptr);
    wall_ptr->status = ActivityStatus::NotStarted;  // Exercise percent > 0 + end_date < eval_date overdue branch
    const auto report_late = bim.simulate(sched, QDateTime(QDate(2026, 5, 15), QTime(12, 0), QTimeZone::utc()));
    QCOMPARE(report_late.delayed_overdue_count, 1);
}

void TestPluginGantt::test_security_service_rbac_and_ratelimit()
{
    SecurityService sec;
    const QString token = sec.issue_token(QStringLiteral("eng_user"), UserRole::Engineer, 4);
    QVERIFY(!token.isEmpty());

    UserRole role = UserRole::Viewer;
    QString uid;
    QVERIFY(sec.validate_token(token, role, uid));
    QCOMPARE(role, UserRole::Engineer);
    QCOMPARE(uid, QStringLiteral("eng_user"));

    QVERIFY(!sec.validate_token(QStringLiteral("invalid-token"), role, uid));

    QVERIFY(sec.can_view(UserRole::Viewer));
    QVERIFY(!sec.can_edit(UserRole::Viewer));
    QVERIFY(sec.can_edit(UserRole::Engineer));
    QVERIFY(!sec.can_administer(UserRole::Engineer));
    QVERIFY(sec.can_administer(UserRole::Admin));
    QVERIFY(sec.can_manage_baselines(UserRole::Admin));
    QVERIFY(sec.can_manage_baselines(UserRole::Engineer));
    QVERIFY(!sec.can_manage_baselines(UserRole::Manager));
    QVERIFY(!sec.can_manage_baselines(UserRole::Viewer));

    QVERIFY(sec.check_rate_limit(QStringLiteral("client-1"), 2));
    QVERIFY(sec.check_rate_limit(QStringLiteral("client-1"), 2));
    QVERIFY(!sec.check_rate_limit(QStringLiteral("client-1"), 2));
}

void TestPluginGantt::test_gantt_plugin_interface()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    GanttPlugin plugin;
    QCOMPARE(plugin.plugin_id(), QStringLiteral("gantt"));
    QVERIFY(!plugin.display_name().isEmpty());
    QVERIFY(!plugin.open_menu_text().isEmpty());
    plugin.retranslate_ui();
    plugin.close_window();
}

QTEST_MAIN(TestPluginGantt)
#include "test_plugin_gantt.moc"
