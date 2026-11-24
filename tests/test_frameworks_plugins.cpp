#include <QtTest>

#include <QAudioDevice>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLibrary>
#include <QMediaDevices>
#include <QNetworkAccessManager>
#include <QPdfDocument>
#include <QPluginLoader>
#include <QSqlDatabase>
#include <QTranslator>
#include <QWebSocket>

#include "test_common.hpp"
#include "source/ui/ui_02_OpcoesSobre.hpp"
#include "source/modules/agenda/agenda_plugin.hpp"
#include "source/modules/audiolivro/audiolivro_plugin.hpp"
#include "source/modules/feynman/feynman_plugin.hpp"
#include "source/modules/gantt/gantt_plugin.hpp"
#include "source/modules/mapamental/mapamental_plugin.hpp"
#include "source/modules/modelo/modelo_plugin.hpp"
#include "source/modules/pomodoro/pomodoro_plugin.hpp"

using namespace source::plugin;
using namespace eisenhower_tests;

class TestFrameworksPlugins : public QObject {
    Q_OBJECT

private slots:
    void test_qt6_12_runtime_and_modules();
    void test_translations_assets_and_licenses_integrity();
    void test_inprocess_plugin_interfaces_contract();
    void test_dynamic_plugin_dll_loading();
};

void TestFrameworksPlugins::test_qt6_12_runtime_and_modules()
{
    const QString qt_ver = QString::fromLatin1(qVersion());
    QVERIFY2(qt_ver.startsWith(QStringLiteral("6.12.")),
             qPrintable(QStringLiteral("Expected Qt 6.12.x, got %1").arg(qt_ver)));

    QVERIFY2(QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE")),
             "QSQLITE driver must be available in Qt6::Sql runtime");

    QNetworkAccessManager nam;
    QPdfDocument pdf_doc;
    QWebSocket socket;
    const auto audio_outputs = QMediaDevices::audioOutputs();
    Q_UNUSED(nam);
    Q_UNUSED(pdf_doc);
    Q_UNUSED(socket);
    Q_UNUSED(audio_outputs);
}

