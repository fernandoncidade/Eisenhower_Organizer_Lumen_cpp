#pragma once

#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QPair>
#include <QPointer>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <Qt>
#include <QtGlobal>

class QCheckBox;
class QComboBox;
class QDateEdit;
class QDialog;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QMenu;
class QMenuBar;
class QPluginLoader;
class QPushButton;
class QShortcut;
class QSplitter;
class QGridLayout;
class QStackedLayout;
class QTabWidget;
class QDoubleSpinBox;
class QTimeEdit;
class QWidget;
class QEvent;
class QCloseEvent;
class QGroupBox;
class QResizeEvent;
class QScrollArea;

#include "source/modules/gantt/Gantt/application/services/IEisenhowerHost.hpp"

namespace source::GerenciamentoUI
{
    class Calendar;
}

namespace source::InterfaceCore
{
    class Arquivo;
}

namespace source::language
{
    class GerenciadorTraducao;
}

namespace source::plugin
{
    class IFeynmanPlugin;
    class FeynmanHostBridge;
    class IPomodoroPlugin;
    class PomodoroHostBridge;
    class IMapaMentalPlugin;
    class MapaMentalHostBridge;
    class IModeloPlugin;
    class IAudiolivroPlugin;
    class AudiolivroHostBridge;
    class IAgendaPlugin;
    class AgendaHostBridge;
    class IGanttPlugin;
    class GanttHostBridge;
}

namespace source
{

    class EisenhowerMatrixApp : public QMainWindow, public source::gantt::IEisenhowerHost
    {
        Q_OBJECT

    public:
        enum class TaskLayoutMode
        {
            Tabs,
            TabsHorizontal,
            Quadrants,
            Matrix2x2,
        };

        enum class TaskSortMode
        {
            None,
            DateAscending,
            DateDescending,
            TimeAscending,
            TimeDescending,
            AlphabeticalAscending,
            AlphabeticalDescending,
        };

        enum class TaskSortCriterion
        {
            Date,
            Alphabetical,
            Time,
        };

        explicit EisenhowerMatrixApp(source::language::GerenciadorTraducao *translation_manager,
                                     QWidget *parent = nullptr);
        ~EisenhowerMatrixApp() override;

        bool calendar_start_expanded = false;
        bool feynman_task_panel_start_expanded = false;
        bool pomodoro_in_progress_panel_start_expanded = false;
        int calendar_min_width = 400;
        int quadrants_min_width = 400;
        int calendar_splitter_handle_width = 2;

        source::language::GerenciadorTraducao *gerenciador_traducao = nullptr;
        QString tasks_path;
        QString export_path;
        source::InterfaceCore::Arquivo *arquivo = nullptr;

        QSplitter *main_splitter = nullptr;
        source::GerenciamentoUI::Calendar *calendar_pane = nullptr;
        QTabWidget *module_tabs = nullptr;
        QWidget *menu_bar_host = nullptr;
        QWidget *module_task_counter_container = nullptr;
        QLabel *module_task_counter_label = nullptr;
        QWidget *eisenhower_module_page = nullptr;
        QWidget *feynman_module_page = nullptr;
        QWidget *pomodoro_module_page = nullptr;
        QWidget *mapamental_module_page = nullptr;
        QWidget *audiolivro_module_page = nullptr;
        QWidget *_main_content = nullptr;
        QWidget *_root_container = nullptr;
        QWidget *task_layout_host = nullptr;
        QWidget *quadrant_tabs_page = nullptr;
        QWidget *quadrants_page = nullptr;
        QWidget *matrix2x2_page = nullptr;
        QScrollArea *quadrant_tabs_scroll = nullptr;
        QScrollArea *quadrants_scroll = nullptr;
        QScrollArea *matrix2x2_scroll = nullptr;
        QHBoxLayout *quadrants_layout = nullptr;
        QGridLayout *matrix2x2_layout = nullptr;
        QStackedLayout *task_layout_stack = nullptr;
        QPointer<QDialog> _sobre_dialog;
        QPointer<QDialog> _manual_dialog;
        QPointer<QDialog> _plugin_manager_dialog;
        QPointer<QDialog> _log_manager_dialog;
        QPointer<QDialog> _ollama_instructions_dialog;
        QPointer<QDialog> _model_download_dialog;

