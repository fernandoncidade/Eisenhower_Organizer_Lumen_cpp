#include <QtTest>

#include <QDir>
#include <QFileInfo>
#include <QVariantList>
#include <QVariantMap>

#include "test_common.hpp"
#include "source/modules/modelo/modelo_plugin.hpp"
#include "source/modules/modelo/metodoModelo/qwen/ia_01_llama_local.hpp"
#include "source/modules/modelo/metodoModelo/ollama/ia_02_ollama_local.hpp"

using namespace source::plugin;
using namespace source::plugin::Modelo;
using namespace eisenhower_tests;

class TestPluginModelo : public QObject {
    Q_OBJECT

private slots:
    void test_modelo_plugin_interface_and_switching();
    void test_llama_local_paths_and_diagnostics();
    void test_ollama_local_nonblocking_diagnostics();
};

void TestPluginModelo::test_modelo_plugin_interface_and_switching()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    ModeloPlugin plugin;
    QCOMPARE(plugin.plugin_id(), QStringLiteral("modelo"));
    QVERIFY(!plugin.display_name().isEmpty());

    const QVariantMap caps = plugin.capabilities();
    QVERIFY(!caps.isEmpty());

    QCOMPARE(plugin.active_model(), QStringLiteral("qwen"));
    plugin.set_active_model(QStringLiteral("heuristic"));
    QCOMPARE(plugin.active_model(), QStringLiteral("heuristic"));
    QVERIFY(!plugin.is_available());
    plugin.set_active_model(QStringLiteral("qwen"));
    QVERIFY(!plugin.is_available());

    plugin.refresh_models();
    const QVariantList models = plugin.available_models();
    Q_UNUSED(models);

    const QVariantMap diag = plugin.diagnostic();
    QVERIFY(!diag.isEmpty());

    // Should fail/degrade gracefully if model is absent without crashing.
    const QVariantMap analysis = plugin.analyze_document(
        QStringLiteral("Texto curto de teste."),
        QStringLiteral("pt_BR"),
        {},
        {});
    Q_UNUSED(analysis);
}

void TestPluginModelo::test_llama_local_paths_and_diagnostics()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    const QString models_dir = ProcessadorIAFns::diretorio_modelos_persistente();
    QVERIFY(!models_dir.isEmpty());
    QVERIFY(QDir::fromNativeSeparators(models_dir).contains(QDir::fromNativeSeparators(sandbox.root_path()), Qt::CaseInsensitive));

    const QString dest_gguf = ProcessadorIAFns::caminho_destino_modelo_gguf();
    const QString filename = ProcessadorIAFns::nome_arquivo_modelo_gguf();
    QVERIFY(!filename.isEmpty());
    QVERIFY(dest_gguf.endsWith(filename));
    QVERIFY(filename.endsWith(QStringLiteral(".gguf"), Qt::CaseInsensitive));
    QVERIFY(ProcessadorIAFns::url_download_modelo_gguf().startsWith(QStringLiteral("https://")));
    QVERIFY(ProcessadorIAFns::tamanho_esperado_modelo_gguf() > 0);
    QVERIFY(!ProcessadorIAFns::modelo_gguf_instalado());
    QCOMPARE(ProcessadorIAFns::tamanho_modelo_gguf_instalado(), 0);
}

void TestPluginModelo::test_ollama_local_nonblocking_diagnostics()
{
    // Very short timeout so test finishes quickly even when Ollama service is stopped.
    const bool connected = ProcessadorIAFns::testar_conexao_ollama_api(150);
    Q_UNUSED(connected);

    const QVariantMap diag = ProcessadorIAFns::diagnosticar_ollama();
    QVERIFY(!diag.isEmpty());
}

QTEST_MAIN(TestPluginModelo)
#include "test_plugin_modelo.moc"