void TestFrameworksPlugins::test_translations_assets_and_licenses_integrity()
{
    const QString translations_dir = source_path(QStringLiteral("source/language/translations"));
    QVERIFY(QDir(translations_dir).exists());

    const QStringList required_qms = {
        QStringLiteral("eisenhower_organizer_pt_BR.qm"),
        QStringLiteral("eisenhower_organizer_en_US.qm"),
        QStringLiteral("pomodoro_pt_BR.qm"),
        QStringLiteral("pomodoro_en_US.qm"),
        QStringLiteral("feynman_pt_BR.qm"),
        QStringLiteral("feynman_en_US.qm"),
        QStringLiteral("mapamental_pt_BR.qm"),
        QStringLiteral("mapamental_en_US.qm"),
        QStringLiteral("leitor_pt_BR.qm"),
        QStringLiteral("leitor_en_US.qm"),
        QStringLiteral("agenda_pt_BR.qm"),
        QStringLiteral("agenda_en_US.qm"),
        QStringLiteral("languagegantt_pt_BR.qm"),
        QStringLiteral("languagegantt_en_US.qm"),
    };

    for (const QString &qm : required_qms) {
        QTranslator tr;
        QVERIFY2(tr.load(qm, translations_dir), qPrintable(QStringLiteral("Failed to load %1").arg(qm)));
    }

    QVERIFY(QFileInfo::exists(source_path(QStringLiteral("source/assets/eisenhower/icones/organizador.ico"))));
    QVERIFY(QFileInfo::exists(source_path(QStringLiteral("source/assets/lumen/icones/autismo.ico"))));
    QVERIFY(QDir(source_path(QStringLiteral("source/assets/LICENSES"))).exists());
    QVERIFY(QDir(source_path(QStringLiteral("source/modules/modelo/metodoModelo/qwen/llama-b8987-bin-win-cpu-x64"))).exists());
    QVERIFY(QDir(source_path(QStringLiteral("source/modules/modelo/metodoModelo/qwen/llama-b8987-bin-win-vulkan-x64"))).exists());

    auto read_utf8 = [](const QString &rel_path) -> QString {
        QFile f(source_path(rel_path));
        if (!f.open(QIODevice::ReadOnly)) {
            return {};
        }
        return QString::fromUtf8(f.readAll());
    };

    const QString root_license = read_utf8(QStringLiteral("LICENSE"));
    QVERIFY2(root_license.contains(QStringLiteral("GNU AFFERO GENERAL PUBLIC LICENSE")),
             "Root LICENSE must contain GNU AGPLv3");

    const QString bundled_agpl = read_utf8(
        QStringLiteral("source/assets/LICENSES/GNU AFFERO GENERAL PUBLIC LICENSE Version 3.txt"));
    QVERIFY2(bundled_agpl.contains(QStringLiteral("GNU AFFERO GENERAL PUBLIC LICENSE")),
             "source/assets/LICENSES must bundle GNU AGPLv3");

    QVERIFY2(source::GerenciamentoUI::OpcoesSobre::SITE_LICENSES().contains(
                 QStringLiteral("GNU AFFERO GENERAL PUBLIC LICENSE Version 3.txt")),
             "OpcoesSobre::SITE_LICENSES must list GNU AGPLv3");

    const QStringList required_docs = {
        QStringLiteral("README.md"),
        QStringLiteral("OBSERVACAO.md"),
        QStringLiteral("MANUAL.md"),
        QStringLiteral("CODE_OF_CONDUCT.md"),
        QStringLiteral("CONTRIBUTING.md"),
        QStringLiteral("SECURITY.md"),
        QStringLiteral("docs/architecture.md"),
        QStringLiteral("docs/user-guide.md"),
        QStringLiteral("docs/compliance.md"),
        QStringLiteral("docs/licensing.md"),
        QStringLiteral("docs/privacy-policy.md"),
        QStringLiteral("docs/security-model.md"),
        QStringLiteral("docs/suggested-issues.md"),
        QStringLiteral("docs/technical-audit-and-roadmap.md"),
    };
    for (const QString &doc : required_docs) {
        const QString text = read_utf8(doc);
        QVERIFY2(text.size() > 200, qPrintable(QStringLiteral("Missing or empty documentation file: %1").arg(doc)));
    }

    QVERIFY2(!QFileInfo::exists(source_path(QStringLiteral("docs/cli-reference.md"))),
             "Removed docs/cli-reference.md must not exist");

    const QStringList release_docs = QDir(source_path(QStringLiteral("docs")))
                                         .entryList(QStringList{QStringLiteral("release-v*.md")}, QDir::Files);
    QVERIFY2(!release_docs.isEmpty(), "Expected at least one docs/release-v*.md file");
    const QString release_doc_text = read_utf8(QStringLiteral("docs/") + release_docs.first());
    QVERIFY2(release_doc_text.size() > 200, "Release notes markdown file must not be empty");

    QVERIFY2(QDir(source_path(QStringLiteral("images/interfaces"))).exists(),
             "images/interfaces directory must exist");
    QVERIFY2(!QDir(source_path(QStringLiteral("repo/LISTAGEM"))).exists(),
             "Legacy repo/LISTAGEM directory must not exist");

    const QStringList expected_library_files = {
        QStringLiteral("library/Eisenhower_Organizer.iss"),
        QStringLiteral("library/Eisenhower_Organizer_MinGW.iss"),
        QStringLiteral("library/Eisenhower_Organizer_MSVC.iss"),
        QStringLiteral("library/Lumen.iss"),
        QStringLiteral("library/Lumen_MinGW.iss"),
        QStringLiteral("library/Lumen_MSVC.iss"),
        QStringLiteral("library/E-AppxManifest.xml"),
        QStringLiteral("library/L-AppxManifest.xml"),
        QStringLiteral("library/limpar_artefatos_regeneraveis_qt.ps1"),
        QStringLiteral("library/MANUAL_TECNICO_CONFIGURACAO_AMBIENTE_CPP_QT6.txt"),
        QStringLiteral("library/Eisenhower_Organizer_Lumen_cpp.txt"),
    };
    for (const QString &lib_file : expected_library_files) {
        QVERIFY2(QFileInfo::exists(source_path(lib_file)),
                 qPrintable(QStringLiteral("Missing expected library file: %1").arg(lib_file)));
    }

    const QStringList dual_license_assets = {
        QStringLiteral("source/assets/eisenhower/ABOUT/ABOUT_pt_BR.txt"),
        QStringLiteral("source/assets/eisenhower/ABOUT/ABOUT_en_US.txt"),
        QStringLiteral("source/assets/eisenhower/CLC/CLC_pt_BR.txt"),
        QStringLiteral("source/assets/eisenhower/CLC/CLC_en_US.txt"),
        QStringLiteral("source/assets/eisenhower/COPYRIGHT/AVISO DE COPYRIGHT E MARCA REGISTRA_pt_BR.txt"),
        QStringLiteral("source/assets/eisenhower/COPYRIGHT/COPYRIGHT AND TRADEMARK NOTICE_en_US.txt"),
        QStringLiteral("source/assets/eisenhower/EULA/EULA_pt_BR.txt"),
        QStringLiteral("source/assets/eisenhower/EULA/EULA_en_US.txt"),
        QStringLiteral("source/assets/eisenhower/NOTICES/NOTICE_pt_BR.txt"),
        QStringLiteral("source/assets/eisenhower/NOTICES/NOTICE_en_US.txt"),
        QStringLiteral("source/assets/lumen/ABOUT/ABOUT_pt_BR.txt"),
        QStringLiteral("source/assets/lumen/ABOUT/ABOUT_en_US.txt"),
        QStringLiteral("source/assets/lumen/CLC/CLC_pt_BR.txt"),
        QStringLiteral("source/assets/lumen/CLC/CLC_en_US.txt"),
        QStringLiteral("source/assets/lumen/COPYRIGHT/AVISO DE COPYRIGHT E MARCA REGISTRA_pt_BR.txt"),
        QStringLiteral("source/assets/lumen/COPYRIGHT/COPYRIGHT AND TRADEMARK NOTICE_en_US.txt"),
        QStringLiteral("source/assets/lumen/EULA/EULA_pt_BR.txt"),
        QStringLiteral("source/assets/lumen/EULA/EULA_en_US.txt"),
        QStringLiteral("source/assets/lumen/NOTICES/NOTICE_pt_BR.txt"),
        QStringLiteral("source/assets/lumen/NOTICES/NOTICE_en_US.txt"),
    };
    for (const QString &asset : dual_license_assets) {
        const QString text = read_utf8(asset);
        QVERIFY2(text.contains(QStringLiteral("GNU AGPLv3")),
                 qPrintable(QStringLiteral("Dual-licensing reference missing in asset: %1").arg(asset)));
    }
}

