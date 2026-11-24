#include "Bim4DSimulationPanel.hpp"
#include <cmath>
#include <QApplication>
#include <QEvent>
#include <QGroupBox>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QStyle>
#include <QStyleOptionButton>
#include "BiDirectionalResizeHeaderView.hpp"
#include "GanttCellHoverDelegate.hpp"
#include "GanttWbsTreeWidget.hpp"

namespace source::gantt {

Bim4DSimulationPanel::Bim4DSimulationPanel(QWidget *parent)
    : QWidget(parent),
      anim_timer_(new QTimer(this))
{
    setup_ui();
    setup_worker_thread();
    connect(anim_timer_, &QTimer::timeout, this, &Bim4DSimulationPanel::on_animation_tick);
}

Bim4DSimulationPanel::~Bim4DSimulationPanel()
{
    if (anim_timer_) {
        anim_timer_->stop();
    }
    if (sim_thread_) {
        if (sim_worker_) {
            disconnect(sim_worker_, nullptr, this, nullptr);
        }
        if (sim_thread_->isRunning()) {
            sim_thread_->requestInterruption();
            sim_thread_->quit();
            if (!sim_thread_->wait(2000)) {
                sim_thread_->setParent(nullptr);
                connect(sim_thread_, &QThread::finished, sim_thread_, &QObject::deleteLater);
                sim_thread_ = nullptr;
                return;
            }
        }
    }
}

void Bim4DSimulationPanel::setup_worker_thread()
{
    sim_thread_ = new QThread(this);
    sim_worker_ = new GanttSimulationWorker();
    sim_worker_->moveToThread(sim_thread_);

    connect(this, &Bim4DSimulationPanel::request_worker_simulation,
            sim_worker_, &GanttSimulationWorker::do_simulate,
            Qt::QueuedConnection);

    connect(sim_worker_, &GanttSimulationWorker::simulation_finished,
            this, &Bim4DSimulationPanel::on_simulation_ready,
            Qt::QueuedConnection);

    connect(sim_thread_, &QThread::finished, sim_worker_, &QObject::deleteLater);

    sim_thread_->start();
}

void Bim4DSimulationPanel::setup_ui()
{
    auto *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);
    main_layout->setSpacing(6);

    // Top control bar
    ctrl_box_ = new QGroupBox(this);
    ctrl_box_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    ctrl_vlayout_ = new QVBoxLayout(ctrl_box_);
    ctrl_vlayout_->setContentsMargins(8, 6, 8, 6);
    ctrl_vlayout_->setSpacing(4);

    // Row 1: Simulation controls
    row1_layout_ = new QHBoxLayout();
    row1_layout_->setContentsMargins(0, 0, 0, 0);
    row1_layout_->setSpacing(6);

    btn_resume_ = new QPushButton(this);
    btn_resume_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(btn_resume_, &QPushButton::clicked, this, &Bim4DSimulationPanel::start_or_resume);

    btn_stop_ = new QPushButton(this);
    btn_stop_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btn_stop_->setEnabled(false);
    connect(btn_stop_, &QPushButton::clicked, this, &Bim4DSimulationPanel::stop_simulation);

    btn_restart_ = new QPushButton(this);
    btn_restart_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(btn_restart_, &QPushButton::clicked, this, &Bim4DSimulationPanel::restart_simulation);

    ctrl_vlayout_->addLayout(row1_layout_);

    // Row Date container (used in ThreeLines mode)
    row_date_container_ = new QWidget(ctrl_box_);
    row_date_container_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    row_date_layout_ = new QHBoxLayout(row_date_container_);
    row_date_layout_->setContentsMargins(0, 0, 0, 0);
    row_date_layout_->setSpacing(6);

    // Date group container: agrupa a label "Data de Referência:" e o calendário com espaçamento de 3px
    date_group_container_ = new QWidget(ctrl_box_);
    date_group_container_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    date_group_layout_ = new QHBoxLayout(date_group_container_);
    date_group_layout_->setContentsMargins(0, 0, 0, 0);
    date_group_layout_->setSpacing(3); // Aproxima o calendário da string "Data de Referência:"

    lbl_cutoff_ = new QLabel(date_group_container_);
    lbl_cutoff_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lbl_cutoff_->setMargin(0);
    lbl_cutoff_->setIndent(0);
    lbl_cutoff_->setContentsMargins(0, 0, 0, 0);
    lbl_cutoff_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    date_picker_ = new QDateEdit(QDate::currentDate(), date_group_container_);
    date_picker_->setCalendarPopup(true);
    date_picker_->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
    date_picker_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(date_picker_, &QDateEdit::dateChanged, this, &Bim4DSimulationPanel::on_date_edit_changed);

    date_group_layout_->addWidget(lbl_cutoff_);
    date_group_layout_->addWidget(date_picker_);

    slider_ = new QSlider(Qt::Horizontal, this);
    slider_->setRange(0, 100);
    slider_->setMinimumWidth(30);
    slider_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(slider_, &QSlider::valueChanged, this, &Bim4DSimulationPanel::on_slider_moved);

    ctrl_vlayout_->addWidget(row_date_container_);

