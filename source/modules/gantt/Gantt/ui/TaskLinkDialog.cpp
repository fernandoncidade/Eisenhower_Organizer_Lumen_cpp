#include "TaskLinkDialog.hpp"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFormLayout>

namespace source::gantt {

TaskLinkDialog::TaskLinkDialog(const QString &eisenhower_task_id,
                               const QString &eisenhower_task_title,
                               const Schedule &schedule,
                               QWidget *parent)
    : QDialog(parent),
      task_id_(eisenhower_task_id),
      task_title_(eisenhower_task_title),
      schedule_(schedule)
{
    setWindowTitle(tr("Vincular Tarefa ao Cronograma de Gantt"));
    resize(480, 220);
    setup_ui();
}

void TaskLinkDialog::setup_ui()
{
    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    auto *lbl_task = new QLabel(QStringLiteral("<b>%1</b>").arg(task_title_), this);
    form->addRow(tr("Tarefa no Organizador:"), lbl_task);

    combo_activities_ = new QComboBox(this);
    combo_activities_->addItem(tr("-- Nenhuma (Desvincular) --"), QString());

    QList<ScheduleItem> items = schedule_.get_sorted_by_wbs();
    int current_index = 0;
    int idx = 1;

    for (const auto &item : items) {
        QString label = QStringLiteral("[%1] %2 (%3d)").arg(item.wbs_code, item.name).arg(item.duration_hours / 8.0, 0, 'f', 1);
        combo_activities_->addItem(label, item.id);

        if (item.task_id == task_id_) {
            current_index = idx;
        }
        idx++;
    }

    combo_activities_->setCurrentIndex(current_index);
    form->addRow(tr("Atividade no Gantt:"), combo_activities_);

    layout->addLayout(form);

    auto *desc = new QLabel(tr("Ao vincular, o avanço ou conclusão da tarefa no Organizador atualizará automaticamente o percentual e o status físico da atividade no cronograma da obra."), this);
    desc->setWordWrap(true);
    desc->setStyleSheet(QStringLiteral("color: #555; font-size: 11px;"));
    layout->addWidget(desc);

    auto *btn_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    if (auto *btn_ok = btn_box->button(QDialogButtonBox::Ok)) btn_ok->setText(tr("Vincular"));
    if (auto *btn_cancel = btn_box->button(QDialogButtonBox::Cancel)) btn_cancel->setText(tr("Cancelar"));
    connect(btn_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(btn_box);
}

QString TaskLinkDialog::selected_activity_id() const
{
    return combo_activities_->currentData().toString();
}

} // namespace source::gantt
