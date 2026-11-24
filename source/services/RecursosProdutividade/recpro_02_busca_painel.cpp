#include "source/services/RecursosProdutividade/recpro_02_busca_painel.hpp"
#include "source/services/RecursosProdutividade/recpro_01_utilitarios_compartilhados.hpp"

namespace source::InterfaceCore::ProductivityFeatures {

void show_global_search(source::EisenhowerMatrixApp *app)
{
    show_model_dialog(app,
                      get_text(QStringLiteral("Busca global")),
                      collect_global_search_entries(app),
                      true);
}

void show_today_panel(source::EisenhowerMatrixApp *app)
{
    const QDate today = QDate::currentDate();
    const QDate horizon = today.addDays(7);
    QVariantList entries;
    for (const QVariant &value : records_for_app(app)) {
        const QVariantMap record = value.toMap();
        const QDate date = QDate::fromString(date_for_record(record), Qt::ISODate);
        if (!date.isValid() || date < today || date > horizon) {
            continue;
        }
        const QString title = title_for_record(record);
        const QString detail = QStringLiteral("%1%2%3")
                                   .arg(date.toString(QStringLiteral("dd/MM/yyyy")),
                                        time_for_record(record).isEmpty() ? QString() : QStringLiteral(" "),
                                        time_for_record(record));
        entries.append(display_entry(record.value(QStringLiteral("modulo")).toString(),
                                     title,
                                     detail,
                                     searchable_text(record),
                                     record));
    }
    show_model_dialog(app, get_text(QStringLiteral("Hoje / Próximos 7 dias")), entries, false);
}

void configure_recurrence_for_item(source::EisenhowerMatrixApp *app, QListWidgetItem *item)
{
    if (app == nullptr || item == nullptr) {
        return;
    }

    QDialog dialog(app);
    dialog.setWindowTitle(get_text(QStringLiteral("Recorrência")));
    auto *layout = new QFormLayout(&dialog);
    auto *combo = new QComboBox(&dialog);
    combo->addItem(get_text(QStringLiteral("Sem recorrência")), QString());
    combo->addItem(get_text(QStringLiteral("Diária")), QStringLiteral("daily"));
    combo->addItem(get_text(QStringLiteral("Semanal")), QStringLiteral("weekly"));
    combo->addItem(get_text(QStringLiteral("Mensal")), QStringLiteral("monthly"));

    auto *until_enabled = new QCheckBox(get_text(QStringLiteral("Definir data final")), &dialog);
    auto *until = new QDateEdit(QDate::currentDate().addMonths(3), &dialog);
    until->setCalendarPopup(true);
    until->setEnabled(false);
    QObject::connect(until_enabled, &QCheckBox::toggled, until, &QDateEdit::setEnabled);

    QVariantMap data = item->data(Qt::UserRole).toMap();
    QString current = data.value(QStringLiteral("recurrence")).toString().trimmed();
    if (current.isEmpty()) {
        current = data.value(QStringLiteral("recorrencia")).toString().trimmed();
    }
    QString until_str = data.value(QStringLiteral("recurrence_until")).toString().trimmed();
    if (until_str.isEmpty()) {
        until_str = data.value(QStringLiteral("recorrencia_ate")).toString().trimmed();
    }

    const QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
    if (current.isEmpty() && !task_id.isEmpty()) {
        const QVariantList unified = records_for_app(app);
        for (const QVariant &var : unified) {
            const QVariantMap rec = var.toMap();
            if (rec.value(QStringLiteral("id")).toString().trimmed() == task_id) {
                current = recurrence_for_record(rec);
                if (until_str.isEmpty()) {
                    until_str = recurrence_until_for_record(rec);
                }
                break;
            }
        }
    }

    const int idx = combo->findData(current);
    combo->setCurrentIndex(idx >= 0 ? idx : 0);
    const QDate current_until = QDate::fromString(until_str, Qt::ISODate);
    if (current_until.isValid()) {
        until_enabled->setChecked(true);
        until->setDate(current_until);
    }

    layout->addRow(get_text(QStringLiteral("Frequência")), combo);
    layout->addRow(until_enabled, until);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    translate_standard_buttons(app, buttons);
    layout->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString chosen_recurrence = combo->currentData().toString();
    const QString chosen_until = until_enabled->isChecked() ? until->date().toString(Qt::ISODate) : QString();
    data.insert(QStringLiteral("recurrence"), chosen_recurrence);
    data.insert(QStringLiteral("recorrencia"), chosen_recurrence);
    data.insert(QStringLiteral("recurrence_until"), chosen_until);
    data.insert(QStringLiteral("recorrencia_ate"), chosen_until);
    item->setData(Qt::UserRole, data);
    app->save_tasks_now();

    if (!task_id.isEmpty()) {
        QVariantList records = source::utils::UnifiedTaskStore::load_records(app->tasks_path);
        bool found = false;
        for (int i = 0; i < records.size(); ++i) {
            QVariantMap rec = records.at(i).toMap();
            if (rec.value(QStringLiteral("id")).toString().trimmed() == task_id) {
                rec.insert(QStringLiteral("recurrence"), chosen_recurrence);
                rec.insert(QStringLiteral("recorrencia"), chosen_recurrence);
                rec.insert(QStringLiteral("recurrence_until"), chosen_until);
                rec.insert(QStringLiteral("recorrencia_ate"), chosen_until);
                records[i] = rec;
                found = true;
                break;
            }
        }
        if (found) {
            source::utils::UnifiedTaskStore::save_records(app->tasks_path, records);
        }
    }
}


}  // namespace source::InterfaceCore::ProductivityFeatures