    // KPI Badges
    lbl_total_ = new QLabel(this);
    lbl_total_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    lbl_executed_ = new QLabel(this);
    lbl_executed_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lbl_executed_->setStyleSheet(QStringLiteral("color: #2E7D32; font-weight: bold;"));

    lbl_in_progress_ = new QLabel(this);
    lbl_in_progress_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lbl_in_progress_->setStyleSheet(QStringLiteral("color: #1565C0; font-weight: bold;"));

    lbl_delayed_ = new QLabel(this);
    lbl_delayed_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lbl_delayed_->setStyleSheet(QStringLiteral("color: #C62828; font-weight: bold;"));

    lbl_unstarted_ = new QLabel(this);
    lbl_unstarted_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lbl_unstarted_->setStyleSheet(QStringLiteral("color: #757575;"));

    // Row 2 container for KPI Badges when wrapped to second or third line
    row2_container_ = new QWidget(ctrl_box_);
    row2_container_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    row2_layout_ = new QHBoxLayout(row2_container_);
    row2_layout_->setContentsMargins(0, 2, 0, 0);
    row2_layout_->setSpacing(8);

    ctrl_vlayout_->addWidget(row2_container_);

    retranslate_ui();

    main_layout->addWidget(ctrl_box_, 0);

    // Elements Table
    table_elements_ = new QTableWidget(this);

    // Install custom headers that support left-edge and right-edge resizing
    auto *h_header = new BiDirectionalResizeHeaderView(Qt::Horizontal, table_elements_);
    table_elements_->setHorizontalHeader(h_header);

    auto *v_header = new BiDirectionalResizeHeaderView(Qt::Vertical, table_elements_);
    table_elements_->setVerticalHeader(v_header);

    table_elements_->setColumnCount(7);

    // Interactive resize mode allows user to drag column borders to resize
    table_elements_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table_elements_->horizontalHeader()->setStretchLastSection(false);

    // Set reasonable default widths
    table_elements_->horizontalHeader()->resizeSection(0, 120);  // Status 4D
    table_elements_->horizontalHeader()->resizeSection(1, 250);  // GUID IFC
    table_elements_->horizontalHeader()->resizeSection(2, 200);  // Nome do Elemento
    table_elements_->horizontalHeader()->resizeSection(3, 100);  // WBS
    table_elements_->horizontalHeader()->resizeSection(4, 120);  // Categoria
    table_elements_->horizontalHeader()->resizeSection(5, 100);  // Nível
    table_elements_->horizontalHeader()->resizeSection(6, 130);  // Progresso Físico

    // Enable vertical header for row height resizing by dragging row borders
    table_elements_->verticalHeader()->setVisible(true);
    table_elements_->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table_elements_->verticalHeader()->setDefaultSectionSize(30);
    table_elements_->verticalHeader()->setMinimumSectionSize(18);

    table_elements_->setAlternatingRowColors(true);
    table_elements_->setSelectionBehavior(QAbstractItemView::SelectItems);
    table_elements_->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Install hover highlight delegate (lighter in dark theme, darker in light theme)
    table_elements_->setItemDelegate(new GanttCellHoverDelegate(table_elements_));
    table_elements_->setMouseTracking(true);
    if (table_elements_->viewport()) {
        table_elements_->viewport()->setMouseTracking(true);
        table_elements_->viewport()->setAttribute(Qt::WA_Hover, true);
    }

    // Column selection on header click (manual cell-by-cell selection, supporting single and multiple columns)
    table_elements_->horizontalHeader()->setSectionsClickable(true);
    connect(table_elements_->horizontalHeader(), &QHeaderView::sectionClicked, this, [this](int logicalIndex) {
        const auto modifiers = QApplication::keyboardModifiers();
        const bool ctrl = modifiers.testFlag(Qt::ControlModifier);
        const bool shift = modifiers.testFlag(Qt::ShiftModifier);

        if (!ctrl && !shift) {
            table_elements_->clearSelection();
        }

        int start_col = logicalIndex;
        int end_col = logicalIndex;
        if (shift && last_clicked_col_ >= 0) {
            start_col = std::min(last_clicked_col_, logicalIndex);
            end_col = std::max(last_clicked_col_, logicalIndex);
        } else {
            last_clicked_col_ = logicalIndex;
        }

        for (int col = start_col; col <= end_col; ++col) {
            for (int row = 0; row < table_elements_->rowCount(); ++row) {
                if (auto *item = table_elements_->item(row, col)) {
                    item->setSelected(true);
                } else {
                    QModelIndex idx = table_elements_->model()->index(row, col);
                    table_elements_->selectionModel()->select(idx, QItemSelectionModel::Select);
                }
            }
        }
    });

