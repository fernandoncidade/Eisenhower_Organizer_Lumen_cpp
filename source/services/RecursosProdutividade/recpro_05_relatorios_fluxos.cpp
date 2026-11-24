#include "source/services/RecursosProdutividade/recpro_05_relatorios_fluxos.hpp"
#include "source/services/RecursosProdutividade/recpro_01_utilitarios_compartilhados.hpp"
#include "source/services/incore_13_prioridade_display.hpp"

namespace source::InterfaceCore::ProductivityFeatures {

void export_calendar_ics(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }
    const QString path = QFileDialog::getSaveFileName(app,
                                                      get_text(QStringLiteral("Exportar calendário")),
                                                      QDir::home().filePath(QStringLiteral("tarefas.ics")),
                                                      get_text(QStringLiteral("iCalendar (*.ics)")));
    if (path.isEmpty()) {
        return;
    }

    QStringList lines = {
        QStringLiteral("BEGIN:VCALENDAR"),
        QStringLiteral("VERSION:2.0"),
        QStringLiteral("PRODID:-//Lumen Eisenhower Organizer//Tasks//PT-BR"),
        QStringLiteral("CALSCALE:GREGORIAN"),
    };

    const QString dtstamp = QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd'T'HHmmss'Z'"));
    for (const QVariant &value : records_for_app(app)) {
        const QVariantMap record = value.toMap();
        const QDate date = QDate::fromString(date_for_record(record), Qt::ISODate);
        if (!date.isValid()) {
            continue;
        }
        const QTime time = parse_time(time_for_record(record));
        QString uid = record.value(QStringLiteral("id")).toString();
        if (uid.trimmed().isEmpty()) {
            uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }
        lines << QStringLiteral("BEGIN:VEVENT");
        lines << QStringLiteral("UID:%1@lumen-eisenhower").arg(uid);
        lines << QStringLiteral("DTSTAMP:%1").arg(dtstamp);
        if (time.isValid()) {
            lines << QStringLiteral("DTSTART:%1").arg(QDateTime(date, time).toString(QStringLiteral("yyyyMMdd'T'HHmmss")));
            lines << QStringLiteral("DURATION:PT30M");
        } else {
            lines << QStringLiteral("DTSTART;VALUE=DATE:%1").arg(date.toString(QStringLiteral("yyyyMMdd")));
        }
        lines << QStringLiteral("SUMMARY:%1").arg(ics_escape(title_for_record(record)));
        const QString description = record.value(QStringLiteral("descricao")).toString();
        if (!description.trimmed().isEmpty()) {
            lines << QStringLiteral("DESCRIPTION:%1").arg(ics_escape(description));
        }
        lines << QStringLiteral("END:VEVENT");
    }
    lines << QStringLiteral("END:VCALENDAR");

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(app, get_text(QStringLiteral("Erro")), file.errorString());
        return;
    }
    const QByteArray payload = lines.join(QStringLiteral("\r\n")).toUtf8() + QByteArray("\r\n");
    if (file.write(payload) != payload.size() || !file.commit()) {
        QMessageBox::warning(app, get_text(QStringLiteral("Erro")), file.errorString());
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void show_productivity_history(source::EisenhowerMatrixApp *app)
{
    QMap<QString, int> completed_by_week;
    int pomodoros = 0;
    int feynman_reviews = 0;
    for (const QVariant &value : records_for_app(app)) {
        const QVariantMap record = value.toMap();
        if (record.value(QStringLiteral("concluida")).toBool()) {
            QDate date = QDateTime::fromString(record.value(QStringLiteral("completed_at")).toString(), Qt::ISODate).date();
            if (!date.isValid()) {
                date = QDate::fromString(date_for_record(record), Qt::ISODate);
            }
            if (!date.isValid()) {
                date = QDate::currentDate();
            }
            completed_by_week[QStringLiteral("%1-W%2").arg(date.year()).arg(date.weekNumber(), 2, 10, QLatin1Char('0'))]++;
        }
        pomodoros += record.value(QStringLiteral("pomodoros")).toInt();
        if (record.value(QStringLiteral("modulo")).toString() == QStringLiteral("feynman")
            && !record.value(QStringLiteral("revisao")).toString().trimmed().isEmpty()) {
            ++feynman_reviews;
        }
    }

    const int map_nodes = read_json_object(mapamental_backup_path()).value(QStringLiteral("nodes")).toArray().size();
    const int reading_minutes = current_audiolivro_text(app).split(QRegularExpression(QStringLiteral("\\s+")),
                                                                   Qt::SkipEmptyParts)
                                    .size()
                                / 180;

    QStringList lines;
    lines << get_text(QStringLiteral("Histórico de produtividade"));
    lines << QString();
    for (auto it = completed_by_week.constBegin(); it != completed_by_week.constEnd(); ++it) {
        lines << get_text(QStringLiteral("%1: %2 tarefa(s) concluída(s)")).arg(it.key()).arg(it.value());
    }
    lines << QString();
    lines << get_text(QStringLiteral("Pomodoros: %1")).arg(pomodoros);
    lines << get_text(QStringLiteral("Conceitos revisados: %1")).arg(feynman_reviews);
    lines << get_text(QStringLiteral("Nós no Mapa Mental: %1")).arg(map_nodes);
    lines << get_text(QStringLiteral("Leitura estimada aberta: %1 min")).arg(reading_minutes);

    QMessageBox::information(app, get_text(QStringLiteral("Histórico")), lines.join(QLatin1Char('\n')));
}

void run_study_flow(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    QString text = current_audiolivro_text(app);

    RuntimeTranslatedDialog dialog(app);
    dialog.setWindowTitle(get_text(QStringLiteral("Fluxo integrado de estudo")));
    dialog.resize(560, 420);
    auto *layout = new QVBoxLayout(&dialog);

    auto *lbl_text = new QLabel(&dialog);
    auto *text_edit = new QPlainTextEdit(&dialog);
    text_edit->setPlainText(text);
    text_edit->setPlaceholderText(get_text(QStringLiteral("Digite ou cole o trecho do conteúdo para estudo...")));
    layout->addWidget(lbl_text);
    layout->addWidget(text_edit, 1);

    auto *form = new QFormLayout();
    auto *lbl_priority = new QLabel(&dialog);
    auto *priority_combo = new QComboBox(&dialog);
    priority_combo->addItem(source::InterfaceCore::prioridade_para_texto(1, app), 1);
    priority_combo->addItem(source::InterfaceCore::prioridade_para_texto(2, app), 2);
    priority_combo->addItem(source::InterfaceCore::prioridade_para_texto(3, app), 3);
    priority_combo->addItem(source::InterfaceCore::prioridade_para_texto(4, app), 4);
    priority_combo->setCurrentIndex(1);

    auto *lbl_domain = new QLabel(&dialog);
    auto *domain_combo = new QComboBox(&dialog);
    domain_combo->addItem(source::InterfaceCore::dominio_para_texto(0, false), 0);
    domain_combo->addItem(source::InterfaceCore::dominio_para_texto(1, false), 1);
    domain_combo->addItem(source::InterfaceCore::dominio_para_texto(2, false), 2);
    domain_combo->setCurrentIndex(0);

    form->addRow(lbl_priority, priority_combo);
    form->addRow(lbl_domain, domain_combo);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    translate_standard_buttons(app, buttons);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    const auto retranslate_ui = [&]() {
        dialog.setWindowTitle(get_text(QStringLiteral("Fluxo integrado de estudo")));
        lbl_text->setText(get_text(QStringLiteral("Trecho para transformar em estudo:")));
        text_edit->setPlaceholderText(get_text(QStringLiteral("Digite ou cole o trecho do conteúdo para estudo...")));
        lbl_priority->setText(get_text(QStringLiteral("Prioridade (1 a 4):")));
        priority_combo->setItemText(0, source::InterfaceCore::prioridade_para_texto(1, app));
        priority_combo->setItemText(1, source::InterfaceCore::prioridade_para_texto(2, app));
        priority_combo->setItemText(2, source::InterfaceCore::prioridade_para_texto(3, app));
        priority_combo->setItemText(3, source::InterfaceCore::prioridade_para_texto(4, app));
        lbl_domain->setText(get_text(QStringLiteral("Domínio (0 a 2):")));
        domain_combo->setItemText(0, source::InterfaceCore::dominio_para_texto(0, false));
        domain_combo->setItemText(1, source::InterfaceCore::dominio_para_texto(1, false));
        domain_combo->setItemText(2, source::InterfaceCore::dominio_para_texto(2, false));
        translate_standard_buttons(app, buttons);
    };
    dialog.set_retranslate_handler([&]() { retranslate_ui(); });
    retranslate_ui();

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    text = text_edit->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }

    const int selected_priority = priority_combo->currentData().toInt();
    const int selected_domain = domain_combo->currentData().toInt();

    QString title = text.section(QRegularExpression(QStringLiteral("[\\r\\n\\.]")), 0, 0).simplified().left(90);
    if (title.isEmpty()) {
        title = get_text(QStringLiteral("Novo conceito"));
    }

    QVariantList additions;
    const QString concept_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    additions.append(source::utils::UnifiedTaskStore::normalize_record({
        {QStringLiteral("id"), concept_id},
        {QStringLiteral("modulo"), QStringLiteral("feynman")},
        {QStringLiteral("tipo"), QStringLiteral("conceito")},
        {QStringLiteral("titulo"), title},
        {QStringLiteral("explicacao"), text.left(4000)},
        {QStringLiteral("descricao_customizada"), true},
        {QStringLiteral("prioridade"), selected_priority},
        {QStringLiteral("dominio"), selected_domain},
        {QStringLiteral("categoria"), QStringLiteral("conceito")},
    }));
    QVariantMap review =
        eisenhower_task_record(get_text(QStringLiteral("Revisar: %1")).arg(title), QDate::currentDate().addDays(2), selected_priority, text.left(1000));
    review.insert(QStringLiteral("integracao_origem"), QStringLiteral("study_flow"));
    review.insert(QStringLiteral("integracao_task_id"), concept_id);
    review.insert(QStringLiteral("dominio"), selected_domain);
    additions.append(source::utils::UnifiedTaskStore::normalize_record(review));
    append_records_and_reload(app, additions);

    QJsonObject map = read_json_object(mapamental_backup_path());
    QJsonArray nodes = map.value(QStringLiteral("nodes")).toArray();
    QJsonObject node;
    node.insert(QStringLiteral("texto"), title);
    node.insert(QStringLiteral("x"), 80 + nodes.size() * 24);
    node.insert(QStringLiteral("y"), 80 + nodes.size() * 16);
    node.insert(QStringLiteral("cor"), QStringLiteral("#ff0078d4"));
    node.insert(QStringLiteral("notas"), text.left(2000));
    nodes.append(node);
    map.insert(QStringLiteral("version"), 2);
    map.insert(QStringLiteral("saved_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    map.insert(QStringLiteral("nodes"), nodes);
    if (!map.contains(QStringLiteral("connections"))) {
        map.insert(QStringLiteral("connections"), QJsonArray{});
    }
    write_json_object(mapamental_backup_path(), map);

    QMessageBox::information(app,
                             get_text(QStringLiteral("Fluxo de estudo")),
                             get_text(QStringLiteral("Conceito, nó de mapa mental e tarefa de revisão criados.")));
}

void show_templates_dialog(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    RuntimeTranslatedDialog dialog(app);
    dialog.setWindowTitle(get_text(QStringLiteral("Templates")));
    dialog.resize(520, 420);
    auto *layout = new QVBoxLayout(&dialog);

    auto *lbl_templates = new QLabel(&dialog);
    auto *list = new QListWidget(&dialog);
    layout->addWidget(lbl_templates);
    layout->addWidget(list);

    auto *form = new QFormLayout();
    auto *lbl_priority = new QLabel(&dialog);
    auto *priority_combo = new QComboBox(&dialog);
    priority_combo->addItem(source::InterfaceCore::prioridade_para_texto(1, app), 1);
    priority_combo->addItem(source::InterfaceCore::prioridade_para_texto(2, app), 2);
    priority_combo->addItem(source::InterfaceCore::prioridade_para_texto(3, app), 3);
    priority_combo->addItem(source::InterfaceCore::prioridade_para_texto(4, app), 4);
    priority_combo->setCurrentIndex(1);

    auto *lbl_domain = new QLabel(&dialog);
    auto *domain_combo = new QComboBox(&dialog);
    domain_combo->addItem(source::InterfaceCore::dominio_para_texto(0, false), 0);
    domain_combo->addItem(source::InterfaceCore::dominio_para_texto(1, false), 1);
    domain_combo->addItem(source::InterfaceCore::dominio_para_texto(2, false), 2);
    domain_combo->setCurrentIndex(0);

    form->addRow(lbl_priority, priority_combo);
    form->addRow(lbl_domain, domain_combo);
    layout->addLayout(form);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    translate_standard_buttons(app, buttons);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    const auto retranslate_ui = [&]() {
        dialog.setWindowTitle(get_text(QStringLiteral("Templates")));
        lbl_templates->setText(get_text(QStringLiteral("Escolha um modelo de tarefas:")));
        const int current_row = list->currentRow();
        list->clear();
        list->addItems({
            get_text(QStringLiteral("Estudo")),
            get_text(QStringLiteral("Trabalho")),
            get_text(QStringLiteral("Revisão")),
            get_text(QStringLiteral("Leitura")),
            get_text(QStringLiteral("Planejamento semanal")),
        });
        if (current_row >= 0 && current_row < list->count()) {
            list->setCurrentRow(current_row);
        } else {
            list->setCurrentRow(0);
        }
        lbl_priority->setText(get_text(QStringLiteral("Prioridade (1 a 4):")));
        priority_combo->setItemText(0, source::InterfaceCore::prioridade_para_texto(1, app));
        priority_combo->setItemText(1, source::InterfaceCore::prioridade_para_texto(2, app));
        priority_combo->setItemText(2, source::InterfaceCore::prioridade_para_texto(3, app));
        priority_combo->setItemText(3, source::InterfaceCore::prioridade_para_texto(4, app));
        lbl_domain->setText(get_text(QStringLiteral("Domínio (0 a 2):")));
        domain_combo->setItemText(0, source::InterfaceCore::dominio_para_texto(0, false));
        domain_combo->setItemText(1, source::InterfaceCore::dominio_para_texto(1, false));
        domain_combo->setItemText(2, source::InterfaceCore::dominio_para_texto(2, false));
        translate_standard_buttons(app, buttons);
    };
    dialog.set_retranslate_handler([&]() { retranslate_ui(); });
    retranslate_ui();

    if (dialog.exec() != QDialog::Accepted || list->currentRow() < 0) {
        return;
    }

    const int selected_priority = priority_combo->currentData().toInt();
    const int selected_domain = domain_combo->currentData().toInt();
    const QDate today = QDate::currentDate();
    QVariantList additions;
    switch (list->currentRow()) {
    case 0:
        additions << eisenhower_task_record(get_text(QStringLiteral("Definir objetivo de estudo")), today, selected_priority)
                  << eisenhower_task_record(get_text(QStringLiteral("Estudar bloco principal")), today, selected_priority)
                  << eisenhower_task_record(get_text(QStringLiteral("Criar resumo Feynman")), today.addDays(1), selected_priority);
        break;
    case 1:
        additions << eisenhower_task_record(get_text(QStringLiteral("Priorizar entregas do dia")), today, selected_priority)
                  << eisenhower_task_record(get_text(QStringLiteral("Responder pendências urgentes")), today, selected_priority)
                  << eisenhower_task_record(get_text(QStringLiteral("Planejar próxima entrega")), today.addDays(1), selected_priority);
        break;
    case 2:
        additions << eisenhower_task_record(get_text(QStringLiteral("Revisar pontos fracos")), today, selected_priority)
                  << eisenhower_task_record(get_text(QStringLiteral("Resolver exercício de fixação")), today.addDays(1), selected_priority);
        break;
    case 3:
        additions << eisenhower_task_record(get_text(QStringLiteral("Ler capítulo selecionado")), today, selected_priority)
                  << eisenhower_task_record(get_text(QStringLiteral("Extrair conceitos principais")), today, selected_priority);
        break;
    default:
        for (int i = 0; i < 7; ++i) {
            additions << eisenhower_task_record(get_text(QStringLiteral("Planejamento do dia %1")).arg(i + 1),
                                                today.addDays(i),
                                                selected_priority);
        }
        break;
    }

    for (int i = 0; i < additions.size(); ++i) {
        QVariantMap record = additions.at(i).toMap();
        record.insert(QStringLiteral("dominio"), selected_domain);
        additions[i] = record;
    }

    append_records_and_reload(app, additions);
}


}  // namespace source::InterfaceCore::ProductivityFeatures