        QTabWidget *quadrant_tabs = nullptr;
        QTabWidget *quadrant1_status_tabs = nullptr;
        QTabWidget *quadrant2_status_tabs = nullptr;
        QTabWidget *quadrant3_status_tabs = nullptr;
        QTabWidget *quadrant4_status_tabs = nullptr;
        QWidget *quadrant1_tab = nullptr;
        QWidget *quadrant2_tab = nullptr;
        QWidget *quadrant3_tab = nullptr;
        QWidget *quadrant4_tab = nullptr;

        QLabel *quadrant1_label = nullptr;
        QLabel *quadrant2_label = nullptr;
        QLabel *quadrant3_label = nullptr;
        QLabel *quadrant4_label = nullptr;
        QLabel *quadrant1_completed_label = nullptr;
        QLabel *quadrant2_completed_label = nullptr;
        QLabel *quadrant3_completed_label = nullptr;
        QLabel *quadrant4_completed_label = nullptr;

        QLabel *quadrant1_in_progress_label = nullptr;
        QLabel *quadrant2_in_progress_label = nullptr;
        QLabel *quadrant3_in_progress_label = nullptr;
        QLabel *quadrant4_in_progress_label = nullptr;

        QGroupBox *quadrant1_group = nullptr;
        QGroupBox *quadrant2_group = nullptr;
        QGroupBox *quadrant3_group = nullptr;
        QGroupBox *quadrant4_group = nullptr;
        QGroupBox *quadrant1_completed_group = nullptr;
        QGroupBox *quadrant2_completed_group = nullptr;
        QGroupBox *quadrant3_completed_group = nullptr;
        QGroupBox *quadrant4_completed_group = nullptr;

        QGroupBox *quadrant1_in_progress_group = nullptr;
        QGroupBox *quadrant2_in_progress_group = nullptr;
        QGroupBox *quadrant3_in_progress_group = nullptr;
        QGroupBox *quadrant4_in_progress_group = nullptr;

        QListWidget *quadrant1_list = nullptr;
        QListWidget *quadrant2_list = nullptr;
        QListWidget *quadrant3_list = nullptr;
        QListWidget *quadrant4_list = nullptr;
        QListWidget *quadrant1_completed_list = nullptr;
        QListWidget *quadrant2_completed_list = nullptr;
        QListWidget *quadrant3_completed_list = nullptr;
        QListWidget *quadrant4_completed_list = nullptr;

        QListWidget *quadrant1_in_progress_list = nullptr;
        QListWidget *quadrant2_in_progress_list = nullptr;
        QListWidget *quadrant3_in_progress_list = nullptr;
        QListWidget *quadrant4_in_progress_list = nullptr;

        QGroupBox *new_task_group = nullptr;
        QScrollArea *new_task_scroll = nullptr;
        QWidget *new_task_content = nullptr;
        QLineEdit *task_input = nullptr;
        QPushButton *add_button = nullptr;
        QPushButton *calendar_button = nullptr;
        QCheckBox *date_checkbox = nullptr;
        QDateEdit *date_input = nullptr;
        QCheckBox *time_checkbox = nullptr;
        QTimeEdit *time_input = nullptr;
        QLabel *duration_label = nullptr;
        QDoubleSpinBox *duration_spin = nullptr;
        QLabel *priority_label = nullptr;
        QComboBox *quadrant_selector = nullptr;
        QLabel *domain_label = nullptr;
        QComboBox *domain_selector = nullptr;
        QPushButton *add_task_button = nullptr;
        QMenuBar *menu_bar = nullptr;
        QMenu *idioma_menu = nullptr;
        QShortcut *atalho_arquivo = nullptr;
        QShortcut *atalho_idioma = nullptr;
        QShortcut *atalho_opcoes = nullptr;
        QMap<QString, QVariant> widget_heights;
        int widget_height_override = 0;