    // Row selection on vertical header click (manual cell-by-cell selection, supporting single and multiple rows)
    table_elements_->verticalHeader()->setSectionsClickable(true);
    connect(table_elements_->verticalHeader(), &QHeaderView::sectionClicked, this, [this](int logicalIndex) {
        const auto modifiers = QApplication::keyboardModifiers();
        const bool ctrl = modifiers.testFlag(Qt::ControlModifier);
        const bool shift = modifiers.testFlag(Qt::ShiftModifier);

        if (!ctrl && !shift) {
            table_elements_->clearSelection();
        }

        int start_row = logicalIndex;
        int end_row = logicalIndex;
        if (shift && last_clicked_row_ >= 0) {
            start_row = std::min(last_clicked_row_, logicalIndex);
            end_row = std::max(last_clicked_row_, logicalIndex);
        } else {
            last_clicked_row_ = logicalIndex;
        }

        for (int row = start_row; row <= end_row; ++row) {
            for (int col = 0; col < table_elements_->columnCount(); ++col) {
                if (auto *item = table_elements_->item(row, col)) {
                    item->setSelected(true);
                } else {
                    QModelIndex idx = table_elements_->model()->index(row, col);
                    table_elements_->selectionModel()->select(idx, QItemSelectionModel::Select);
                }
            }
        }
    });

    table_elements_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(table_elements_, &QTableWidget::customContextMenuRequested, this, &Bim4DSimulationPanel::on_table_custom_context_menu);
    connect(table_elements_, &QTableWidget::currentCellChanged, this, &Bim4DSimulationPanel::on_table_current_cell_changed);

    main_layout->addWidget(table_elements_, 1);

    retranslate_ui();
}

void Bim4DSimulationPanel::retranslate_ui()
{
    setUpdatesEnabled(false);

    if (ctrl_box_) {
        ctrl_box_->setTitle(tr("Simulação Temporal - 4D BIM"));
    }
    if (btn_stop_) {
        btn_stop_->setText(tr("⏹️ Parar Simulação"));
    }
    if (btn_restart_) {
        btn_restart_->setText(tr("🔄 Reiniciar Simulação"));
    }
    if (btn_resume_) {
        if (slider_ && slider_->value() > 0 && anim_timer_ && !anim_timer_->isActive()) {
            btn_resume_->setText(tr("▶️ Retomar Simulação"));
        } else {
            btn_resume_->setText(tr("▶️ Iniciar Simulação"));
        }
    }
    if (lbl_cutoff_) {
        lbl_cutoff_->setText(tr("Data de Referência:"));
    }
    ctrl_bar_mode_ = ControlBarLayoutMode::None;
    adjust_buttons_width(width());
    if (table_elements_) {
        const QStringList headers = {
            tr("Status 4D"),
            tr("GUID IFC / Id Único"),
            tr("Atividade do Cronograma"),
            tr("WBS"),
            tr("Categoria"),
            tr("Nível"),
            tr("Progresso Físico")
        };
        table_elements_->setHorizontalHeaderLabels(headers);
        for (int c = 0; c < headers.size() && c < table_elements_->columnCount(); ++c) {
            if (auto *item = table_elements_->horizontalHeaderItem(c)) {
                item->setText(headers.at(c));
            } else {
                table_elements_->setHorizontalHeaderItem(c, new QTableWidgetItem(headers.at(c)));
            }
        }
        if (auto *hh = table_elements_->horizontalHeader()) {
            hh->viewport()->update();
            hh->update();
        }
    }
    update_kpi_badges();

    // Atualiza imediatamente a tabela secundária traduzindo as colunas "4D Status", "Atividade", tooltips e "Nível" em-lugar
    if (!last_report_.elements.isEmpty() && table_elements_) {
        table_elements_->setUpdatesEnabled(false);
        const int row_count = std::min<int>(table_elements_->rowCount(), static_cast<int>(last_report_.elements.size()));
        const QString delay_tip = GanttWbsTreeWidget::delay_tooltip(true);
        for (int row = 0; row < row_count; ++row) {
            const auto &el = last_report_.elements.at(row);
            QString status_txt;
            switch (el.status) {
                case Bim4DStatus::Executed:
                    status_txt = tr("Executado");
                    break;
                case Bim4DStatus::PlannedInProgress:
                    status_txt = tr("Em Andamento");
                    break;
                case Bim4DStatus::DelayedStart:
                case Bim4DStatus::DelayedOverdue:
                    status_txt = tr("Atrasado");
                    break;
                case Bim4DStatus::Unstarted:
                default:
                    status_txt = tr("Futuro");
                    break;
            }
            const bool is_delayed = (el.status == Bim4DStatus::DelayedStart || el.status == Bim4DStatus::DelayedOverdue);
            if (auto *item0 = table_elements_->item(row, 0)) {
                item0->setText(status_txt.toUpper());
                if (is_delayed) {
                    item0->setToolTip(delay_tip);
                } else {
                    item0->setToolTip(QString());
                }
            }
            if (auto *item2 = table_elements_->item(row, 2)) {
                if (is_delayed) {
                    item2->setText(QStringLiteral("⚠️ %1").arg(el.activity_name));
                    item2->setForeground(QBrush(QColor(211, 47, 47)));
                    item2->setToolTip(delay_tip);
                } else {
                    item2->setText(el.activity_name);
                    item2->setForeground(table_elements_->palette().text());
                    item2->setToolTip(QString());
                }
            }
            if (auto *item5 = table_elements_->item(row, 5)) {
                item5->setText(format_level_text(el.level));
            }
        }
        table_elements_->setUpdatesEnabled(true);
    }

    update_control_bar_layout(width());
    setUpdatesEnabled(true);
}

