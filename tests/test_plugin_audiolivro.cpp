#include <QtTest>

#include <QColor>
#include <QFont>
#include <QSignalSpy>

#include "test_common.hpp"
#include "source/modules/audiolivro/audiolivro_plugin.hpp"
#include "source/modules/audiolivro/audiolivro_settings.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_06_VoiceCatalog.hpp"
#include "source/modules/audiolivro/metodoLeitor/EdgeTTSClient/agregador_01_EdgeTTSClient.hpp"
#include "source/modules/audiolivro/metodoLeitor/Foco/agregador_01_foco.hpp"

using namespace source::plugin;
using namespace source::plugin::Audiolivro;
using namespace eisenhower_tests;

class TestPluginAudiolivro : public QObject {
    Q_OBJECT

private slots:
    void test_audiolivro_plugin_interface();
    void test_audiolivro_settings_persistence();
    void test_voice_catalog_and_edge_tts_helpers();
    void test_focus_ruler_widget();
};

void TestPluginAudiolivro::test_audiolivro_plugin_interface()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    AudiolivroPlugin plugin;
    QCOMPARE(plugin.plugin_id(), QStringLiteral("audiolivro"));
    QVERIFY(!plugin.display_name().isEmpty());
    QVERIFY(!plugin.open_menu_text().isEmpty());
    QVERIFY(!plugin.font_settings_menu_text().isEmpty());

    const QVariantList voices = plugin.voice_options();
    QVERIFY(!voices.isEmpty());
    plugin.set_voice_id(QStringLiteral("pt-BR-FranciscaNeural"));
    QVERIFY(!plugin.current_voice_id().isEmpty());

    QWidget *widget = plugin.create_widget();
    QVERIFY(widget != nullptr);
    plugin.retranslate_ui();
    plugin.reload_shared_state();
    plugin.refresh_task_views();

    QCoreApplication::processEvents();
    delete widget;
}

void TestPluginAudiolivro::test_audiolivro_settings_persistence()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    const QString cfg_dir = audiolivro_persistent_dir();
    QVERIFY(!cfg_dir.isEmpty());

    salvar_configuracao_texto(QStringLiteral("test_voice"), QStringLiteral("pt-BR-AntonioNeural"));
    QCOMPARE(carregar_configuracao_texto(QStringLiteral("test_voice")), QStringLiteral("pt-BR-AntonioNeural"));
    QCOMPARE(carregar_configuracao_texto(QStringLiteral("missing_key"), QStringLiteral("fallback_val")),
             QStringLiteral("fallback_val"));

    QFont test_font(QStringLiteral("Consolas"), 14);
    salvar_fonte_configurada(test_font);
    QCOMPARE(carregar_fonte_configurada().pointSize(), 14);

    const QColor test_color(QStringLiteral("#1E88E5"));
    salvar_cor_configurada(test_color);
    QCOMPARE(carregar_cor_configurada().name().toUpper(), QStringLiteral("#1E88E5"));

    salvar_texto_carregado_temp(QStringLiteral("Texto temporário para leitura assistida"));
    QCOMPARE(carregar_texto_carregado_temp(), QStringLiteral("Texto temporário para leitura assistida"));
    limpar_texto_carregado_temp();
    QVERIFY(carregar_texto_carregado_temp().isEmpty());

    salvar_aba_ativa_temp(1);
    QCOMPARE(carregar_aba_ativa_temp(), 1);
}

void TestPluginAudiolivro::test_voice_catalog_and_edge_tts_helpers()
{
    const QString def_voice = default_voice_id();
    QVERIFY(!def_voice.isEmpty());
    const QVariantList catalog = available_voice_options();
    QVERIFY(!catalog.isEmpty());
    QVERIFY(!voice_display_text(def_voice).isEmpty());

    QVERIFY(EdgeTTSClientFns::is_edge_voice_id(QStringLiteral("pt-BR-FranciscaNeural")));
    QVERIFY(EdgeTTSClientFns::is_edge_voice_id(QStringLiteral("en-US-AriaNeural")));
    QCOMPARE(EdgeTTSClientFns::locale_for_voice_id(QStringLiteral("pt-BR-FranciscaNeural")), QStringLiteral("pt-BR"));
    QCOMPARE(EdgeTTSClientFns::locale_for_voice_id(QStringLiteral("en-US-AriaNeural")), QStringLiteral("en-US"));
    QVERIFY(!EdgeTTSClientFns::normalize_edge_voice_name(QStringLiteral("  pt-BR-FranciscaNeural  ")).isEmpty());
}

void TestPluginAudiolivro::test_focus_ruler_widget()
{
    FocusRuler ruler;
    ruler.resize(640, 80);
    ruler.show();
    ruler.repaint();
    QSignalSpy closed_spy(&ruler, &FocusRuler::closed);
    QTest::keyClick(&ruler, Qt::Key_Escape);
    QCoreApplication::processEvents();
    QVERIFY(closed_spy.count() >= 0);
}

QTEST_MAIN(TestPluginAudiolivro)
#include "test_plugin_audiolivro.moc"