        void criar_menu_configuracoes();
        void definir_idioma(const QString &codigo_idioma);
        void initUI();
        void _attach_module_task_counter_to_menu_bar();
        void _sync_module_task_counter_geometry();
        bool set_task_layout(TaskLayoutMode mode, bool persist = true);
        TaskLayoutMode current_task_layout() const;
        bool module_tab_adjustment_locked() const;
        void set_module_tab_adjustment_locked(bool locked, bool persist = true);
        void restore_module_tab_order(bool persist = true);
        bool module_tab_available(const QString &key) const;
        bool module_tab_visible(const QString &key) const;
        bool set_module_tab_visible(const QString &key, bool visible, bool persist = true);
        bool set_task_sort_mode(TaskSortMode mode, bool persist = true);
        bool clear_task_sort_criterion(TaskSortCriterion criterion, bool persist = true);
        bool has_task_sort_criterion(TaskSortCriterion criterion) const;
        Qt::SortOrder task_sort_order(TaskSortCriterion criterion) const;
        int task_sort_priority(TaskSortCriterion criterion) const;
        bool has_active_task_sort() const;
        bool task_sort_less_for_payloads(const QVariantMap &left, const QVariantMap &right) const;
        void add_placeholder(QListWidget *list_widget, const QString &text);
        void insert_task_into_quadrant_list(QListWidget *list, QListWidgetItem *item);
        void cleanup_time_groups(QListWidget *list);
        void add_task();
        void handle_item_checked(QListWidgetItem *item, QListWidget *source_list, QListWidget *target_list);
        void move_item_between_lists(QListWidgetItem *item,
                                     QListWidget *source,
                                     QListWidget *target,
                                     Qt::CheckState new_check_state);
        bool remove_task(QListWidgetItem *item, QListWidget *list_widget, bool confirm = true);
        void save_tasks();
        void save_tasks_now();
        void load_tasks();
        void atualizar_textos();
        void atualizar_placeholders();
        void exibir_sobre();
        void show_context_menu(const QPoint &point, QListWidget *list_widget);
        void open_calendar();
        void open_eisenhower();
        void nova_sessao();
        void abrir_arquivo();
        void salvar();
        void salvar_como();
        void limpar_tudo();
        void sair_app();
        void edit_task_datetime(QListWidgetItem *item, QListWidget *list_widget);
        void move_task_to_quadrant(QListWidgetItem *item, QListWidget *source_list, QListWidget *target_list);
        bool handle_linked_file_open_request(QListWidgetItem *item, QWidget *parent = nullptr);
        bool has_feynman_plugin() const;
        QString feynman_display_name() const;
        QString feynman_open_menu_text() const;
        QString feynman_ui_layout_mode_key() const;
        bool set_feynman_ui_layout_mode_key(const QString &mode_key, bool persist = true);
        void open_feynman();
        bool has_pomodoro_plugin() const;
        QString pomodoro_display_name() const;
        QString pomodoro_open_menu_text() const;
        QString pomodoro_alarm_menu_text() const;
        QString pomodoro_automatic_mode_menu_text() const;
        QString pomodoro_ui_layout_mode_key() const;
        bool set_pomodoro_ui_layout_mode_key(const QString &mode_key, bool persist = true);
        void open_pomodoro();
        bool open_pomodoro_alarm_settings(QWidget *parent = nullptr);
        bool open_pomodoro_automatic_mode_settings(QWidget *parent = nullptr);
        int default_pomodoro_cycle_count() const;
        void on_default_pomodoro_cycle_count_changed(int new_count);
        void reset_in_progress_task_cycles_across_modules(int new_cycles, const QVariantList &affected_tasks);
        bool has_mapamental_plugin() const;
        bool has_modelo_plugin() const;
        QString mapamental_display_name() const;
        QString mapamental_open_menu_text() const;
        QString mapamental_layout_mode_key() const;
        void set_mapamental_layout_mode_key(const QString &mode_key);
        QString mapamental_summary_method_key() const;
        void set_mapamental_summary_method_key(const QString &method_key);
        QVariantList ollama_installed_models() const;
        void refresh_ollama_models();
        QVariantMap ollama_diagnostic() const;
        void show_ollama_instructions_dialog();
        void show_model_download_dialog();
        bool is_qwen_model_installed() const;
        QString qwen_models_dir() const;
        QString qwen_model_path() const;
        QString qwen_download_url() const;
        qint64 qwen_model_size_bytes() const;
        bool remove_qwen_model();
        void open_mapamental();
        void reload_mapamental_from_backup();
        bool has_audiolivro_plugin() const;
        QString audiolivro_display_name() const;
        QString audiolivro_open_menu_text() const;
        QString audiolivro_font_settings_menu_text() const;
        QVariantList audiolivro_voice_options() const;
        QString audiolivro_current_voice_id() const;
        void set_audiolivro_voice_id(const QString &voice_id);
        bool open_audiolivro_font_settings(QWidget *parent = nullptr);
        void open_audiolivro();
        bool has_agenda_plugin() const;
        QString agenda_display_name() const;
        QString agenda_open_menu_text() const;
        void open_agenda();
        void populate_agenda_delete_item_menu(QMenu *menu);
        void populate_agenda_color_menu(QMenu *menu);
        bool has_gantt_plugin() const;
        source::plugin::IGanttPlugin *gantt_plugin() const { return gantt_plugin_; }
        QString gantt_display_name() const;
        QString gantt_open_menu_text() const;
        void open_gantt();
        void open_gantt_for_task(const QString &schedule_item_id);
        bool open_gantt_link_dialog(QListWidgetItem *item);
        bool send_task_to_gantt(QListWidgetItem *item, QString *out_item_id = nullptr, QString *error_message = nullptr);