void Bim4DSimulationPanel::set_schedule(const Schedule &schedule)
{
    schedule_ = schedule;
    min_date_ = schedule.project_start_date();
    max_date_ = schedule.project_finish_date();

    if (!min_date_.isValid()) min_date_ = QDateTime::currentDateTime().addDays(-1);
    if (!max_date_.isValid()) max_date_ = min_date_.addDays(30);

    qint64 total_days = min_date_.daysTo(max_date_);
    slider_->blockSignals(true);
    slider_->setRange(0, static_cast<int>(std::max<qint64>(1, total_days)));
    slider_->setValue(0);
    slider_->blockSignals(false);

    current_ref_date_ = min_date_;
    date_picker_->blockSignals(true);
    date_picker_->setDate(min_date_.date());
    date_picker_->blockSignals(false);

    if (btn_resume_) {
        btn_resume_->setText(tr("▶️ Iniciar Simulação"));
        btn_resume_->setEnabled(true);
    }
    if (btn_stop_) {
        btn_stop_->setEnabled(false);
    }
    if (btn_restart_) {
        btn_restart_->setEnabled(true);
    }
    adjust_buttons_width(width());

    update_simulation();
}

QDateTime Bim4DSimulationPanel::current_reference_date() const
{
    return current_ref_date_;
}

void Bim4DSimulationPanel::on_slider_moved(int value)
{
    current_ref_date_ = min_date_.addDays(value);
    date_picker_->blockSignals(true);
    date_picker_->setDate(current_ref_date_.date());
    date_picker_->blockSignals(false);

    if (btn_resume_ && !anim_timer_->isActive()) {
        btn_resume_->setText(value > 0 ? tr("▶️ Retomar Simulação") : tr("▶️ Iniciar Simulação"));
        adjust_buttons_width(width());
    }

    update_simulation();
    emit reference_date_changed(current_ref_date_);
}

void Bim4DSimulationPanel::on_date_edit_changed(const QDate &date)
{
    current_ref_date_.setDate(date);
    int days = static_cast<int>(min_date_.daysTo(current_ref_date_));
    slider_->blockSignals(true);
    slider_->setValue(std::max(0, std::min(days, slider_->maximum())));
    slider_->blockSignals(false);

    if (btn_resume_ && !anim_timer_->isActive()) {
        btn_resume_->setText(slider_->value() > 0 ? tr("▶️ Retomar Simulação") : tr("▶️ Iniciar Simulação"));
        adjust_buttons_width(width());
    }

    update_simulation();
    emit reference_date_changed(current_ref_date_);
}

void Bim4DSimulationPanel::start_or_resume()
{
    if (slider_->value() >= slider_->maximum()) {
        slider_->setValue(0);
    }
    anim_timer_->start(250); // avança 1 dia a cada 250ms
    btn_resume_->setEnabled(false);
    btn_stop_->setEnabled(true);
    btn_restart_->setEnabled(true);
}

void Bim4DSimulationPanel::stop_simulation()
{
    anim_timer_->stop();
    btn_resume_->setText(tr("▶️ Retomar Simulação"));
    adjust_buttons_width(width());
    btn_resume_->setEnabled(true);
    btn_stop_->setEnabled(false);
    btn_restart_->setEnabled(true);
}

void Bim4DSimulationPanel::restart_simulation()
{
    anim_timer_->stop();
    slider_->blockSignals(true);
    slider_->setValue(0);
    slider_->blockSignals(false);

    current_ref_date_ = min_date_;
    date_picker_->blockSignals(true);
    date_picker_->setDate(min_date_.date());
    date_picker_->blockSignals(false);

    btn_resume_->setText(tr("▶️ Iniciar Simulação"));
    adjust_buttons_width(width());
    btn_resume_->setEnabled(true);
    btn_stop_->setEnabled(false);
    btn_restart_->setEnabled(true);

    update_simulation();
    emit reference_date_changed(current_ref_date_);
}

void Bim4DSimulationPanel::on_animation_tick()
{
    if (slider_->value() >= slider_->maximum()) {
        anim_timer_->stop();
        slider_->setValue(0);
        btn_resume_->setText(tr("▶️ Iniciar Simulação"));
        adjust_buttons_width(width());
        btn_resume_->setEnabled(true);
        btn_stop_->setEnabled(false);
        return;
    }
    slider_->setValue(slider_->value() + 1);
}

void Bim4DSimulationPanel::update_simulation()
{
    if (sim_thread_ && sim_thread_->isRunning()) {
        emit request_worker_simulation(schedule_, current_ref_date_);
    } else {
        Bim4DSimulationReport rep = bim_svc_.simulate(schedule_, current_ref_date_);
        on_simulation_ready(rep);
    }
}

