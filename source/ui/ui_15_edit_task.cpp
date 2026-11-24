#include "source/ui/ui_15_edit_task.hpp"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QTime>
#include <QTimeEdit>
#include <QVBoxLayout>

#include "source/ui/agregador_01_ui.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"

namespace source::GerenciamentoUI {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

}  // namespace

EditDateTimeDialog::EditDateTimeDialog(source::EisenhowerMatrixApp *app,
                                       const QString &current_date_iso,
                                       const QString &current_time_str,
                                       QWidget *parent)
    : QDialog(parent != nullptr ? parent : app), app_(app)
{
    setModal(true);
    setWindowTitle(get_text(QStringLiteral("Redefinir data da tarefa")));

    auto *layout = new QVBoxLayout(this);
    date_enabled_ = new QCheckBox(get_text(QStringLiteral("Vincular data")), this);
    date_enabled_->setChecked(!current_date_iso.isEmpty());
    layout->addWidget(date_enabled_);

    date_edit_ = new QDateEdit(this);
    date_edit_->setCalendarPopup(true);
    date_edit_->setDisplayFormat(app_->date_input->displayFormat());
    const QDate current_date = QDate::fromString(current_date_iso, Qt::ISODate);
    date_edit_->setDate(current_date.isValid() ? current_date : QDate::currentDate());
    layout->addWidget(date_edit_);

    time_enabled_ = new QCheckBox(get_text(QStringLiteral("Vincular horário")), this);
    time_enabled_->setChecked(!current_date_iso.isEmpty() && !current_time_str.isEmpty());
    layout->addWidget(time_enabled_);

    time_edit_ = new QTimeEdit(this);
    time_edit_->setDisplayFormat(QStringLiteral("HH:mm"));
    const QTime current_time = QTime::fromString(current_time_str, QStringLiteral("HH:mm"));
    if (current_time.isValid()) {
        time_edit_->setTime(current_time);
    }
    layout->addWidget(time_edit_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    if (app_->gerenciador_traducao != nullptr) {
        app_->gerenciador_traducao->traduzir_botoes_padrao(buttons);
    }
    layout->addWidget(buttons);

    auto sync_enabled = [this]() {
        const bool date_on = date_enabled_->isChecked();
        date_edit_->setEnabled(date_on);
        time_enabled_->setEnabled(date_on);
        if (!date_on) {
            time_enabled_->setChecked(false);
        }
        time_edit_->setEnabled(date_on && time_enabled_->isChecked());
    };

    connect(date_enabled_, &QCheckBox::toggled, this, [sync_enabled]() { sync_enabled(); });
    connect(time_enabled_, &QCheckBox::toggled, this, [sync_enabled]() { sync_enabled(); });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    sync_enabled();
}

QPair<QString, QString> EditDateTimeDialog::values() const
{
    if (!date_enabled_->isChecked()) {
        return {QString(), QString()};
    }

    const QString date_iso = date_edit_->date().toString(Qt::ISODate);
    const QString time_str =
        time_enabled_->isChecked() ? time_edit_->time().toString(QStringLiteral("HH:mm")) : QString();
    return {date_iso, time_str};
}

}  // namespace source::GerenciamentoUI

namespace source::GerenciamentoUI::ui_15_edit_task {

void edit_task_datetime(source::EisenhowerMatrixApp *app, QListWidgetItem *item, QListWidget *list_widget)
{
    source::GerenciamentoUI::edit_task_datetime(app, item, list_widget);
}

void move_task_to_quadrant(source::EisenhowerMatrixApp *app,
                           QListWidgetItem *item,
                           QListWidget *source_list,
                           QListWidget *target_list)
{
    source::GerenciamentoUI::move_task_to_quadrant(app, item, source_list, target_list);
}

}  // namespace source::GerenciamentoUI::ui_15_edit_task
