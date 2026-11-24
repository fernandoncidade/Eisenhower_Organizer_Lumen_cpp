#pragma once

#include "source/app/app_01_eisenhower_matrix_app.hpp"

#include <algorithm>
#include <limits>
#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QApplication>
#include <QCloseEvent>
#include <QCollator>
#include <QColor>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDateEdit>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMargins>
#include <QMessageBox>
#include <QMetaObject>
#include <QMenuBar>
#include <QPalette>
#include <QProcess>
#include <QPluginLoader>
#include <QPushButton>
#include <QRect>
#include <QSaveFile>
#include <QScreen>
#include <QSignalBlocker>
#include <QSet>
#include <QSizePolicy>
#include <QSplitter>
#include <QStandardPaths>
#include <QStackedLayout>
#include <QStringList>
#include <QTabBar>
#include <QTabWidget>
#include <QTime>
#include <QTimeEdit>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QWidget>
#include <QWindow>

#include "source/ui/agregador_01_ui.hpp"
#include "source/services/agregador_01_services.hpp"
#include "source/services/MostrarMenuContexto/mmc_09_base_text_from_item.hpp"
#include "source/services/incore_09_Arquivo.hpp"
#include "source/services/incore_13_prioridade_display.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/modules/agenda/agenda_host_bridge.hpp"
#include "source/modules/agenda/agenda_plugin_interface.hpp"
#include "source/modules/audiolivro/audiolivro_host_bridge.hpp"
#include "source/modules/audiolivro/audiolivro_plugin_interface.hpp"
#include "source/modules/gantt/gantt_host_bridge.hpp"
#include "source/modules/gantt/gantt_plugin_interface.hpp"
#include "source/modules/feynman/feynman_host_bridge.hpp"
#include "source/modules/feynman/feynman_plugin_interface.hpp"
#include "source/modules/mapamental/mapamental_host_bridge.hpp"
#include "source/modules/mapamental/mapamental_plugin_interface.hpp"
#include "source/modules/modelo/modelo_plugin_interface.hpp"
#include "source/modules/pomodoro/pomodoro_host_bridge.hpp"
#include "source/modules/pomodoro/pomodoro_plugin_interface.hpp"
#include "source/utils/AccentTabStyle.hpp"
#include "source/utils/BarraTitulosUtils.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/IconUtils.hpp"
#include "source/utils/LogManager.hpp"
#include "source/utils/PluginRegistry.hpp"
#include "source/utils/UnifiedTaskStore.hpp"

namespace source::app {

inline constexpr int kInitialWindowMinWidth = 960;
inline constexpr int kInitialWindowMinHeight = 640;

struct PomodoroUiLayoutParts {
    QString mode;
    QString panel;
};

QString get_text(const QString &text);
source::utils::LogManager *logger();
QScreen *effective_screen_for_widget(const QWidget *widget);
QMargins top_level_frame_margins(const QWidget *widget);
QSize max_client_size_for_screen(const QWidget *widget, const QRect &available);
QSize initial_window_minimum_for_screen(const QSize &max_client_size);
void relax_module_page(QWidget *page);
QString eisenhower_module_tab_text();
QString feynman_module_tab_text();
QString pomodoro_module_tab_text();
QString mapamental_module_tab_text();
QString mapamental_module_tab_text(const QString &display_name);
QString audiolivro_module_tab_text(const QString &display_name);
int count_selectable_items(const QListWidget *list);
QString selected_task_key(const QListWidgetItem *item);
void collect_selected_task_keys(const QListWidget *list, QSet<QString> &selected_keys);
QString module_task_counter_text(const QString &module_name, int selected_count, int total_count);
QStringList legacy_appdata_task_paths();
void remove_legacy_task_datastores();
QString task_layout_key(source::EisenhowerMatrixApp::TaskLayoutMode mode);
PomodoroUiLayoutParts normalized_pomodoro_ui_layout_parts(const QString &raw_key);
QString normalized_pomodoro_ui_layout_key(const QString &raw_key);
QString normalized_feynman_ui_layout_key(const QString &raw_key);
QStringList default_module_tab_order();
QString normalized_module_tab_key(const QString &raw_key);
QMap<QString, bool> default_module_tab_visibility();
QStringList normalized_module_tab_order(const QStringList &raw_order);
QStringList json_string_array_to_list(const QJsonValue &value);
QMap<QString, bool> normalized_module_tab_visibility(const QMap<QString, bool> &raw_visibility);
source::EisenhowerMatrixApp::TaskSortCriterion task_sort_criterion_from_mode(source::EisenhowerMatrixApp::TaskSortMode mode);
Qt::SortOrder task_sort_order_from_mode(source::EisenhowerMatrixApp::TaskSortMode mode);
QString task_sort_criterion_key(source::EisenhowerMatrixApp::TaskSortCriterion criterion);
QString task_sort_order_key(Qt::SortOrder order);
void clear_layout(QLayout *layout);
QString normalized_text(const QString &value);
QString base_task_text(QListWidgetItem *item);
QString priority_label_without_emoji(const QVariant &priority, source::EisenhowerMatrixApp *app);
int quadrant_from_priority(const QVariant &priority, source::EisenhowerMatrixApp *app);
int normalized_domain_value(const QVariant &domain);
bool payload_completed(const QVariantMap &task);
QList<QListWidget *> all_task_lists(source::EisenhowerMatrixApp *app);
QString placeholder_text_for_list(source::EisenhowerMatrixApp *app, const QListWidget *list);
QString human_date_for_app(const source::EisenhowerMatrixApp *app, const QString &date_iso);
QString visible_task_text(const source::EisenhowerMatrixApp *app,
                          const QString &base_text,
                          const QString &date_iso,
                          const QString &time_str);
QString compose_task_display_text(const source::EisenhowerMatrixApp *app,
                                  const QString &base_text,
                                  const QString &date_iso,
                                  const QString &time_str);
QDate task_date_value(const QVariantMap &data);
QTime task_time_value(const QVariantMap &data);
QVariantMap normalized_task_sort_payload(const QVariantMap &payload);
bool task_matches_payload(QListWidgetItem *item, const QVariantMap &payload);
bool task_matches_sync_payload(QListWidgetItem *item, const QVariantMap &payload);
QPair<QString, QString> display_and_tooltip_for_task(source::EisenhowerMatrixApp *app,
                                                     const QVariantMap &payload,
                                                     int quadrant);
QString format_duration_text(double duration_days);
QString format_duration_tooltip_line(double duration_days);
QStringList pomodoro_plugin_filenames();
QStringList pomodoro_plugin_candidates();
QStringList feynman_plugin_filenames();
QStringList feynman_plugin_candidates();
QStringList mapamental_plugin_filenames();
QStringList mapamental_plugin_candidates();
QStringList modelo_plugin_filenames();
QStringList modelo_plugin_candidates();
QStringList audiolivro_plugin_filenames();
QStringList audiolivro_plugin_candidates();
QStringList agenda_plugin_filenames();
QStringList agenda_plugin_candidates();
QStringList gantt_plugin_filenames();
QStringList gantt_plugin_candidates();
void prepend_process_path(const QStringList &paths);

}  // namespace source::app