void Bim4DSimulationPanel::on_simulation_ready(const Bim4DSimulationReport &rep)
{
    last_report_ = rep;

    count_total_ = rep.total_elements;
    count_executed_ = rep.executed_count;
    count_in_progress_ = rep.planned_in_progress_count;
    count_delayed_ = rep.delayed_start_count + rep.delayed_overdue_count;
    count_unstarted_ = rep.unstarted_count;

    update_kpi_badges();

    if (!table_elements_) return;

    table_elements_->setUpdatesEnabled(false);

    if (table_elements_->rowCount() != rep.elements.size()) {
        table_elements_->setRowCount(rep.elements.size());
    }

    int row = 0;
    for (const auto &el : rep.elements) {
        QString status_txt;
        switch (el.status) {
            case Bim4DStatus::Executed:
                status_txt = tr("Executado");
                break;
            case Bim4DStatus::PlannedInProgress:
                status_txt = tr("Em Andamento");
                break;
            case Bim4DStatus::DelayedStart:
            case Bim4DStatus::DelayedOverdue:
                status_txt = tr("Atrasado");
                break;
            case Bim4DStatus::Unstarted:
            default:
                status_txt = tr("Futuro");
                break;
        }

        const bool is_delayed = (el.status == Bim4DStatus::DelayedStart || el.status == Bim4DStatus::DelayedOverdue);
        const QString delay_tip = GanttWbsTreeWidget::delay_tooltip(true);

        // Coluna 0: Status 4D (traduzido dinamicamente)
        QTableWidgetItem *item0 = table_elements_->item(row, 0);
        if (!item0) {
            item0 = new QTableWidgetItem();
            table_elements_->setItem(row, 0, item0);
        }
        item0->setText(status_txt.toUpper());
        item0->setForeground(QBrush(QColor(el.color_hex)));
        item0->setFont(QFont(table_elements_->font().family(), -1, QFont::Bold));
        item0->setData(Qt::UserRole, el.schedule_item_id);
        if (is_delayed) {
            item0->setToolTip(delay_tip);
        } else {
            item0->setToolTip(QString());
        }

        // Coluna 1: GUID / UniqueId
        QTableWidgetItem *item1 = table_elements_->item(row, 1);
        if (!item1) {
            item1 = new QTableWidgetItem();
            table_elements_->setItem(row, 1, item1);
        }
        item1->setText(el.bim_element_id);

        // Coluna 2: Atividade do Cronograma
        QTableWidgetItem *item2 = table_elements_->item(row, 2);
        if (!item2) {
            item2 = new QTableWidgetItem();
            table_elements_->setItem(row, 2, item2);
        }
        if (is_delayed) {
            item2->setText(QStringLiteral("⚠️ %1").arg(el.activity_name));
            item2->setForeground(QBrush(QColor(211, 47, 47))); // Dark red
            item2->setToolTip(delay_tip);
        } else {
            item2->setText(el.activity_name);
            item2->setForeground(table_elements_->palette().text());
            item2->setToolTip(QString());
        }

        // Coluna 3: WBS
        QTableWidgetItem *item3 = table_elements_->item(row, 3);
        if (!item3) {
            item3 = new QTableWidgetItem();
            table_elements_->setItem(row, 3, item3);
        }
        item3->setText(el.wbs_code);

        // Coluna 4: Categoria
        QTableWidgetItem *item4 = table_elements_->item(row, 4);
        if (!item4) {
            item4 = new QTableWidgetItem();
            table_elements_->setItem(row, 4, item4);
        }
        item4->setText(el.category);

        // Coluna 5: Nível (traduzido dinamicamente)
        QTableWidgetItem *item5 = table_elements_->item(row, 5);
        if (!item5) {
            item5 = new QTableWidgetItem();
            table_elements_->setItem(row, 5, item5);
        }
        item5->setText(format_level_text(el.level));

        // Coluna 6: Progresso Físico
        QTableWidgetItem *item6 = table_elements_->item(row, 6);
        if (!item6) {
            item6 = new QTableWidgetItem();
            table_elements_->setItem(row, 6, item6);
        }
        const QString prog_str = QString::number(el.progress_percent, 'f',
            (el.progress_percent == std::floor(el.progress_percent)) ? 0 : 2) + QLatin1Char('%');
        item6->setText(prog_str);

        row++;
    }

    table_elements_->setUpdatesEnabled(true);
}

QString Bim4DSimulationPanel::format_level_text(const QString &raw_level) const
{
    const QString trimmed = raw_level.trimmed();
    static const QRegularExpression nivel_re(QStringLiteral("^(?:N[ií]vel|Level)\\s*(\\d+)$"), QRegularExpression::CaseInsensitiveOption);
    const auto match = nivel_re.match(trimmed);
    if (match.hasMatch()) {
        return tr("Nível %1").arg(match.captured(1));
    }

    static const QRegularExpression pav_re(QStringLiteral("^(?:Pavimento|Floor)\\s*(\\d+)$"), QRegularExpression::CaseInsensitiveOption);
    const auto pav_match = pav_re.match(trimmed);
    if (pav_match.hasMatch()) {
        return tr("Pavimento %1").arg(pav_match.captured(1));
    }

    if (trimmed.compare(QStringLiteral("Geral"), Qt::CaseInsensitive) == 0 ||
        trimmed.compare(QStringLiteral("General"), Qt::CaseInsensitive) == 0) {
        return tr("Geral");
    }
    return raw_level;
}

