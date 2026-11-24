#include "GanttWorkingDaysDialog.hpp"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace source::gantt {

GanttWorkingDaysDialog::GanttWorkingDaysDialog(const WorkCalendar &current_cal, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Configurar Dias Úteis e Jornada de Trabalho"));
    setMinimumWidth(440);
    setup_ui(current_cal);
}

void GanttWorkingDaysDialog::setup_ui(const WorkCalendar &cal)
{
    auto *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(12);

    // Group: Dias da Semana
    auto *group_days = new QGroupBox(tr("Dias da Semana Considerados Úteis"), this);
    auto *days_layout = new QVBoxLayout(group_days);
    days_layout->setSpacing(6);

    const QString day_names[7] = {
        tr("Segunda-feira"),
        tr("Terça-feira"),
        tr("Quarta-feira"),
        tr("Quinta-feira"),
        tr("Sexta-feira"),
        tr("Sábado"),
        tr("Domingo")
    };

    for (int i = 0; i < 7; ++i) {
        int day_num = i + 1; // 1 = Mon ... 7 = Sun
        chk_days_[i] = new QCheckBox(day_names[i], group_days);
        chk_days_[i]->setChecked(cal.working_days.contains(day_num));
        days_layout->addWidget(chk_days_[i]);
    }
    main_layout->addWidget(group_days);

    // Group: Atalhos / Predefinições
    auto *group_presets = new QGroupBox(tr("Seleção Rápida"), this);
    auto *preset_layout = new QHBoxLayout(group_presets);
    preset_layout->setSpacing(8);

    auto *btn_mon_fri = new QPushButton(tr("Segunda a Sexta"), group_presets);
    btn_mon_fri->setToolTip(tr("Padrão comercial: Seg a Sex"));
    connect(btn_mon_fri, &QPushButton::clicked, this, &GanttWorkingDaysDialog::on_preset_mon_fri);
    preset_layout->addWidget(btn_mon_fri);

    auto *btn_mon_sat = new QPushButton(tr("Segunda a Sábado"), group_presets);
    btn_mon_sat->setToolTip(tr("Regime de obras e comércio: Seg a Sáb"));
    connect(btn_mon_sat, &QPushButton::clicked, this, &GanttWorkingDaysDialog::on_preset_mon_sat);
    preset_layout->addWidget(btn_mon_sat);

    auto *btn_all = new QPushButton(tr("Todos os Dias (7d)"), group_presets);
    btn_all->setToolTip(tr("Operação contínua 7 dias por semana"));
    connect(btn_all, &QPushButton::clicked, this, &GanttWorkingDaysDialog::on_preset_all_days);
    preset_layout->addWidget(btn_all);

    main_layout->addWidget(group_presets);

    // Group: Carga Horária Diária
    auto *group_hours = new QGroupBox(tr("Jornada Diária de Trabalho"), this);
    auto *form_hours = new QFormLayout(group_hours);
    spin_hours_ = new QDoubleSpinBox(group_hours);
    spin_hours_->setRange(1.0, 24.0);
    spin_hours_->setSingleStep(0.5);
    spin_hours_->setDecimals(1);
    spin_hours_->setSuffix(tr(" horas/dia"));
    spin_hours_->setValue(cal.hours_per_day > 0.0 ? cal.hours_per_day : 8.0);
    form_hours->addRow(tr("Horas Úteis Diárias:"), spin_hours_);
    main_layout->addWidget(group_hours);

    // Dialog Button Box
    auto *button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, &GanttWorkingDaysDialog::on_validate_and_accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    main_layout->addWidget(button_box);
}

void GanttWorkingDaysDialog::on_preset_mon_fri()
{
    for (int i = 0; i < 5; ++i) chk_days_[i]->setChecked(true);
    chk_days_[5]->setChecked(false);
    chk_days_[6]->setChecked(false);
}

void GanttWorkingDaysDialog::on_preset_mon_sat()
{
    for (int i = 0; i < 6; ++i) chk_days_[i]->setChecked(true);
    chk_days_[6]->setChecked(false);
}

void GanttWorkingDaysDialog::on_preset_all_days()
{
    for (int i = 0; i < 7; ++i) chk_days_[i]->setChecked(true);
}

void GanttWorkingDaysDialog::on_validate_and_accept()
{
    if (selected_working_days().isEmpty()) {
        QMessageBox::warning(this, tr("Aviso"), tr("Selecione ao menos um dia útil para o cronograma."));
        return;
    }
    accept();
}

QSet<int> GanttWorkingDaysDialog::selected_working_days() const
{
    QSet<int> days;
    for (int i = 0; i < 7; ++i) {
        if (chk_days_[i] && chk_days_[i]->isChecked()) {
            days.insert(i + 1); // 1 = Seg ... 7 = Dom
        }
    }
    return days;
}

double GanttWorkingDaysDialog::selected_hours_per_day() const
{
    return spin_hours_ ? spin_hours_->value() : 8.0;
}

} // namespace source::gantt
