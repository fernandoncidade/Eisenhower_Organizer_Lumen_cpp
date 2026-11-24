#pragma once

#include <QDateEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTableWidget>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include "../application/services/Bim4DService.hpp"
#include "../application/services/GanttSimulationWorker.hpp"
#include "../domain/models/Schedule.hpp"

namespace source::gantt {

class Bim4DSimulationPanel : public QWidget {
    Q_OBJECT

public:
    explicit Bim4DSimulationPanel(QWidget *parent = nullptr);
    ~Bim4DSimulationPanel() override;

    void set_schedule(const Schedule &schedule);
    QDateTime current_reference_date() const;
    QString selected_item_id() const;
    void retranslate_ui();
    void update_control_bar_layout(int panel_width = 0);
    int calculate_button_content_width(const QPushButton *btn) const;
    void adjust_buttons_width(int panel_width = 0);

protected:
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void reference_date_changed(const QDateTime &ref_date);
    void request_worker_simulation(const Schedule &schedule, const QDateTime &ref_date);
    void item_selected(const QString &item_id);
    void request_delete_item(const QString &item_id);

private slots:
    void on_slider_moved(int value);
    void on_date_edit_changed(const QDate &date);
    void start_or_resume();
    void stop_simulation();
    void restart_simulation();
    void on_animation_tick();
    void on_simulation_ready(const Bim4DSimulationReport &report);
    void on_table_custom_context_menu(const QPoint &pos);
    void on_table_current_cell_changed(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
    Schedule schedule_;
    Bim4DService bim_svc_;
    QDateTime min_date_;
    QDateTime max_date_;
    QDateTime current_ref_date_;

    int count_total_ = 0;
    int count_executed_ = 0;
    int count_in_progress_ = 0;
    int count_delayed_ = 0;
    int count_unstarted_ = 0;

    QGroupBox *ctrl_box_ = nullptr;
    QLabel *lbl_cutoff_ = nullptr;
    QSlider *slider_ = nullptr;
    QDateEdit *date_picker_ = nullptr;
    QPushButton *btn_resume_ = nullptr;
    QPushButton *btn_stop_ = nullptr;
    QPushButton *btn_restart_ = nullptr;
    QTimer *anim_timer_ = nullptr;

    QLabel *lbl_total_ = nullptr;
    QLabel *lbl_executed_ = nullptr;
    QLabel *lbl_in_progress_ = nullptr;
    QLabel *lbl_delayed_ = nullptr;
    QLabel *lbl_unstarted_ = nullptr;

    QTableWidget *table_elements_ = nullptr;

    // Worker em thread separada para não travar a UI
    QThread *sim_thread_ = nullptr;
    GanttSimulationWorker *sim_worker_ = nullptr;
    Bim4DSimulationReport last_report_;

    int last_clicked_col_ = -1;
    int last_clicked_row_ = -1;

    enum class ControlBarLayoutMode {
        None,
        SingleLine,
        TwoLines,
        ThreeLines
    };

    QVBoxLayout *ctrl_vlayout_ = nullptr;
    QHBoxLayout *row1_layout_ = nullptr;
    QWidget *row_date_container_ = nullptr;
    QHBoxLayout *row_date_layout_ = nullptr;
    QWidget *date_group_container_ = nullptr;
    QHBoxLayout *date_group_layout_ = nullptr;
    QWidget *row2_container_ = nullptr;
    QHBoxLayout *row2_layout_ = nullptr;
    ControlBarLayoutMode ctrl_bar_mode_ = ControlBarLayoutMode::None;

    void setup_ui();
    void setup_worker_thread();
    void update_simulation();
    void update_kpi_badges();
    QString format_level_text(const QString &raw_level) const;
};

} // namespace source::gantt