int Bim4DSimulationPanel::calculate_button_content_width(const QPushButton *btn) const
{
    if (!btn) return 0;
    const QString text = btn->text();
    if (text.isEmpty()) return 40;

    const QFontMetrics fm = btn->fontMetrics();
    int text_w = fm.horizontalAdvance(text);

    // Se for o botão de iniciar/retomar simulação, usa o máximo de ambos os estados para evitar jitter
    if (btn == btn_resume_) {
        const int w_start = fm.horizontalAdvance(tr("▶️ Iniciar Simulação"));
        const int w_resume = fm.horizontalAdvance(tr("▶️ Retomar Simulação"));
        text_w = std::max({text_w, w_start, w_resume});
    }

    QStyleOptionButton opt;
    opt.initFrom(btn);
    opt.text = text;
    opt.icon = btn->icon();
    opt.iconSize = btn->iconSize();

    int styled_w = 0;
    if (btn->style()) {
        styled_w = btn->style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(text_w, 24), btn).width();
    }

    const int hint_w = btn->sizeHint().width();
    return std::max({styled_w, text_w + 24, hint_w});
}

void Bim4DSimulationPanel::adjust_buttons_width(int panel_width)
{
    if (!btn_resume_ || !btn_stop_ || !btn_restart_) {
        return;
    }

    if (panel_width <= 0) {
        panel_width = width();
    }

    const int ideal_resume = calculate_button_content_width(btn_resume_);
    const int ideal_stop = calculate_button_content_width(btn_stop_);
    const int ideal_restart = calculate_button_content_width(btn_restart_);
    const int total_buttons_w = ideal_resume + ideal_stop + ideal_restart;

    // A largura mínima dos botões DEVE ficar estritamente limitada com base no comprimento total do texto interno destes botões
    btn_resume_->setMinimumWidth(ideal_resume);
    btn_resume_->setMaximumWidth(ideal_resume);

    btn_stop_->setMinimumWidth(ideal_stop);
    btn_stop_->setMaximumWidth(ideal_stop);

    btn_restart_->setMinimumWidth(ideal_restart);
    btn_restart_->setMaximumWidth(ideal_restart);

    btn_resume_->setToolTip(btn_resume_->text());
    btn_stop_->setToolTip(btn_stop_->text());
    btn_restart_->setToolTip(btn_restart_->text());

    // Fixa rigidamente a largura e política da label de Data de Referência para nunca ser comprimida ou truncada
    if (lbl_cutoff_) {
        lbl_cutoff_->setText(tr("Data de Referência:"));
        const int text_w = lbl_cutoff_->fontMetrics().horizontalAdvance(lbl_cutoff_->text());
        const int cutoff_w = text_w + 2;
        lbl_cutoff_->setMinimumWidth(cutoff_w);
        lbl_cutoff_->setMaximumWidth(cutoff_w);
        lbl_cutoff_->setVisible(true);
    }

    // Fixa rigidamente a largura do date picker para o botão do calendário nunca estourar ou sobrepor
    if (date_picker_) {
        const int dp_w = std::max(date_picker_->sizeHint().width(), 105);
        date_picker_->setMinimumWidth(dp_w);
        date_picker_->setMaximumWidth(dp_w);
        date_picker_->setVisible(true);
    }

    // Agrupa a label e o date picker mantendo-os sempre bem próximos (3px) em todos os layouts
    if (date_group_container_ && lbl_cutoff_ && date_picker_) {
        const int dg_spacing = (date_group_layout_ ? date_group_layout_->spacing() : 3);
        const int dg_w = lbl_cutoff_->minimumWidth() + date_picker_->minimumWidth() + dg_spacing;
        date_group_container_->setMinimumWidth(dg_w);
        date_group_container_->setMaximumWidth(dg_w);
        date_group_container_->setVisible(true);
    }

    // A largura mínima total do painel de simulação garante que os botões (linha 1) caibam perfeitamente
    // sem estourar e respeitando o limite físico mínimo ao arrastar os splitters
    const int min_panel_w = total_buttons_w + 44;
    setMinimumWidth(min_panel_w);
}

void Bim4DSimulationPanel::update_kpi_badges()
{
    if (!lbl_total_ || !lbl_executed_ || !lbl_in_progress_ || !lbl_delayed_ || !lbl_unstarted_) {
        return;
    }

    const int panel_w = width();
    const bool compact = (panel_w < 540);

    if (compact) {
        lbl_total_->setText(tr("Total: %1").arg(count_total_));
        lbl_executed_->setText(QStringLiteral("✅ %1").arg(count_executed_));
        lbl_in_progress_->setText(QStringLiteral("🔄 %1").arg(count_in_progress_));
        lbl_delayed_->setText(QStringLiteral("⚠️ %1").arg(count_delayed_));
        lbl_unstarted_->setText(QStringLiteral("⏳ %1").arg(count_unstarted_));

        lbl_total_->setToolTip(tr("Total: %1").arg(count_total_));
        lbl_executed_->setToolTip(tr("✅ Executado: %1").arg(count_executed_));
        lbl_in_progress_->setToolTip(tr("🔄 Em Andamento: %1").arg(count_in_progress_));
        lbl_delayed_->setToolTip(tr("⚠️ Atrasado: %1").arg(count_delayed_));
        lbl_unstarted_->setToolTip(tr("⏳ Futuro: %1").arg(count_unstarted_));
    } else {
        lbl_total_->setText(tr("Total: %1").arg(count_total_));
        lbl_executed_->setText(tr("✅ Executado: %1").arg(count_executed_));
        lbl_in_progress_->setText(tr("🔄 Em Andamento: %1").arg(count_in_progress_));
        lbl_delayed_->setText(tr("⚠️ Atrasado: %1").arg(count_delayed_));
        lbl_unstarted_->setText(tr("⏳ Futuro: %1").arg(count_unstarted_));

        lbl_total_->setToolTip(QString());
        lbl_executed_->setToolTip(QString());
        lbl_in_progress_->setToolTip(QString());
        lbl_delayed_->setToolTip(QString());
        lbl_unstarted_->setToolTip(QString());
    }
}

