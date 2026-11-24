#include <QtTest>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <ranges>
#include <expected>

#include "test_common.hpp"
#include "source/utils/AppConfig.hpp"
#include "source/utils/ApplicationPathUtils.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/IconUtils.hpp"
#include "source/utils/LogManager.hpp"
#include "source/utils/PluginRegistry.hpp"
#include "source/utils/TaskProgressHelper.hpp"
#include "source/utils/TrialManager.hpp"

using namespace source::utils;
using namespace eisenhower_tests;

class TestCoreUtils : public QObject {
    Q_OBJECT

private slots:
    void test_cpp23_standard_and_utf8_support();
    void test_app_config_dual_identity();
    void test_task_progress_helper();
    void test_application_path_utils();
    void test_caminho_persistente_and_lumen_migration();
    void test_icon_utils();
    void test_log_manager();
    void test_plugin_registry();
    void test_trial_manager_readonly();
};

void TestCoreUtils::test_cpp23_standard_and_utf8_support()
{
#if defined(_MSVC_LANG)
    QVERIFY2(_MSVC_LANG >= 202302L, "MSVC build must use C++23 (/std:c++23latest or /std:c++23)");
#else
    QVERIFY2(__cplusplus >= 202302L, "Compiler must target C++23 or newer");
#endif

    const auto parse_positive = [](int val) -> std::expected<int, QString> {
        if (val <= 0) {
            return std::unexpected(QStringLiteral("Non-positive value"));
        }
        return val * 2;
    };
    const auto ok_res = parse_positive(21);
    QVERIFY(ok_res.has_value());
    QCOMPARE(ok_res.value(), 42);
    QVERIFY(!parse_positive(-1).has_value());

    const std::array<int, 4> nums{1, 2, 3, 4};
    int even_sum = 0;
    for (int v : nums | std::views::filter([](int x) { return x % 2 == 0; })) {
        even_sum += v;
    }
    QCOMPARE(even_sum, 6);

    const QString pt_utf8 = QString::fromUtf8("Ação, Revisão Técnica, Lúcido, Caçamba");
    QCOMPARE(pt_utf8.toUtf8().constData(), "Ação, Revisão Técnica, Lúcido, Caçamba");
}

void TestCoreUtils::test_app_config_dual_identity()
{
    QCOMPARE(AppConfig::normalized_build_target_name(QStringLiteral("lúmen")), QStringLiteral("LUMEN"));
    QCOMPARE(AppConfig::normalized_build_target_name(QStringLiteral("APP_EISENHOWER")), QStringLiteral("EISENHOWER"));
    QVERIFY(AppConfig::normalized_build_target_name(QStringLiteral("unknown")).isEmpty());

    {
        ScopedAppIdentity guard("EISENHOWER");
        QVERIFY(!AppConfig::is_lumen());
        QCOMPARE(AppConfig::build_target_name(), QStringLiteral("EISENHOWER"));
        QCOMPARE(AppConfig::assets_subdir_name(), QStringLiteral("eisenhower"));
        QCOMPARE(AppConfig::local_app_data_dir_name(), QStringLiteral("EisenhowerOrganizer"));
        QCOMPARE(AppConfig::default_window_icon_name(), QStringLiteral("organizador.ico"));
        QCOMPARE(AppConfig::unified_tasks_schema_name(), QStringLiteral("eisenhower_unified_tasks"));
    }

    {
        ScopedAppIdentity guard("LUMEN");
        QVERIFY(AppConfig::is_lumen());
        QCOMPARE(AppConfig::build_target_name(), QStringLiteral("LUMEN"));
        QCOMPARE(AppConfig::assets_subdir_name(), QStringLiteral("lumen"));
        QCOMPARE(AppConfig::local_app_data_dir_name(), QStringLiteral("Lumen"));
        QCOMPARE(AppConfig::default_window_icon_name(), QStringLiteral("autismo.ico"));
        QCOMPARE(AppConfig::unified_tasks_schema_name(), QStringLiteral("lumen_unified_tasks"));
        QCOMPARE(AppConfig::legacy_unified_tasks_schema_name(), QStringLiteral("eisenhower_unified_tasks"));
    }
}

void TestCoreUtils::test_task_progress_helper()
{
    QCOMPARE(calculate_task_completion_percentage(0, 4, false), 0.0);
    QCOMPARE(calculate_task_completion_percentage(1, 4, false), 25.0);
    QCOMPARE(calculate_task_completion_percentage(2, 4, false), 50.0);
    QCOMPARE(calculate_task_completion_percentage(4, 4, false), 99.0);
    QCOMPARE(calculate_task_completion_percentage(10, 4, false), 99.0);
    QCOMPARE(calculate_task_completion_percentage(0, 0, true), 100.0);
    QCOMPARE(calculate_task_completion_percentage(-5, 4, false), 0.0);
    QCOMPARE(calculate_task_completion_percentage(2, 0, false), 0.0);
}

void TestCoreUtils::test_application_path_utils()
{
    const QString base = get_app_base_path();
    QVERIFY(!base.isEmpty());
    QVERIFY(QDir(base).exists());

    ScopedAppIdentity eisenhower_guard("EISENHOWER");
    const QString eisenhower_assets = get_assets_base_path();
    QVERIFY(!eisenhower_assets.isEmpty());
    QVERIFY(QDir(eisenhower_assets).exists());

    ScopedAppIdentity lumen_guard("LUMEN");
    const QString lumen_assets = get_assets_base_path();
    QVERIFY(!lumen_assets.isEmpty());
    QVERIFY(QDir(lumen_assets).exists());
}

