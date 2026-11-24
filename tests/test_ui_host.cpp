#include <QtTest>

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDir>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSignalSpy>

#include "test_common.hpp"
#include "source/app/app_01_eisenhower_matrix_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/ui/ui_02_OpcoesSobre.hpp"
#include "source/ui/ui_10_BadgeCalendarWidget.hpp"
#include "source/ui/ui_13_RotatedTabButton.hpp"
#include "source/ui/ui_16_YearCalendarWidget.hpp"

using namespace source;
using namespace source::GerenciamentoUI;
using namespace source::language;
using namespace source::utils;
using namespace eisenhower_tests;

class TestUiHost : public QObject {
    Q_OBJECT

private slots:
    void test_gerenciador_traducao();
    void test_opcoes_sobre_eisenhower_and_lumen();
    void test_badge_calendar_widget();
    void test_year_calendar_widget();
    void test_rotated_tab_button();
    void test_eisenhower_matrix_app_offscreen_eisenhower();
    void test_eisenhower_matrix_app_offscreen_lumen();
    void test_mapamental_specific_settings_menu_without_installed_models();
};

void TestUiHost::test_gerenciador_traducao()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());
    QDir().setCurrent(source_path());

    GerenciadorTraducao mgr;
    QVERIFY(!mgr.idiomas_disponiveis().isEmpty());

    mgr.definir_idioma(QStringLiteral("en_US"));
    QCOMPARE(mgr.obter_idioma_atual(), QStringLiteral("en_US"));

    mgr.definir_idioma(QStringLiteral("pt_BR"));
    QCOMPARE(mgr.obter_idioma_atual(), QStringLiteral("pt_BR"));
}

void TestUiHost::test_opcoes_sobre_eisenhower_and_lumen()
{
    {
        ScopedAppIdentity guard("EISENHOWER");
        QVERIFY(!OpcoesSobre::LICENSE_TEXT_PT_BR(true, true, true, true, true, true, true).isEmpty());
        QVERIFY(!OpcoesSobre::LICENSE_TEXT_EN_US(true, true, true, true, true, true, true).isEmpty());
        QVERIFY(!OpcoesSobre::NOTICE_TEXT_PT_BR(false, false, false, false, false, false, false).isEmpty());
        QVERIFY(!OpcoesSobre::NOTICE_TEXT_EN_US(true, true, true, true, true, true, true).isEmpty());
        QVERIFY(!OpcoesSobre::ABOUT_TEXT_PT_BR(true, true, true, true, true, true, true).isEmpty());
        QVERIFY(!OpcoesSobre::ABOUT_TEXT_EN_US(true, true, true, true, true, true, true).isEmpty());
        QVERIFY(!OpcoesSobre::Privacy_Policy_pt_BR(true, true, true, true, true, true, true).isEmpty());
        QVERIFY(!OpcoesSobre::Privacy_Policy_en_US(true, true, true, true, true, true, true).isEmpty());
        QVERIFY(!OpcoesSobre::History_APP_pt_BR().isEmpty());
        QVERIFY(!OpcoesSobre::History_APP_en_US().isEmpty());
        QVERIFY(!OpcoesSobre::RELEASE_NOTES_pt_BR().isEmpty());
        QVERIFY(!OpcoesSobre::RELEASE_NOTES_en_US().isEmpty());
        QVERIFY(!OpcoesSobre::About_Documents_pt_BR(true, true, true, true, true, true, true).isEmpty());
        QVERIFY(!OpcoesSobre::License_Documents_en_US(true, true, true, true, true, true, true).isEmpty());
        QVERIFY(!OpcoesSobre::Contract_Documents_pt_BR().isEmpty());
        QVERIFY(!OpcoesSobre::Copyright_Documents_en_US().isEmpty());
        QVERIFY(!OpcoesSobre::Notice_Documents_pt_BR().isEmpty());
        QVERIFY(!OpcoesSobre::Privacy_Policy_Documents_en_US().isEmpty());
    }

    {
        ScopedAppIdentity guard("LUMEN");
        const QString about_lumen = OpcoesSobre::ABOUT_TEXT_PT_BR(true, true, true, true, true, true, true);
        QVERIFY(!about_lumen.isEmpty());
        QVERIFY(!OpcoesSobre::History_APP_pt_BR(true, true, true, true, true, true, true).isEmpty());
        QVERIFY(!OpcoesSobre::RELEASE_NOTES_pt_BR(true, true, true, true, true, true, true).isEmpty());
    }
}