void Bim4DSimulationPanel::update_control_bar_layout(int panel_width)
{
    if (panel_width <= 0) {
        panel_width = width();
    }

    adjust_buttons_width(panel_width);
    update_kpi_badges();

    const int w_buttons = (btn_resume_ ? btn_resume_->minimumWidth() : 0) +
                          (btn_stop_ ? btn_stop_->minimumWidth() : 0) +
                          (btn_restart_ ? btn_restart_->minimumWidth() : 0);

    const int cutoff_w = (lbl_cutoff_ ? lbl_cutoff_->minimumWidth() : 110);
    const int dp_w = (date_picker_ ? date_picker_->minimumWidth() : 105);
    const int dg_spacing = (date_group_layout_ ? date_group_layout_->spacing() : 3);
    const int w_date_group = cutoff_w + dp_w + dg_spacing;
    const int slider_comfy_w = 80;

    int w_kpis = 0;
    if (lbl_total_) w_kpis += lbl_total_->sizeHint().width();
    if (lbl_executed_) w_kpis += lbl_executed_->sizeHint().width();
    if (lbl_in_progress_) w_kpis += lbl_in_progress_->sizeHint().width();
    if (lbl_delayed_) w_kpis += lbl_delayed_->sizeHint().width();
    if (lbl_unstarted_) w_kpis += lbl_unstarted_->sizeHint().width();
    w_kpis += 32;

    const int required_two_line_w = w_buttons + w_date_group + slider_comfy_w + 50;
    const int required_single_line_w = required_two_line_w + w_kpis + 30;

    ControlBarLayoutMode target_mode = ControlBarLayoutMode::ThreeLines;
    if (panel_width >= required_single_line_w) {
        target_mode = ControlBarLayoutMode::SingleLine;
    } else if (panel_width >= required_two_line_w) {
        target_mode = ControlBarLayoutMode::TwoLines;
    } else {
        target_mode = ControlBarLayoutMode::ThreeLines;
    }

    const bool mode_changed = (target_mode != ctrl_bar_mode_) ||
                              (row1_layout_ && row1_layout_->count() == 0);

    if (mode_changed && row1_layout_ && row_date_layout_ && row2_layout_ &&
        row_date_container_ && row2_container_ && date_group_container_) {

        ctrl_bar_mode_ = target_mode;

        auto clear_layout = [](QLayout *layout) {
            if (!layout) return;
            while (QLayoutItem *item = layout->takeAt(0)) {
                delete item;
            }
        };

        clear_layout(row1_layout_);
        clear_layout(row_date_layout_);
        clear_layout(row2_layout_);

        if (target_mode == ControlBarLayoutMode::SingleLine) {
            date_group_container_->setParent(ctrl_box_);
            slider_->setParent(ctrl_box_);
            lbl_total_->setParent(ctrl_box_);
            lbl_executed_->setParent(ctrl_box_);
            lbl_in_progress_->setParent(ctrl_box_);
            lbl_delayed_->setParent(ctrl_box_);
            lbl_unstarted_->setParent(ctrl_box_);

            row1_layout_->addWidget(btn_resume_);
            row1_layout_->addWidget(btn_stop_);
            row1_layout_->addWidget(btn_restart_);
            row1_layout_->addWidget(date_group_container_);
            row1_layout_->addWidget(slider_);
            row1_layout_->addWidget(lbl_total_);
            row1_layout_->addWidget(lbl_executed_);
            row1_layout_->addWidget(lbl_in_progress_);
            row1_layout_->addWidget(lbl_delayed_);
            row1_layout_->addWidget(lbl_unstarted_);

            row_date_container_->setVisible(false);
            row2_container_->setVisible(false);
        } else if (target_mode == ControlBarLayoutMode::TwoLines) {
            date_group_container_->setParent(ctrl_box_);
            slider_->setParent(ctrl_box_);
            lbl_total_->setParent(row2_container_);
            lbl_executed_->setParent(row2_container_);
            lbl_in_progress_->setParent(row2_container_);
            lbl_delayed_->setParent(row2_container_);
            lbl_unstarted_->setParent(row2_container_);

            row1_layout_->addWidget(btn_resume_);
            row1_layout_->addWidget(btn_stop_);
            row1_layout_->addWidget(btn_restart_);
            row1_layout_->addWidget(date_group_container_);
            row1_layout_->addWidget(slider_);

            row2_layout_->addWidget(lbl_total_);
            row2_layout_->addWidget(lbl_executed_);
            row2_layout_->addWidget(lbl_in_progress_);
            row2_layout_->addWidget(lbl_delayed_);
            row2_layout_->addWidget(lbl_unstarted_);
            row2_layout_->addStretch(1);

            row_date_container_->setVisible(false);
            row2_container_->setVisible(true);
        } else { // ThreeLines
            date_group_container_->setParent(row_date_container_);
            slider_->setParent(row_date_container_);
            lbl_total_->setParent(row2_container_);
            lbl_executed_->setParent(row2_container_);
            lbl_in_progress_->setParent(row2_container_);
            lbl_delayed_->setParent(row2_container_);
            lbl_unstarted_->setParent(row2_container_);

            row1_layout_->addWidget(btn_resume_);
            row1_layout_->addWidget(btn_stop_);
            row1_layout_->addWidget(btn_restart_);
            row1_layout_->addStretch(1);

            row_date_layout_->addWidget(date_group_container_);
            row_date_layout_->addWidget(slider_);

            row2_layout_->addWidget(lbl_total_);
            row2_layout_->addWidget(lbl_executed_);
            row2_layout_->addWidget(lbl_in_progress_);
            row2_layout_->addWidget(lbl_delayed_);
            row2_layout_->addWidget(lbl_unstarted_);
            row2_layout_->addStretch(1);

            row_date_container_->setVisible(true);
            row2_container_->setVisible(true);
        }

        btn_resume_->show();
        btn_stop_->show();
        btn_restart_->show();
        date_group_container_->show();
        lbl_cutoff_->show();
        date_picker_->show();
        slider_->show();
        lbl_total_->show();
        lbl_executed_->show();
        lbl_in_progress_->show();
        lbl_delayed_->show();
        lbl_unstarted_->show();
    }

    if (ctrl_box_ && ctrl_vlayout_) {
        ctrl_box_->setMinimumHeight(0);
        ctrl_box_->setMaximumHeight(QWIDGETSIZE_MAX);
        ctrl_vlayout_->invalidate();
        ctrl_vlayout_->activate();

        int content_h = (row1_layout_ ? row1_layout_->sizeHint().height() : 28);
        if (ctrl_bar_mode_ == ControlBarLayoutMode::ThreeLines) {
            content_h += (row_date_layout_ ? row_date_layout_->sizeHint().height() : 28) + 4;
        }
        if (ctrl_bar_mode_ != ControlBarLayoutMode::SingleLine) {
            content_h += (row2_layout_ ? row2_layout_->sizeHint().height() : 22) + 4;
        }

        const int desired_h = content_h + 30;
        ctrl_box_->setFixedHeight(desired_h);
        ctrl_box_->updateGeometry();
    }

    if (layout()) {
        layout()->activate();
    }
    updateGeometry();
}