void TestCoreUtils::test_caminho_persistente_and_lumen_migration()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    QString eisenhower_dir;
    {
        ScopedAppIdentity eisenhower_guard("EISENHOWER");
        eisenhower_dir = obter_caminho_persistente();
        QVERIFY(QDir(eisenhower_dir).exists());
        QVERIFY(eisenhower_dir.contains(QStringLiteral("EisenhowerOrganizer")));

        QFile tasks_file(QDir(eisenhower_dir).filePath(QStringLiteral("tarefas.json")));
        QVERIFY(tasks_file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        tasks_file.write("{\"migrated_seed\":true}\n");
        tasks_file.close();

        QDir().mkpath(QDir(eisenhower_dir).filePath(QStringLiteral("logs")));
        QFile log_file(QDir(eisenhower_dir).filePath(QStringLiteral("logs/sample.log")));
        QVERIFY(log_file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        log_file.write("sample\n");
        log_file.close();
    }

    {
        ScopedAppIdentity lumen_guard("LUMEN");
        const QString lumen_dir = obter_caminho_persistente();
        QVERIFY(QDir(lumen_dir).exists());
        QVERIFY(lumen_dir.contains(QStringLiteral("Lumen")));

        const QString migrated_tasks = QDir(lumen_dir).filePath(QStringLiteral("tarefas.json"));
        const QString migrated_marker = QDir(lumen_dir).filePath(QStringLiteral(".lumen_persistence_migrated"));
        const QString migrated_log = QDir(lumen_dir).filePath(QStringLiteral("logs/sample.log"));
        QVERIFY(QFileInfo::exists(migrated_tasks));
        QVERIFY(QFileInfo::exists(migrated_marker));
        QVERIFY(QFileInfo::exists(migrated_log));

        // Ensure second call is idempotent and preserves user edits in Lumen dir.
        QFile edit_file(migrated_tasks);
        QVERIFY(edit_file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        edit_file.write("{\"lumen_custom\":true}\n");
        edit_file.close();

        QCOMPARE(obter_caminho_persistente(), lumen_dir);
        QFile verify_file(migrated_tasks);
        QVERIFY(verify_file.open(QIODevice::ReadOnly));
        QCOMPARE(QString::fromUtf8(verify_file.readAll()).trimmed(), QStringLiteral("{\"lumen_custom\":true}"));
    }
}

void TestCoreUtils::test_icon_utils()
{
    {
        ScopedAppIdentity guard("EISENHOWER");
        QCOMPARE(get_default_window_icon_name(), QStringLiteral("organizador.ico"));
        const QString icon_path = get_default_window_icon_path();
        QVERIFY(!icon_path.isEmpty());
        QVERIFY(QFileInfo::exists(icon_path));
    }

    {
        ScopedAppIdentity guard("LUMEN");
        QCOMPARE(get_default_window_icon_name(), QStringLiteral("autismo.ico"));
        const QString icon_path = get_default_window_icon_path();
        QVERIFY(!icon_path.isEmpty());
        QVERIFY(QFileInfo::exists(icon_path));
        const QString mapped_path = get_icon_path(QStringLiteral("organizador.ico"));
        QVERIFY(mapped_path.endsWith(QStringLiteral("autismo.ico"), Qt::CaseInsensitive));
    }
}

void TestCoreUtils::test_log_manager()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());

    const QString isolated_log = sandbox.file_path(QStringLiteral("test_core.log"));
    ScopedEnvVar log_env(AppConfig::runtime_log_file_env_var(), isolated_log.toUtf8());

    QCOMPARE(LogManager::ensure_unicode(QStringLiteral("Teste Ação")), QStringLiteral("Teste Ação"));
    const QByteArray invalid_utf8("\xFF\xFE" "abc", 5);
    const QString decoded = LogManager::ensure_unicode(invalid_utf8);
    QVERIFY(!decoded.isEmpty());

    LogManager *logger = LogManager::get_logger();
    QVERIFY(logger != nullptr);
    logger->info(QStringLiteral("Unit test info message"));
    logger->warning(QStringLiteral("Unit test warning message"));
    logger->error(QStringLiteral("Unit test error message"), false);

    QVERIFY(!LogManager::get_log_file().isEmpty());
}

void TestCoreUtils::test_plugin_registry()
{
    const QStringList dlls = PluginRegistry::binary_names(QStringLiteral("PomodoroPlugin"));
    QVERIFY(dlls.contains(QStringLiteral("PomodoroPlugin.dll")));

    const QStringList candidates = PluginRegistry::candidate_paths(QStringLiteral("pomodoro"), QStringLiteral("PomodoroPlugin"));
    QVERIFY(!candidates.isEmpty());
    bool has_pomodoro_candidate = false;
    for (const QString &candidate : candidates) {
        if (candidate.contains(QStringLiteral("PomodoroPlugin.dll"))) {
            has_pomodoro_candidate = true;
            break;
        }
    }
    QVERIFY(has_pomodoro_candidate);
}

void TestCoreUtils::test_trial_manager_readonly()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());
    ScopedAppIdentity guard("EISENHOWER");

    const QString config_path = TrialManager::get_config_path();
    QVERIFY(config_path.startsWith(QDir(sandbox.root_path()).absolutePath(), Qt::CaseInsensitive)
            || QDir::fromNativeSeparators(config_path).startsWith(QDir::fromNativeSeparators(sandbox.root_path()), Qt::CaseInsensitive));
    QVERIFY(config_path.endsWith(QStringLiteral("trial_info.json")));

    // Read-only registry probe: must not write to HKCU.
    (void)TrialManager::get_first_run_timestamp();
}

QTEST_MAIN(TestCoreUtils)
#include "test_core_utils.moc"