void TestUiHost::test_badge_calendar_widget()
{
    BadgeCalendarWidget calendar;
    calendar.resize(420, 320);

    const QDate target_date(2026, 11, 23);
    const QString iso = target_date.toString(Qt::ISODate);

    calendar.set_task_counts({iso, iso, QStringLiteral("2026-11-24")});
    calendar.set_task_tooltip_map({{iso, {QStringLiteral("Revisar arquitetura"), QStringLiteral("Escrever testes")}}});
    calendar.setSelectedDate(target_date);
    calendar.refresh_all_cells();
    calendar.show();
    calendar.repaint();

    QCOMPARE(calendar.get_month_count(2026, 11), 3);
    QVERIFY(calendar.get_week_count(target_date.weekNumber()) >= 2);
}

void TestUiHost::test_year_calendar_widget()
{
    YearCalendarWidget year_cal;
    year_cal.resize(480, 360);
    year_cal.setLocale(QLocale(QLocale::Portuguese, QLocale::Brazil));
    year_cal.setYear(2026);
    QCOMPARE(year_cal.year(), 2026);

    QMap<QPair<int, int>, int> counts;
    counts.insert({2026, 11}, 5);
    QMap<QPair<int, int>, QStringList> tooltips;
    tooltips.insert({2026, 11}, {QStringLiteral("Marco 1"), QStringLiteral("Entrega Final")});
    year_cal.set_month_data(counts, tooltips);

    const QDate nov(2026, 11, 1);
    year_cal.setSelectedDate(nov);
    QCOMPARE(year_cal.selectedDate().month(), 11);
    QCOMPARE(year_cal.month_counts().value({2026, 11}), 5);

    year_cal.show();
    year_cal.repaint();
}

void TestUiHost::test_rotated_tab_button()
{
    RotatedTabButton btn(QStringLiteral("Matriz"));
    btn.setSizeReferenceTexts({QStringLiteral("Matriz"), QStringLiteral("Cronograma")});
    btn.setRotationDirection(RotatedTabButton::RotationDirection::CounterClockwise90);

    bool clicked = false;
    btn.setClickHandler([&]() { clicked = true; });
    btn.resize(36, 120);
    btn.show();
    btn.repaint();

    QTest::mouseClick(&btn, Qt::LeftButton);
    QVERIFY(clicked);

    btn.setText(QStringLiteral("Feynman"));
    QCOMPARE(btn.text(), QStringLiteral("Feynman"));
}

void TestUiHost::test_eisenhower_matrix_app_offscreen_eisenhower()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());
    ScopedAppIdentity guard("EISENHOWER");

    // Isolate current dir inside sandbox so PluginRegistry does not pick up wrong-ABI sibling DLLs.
    const QString prev_cwd = QDir::currentPath();
    QDir::setCurrent(sandbox.root_path());

    GerenciadorTraducao tm;
    tm.definir_idioma(QStringLiteral("pt_BR"));

    {
        EisenhowerMatrixApp app(&tm);
        QVERIFY(app.quadrant1_list != nullptr);
        QVERIFY(app.quadrant2_list != nullptr);
        QVERIFY(app.task_input != nullptr);
        QVERIFY(!app.windowTitle().isEmpty());

        QVERIFY(app.set_task_layout(EisenhowerMatrixApp::TaskLayoutMode::Matrix2x2, false));
        QCOMPARE(app.current_task_layout(), EisenhowerMatrixApp::TaskLayoutMode::Matrix2x2);
        QVERIFY(app.set_task_layout(EisenhowerMatrixApp::TaskLayoutMode::Tabs, false));

        app.set_sync_eisenhower_enabled(false);
        app.task_input->setText(QStringLiteral("Implementar suite de testes C++23"));
        app.quadrant_selector->setCurrentIndex(0);
        app.add_task();
        QCoreApplication::processEvents();

        bool found_task = false;
        for (int i = 0; i < app.quadrant1_list->count(); ++i) {
            if (app.quadrant1_list->item(i)->text().contains(QStringLiteral("Implementar suite de testes"))) {
                found_task = true;
                break;
            }
        }
        QVERIFY(found_task);

        app.save_tasks_now();
        QVERIFY(QFileInfo::exists(app.tasks_path));

        const QVariantList available = app.get_available_tasks();
        QVERIFY(!available.isEmpty());
    }

    QDir::setCurrent(prev_cwd);
}