void Bim4DSimulationPanel::changeEvent(QEvent *event)
{
    if (event && event->type() == QEvent::LanguageChange) {
        retranslate_ui();
        adjust_buttons_width(width());
    }
    QWidget::changeEvent(event);
}

void Bim4DSimulationPanel::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    update_control_bar_layout(width());
    retranslate_ui();
    adjust_buttons_width(width());
}

void Bim4DSimulationPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (event) {
        update_control_bar_layout(event->size().width());
    }
}

QString Bim4DSimulationPanel::selected_item_id() const
{
    if (!table_elements_) return {};
    int row = table_elements_->currentRow();
    if (row < 0 && table_elements_->selectionModel()) {
        const auto sel = table_elements_->selectionModel()->selectedRows();
        if (!sel.isEmpty()) {
            row = sel.first().row();
        } else {
            const auto indexes = table_elements_->selectionModel()->selectedIndexes();
            if (!indexes.isEmpty()) {
                row = indexes.first().row();
            }
        }
    }
    if (row >= 0 && row < table_elements_->rowCount()) {
        if (auto *it = table_elements_->item(row, 0)) {
            return it->data(Qt::UserRole).toString();
        }
    }
    return {};
}

void Bim4DSimulationPanel::on_table_custom_context_menu(const QPoint &pos)
{
    if (!table_elements_) return;
    int row = table_elements_->rowAt(pos.y());
    if (row < 0) return;

    QTableWidgetItem *item0 = table_elements_->item(row, 0);
    if (!item0) return;

    const QString item_id = item0->data(Qt::UserRole).toString();
    if (item_id.trimmed().isEmpty()) return;

    QMenu menu(this);
    auto *act_del = menu.addAction(tr("❌ Excluir Atividade"));
    connect(act_del, &QAction::triggered, this, [this, item_id]() {
        emit request_delete_item(item_id);
    });
    menu.exec(table_elements_->viewport()->mapToGlobal(pos));
}

void Bim4DSimulationPanel::on_table_current_cell_changed(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn);
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);
    if (currentRow >= 0 && table_elements_) {
        if (auto *it = table_elements_->item(currentRow, 0)) {
            const QString item_id = it->data(Qt::UserRole).toString();
            if (!item_id.trimmed().isEmpty()) {
                emit item_selected(item_id);
            }
        }
    }
}

void Bim4DSimulationPanel::keyPressEvent(QKeyEvent *event)
{
    if (event && event->key() == Qt::Key_Delete) {
        const QString item_id = selected_item_id();
        if (!item_id.isEmpty()) {
            emit request_delete_item(item_id);
            return;
        }
    }
    QWidget::keyPressEvent(event);
}

} // namespace source::gantt