void TestFrameworksPlugins::test_inprocess_plugin_interfaces_contract()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    PomodoroPlugin pomodoro;
    FeynmanPlugin feynman;
    MapaMentalPlugin mapamental;
    ModeloPlugin modelo;
    AudiolivroPlugin audiolivro;
    AgendaPlugin agenda;
    GanttPlugin gantt;

    QVERIFY(qobject_cast<IPomodoroPlugin *>(&pomodoro) != nullptr);
    QVERIFY(qobject_cast<IFeynmanPlugin *>(&feynman) != nullptr);
    QVERIFY(qobject_cast<IMapaMentalPlugin *>(&mapamental) != nullptr);
    QVERIFY(qobject_cast<IModeloPlugin *>(&modelo) != nullptr);
    QVERIFY(qobject_cast<IAudiolivroPlugin *>(&audiolivro) != nullptr);
    QVERIFY(qobject_cast<IAgendaPlugin *>(&agenda) != nullptr);
    QVERIFY(qobject_cast<IGanttPlugin *>(&gantt) != nullptr);

    QCOMPARE(pomodoro.plugin_id(), QStringLiteral("pomodoro"));
    QCOMPARE(feynman.plugin_id(), QStringLiteral("feynman"));
    QCOMPARE(mapamental.plugin_id(), QStringLiteral("mapamental"));
    QCOMPARE(modelo.plugin_id(), QStringLiteral("modelo"));
    QCOMPARE(audiolivro.plugin_id(), QStringLiteral("audiolivro"));
    QCOMPARE(agenda.plugin_id(), QStringLiteral("agenda"));
    QCOMPARE(gantt.plugin_id(), QStringLiteral("gantt"));
}

void TestFrameworksPlugins::test_dynamic_plugin_dll_loading()
{
    struct PluginSpec {
        const char *folder_key;
        const char *dll_name;
    };

    const PluginSpec specs[] = {
        {"pomodoro",   "PomodoroPlugin.dll"},
        {"feynman",    "FeynmanPlugin.dll"},
        {"mapamental", "MapaMentalPlugin.dll"},
        {"modelo",     "ModeloPlugin.dll"},
        {"audiolivro", "AudiolivroPlugin.dll"},
        {"agenda",     "AgendaPlugin.dll"},
        {"gantt",      "GanttPlugin.dll"},
    };

    const QString suffix = toolchain_suffix();
    int loaded_count = 0;

    for (const PluginSpec &spec : specs) {
        const QString rel_dll = QStringLiteral("build/build_%1_%2/plugins/%1/%3")
                                    .arg(QString::fromLatin1(spec.folder_key),
                                         suffix,
                                         QString::fromLatin1(spec.dll_name));
        const QString dll_path = source_path(rel_dll);
        QVERIFY2(QFileInfo::exists(dll_path),
                 qPrintable(QStringLiteral("Missing standalone plugin DLL: %1").arg(dll_path)));

        QLibrary lib(dll_path);
        QVERIFY2(lib.load(),
                 qPrintable(QStringLiteral("Failed to load plugin DLL %1: %2").arg(dll_path, lib.errorString())));
        QVERIFY(lib.isLoaded());
        QVERIFY(lib.unload());

        QPluginLoader plugin_loader(dll_path);
        const QJsonObject meta = plugin_loader.metaData();
        QVERIFY2(!meta.isEmpty(),
                 qPrintable(QStringLiteral("Missing Qt Q_PLUGIN_METADATA in %1").arg(dll_path)));
        QVERIFY2(!meta.value(QStringLiteral("IID")).toString().isEmpty(),
                 qPrintable(QStringLiteral("Missing IID in plugin metadata for %1").arg(dll_path)));
        ++loaded_count;
    }

    QCOMPARE(loaded_count, 7);
}

QTEST_MAIN(TestFrameworksPlugins)
#include "test_frameworks_plugins.moc"
