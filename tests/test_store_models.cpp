#include <QtTest>

#include <QDir>
#include <QFile>
#include <QSignalSpy>

#include "test_common.hpp"
#include "source/models/model_01_TaskRecordListModel.hpp"
#include "source/services/Arquivo/arq_05_parse_text_date_time.hpp"
#include "source/utils/UnifiedTaskStore.hpp"

using namespace source::utils;
using namespace source::models;
using namespace eisenhower_tests;

class TestStoreModels : public QObject {
    Q_OBJECT

private slots:
    void test_unified_task_store_normalization_and_roundtrip();
    void test_unified_task_store_merge_projection_and_remove();
    void test_unified_task_store_legacy_import_and_corrupt_recovery();
    void test_task_record_list_model_contract();
    void test_arquivo_parse_text_date_time_data();
    void test_arquivo_parse_text_date_time();
};

void TestStoreModels::test_unified_task_store_normalization_and_roundtrip()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());
    ScopedAppIdentity guard("LUMEN");

    const QString store_path = UnifiedTaskStore::shared_tasks_path();
    QVERIFY(store_path.endsWith(QStringLiteral("tarefas.json")));

    QVariantMap raw;
    raw.insert(QStringLiteral("modulo"), QStringLiteral("eisenhower"));
    raw.insert(QStringLiteral("titulo"), QStringLiteral("  Revisar Teorema de Stokes  "));
    raw.insert(QStringLiteral("prioridade"), 2);
    raw.insert(QStringLiteral("dominio"), 5);  // clamped to 2
    raw.insert(QStringLiteral("data"), QStringLiteral("2026-12-01"));
    raw.insert(QStringLiteral("hora"), QStringLiteral("14:30"));
    raw.insert(QStringLiteral("duracao"), 3.5);
    raw.insert(QStringLiteral("arquivo_vinculado"), QStringLiteral("C:/docs/calculo.pdf"));

    const QVariantMap normalized = UnifiedTaskStore::normalize_record(raw);
    QVERIFY(!normalized.value(QStringLiteral("id")).toString().isEmpty());
    QCOMPARE(normalized.value(QStringLiteral("titulo")).toString(), QStringLiteral("Revisar Teorema de Stokes"));
    QCOMPARE(normalized.value(QStringLiteral("prioridade")).toInt(), 2);
    QCOMPARE(normalized.value(QStringLiteral("quadrante")).toInt(), 2);
    QCOMPARE(normalized.value(QStringLiteral("dominio")).toInt(), 2);
    QCOMPARE(normalized.value(QStringLiteral("duracao")).toDouble(), 3.5);

    QVERIFY(UnifiedTaskStore::save_records(store_path, QVariantList{normalized}));
    const QVariantList reloaded = UnifiedTaskStore::load_records(store_path);
    QCOMPARE(reloaded.size(), 1);
    QCOMPARE(reloaded.first().toMap().value(QStringLiteral("id")), normalized.value(QStringLiteral("id")));
    QCOMPARE(reloaded.first().toMap().value(QStringLiteral("arquivo_vinculado")).toString(), QStringLiteral("C:/docs/calculo.pdf"));
}

void TestStoreModels::test_unified_task_store_merge_projection_and_remove()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    QVariantMap eisen_rec{
        {QStringLiteral("id"), QStringLiteral("task-42")},
        {QStringLiteral("modulo"), QStringLiteral("eisenhower")},
        {QStringLiteral("titulo"), QStringLiteral("Estudar Redes de Computadores")},
        {QStringLiteral("prioridade"), 1},
        {QStringLiteral("quadrante"), 1},
        {QStringLiteral("dominio"), 1},
        {QStringLiteral("status"), QStringLiteral("Doing")},
        {QStringLiteral("in_progress"), true},
        {QStringLiteral("duracao"), 2.0},
    };
    QVariantMap promo_rec{
        {QStringLiteral("id"), QStringLiteral("task-42")},
        {QStringLiteral("modulo"), QStringLiteral("pomodoro")},
        {QStringLiteral("titulo"), QStringLiteral("Estudar Redes de Computadores")},
        {QStringLiteral("prioridade"), 1},
        {QStringLiteral("status"), QStringLiteral("Doing")},
        {QStringLiteral("pomodoros_completados"), 2},
        {QStringLiteral("ciclos_totais"), 4},
        {QStringLiteral("sincronizada_com_eisenhower"), true},
    };

    QVariantList merged = UnifiedTaskStore::merge_module_records({}, QStringLiteral("eisenhower"), {eisen_rec});
    merged = UnifiedTaskStore::merge_module_records(merged, QStringLiteral("pomodoro"), {promo_rec});

    const QVariantList eisen_filtered = UnifiedTaskStore::filter_records_by_module(merged, QStringLiteral("eisenhower"));
    const QVariantList promo_filtered = UnifiedTaskStore::filter_records_by_module(merged, QStringLiteral("pomodoro"));
    QCOMPARE(eisen_filtered.size(), 1);
    QCOMPARE(promo_filtered.size(), 1);

    const QVariantList feynman_projected = UnifiedTaskStore::project_records_for_module(merged, QStringLiteral("feynman"));
    QVERIFY(!feynman_projected.isEmpty());

    const QVariantList after_remove = UnifiedTaskStore::remove_linked_records(
        merged,
        eisen_rec,
        {QStringLiteral("eisenhower"), QStringLiteral("pomodoro")});
    QCOMPARE(after_remove.size(), 0);
}

