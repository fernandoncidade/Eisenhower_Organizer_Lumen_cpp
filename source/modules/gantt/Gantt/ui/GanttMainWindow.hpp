#pragma once

#include <QJsonValue>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QString>
#include <QToolBar>
#include "Bim4DSimulationPanel.hpp"
#include "GanttTimelineWidget.hpp"
#include "GanttWbsTreeWidget.hpp"
#include "../application/services/Bim4DService.hpp"
#include "../application/services/CsvExchangeService.hpp"
#include "../application/services/FieldProgressService.hpp"
#include "../application/services/PlanningEngineService.hpp"
#include "../application/services/ScheduleService.hpp"
#include "../application/services/SecurityService.hpp"
#include "../application/services/WebhookDispatcher.hpp"
#include "../infrastructure/persistence/GanttDatabase.hpp"

namespace source::gantt {

class GanttMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit GanttMainWindow(QWidget *parent = nullptr);
    ~GanttMainWindow() override;

    ScheduleService *schedule_service() { return &schedule_svc_; }
    FieldProgressService *progress_service() { return &progress_svc_; }
    GanttDatabase *database() { return &database_; }

    void load_or_create_default_schedule();
    void sync_all_eisenhower_tasks(bool replace_template_if_empty = true);
    void on_eisenhower_task_duration_changed(const QString &task_id, double duration_days);
    void on_eisenhower_task_removed(const QString &task_id);
    void focus_activity(const QString &item_id);
    void refresh_view();
    void retranslate_ui();
    void update_window_title();

protected:
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

public slots:
    void on_activity_selected(const QString &item_id);
    void delete_activity_with_dialogs(const QString &item_id);

private slots:
    void on_action_sync_all_eisenhower_tasks();
    void on_action_new_activity();
    void on_action_edit_activity();
    void on_action_delete_activity();
    void on_action_add_dependency();
    void on_action_recalculate_cpm();
    void on_action_create_baseline();
    void on_action_compare_baseline();
    void on_action_toggle_baseline(bool checked);
    void on_action_zoom_day();
    void on_action_zoom_week();
    void on_action_zoom_month();
    void on_action_export_csv();
    void on_action_import_csv();
    void on_action_import_eisenhower_tasks();
    void on_action_toggle_wbs_table(bool checked);
    void on_action_toggle_bim_panel(bool checked);
    void on_action_configure_working_days();
    void on_working_day_toggled(int day_num, bool checked);
    void on_action_manage_holidays();
    void on_request_link_task(const QString &item_id);
    void on_request_unlink_task(const QString &item_id);
    void on_request_sync_task(const QString &item_id);

private:
    // Core Services & DB
    GanttDatabase database_;
    ScheduleService schedule_svc_;
    PlanningEngineService planning_svc_;
    FieldProgressService progress_svc_;
    Bim4DService bim_svc_;
    CsvExchangeService csv_svc_;
    SecurityService security_svc_;
    WebhookDispatcher webhook_dispatcher_;

    // Current State
    QString current_schedule_id_;

    // UI Widgets
public:
    enum class TableLayoutMode {
        Original = 0,
        VerticalTopo = 1,
        VerticalInferior = 2,
        HorizontalEsquerda = 3,
        HorizontalDireita = 4
    };

    void apply_table_layout(TableLayoutMode mode, bool persist = true);
    TableLayoutMode table_layout_mode() const { return current_layout_mode_; }

    static QString table_layout_mode_to_string(TableLayoutMode mode);
    static TableLayoutMode table_layout_mode_from_string(const QString &str, TableLayoutMode fallback = TableLayoutMode::Original);
    static TableLayoutMode table_layout_mode_from_json(const QJsonValue &val, TableLayoutMode fallback = TableLayoutMode::Original);

    static QString layout_json_path();
    static TableLayoutMode load_saved_table_layout_mode();
    static void save_table_layout_mode(TableLayoutMode mode);

private:
    // UI Widgets
    QMenuBar *menu_bar_ = nullptr;
    QMenu *menu_arquivo_ = nullptr;
    QMenu *menu_configuracoes_ = nullptr;
    QMenu *menu_layout_ = nullptr;
    QMenu *menu_tabelas_ = nullptr;
    QMenu *menu_dias_uteis_ = nullptr;
    QToolBar *toolbar_ = nullptr;
    QSplitter *main_splitter_ = nullptr;
    QSplitter *vertical_splitter_ = nullptr;
    GanttWbsTreeWidget *wbs_tree_ = nullptr;
    GanttTimelineWidget *timeline_ = nullptr;
    Bim4DSimulationPanel *bim_panel_ = nullptr;

    QActionGroup *group_layout_tabelas_ = nullptr;
    QAction *act_layout_original_ = nullptr;
    QAction *act_layout_vertical_topo_ = nullptr;
    QAction *act_layout_vertical_inferior_ = nullptr;
    QAction *act_layout_horizontal_esquerda_ = nullptr;
    QAction *act_layout_horizontal_direita_ = nullptr;

    QAction *act_toggle_wbs_ = nullptr;
    QAction *act_new_ = nullptr;
    QAction *act_edit_ = nullptr;
    QAction *act_del_ = nullptr;
    QAction *act_dep_ = nullptr;
    QAction *act_cpm_ = nullptr;
    QAction *act_config_dias_uteis_ = nullptr;
    QAction *act_days_[7] = {nullptr};
    QAction *act_feriados_ = nullptr;
    QAction *act_base_ = nullptr;
    QAction *act_comp_ = nullptr;
    QAction *act_toggle_base_ = nullptr;
    QAction *act_zoom_day_ = nullptr;
    QAction *act_zoom_week_ = nullptr;
    QAction *act_zoom_month_ = nullptr;
    QAction *act_bim_ = nullptr;
    QAction *act_import_csv_ = nullptr;
    QAction *act_export_csv_ = nullptr;
    QAction *act_sync_all_ = nullptr;
    QAction *act_import_eisen_ = nullptr;

    QLabel *lbl_status_metrics_ = nullptr;

    void setup_ui();
    void setup_menus();
    void update_status_bar();
    void update_zoom_highlight();
    void update_working_days_menu();
    void update_layout_actions_checked();
    void _retranslate_actions();
    Schedule *current_schedule();
    QString current_selected_item_id() const;

    TableLayoutMode current_layout_mode_ = TableLayoutMode::Original;
    bool is_first_show_ = true;
    bool is_retranslating_ = false;
    bool needs_full_retranslate_ = false;
};

} // namespace source::gantt