        // IEisenhowerHost implementation
        QVariantList get_available_tasks() const override;
        QVariantMap get_task_by_id(const QString &task_id) const override;
        void sync_task_progress(const QString &task_id, double percent, bool is_done) override;
        void sync_task_duration(const QString &task_id, double duration_days) override;
        void sync_task_title(const QString &task_id, const QString &new_title) override;
        bool remove_task_by_id(const QString &task_id) override;
        QWidget *as_widget() override;
        bool calendar_panel_visible() const;
        void set_calendar_panel_visible(bool visible, bool persist = true);
        bool feynman_task_panel_visible() const;
        void set_feynman_task_panel_visible(bool visible, bool persist = true);
        bool pomodoro_in_progress_panel_visible() const;
        void set_pomodoro_in_progress_panel_visible(bool visible, bool persist = true);
        Q_SLOT void refresh_module_task_counter();
        void clear_eisenhower_task_selections(QListWidget *exclude_list = nullptr);
        bool send_task_to_feynman(QListWidgetItem *item, QListWidget *source_list = nullptr, bool activate_tab = false);
        bool send_task_to_pomodoro(QListWidgetItem *item, QListWidget *source_list = nullptr, bool activate_tab = false);
        bool clear_pomodoro_tasks_if_available();
        bool clear_feynman_tasks_if_available();
        void clear_pomodoro_custom_font_settings(bool target_linked);
        void clear_feynman_custom_font_settings(bool target_linked);
        void clear_mapamental_custom_font_settings(bool target_linked);
        QVariantList get_pomodoro_task_font_overrides(bool target_linked) const;
        QVariantList get_feynman_task_font_overrides(bool target_linked) const;
        QVariantList get_mapamental_task_font_overrides(bool target_linked) const;
        void reload_all_modules_from_shared_store();
        bool sync_eisenhower_enabled() const;
        bool sync_pomodoro_enabled() const;
        bool sync_feynman_enabled() const;
        void set_sync_eisenhower_enabled(bool enabled);
        void set_sync_pomodoro_enabled(bool enabled);
        void set_sync_feynman_enabled(bool enabled);
        bool has_active_task_sync(const QString &origin = QString()) const;
        void show_task_sync_add_info(const QString &origin = QString());
        void show_task_sync_delete_info(const QString &origin = QString(), QWidget *parent = nullptr) override;
        void remove_task_from_synced_modules(const QVariantMap &task, const QString &origin);
        bool reserved_space_eisenhower_enabled() const;
        bool reserved_space_pomodoro_enabled() const;
        bool reserved_space_feynman_enabled() const;
        void set_reserved_space_eisenhower_enabled(bool enabled);
        void set_reserved_space_pomodoro_enabled(bool enabled);
        void set_reserved_space_feynman_enabled(bool enabled);
        bool reserved_watermark_eisenhower_enabled() const;
        bool reserved_watermark_pomodoro_enabled() const;
        bool reserved_watermark_feynman_enabled() const;
        void set_reserved_watermark_eisenhower_enabled(bool enabled);
        void set_reserved_watermark_pomodoro_enabled(bool enabled);
        void set_reserved_watermark_feynman_enabled(bool enabled);
        int selected_plugin_task_count() const;
        QString selected_plugin_task_color() const;
        int apply_color_to_selected_plugin_tasks(const QString &color);
        void refresh_task_title_styles();
        QVariantList plugin_status_report() const;

