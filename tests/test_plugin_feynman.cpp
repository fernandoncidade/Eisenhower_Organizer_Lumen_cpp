#include <QtTest>

#include <QDir>
#include <QSignalSpy>

#include "test_common.hpp"
#include "source/modules/feynman/feynman_plugin.hpp"
#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"
#include "source/modules/feynman/Feynman/fey_01_FeynmanWorker.hpp"

using namespace source::plugin;
using namespace source::plugin::Feynman;
using namespace eisenhower_tests;

class TestPluginFeynman : public QObject {
    Q_OBJECT

private slots:
    void test_feynman_plugin_metadata_and_widget();
    void test_feynman_metodo_helpers();
    void test_feynman_worker_roundtrip_and_signals();
};

void TestPluginFeynman::test_feynman_plugin_metadata_and_widget()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    FeynmanPlugin plugin;
    QCOMPARE(plugin.plugin_id(), QStringLiteral("feynman"));
    QVERIFY(!plugin.display_name().isEmpty());
    QVERIFY(!plugin.open_menu_text().isEmpty());

    QWidget *widget = plugin.create_widget();
    QVERIFY(widget != nullptr);
    plugin.retranslate_ui();

    plugin.receive_task(QVariantMap{
        {QStringLiteral("id"), QStringLiteral("eisen-fey-1")},
        {QStringLiteral("titulo"), QStringLiteral("Estudar Teorema Spectral")},
        {QStringLiteral("prioridade"), 1},
        {QStringLiteral("dominio"), 1},
    });
    plugin.receive_mindmap_concept(QVariantMap{
        {QStringLiteral("titulo"), QStringLiteral("Autovalores")},
        {QStringLiteral("resumo"), QStringLiteral("Decomposição de operadores lineares")},
    });
    plugin.refresh_task_views();

    QCoreApplication::processEvents();
    delete widget;
}

void TestPluginFeynman::test_feynman_metodo_helpers()
{
    QCOMPARE(normalized_domain_value(-1), 0);
    QCOMPARE(normalized_domain_value(9), 2);
    QCOMPARE(normalized_priority_value(0), 1);
    QCOMPARE(normalized_priority_value(3), 3);
    QCOMPARE(normalized_priority_value(4), 4);
    QCOMPARE(normalized_priority_value(5), kDefaultPriority);
    QCOMPARE(normalized_priority_value(QStringLiteral("Importante, mas Não Urgente")), 2);

    QVERIFY(!domain_symbol(0).isEmpty());
    QVERIFY(!domain_symbol(2).isEmpty());
    QVERIFY(!priority_emoji(1).isEmpty());
    QVERIFY(!priority_emoji(4).isEmpty());

    QCOMPARE(normalized_ui_layout_key(QStringLiteral("horizontal")), QStringLiteral("horizontal"));
    QVERIFY(is_horizontal_ui_layout(QStringLiteral("horizontal")));
    QVERIFY(!is_vertical_ui_layout(QStringLiteral("horizontal")));

    const QString composed = composed_description(nullptr,
        QStringLiteral("Algebra Linear"),
        QStringLiteral("Espaços vetoriais e transformações"),
        QStringLiteral("Demonstrar teorema núcleo-imagem"),
        QStringLiteral("Revisar exercícios capítulo 4"));
    const DescriptionParts parts = parse_description_text(composed);
    QVERIFY(parts.structured || !composed.isEmpty());

    QVariantMap raw_concept{
        {QStringLiteral("conceito"), QStringLiteral("Derivadas Parciais")},
        {QStringLiteral("prioridade"), 2},
        {QStringLiteral("dominio"), 1},
        {QStringLiteral("status"), QStringLiteral("Doing")},
    };
    const QVariantMap norm = normalized_concept(raw_concept);
    QVERIFY(!is_concept_completed(norm));
    QVERIFY(concept_group_key(norm).isEmpty());
    raw_concept.insert(QStringLiteral("integracao_date"), QStringLiteral("2026-11-30"));
    raw_concept.insert(QStringLiteral("integracao_time"), QStringLiteral("14:00"));
    QCOMPARE(concept_group_key(raw_concept), QStringLiteral("2026-11-30|14:00"));
    QVERIFY(!tooltip_html(nullptr, norm).isEmpty());
}

void TestPluginFeynman::test_feynman_worker_roundtrip_and_signals()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    const QString json_path = sandbox.file_path(QStringLiteral("feynman_conceitos.json"));
    const QVariantList payload{
        QVariantMap{
            {QStringLiteral("id"), QStringLiteral("c-1")},
            {QStringLiteral("titulo"), QStringLiteral("Redes Neurais")},
            {QStringLiteral("explicacao"), QStringLiteral("Retropropagação do gradiente")},
            {QStringLiteral("lacunas"), QStringLiteral("Normalização em lote")},
            {QStringLiteral("revisao"), QStringLiteral("Reimplementar MLP")},
            {QStringLiteral("prioridade"), 1},
            {QStringLiteral("dominio"), 1},
        }
    };

    QVERIFY(FeynmanWorker::save_file(json_path, payload));
    const QVariantList loaded = FeynmanWorker::load_file(json_path);
    QCOMPARE(loaded.size(), 1);
    QCOMPARE(loaded.first().toMap().value(QStringLiteral("titulo")).toString(), QStringLiteral("Redes Neurais"));

    FeynmanWorker worker;
    QSignalSpy saved_spy(&worker, &FeynmanWorker::save_finished);
    QSignalSpy loaded_spy(&worker, &FeynmanWorker::conceitos_loaded);
    emit worker.request_save(json_path, payload);
    QCoreApplication::processEvents();
    emit worker.request_load(json_path);
    QCoreApplication::processEvents();

    QCOMPARE(saved_spy.count(), 1);
    QCOMPARE(saved_spy.takeFirst().at(0).toBool(), true);
    QCOMPARE(loaded_spy.count(), 1);
}

QTEST_MAIN(TestPluginFeynman)
#include "test_plugin_feynman.moc"
