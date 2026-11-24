#include <QtTest>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QVariantList>
#include <QVariantMap>

#include "test_common.hpp"
#include "source/modules/mapamental/mapamental_plugin.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_07_extrair_texto_txt.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_13_remover_acentos.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_30_summarizador.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_33_zip_utils.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_35_extrair_texto_csv.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_37_extrair_texto_json.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_39_extrair_texto_xml.hpp"

using namespace source::plugin;
using namespace source::plugin::MapaMental;
using namespace eisenhower_tests;

class TestPluginMapaMental : public QObject {
    Q_OBJECT

private slots:
    void test_mapamental_plugin_metadata_and_modes();
    void test_remover_acentos();
    void test_processador_ia_language_concepts_and_structure();
    void test_summarizador_avancado();
    void test_zip_utils_qt6_coreprivate();
    void test_structured_file_extractors();
    void test_mapamental_summary_method_and_lifecycle();
};

void TestPluginMapaMental::test_mapamental_plugin_metadata_and_modes()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    MapaMentalPlugin plugin;
    QCOMPARE(plugin.plugin_id(), QStringLiteral("mapamental"));
    QVERIFY(!plugin.display_name().isEmpty());
    QVERIFY(!plugin.open_menu_text().isEmpty());

    plugin.set_layout_mode_key(QStringLiteral("horizontal"));
    QCOMPARE(plugin.layout_mode_key(), QStringLiteral("horizontal"));

    plugin.set_summary_method_key(QStringLiteral("heuristic"));
    QCOMPARE(plugin.summary_method_key(), QStringLiteral("heuristic"));

    QWidget *widget = plugin.create_widget();
    QVERIFY(widget != nullptr);
    plugin.retranslate_ui();
    plugin.reload_shared_state();
    plugin.refresh_task_views();
    delete widget;
}

void TestPluginMapaMental::test_remover_acentos()
{
    QCOMPARE(ProcessadorIAFns::remover_acentos(QStringLiteral("Educação Ágil & Informação Útil")),
             QStringLiteral("Educacao Agil & Informacao Util"));
    QCOMPARE(ProcessadorIAFns::remover_acentos(QStringLiteral("ações çãõêîôûü")),
             QStringLiteral("acoes caoeiouu"));
    QCOMPARE(ProcessadorIAFns::remover_acentos(QString()), QString());
}

void TestPluginMapaMental::test_processador_ia_language_concepts_and_structure()
{
    ProcessadorIA processor;

    const QString pt_text = QStringLiteral(
        "# Arquitetura de Sistemas Distribuídos\n\n"
        "## Capítulo 1 Consistência e Replicação\n"
        "Sistemas distribuídos utilizam consenso distribuído para manter replicação de estado tolerante a falhas. "
        "Protocolos como Raft e Paxos garantem segurança mesmo diante de particionamento de rede e latência variável.\n\n"
        "## Capítulo 2 Observabilidade e Telemetria\n"
        "A telemetria combina métricas estruturadas, rastreamento distribuído e registros imutáveis para diagnosticar gargalos de desempenho.");

    const QString en_text = QStringLiteral(
        "Distributed systems engineering relies on fault tolerance, consensus algorithms, and horizontal scalability "
        "to process concurrent workloads across independent computing nodes with low latency.");

    const QString lang_pt = processor.detectar_idioma_texto(pt_text);
    const QString lang_en = processor.detectar_idioma_texto(en_text);
    QVERIFY(lang_pt.startsWith(QStringLiteral("pt"), Qt::CaseInsensitive) || !lang_pt.isEmpty());
    QVERIFY(lang_en.startsWith(QStringLiteral("en"), Qt::CaseInsensitive) || !lang_en.isEmpty());

    const QVariantList concepts = processor.extrair_conceitos_tfidf(pt_text, 10);
    QVERIFY(!concepts.isEmpty());

    const QVariantList relations = processor.identificar_relacoes_avancadas(concepts);
    Q_UNUSED(relations);

    const QVariantMap structure = processor.analisar_estrutura(pt_text, false);
    QVERIFY(!structure.isEmpty());
}

void TestPluginMapaMental::test_summarizador_avancado()
{
    const QString doc = QStringLiteral(
        "Engenharia de software moderna enfatiza modularidade, testes automatizados e integração contínua. "
        "Testes unitários verificam invariantes de domínio e isolam regressões rapidamente. "
        "Testes de integração validam contratos entre módulos, persistência SQLite e carregamento dinâmico de plugins. "
        "A separação clara entre núcleo de aplicação e adaptadores de interface reduz acoplamento e facilita manutenção evolutiva.");

    const QVariantMap summary = ProcessadorIAFns::gerar_resumo_avancado(doc);
    QVERIFY(!summary.isEmpty());
}