void TestUiHost::test_eisenhower_matrix_app_offscreen_lumen()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());
    ScopedAppIdentity guard("LUMEN");

    const QString prev_cwd = QDir::currentPath();
    QDir::setCurrent(sandbox.root_path());

    GerenciadorTraducao tm;
    tm.definir_idioma(QStringLiteral("pt_BR"));

    {
        EisenhowerMatrixApp app(&tm);
        QVERIFY(app.windowTitle().contains(QStringLiteral("LÚMEN")) || !app.windowTitle().isEmpty());
        app.atualizar_textos();
        app.refresh_module_task_counter();
        QCoreApplication::processEvents();
    }

    QDir::setCurrent(prev_cwd);
}

void TestUiHost::test_mapamental_specific_settings_menu_without_installed_models()
{
    ScopedLocalAppData sandbox;
    QVERIFY(sandbox.is_valid());
    ScopedAppIdentity guard("EISENHOWER");

    const QString prev_cwd = QDir::currentPath();
    QDir::setCurrent(sandbox.root_path());

    GerenciadorTraducao tm;
    tm.definir_idioma(QStringLiteral("pt_BR"));

    {
        EisenhowerMatrixApp app(&tm);
        QVERIFY(app.has_mapamental_plugin());
        QVERIFY(app.has_modelo_plugin());
        QVERIFY(app.menu_bar != nullptr);

        auto find_submenu = [](const QList<QAction *> &actions, const QString &expected_title) -> QMenu * {
            for (QAction *act : actions) {
                if (act != nullptr && act->menu() != nullptr && act->text().contains(expected_title, Qt::CaseInsensitive)) {
                    return act->menu();
                }
            }
            return nullptr;
        };

        QMenu *config_menu = find_submenu(app.menu_bar->actions(), QStringLiteral("Configura"));
        QVERIFY2(config_menu != nullptr, "Configurações menu must exist");

        QMenu *specific_menu = find_submenu(config_menu->actions(), QStringLiteral("Espec"));
        QVERIFY2(specific_menu != nullptr, "Configurações Específicas menu must exist");

        QMenu *mapamental_menu = find_submenu(specific_menu->actions(), app.mapamental_display_name());
        QVERIFY2(mapamental_menu != nullptr, "Mapa Mental submenu inside Configurações Específicas must exist");
        QVERIFY2(mapamental_menu->actions().size() >= 5,
                 "Mapa Mental submenu must be populated even when Qwen/Ollama models are not installed yet");

        QMenu *ollama_menu = find_submenu(mapamental_menu->actions(), QStringLiteral("Ollama"));
        QVERIFY2(ollama_menu != nullptr, "Modelos Ollama submenu must exist");
        QVERIFY2(!ollama_menu->actions().isEmpty(), "Modelos Ollama submenu must have at least the refresh action");

        emit mapamental_menu->aboutToShow();
        emit ollama_menu->aboutToShow();
        QCoreApplication::processEvents();

        QVERIFY(mapamental_menu->actions().size() >= 5);
        QVERIFY(!ollama_menu->actions().isEmpty());
    }

    QDir::setCurrent(prev_cwd);
}

QTEST_MAIN(TestUiHost)
#include "test_ui_host.moc"