void TestStoreModels::test_unified_task_store_legacy_import_and_corrupt_recovery()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    const QString corrupt_path = sandbox.file_path(QStringLiteral("corrupt_tasks.json"));
    {
        QFile f(corrupt_path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write("{ INVALID JSON CONTENT ");
    }

    const QVariantList recovered = UnifiedTaskStore::load_records(corrupt_path);
    QCOMPARE(recovered.size(), 0);

    const QString legacy_path = sandbox.file_path(QStringLiteral("legacy_pomodoro.json"));
    {
        QFile f(legacy_path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write(R"([
            {"id":"legacy-1","titulo":"Legacy Item","prioridade":"Importante e Urgente","status":"Todo","pomodoros":1}
        ])");
    }

    const QVariantList loaded_legacy = UnifiedTaskStore::load_records(legacy_path);
    QCOMPARE(loaded_legacy.size(), 1);
    QCOMPARE(loaded_legacy.first().toMap().value(QStringLiteral("modulo")).toString(), QStringLiteral("pomodoro"));
    QCOMPARE(loaded_legacy.first().toMap().value(QStringLiteral("titulo")).toString(), QStringLiteral("Legacy Item"));
}

void TestStoreModels::test_task_record_list_model_contract()
{
    TaskRecordListModel model;
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(!model.data(model.index(0, 0), Qt::DisplayRole).isValid());
    QVERIFY(model.record_at(0).isEmpty());

    const QHash<int, QByteArray> roles = model.roleNames();
    QCOMPARE(roles.value(TaskRecordListModel::SearchableRole), QByteArray("searchable"));
    QCOMPARE(roles.value(TaskRecordListModel::RecordRole), QByteArray("record"));

    QSignalSpy reset_spy(&model, &QAbstractItemModel::modelReset);
    QVariantList records{
        QVariantMap{
            {QStringLiteral("display"), QStringLiteral("Tarefa A [Q1]")},
            {QStringLiteral("searchable"), QStringLiteral("tarefa a q1 urgente")},
            {QStringLiteral("id"), QStringLiteral("a-1")},
        },
        QVariantMap{
            {QStringLiteral("display"), QStringLiteral("Tarefa B [Q2]")},
            {QStringLiteral("searchable"), QStringLiteral("tarefa b q2")},
            {QStringLiteral("id"), QStringLiteral("b-2")},
        },
    };
    model.set_records(records);

    QCOMPARE(reset_spy.count(), 1);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);
    QCOMPARE(model.data(model.index(0, 0), Qt::DisplayRole).toString(), QStringLiteral("Tarefa A [Q1]"));
    QCOMPARE(model.data(model.index(1, 0), TaskRecordListModel::SearchableRole).toString(), QStringLiteral("tarefa b q2"));
    QCOMPARE(model.data(model.index(1, 0), TaskRecordListModel::RecordRole).toMap().value(QStringLiteral("id")).toString(),
             QStringLiteral("b-2"));
    QCOMPARE(model.record_at(0).value(QStringLiteral("id")).toString(), QStringLiteral("a-1"));
    QVERIFY(model.record_at(-1).isEmpty());
}

void TestStoreModels::test_arquivo_parse_text_date_time_data()
{
    QTest::addColumn<QString>("raw");
    QTest::addColumn<QString>("expected_text");
    QTest::addColumn<QString>("expected_date");
    QTest::addColumn<QString>("expected_time");

    QTest::newRow("plain_task") << QStringLiteral("Ler artigo cientifico")
                                << QStringLiteral("Ler artigo cientifico")
                                << QString() << QString();
    QTest::newRow("date_and_time_dash") << QString::fromUtf8("Reuniao Arquitetura — 2026-12-15 14:30")
                                        << QStringLiteral("Reuniao Arquitetura")
                                        << QStringLiteral("2026-12-15")
                                        << QStringLiteral("14:30");
    QTest::newRow("br_date") << QString::fromUtf8("Prova Calculo — 25/11/2026")
                             << QStringLiteral("Prova Calculo")
                             << QStringLiteral("2026-11-25")
                             << QString();
    QTest::newRow("time_only") << QString::fromUtf8("Standup Diario — 09:00")
                               << QStringLiteral("Standup Diario")
                               << QString()
                               << QStringLiteral("09:00");
}

void TestStoreModels::test_arquivo_parse_text_date_time()
{
    QFETCH(QString, raw);
    QFETCH(QString, expected_text);
    QFETCH(QString, expected_date);
    QFETCH(QString, expected_time);

    const auto parsed = source::InterfaceCore::ArquivoModules::parse_text_date_time(nullptr, raw);
    QCOMPARE(parsed.text, expected_text);
    QCOMPARE(parsed.date_iso, expected_date);
    QCOMPARE(parsed.time_str, expected_time);
}

QTEST_MAIN(TestStoreModels)
#include "test_store_models.moc"
