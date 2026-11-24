#include "CsvMappingDialog.hpp"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace source::gantt {

CsvMappingDialog::CsvMappingDialog(const QStringList &preview_headers, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Mapeamento de Colunas CSV"));
    resize(420, 360);
    setup_ui(preview_headers);
}

void CsvMappingDialog::setup_ui(const QStringList &preview_headers)
{
    auto *layout = new QVBoxLayout(this);

    if (!preview_headers.isEmpty()) {
        auto *lbl_headers = new QLabel(tr("<b>Colunas detectadas no arquivo:</b><br>%1").arg(preview_headers.join(QStringLiteral(" | "))), this);
        lbl_headers->setWordWrap(true);
        layout->addWidget(lbl_headers);
    }

    auto *form = new QFormLayout();

    auto create_spin = [this](int default_val) {
        auto *s = new QSpinBox(this);
        s->setRange(0, 50);
        s->setValue(default_val);
        return s;
    };

    spin_wbs_ = create_spin(0);
    spin_name_ = create_spin(1);
    spin_start_ = create_spin(2);
    spin_end_ = create_spin(3);
    spin_duration_ = create_spin(4);
    spin_progress_ = create_spin(5);
    spin_preds_ = create_spin(6);
    spin_bim_ = create_spin(7);
    spin_milestone_ = create_spin(8);

    form->addRow(tr("Coluna Código WBS (0-based):"), spin_wbs_);
    form->addRow(tr("Coluna Nome da Atividade:"), spin_name_);
    form->addRow(tr("Coluna Data de Início:"), spin_start_);
    form->addRow(tr("Coluna Data de Término:"), spin_end_);
    form->addRow(tr("Coluna Duração (dias):"), spin_duration_);
    form->addRow(tr("Coluna % Progresso Físico:"), spin_progress_);
    form->addRow(tr("Coluna Predecessoras (ex: 1FS):"), spin_preds_);
    form->addRow(tr("Coluna GUID BIM (IFC):"), spin_bim_);
    form->addRow(tr("Coluna Marco/Milestone:"), spin_milestone_);

    layout->addLayout(form);

    auto *btn_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    if (auto *btn_ok = btn_box->button(QDialogButtonBox::Ok)) btn_ok->setText(tr("Confirmar"));
    if (auto *btn_cancel = btn_box->button(QDialogButtonBox::Cancel)) btn_cancel->setText(tr("Cancelar"));
    connect(btn_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(btn_box);
}

CsvColumnMapping CsvMappingDialog::mapping() const
{
    CsvColumnMapping m;
    m.wbs_col = spin_wbs_->value();
    m.name_col = spin_name_->value();
    m.start_date_col = spin_start_->value();
    m.end_date_col = spin_end_->value();
    m.duration_col = spin_duration_->value();
    m.progress_col = spin_progress_->value();
    m.predecessors_col = spin_preds_->value();
    m.bim_guid_col = spin_bim_->value();
    m.is_milestone_col = spin_milestone_->value();
    return m;
}

} // namespace source::gantt
