#include "EisenhowerTaskImportDialog.hpp"
#include <QHeaderView>
#include <QMessageBox>
#include <QEvent>
#include <QCoreApplication>
#include <QLocale>
#include "../application/services/GanttTaskIntegrationBridge.hpp"

namespace source::gantt {

EisenhowerTaskImportDialog::EisenhowerTaskImportDialog(QWidget *parent)
    : QDialog(parent)
{
    resize(720, 480);
    setup_ui();
    load_tasks();
}

void EisenhowerTaskImportDialog::setup_ui()
{
    auto *layout = new QVBoxLayout(this);

    lbl_info_ = new QLabel(this);
    lbl_info_->setWordWrap(true);
    lbl_info_->setStyleSheet(QStringLiteral("color: #444; font-size: 12px; margin-bottom: 6px;"));
    layout->addWidget(lbl_info_);

    // Filters row
    auto *filter_layout = new QHBoxLayout();
    txt_search_ = new QLineEdit(this);
    txt_search_->setClearButtonEnabled(true);
    connect(txt_search_, &QLineEdit::textChanged, this, &EisenhowerTaskImportDialog::on_filter_changed);
    filter_layout->addWidget(txt_search_, 2);

    combo_quadrant_ = new QComboBox(this);
    connect(combo_quadrant_, &QComboBox::currentIndexChanged, this, &EisenhowerTaskImportDialog::on_filter_changed);
    filter_layout->addWidget(combo_quadrant_, 1);

    layout->addLayout(filter_layout);

    // Table
    table_ = new QTableWidget(this);
    table_->setColumnCount(5);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table_->setAlternatingRowColors(true);
    layout->addWidget(table_);

    // Bottom action bar
    auto *bottom_layout = new QHBoxLayout();
    btn_select_all_ = new QPushButton(this);
    connect(btn_select_all_, &QPushButton::clicked, this, &EisenhowerTaskImportDialog::on_select_all);
    bottom_layout->addWidget(btn_select_all_);

    lbl_status_ = new QLabel(this);
    bottom_layout->addWidget(lbl_status_, 1);

    btn_import_ = new QPushButton(this);
    btn_import_->setStyleSheet(QStringLiteral("background-color: #2e7d32; color: white; font-weight: bold; padding: 6px 14px; border-radius: 4px;"));
    connect(btn_import_, &QPushButton::clicked, this, &EisenhowerTaskImportDialog::on_import_clicked);
    bottom_layout->addWidget(btn_import_);

    btn_close_ = new QPushButton(this);
    connect(btn_close_, &QPushButton::clicked, this, &QDialog::reject);
    bottom_layout->addWidget(btn_close_);

    layout->addLayout(bottom_layout);

    retranslate_ui();
}

void EisenhowerTaskImportDialog::retranslate_ui()
{
    QString title = tr("Importar Tarefas Operacionais do Organizador");
    if (title == QStringLiteral("Importar Tarefas Operacionais do Organizador")) {
        const QString fallback = QCoreApplication::translate("EisenhowerTaskImportDialog", "Importar Tarefas Operacionais do Organizador");
        if (!fallback.isEmpty() && fallback != title) {
            title = fallback;
        }
    }
    if (title == QStringLiteral("Importar Tarefas Operacionais do Organizador")) {
        const QString lang = QLocale().name();
        if (lang.startsWith(QLatin1String("en"), Qt::CaseInsensitive)) {
            title = QStringLiteral("Import Operational Tasks from Organizer");
        }
    }
    setWindowTitle(title);

    if (lbl_info_) {
        lbl_info_->setText(tr("Selecione as tarefas operacionais de campo para gerar automaticamente novas atividades no cronograma de obra."));
    }

    if (txt_search_) {
        txt_search_->setPlaceholderText(tr("🔍 Filtrar por título..."));
    }

    if (combo_quadrant_) {
        const int current_val = combo_quadrant_->currentData().toInt();
        combo_quadrant_->blockSignals(true);
        combo_quadrant_->clear();
        combo_quadrant_->addItem(tr("Todos os Quadrantes"), 0);
        combo_quadrant_->addItem(tr("Q1: Urgente & Importante"), 1);
        combo_quadrant_->addItem(tr("Q2: Planejamento"), 2);
        combo_quadrant_->addItem(tr("Q3: Delegação"), 3);
        combo_quadrant_->addItem(tr("Q4: Eliminação"), 4);
        int idx = combo_quadrant_->findData(current_val);
        if (idx >= 0) combo_quadrant_->setCurrentIndex(idx);
        combo_quadrant_->blockSignals(false);
    }

    if (table_) {
        table_->setHorizontalHeaderLabels({
            tr("Quadrante"),
            tr("Título"),
            tr("Data / Horário"),
            tr("Status Tarefa"),
            tr("Vínculo Atual")
        });
    }

    if (btn_select_all_) {
        btn_select_all_->setText(tr("Selecionar Todos"));
    }

    if (btn_import_) {
        btn_import_->setText(tr("➕ Importar Selecionadas para o Cronograma"));
    }

    if (btn_close_) {
        btn_close_->setText(tr("Fechar"));
    }

    if (table_) {
        if (!all_tasks_.isEmpty()) {
            populate_table();
        } else if (lbl_status_) {
            lbl_status_->setText(tr("%1 tarefa(s) listada(s) (%2 disponíveis para vincular)").arg(0).arg(0));
        }
    }
}

void EisenhowerTaskImportDialog::changeEvent(QEvent *event)
{
    if (event && event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    QDialog::changeEvent(event);
}

void EisenhowerTaskImportDialog::load_tasks()
{
    all_tasks_ = GanttTaskIntegrationBridge::instance().get_available_eisenhower_tasks();
    populate_table();
}

void EisenhowerTaskImportDialog::populate_table()
{
    table_->setRowCount(0);

    const QString search = txt_search_->text().trimmed().toLower();
    const int filter_quadrant = combo_quadrant_->currentData().toInt();

    int available_count = 0;

    for (const auto &var : all_tasks_) {
        const QVariantMap task = var.toMap();
        const QString task_id = task.value(QStringLiteral("id")).toString().trimmed();
        QString title = task.value(QStringLiteral("text")).toString().trimmed();
        if (title.isEmpty()) title = task.value(QStringLiteral("titulo")).toString().trimmed();
        if (title.isEmpty()) title = task.value(QStringLiteral("title")).toString().trimmed();

        if (title.isEmpty()) continue;

        int quadrant = 0;
        if (task.contains(QStringLiteral("priority")) && task.value(QStringLiteral("priority")).toInt() > 0) {
            quadrant = task.value(QStringLiteral("priority")).toInt();
        } else if (task.contains(QStringLiteral("prioridade")) && task.value(QStringLiteral("prioridade")).toInt() > 0) {
            quadrant = task.value(QStringLiteral("prioridade")).toInt();
        } else if (task.contains(QStringLiteral("quadrante")) && task.value(QStringLiteral("quadrante")).toInt() > 0) {
            quadrant = task.value(QStringLiteral("quadrante")).toInt();
        }
        if (quadrant < 1 || quadrant > 4) quadrant = 1;

        if (filter_quadrant > 0 && quadrant != filter_quadrant) {
            continue;
        }

        if (!search.isEmpty() && !title.toLower().contains(search)) {
            continue;
        }

        QString date = task.value(QStringLiteral("date")).toString().trimmed();
        if (date.isEmpty()) date = task.value(QStringLiteral("data")).toString().trimmed();

        QString time = task.value(QStringLiteral("time")).toString().trimmed();
        if (time.isEmpty()) time = task.value(QStringLiteral("hora")).toString().trimmed();
        if (time.isEmpty()) time = task.value(QStringLiteral("horario")).toString().trimmed();

        QString datetime_str = date;
        if (!time.isEmpty()) datetime_str += QStringLiteral(" ") + time;
        if (datetime_str.isEmpty()) datetime_str = QStringLiteral("-");

        bool done = false;
        if (task.contains(QStringLiteral("concluida"))) done = task.value(QStringLiteral("concluida")).toBool();
        else if (task.contains(QStringLiteral("completed"))) done = task.value(QStringLiteral("completed")).toBool();

        const QString status_str = task.value(QStringLiteral("status")).toString().toLower();
        const bool in_prog = !done && (
            task.value(QStringLiteral("in_progress")).toBool() ||
            task.value(QStringLiteral("em_progresso")).toBool() ||
            task.value(QStringLiteral("em_andamento")).toBool() ||
            status_str == QStringLiteral("doing") ||
            status_str.contains(QStringLiteral("progresso")) ||
            status_str.contains(QStringLiteral("andamento")) ||
            status_str.contains(QStringLiteral("progress"))
        );

        QString out_act_name;
        const bool is_linked = GanttTaskIntegrationBridge::instance().is_task_linked(task_id, nullptr, nullptr, nullptr, &out_act_name);

        const int row = table_->rowCount();
        table_->insertRow(row);

        // Store task_id in row
        auto *item_q = new QTableWidgetItem(QStringLiteral("Q%1").arg(quadrant));
        item_q->setData(Qt::UserRole, task_id);
        item_q->setTextAlignment(Qt::AlignCenter);
        table_->setItem(row, 0, item_q);

        auto *item_title = new QTableWidgetItem(title);
        table_->setItem(row, 1, item_title);

        auto *item_dt = new QTableWidgetItem(datetime_str);
        item_dt->setTextAlignment(Qt::AlignCenter);
        table_->setItem(row, 2, item_dt);

        auto *item_status = new QTableWidgetItem(done ? tr("Concluída") : (in_prog ? tr("Em Andamento") : tr("Pendente")));
        item_status->setTextAlignment(Qt::AlignCenter);
        if (done) {
            item_status->setForeground(QBrush(QColor(46, 125, 50))); // Dark green
        } else if (in_prog) {
            item_status->setForeground(QBrush(QColor(33, 150, 243))); // Blue
        }
        table_->setItem(row, 3, item_status);

        auto *item_link = new QTableWidgetItem(is_linked ? tr("🔗 Vinculada (%1)").arg(out_act_name) : tr("Disponível"));
        item_link->setTextAlignment(Qt::AlignCenter);
        if (is_linked) {
            item_link->setForeground(QBrush(QColor(25, 118, 210)));
        } else {
            item_link->setForeground(QBrush(QColor(100, 100, 100)));
            available_count++;
        }
        table_->setItem(row, 4, item_link);
    }

    lbl_status_->setText(tr("%1 tarefa(s) listada(s) (%2 disponíveis para vincular)")
                             .arg(table_->rowCount())
                             .arg(available_count));
}

void EisenhowerTaskImportDialog::on_filter_changed()
{
    populate_table();
}

void EisenhowerTaskImportDialog::on_select_all()
{
    table_->selectAll();
}

void EisenhowerTaskImportDialog::on_import_clicked()
{
    const QList<QTableWidgetItem *> selected = table_->selectedItems();
    QSet<int> selected_rows;
    for (auto *it : selected) {
        selected_rows.insert(it->row());
    }

    if (selected_rows.isEmpty()) {
        QMessageBox::information(this, tr("Importar Tarefas"),
                                 tr("Nenhuma tarefa selecionada. Selecione ao menos uma linha na tabela."));
        return;
    }

    int imported = 0;
    auto &bridge = GanttTaskIntegrationBridge::instance();

    for (int row : selected_rows) {
        auto *q_item = table_->item(row, 0);
        if (!q_item) continue;
        const QString task_id = q_item->data(Qt::UserRole).toString();

        QVariantMap task = bridge.get_task_by_id(task_id);
        if (task.isEmpty()) {
            task.insert(QStringLiteral("id"), task_id);
            task.insert(QStringLiteral("text"), table_->item(row, 1)->text());
        }

        QString error;
        if (bridge.convert_eisenhower_task_to_gantt_activity(task, nullptr, &error)) {
            imported++;
        }
    }

    imported_count_ += imported;
    populate_table();

    QMessageBox::information(this, tr("Importação Concluída"),
                             tr("%1 tarefa(s) foram convertidas com sucesso em atividades no cronograma de Gantt!").arg(imported));
    accept();
}

} // namespace source::gantt