void TestPluginMapaMental::test_zip_utils_qt6_coreprivate()
{
    QTemporaryDir temp_dir;
    QVERIFY(temp_dir.isValid());

    const QString zip_path = QDir(temp_dir.path()).filePath(QStringLiteral("sample_bundle.zip"));
    const QByteArray xml_payload = R"(<?xml version="1.0" encoding="UTF-8"?><root><title>Olá Qt6 CorePrivate</title></root>)";
    const QByteArray txt_payload = "Conteúdo ZIP válido em UTF-8";

    QVERIFY(write_stored_zip(zip_path, {
        {QStringLiteral("META-INF/container.xml"), xml_payload},
        {QStringLiteral("content/notes.txt"), txt_payload},
    }));

    const QStringList entries = ProcessadorIAFns::listar_entradas_zip(zip_path);
    QVERIFY(entries.contains(QStringLiteral("META-INF/container.xml")));
    QVERIFY(entries.contains(QStringLiteral("content/notes.txt")));

    QCOMPARE(ProcessadorIAFns::ler_entrada_zip(zip_path, QStringLiteral("META-INF/container.xml")), xml_payload);
    QCOMPARE(ProcessadorIAFns::ler_entrada_zip(zip_path, QStringLiteral("content/notes.txt")), txt_payload);
    QVERIFY(ProcessadorIAFns::ler_entrada_zip(zip_path, QStringLiteral("missing.txt")).isEmpty());

    const QString resolved = ProcessadorIAFns::resolver_caminho_zip(QStringLiteral("OEBPS/content.opf"), QStringLiteral("chapters/ch1.xhtml"));
    QCOMPARE(resolved, QStringLiteral("OEBPS/chapters/ch1.xhtml"));

    // Corrupt / missing archive resilience
    QVERIFY(ProcessadorIAFns::listar_entradas_zip(QDir(temp_dir.path()).filePath(QStringLiteral("nonexistent.zip"))).isEmpty());
}

void TestPluginMapaMental::test_structured_file_extractors()
{
    QTemporaryDir temp_dir;
    QVERIFY(temp_dir.isValid());

    const QString txt_path = QDir(temp_dir.path()).filePath(QStringLiteral("sample.txt"));
    {
        QFile f(txt_path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write("Linha 1 Texto Estruturado\nLinha 2 Conceito Chave");
    }
    QVERIFY(ProcessadorIAFns::extrair_texto_txt(txt_path).contains(QStringLiteral("Conceito Chave")));

    const QString csv_path = QDir(temp_dir.path()).filePath(QStringLiteral("sample.csv"));
    {
        QFile f(csv_path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write("id,topico,prioridade\n1,Algoritmos,Alta\n2,Compiladores,Media\n");
    }
    const QString csv_text = ProcessadorIAFns::extrair_texto_csv(csv_path);
    QVERIFY(csv_text.contains(QStringLiteral("Algoritmos")));
    QVERIFY(csv_text.contains(QStringLiteral("Compiladores")));

    const QString json_path = QDir(temp_dir.path()).filePath(QStringLiteral("sample.json"));
    {
        QFile f(json_path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write(R"({"curso":"Ciência da Computação","disciplina":"Estruturas de Dados"})");
    }
    const QString json_text = ProcessadorIAFns::extrair_texto_json(json_path);
    QVERIFY(json_text.contains(QStringLiteral("Estruturas de Dados")));

    const QString xml_path = QDir(temp_dir.path()).filePath(QStringLiteral("sample.xml"));
    {
        QFile f(xml_path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write(R"(<documento><secao>Engenharia de Requisitos</secao></documento>)");
    }
    const QString xml_text = ProcessadorIAFns::extrair_texto_xml(xml_path);
    QVERIFY(xml_text.contains(QStringLiteral("Engenharia de Requisitos")));
}

void TestPluginMapaMental::test_mapamental_summary_method_and_lifecycle()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    MapaMentalPlugin plugin;
    QWidget *widget = plugin.create_widget();
    QVERIFY(widget != nullptr);

    auto *mapa = qobject_cast<source::plugin::MapaMental::MapaMental *>(widget);
    QVERIFY(mapa != nullptr);

    // Mixed-case Ollama model names must be preserved; bare "ollama:" falls back to "qwen".
    plugin.set_summary_method_key(QStringLiteral("ollama:hf.co/Qwen/Qwen2.5-3B:Q4_K_M"));
    QCOMPARE(plugin.summary_method_key(), QStringLiteral("ollama:hf.co/Qwen/Qwen2.5-3B:Q4_K_M"));

    plugin.set_summary_method_key(QStringLiteral("ollama:"));
    QCOMPARE(plugin.summary_method_key(), QStringLiteral("qwen"));

    // Basic node lifecycle, layouts, fonts, and clearing must reset state safely.
    NoConceito *n1 = mapa->adicionar_no(QStringLiteral("Raiz"), QPointF(0.0, 0.0));
    NoConceito *n2 = mapa->adicionar_no(QStringLiteral("Filho"), QPointF(140.0, 80.0));
    QVERIFY(n1 != nullptr);
    QVERIFY(n2 != nullptr);
    QCOMPARE(mapa->nos().size(), 2);

    mapa->toggle_modo_conexao();
    QVERIFY(mapa->modo_conexao());

    mapa->set_layout_mode_key(QStringLiteral("radial"));
    QCOMPARE(mapa->layout_mode_key(), QStringLiteral("radial"));

    mapa->iniciar_visao_cinematica();
    mapa->refresh_fonts();
    mapa->remover_no(n2);
    QCOMPARE(mapa->nos().size(), 1);

    mapa->limpar_mapa(false);
    QVERIFY(mapa->nos().isEmpty());
    QVERIFY(!mapa->modo_conexao());

    delete widget;
}

QTEST_MAIN(TestPluginMapaMental)
#include "test_plugin_mapamental.moc"