        bool is_in_progress_list(const QListWidget *list) const;
        QListWidget *quadrant_list(int quadrant, bool completed) const;
        QListWidget *quadrant_in_progress_list(int quadrant) const;
        bool is_completed_list(const QListWidget *list) const;

    protected:
        void closeEvent(QCloseEvent *event) override;
        void changeEvent(QEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

    private:
        struct TaskSortCriterionState
        {
            TaskSortCriterion criterion = TaskSortCriterion::Date;
            Qt::SortOrder order = Qt::AscendingOrder;
        };

        QString layout_path;
        TaskLayoutMode task_layout_mode_ = TaskLayoutMode::Tabs;
        QList<TaskSortCriterionState> task_sort_criteria_;
        qint64 task_insertion_counter_ = 0;
        int last_quadrant_tab_index_ = 0;
        QString feynman_plugin_path_;
        QPluginLoader *feynman_plugin_loader_ = nullptr;
        source::plugin::IFeynmanPlugin *feynman_plugin_ = nullptr;
        source::plugin::FeynmanHostBridge *feynman_host_bridge_ = nullptr;
        QString pomodoro_plugin_path_;
        QPluginLoader *pomodoro_plugin_loader_ = nullptr;
        source::plugin::IPomodoroPlugin *pomodoro_plugin_ = nullptr;
        source::plugin::PomodoroHostBridge *pomodoro_host_bridge_ = nullptr;
        QString mapamental_plugin_path_;
        QPluginLoader *mapamental_plugin_loader_ = nullptr;
        source::plugin::IMapaMentalPlugin *mapamental_plugin_ = nullptr;
        source::plugin::MapaMentalHostBridge *mapamental_host_bridge_ = nullptr;
        QString modelo_plugin_path_;
        QPluginLoader *modelo_plugin_loader_ = nullptr;
        source::plugin::IModeloPlugin *modelo_plugin_ = nullptr;
        QString audiolivro_plugin_path_;
        QPluginLoader *audiolivro_plugin_loader_ = nullptr;
        source::plugin::IAudiolivroPlugin *audiolivro_plugin_ = nullptr;
        source::plugin::AudiolivroHostBridge *audiolivro_host_bridge_ = nullptr;
        QString agenda_plugin_path_;
        QPluginLoader *agenda_plugin_loader_ = nullptr;
        source::plugin::IAgendaPlugin *agenda_plugin_ = nullptr;
        source::plugin::AgendaHostBridge *agenda_host_bridge_ = nullptr;
        QString gantt_plugin_path_;
        QPluginLoader *gantt_plugin_loader_ = nullptr;
        source::plugin::IGanttPlugin *gantt_plugin_ = nullptr;
        source::plugin::GanttHostBridge *gantt_host_bridge_ = nullptr;
        QString agenda_color_mode_;
        QString pending_shared_origin_;
        QStringList pending_runtime_sync_origins_;
        bool runtime_sync_flush_scheduled_ = false;
        QStringList pending_save_origins_;
        bool task_save_flush_scheduled_ = false;
        bool sync_eisenhower_enabled_ = true;
        bool sync_pomodoro_enabled_ = true;
        bool sync_feynman_enabled_ = true;
        bool reserved_space_eisenhower_enabled_ = true;
        bool reserved_space_pomodoro_enabled_ = false;
        bool reserved_space_feynman_enabled_ = false;
        bool reserved_watermark_eisenhower_enabled_ = true;
        bool reserved_watermark_pomodoro_enabled_ = false;
        bool reserved_watermark_feynman_enabled_ = false;
        QString feynman_ui_layout_mode_key_ = QStringLiteral("tabs");
        QString pomodoro_ui_layout_mode_key_ = QStringLiteral("simple:tabs");
        QString gantt_table_layout_mode_key_;
        QStringList module_tab_order_;
        QMap<QString, bool> module_tab_visibility_;
        bool module_tab_adjustment_locked_ = true;
        bool applying_module_tab_order_ = false;
        bool window_geometry_stabilization_queued_ = false;
        QMap<QString, QString> plugin_load_errors_;

        void _apply_theme_palette();
        void _hide_legacy_calendar_button();
        bool _window_geometry_adjustment_deferred() const;
        void _queue_window_geometry_stabilization(int delay_ms = 0);
        void _clamp_window_to_screen();
        void _load_task_layout_preference();
        void _save_task_layout_preference() const;
        bool _apply_task_layout();
        bool _attach_quadrant_panels_to_tabs(bool horizontal_layout = false);
        bool _attach_quadrant_panels_to_columns();
        bool _attach_quadrant_panels_to_matrix_2x2();
        bool _open_linked_file_path(const QString &path) const;
        void _setup_module_tabs(QWidget *eisenhower_content);
        void _load_feynman_plugin();
        void _sync_feynman_host_bridge();
        void _load_pomodoro_plugin();
        void _sync_pomodoro_host_bridge();
        void _load_mapamental_plugin();
        void _sync_mapamental_host_bridge();
        void _load_modelo_plugin();
        void _load_audiolivro_plugin();
        void _sync_audiolivro_host_bridge();
        void _load_agenda_plugin();
        void _sync_agenda_host_bridge();
        void _apply_agenda_color_mode(const QString &mode);
        void _load_gantt_plugin();
        void _sync_gantt_host_bridge();
        void _refresh_module_tabs();
        QString _module_tab_key(QWidget *page) const;
        QWidget *_module_page_for_tab_key(const QString &key) const;
        QStringList _current_module_tab_order() const;
        void _capture_module_tab_order(bool persist = true);
        int _visible_available_module_tab_count() const;
        void _sync_module_tab_visibility();
        void _apply_module_tab_order();
        void _sync_module_tab_movable_state();
        void _connect_task_counter_list(QListWidget *list);
        void _connect_feynman_task_counter_sources();
        void _connect_pomodoro_task_counter_sources();
        void _connect_mapamental_task_counter_sources();
        void _connect_audiolivro_task_counter_sources();
        QPair<int, int> _eisenhower_task_counts() const;
        QPair<int, int> _feynman_task_counts() const;
        QPair<int, int> _pomodoro_task_counts() const;
        QPair<int, int> _mapamental_task_counts() const;
        QPair<int, int> _audiolivro_task_counts() const;
        void _sort_task_list(QListWidget *list);
        void _insert_task_item_by_time_group(QListWidget *list, QListWidgetItem *item);
        void _apply_task_item_appearance(QListWidgetItem *item) const;
        QListWidgetItem *_create_task_item(const QVariantMap &data, Qt::CheckState check_state) const;
        bool _task_sort_less(const QVariantMap &left, const QVariantMap &right) const;
        int _task_sort_criterion_index(TaskSortCriterion criterion) const;
        void _ensure_task_metadata(QVariantMap &data);
        qint64 _task_insertion_order(const QVariantMap &data) const;
        void _apply_startup_geometry_constraints();
        bool _send_task_payload_to_feynman(const QVariantMap &task, bool activate_tab = false);
        bool _upsert_task_from_external(const QVariantMap &task);
        void _set_linked_file_for_item(QListWidgetItem *item, const QString &path);
        bool _is_group_header(QListWidgetItem *item) const;
        QString _time_group_label(const QVariantMap &data) const;
        QString _time_group_key(const QVariantMap &data) const;
        QPair<int, int> _time_key(const QString &time_str) const;
        void _sync_splitter_for_calendar();
        void _queue_runtime_sync(const QString &origin);
        void _flush_runtime_sync();
        void _flush_debounced_task_save();
        void _perform_task_save(const QStringList &origins);
        void _reload_eisenhower_from_shared_store();
        void _reload_pomodoro_from_shared_store();
        void _reload_feynman_from_shared_store();
        void _refresh_plugin_task_views();
    };

} // namespace source
