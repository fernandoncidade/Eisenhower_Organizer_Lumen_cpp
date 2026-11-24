#include "source/services/RecursosProdutividade/recpro_06_revisoes_plugins.hpp"
#include "source/services/RecursosProdutividade/recpro_01_utilitarios_compartilhados.hpp"

#include <QFileSystemWatcher>
#include <QFont>
#include <QFontMetrics>
#include <QScrollBar>
#include <QSplitter>

namespace source::InterfaceCore::ProductivityFeatures {
namespace {

enum LogTableColumn {
    LogTableFile = 0,
    LogTableOrigin,
    LogTableSize,
    LogTableModified,
    LogTableRecords,
    LogTablePath,
    LogTableColumnCount
};

struct PluginManagerRuntime {
    QPointer<RuntimeTranslatedDialog> dialog;
    source::EisenhowerMatrixApp *app = nullptr;
    QTableWidget *table = nullptr;
    QPushButton *open_folder = nullptr;
    QPushButton *close = nullptr;
};

struct LogMonitorEntry {
    QDateTime timestamp;
    QString path;
    QString line;
    qint64 sequence = 0;
};

struct LogMonitorRuntime {
    QString log_dir;
    QPointer<QDialog> dialog;
    QTableWidget *table = nullptr;
    QPlainTextEdit *monitor = nullptr;
    QLabel *monitor_label = nullptr;
    QLabel *summary = nullptr;
    QCheckBox *auto_scroll = nullptr;
    QPushButton *open_log = nullptr;
    QPushButton *open_folder = nullptr;
    QPushButton *refresh = nullptr;
    QPushButton *close = nullptr;
    QFileSystemWatcher *watcher = nullptr;
    QTimer *timer = nullptr;
    QMap<QString, qint64> offsets;
    QMap<QString, QByteArray> partial_lines;
    QSet<QString> known_paths;
    QVector<LogMonitorEntry> entries;
    qint64 sequence = 0;
    bool scanning = false;
};

void configure_runtime_dialog(source::EisenhowerMatrixApp *app, RuntimeTranslatedDialog *dialog, const QSize &size)
{
    if (dialog == nullptr) {
        return;
    }
    dialog->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint
                           | Qt::WindowCloseButtonHint);
    dialog->setWindowModality(Qt::NonModal);
    dialog->setModal(false);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    if (app != nullptr && !app->windowIcon().isNull()) {
        dialog->setWindowIcon(app->windowIcon());
    } else if (!QApplication::windowIcon().isNull()) {
        dialog->setWindowIcon(QApplication::windowIcon());
    }
    dialog->resize(size);
}

void plugin_manager_set_headers(QTableWidget *table)
{
    if (table == nullptr) {
        return;
    }
    table->setHorizontalHeaderLabels({get_text(QStringLiteral("Plugin")),
                                      get_text(QStringLiteral("Status")),
                                      get_text(QStringLiteral("Versão")),
                                      get_text(QStringLiteral("Caminho")),
                                      get_text(QStringLiteral("Erro"))});
}

QString plugin_manager_selected_key(QTableWidget *table)
{
    if (table == nullptr || table->currentRow() < 0) {
        return {};
    }
    QTableWidgetItem *item = table->item(table->currentRow(), 0);
    return item != nullptr ? item->data(Qt::UserRole).toString() : QString();
}

void plugin_manager_update_table(const std::shared_ptr<PluginManagerRuntime> &runtime)
{
    if (!runtime || runtime->app == nullptr || runtime->table == nullptr) {
        return;
    }

    const QString selected_key = plugin_manager_selected_key(runtime->table);
    const QVariantList report = runtime->app->plugin_status_report();
    runtime->table->setRowCount(report.size());
    for (int row = 0; row < report.size(); ++row) {
        const QVariantMap item = report.at(row).toMap();
        auto *name_item = new QTableWidgetItem(item.value(QStringLiteral("name")).toString());
        name_item->setData(Qt::UserRole, item.value(QStringLiteral("key")).toString());
        runtime->table->setItem(row, 0, name_item);
        runtime->table->setItem(row, 1, new QTableWidgetItem(item.value(QStringLiteral("status")).toString()));
        runtime->table->setItem(row, 2, new QTableWidgetItem(item.value(QStringLiteral("version")).toString()));
        runtime->table->setItem(row, 3, new QTableWidgetItem(item.value(QStringLiteral("path")).toString()));
        runtime->table->setItem(row, 4, new QTableWidgetItem(item.value(QStringLiteral("error")).toString()));
        if (!selected_key.isEmpty() && item.value(QStringLiteral("key")).toString() == selected_key) {
            runtime->table->selectRow(row);
        }
    }

    if (runtime->table->currentRow() < 0 && report.size() > 0) {
        runtime->table->selectRow(0);
    }
    runtime->table->resizeColumnsToContents();
    if (runtime->table->horizontalHeader() != nullptr) {
        runtime->table->horizontalHeader()->setStretchLastSection(true);
    }
}

void plugin_manager_retranslate(const std::shared_ptr<PluginManagerRuntime> &runtime)
{
    if (!runtime || runtime->dialog.isNull()) {
        return;
    }
    runtime->dialog->setWindowTitle(get_text(QStringLiteral("Gerenciador de plugins")));
    plugin_manager_set_headers(runtime->table);
    if (runtime->open_folder != nullptr) {
        runtime->open_folder->setText(get_text(QStringLiteral("Abrir pasta")));
    }
    if (runtime->close != nullptr) {
        runtime->close->setText(get_text(QStringLiteral("Fechar")));
    }
    plugin_manager_update_table(runtime);
}

QString log_manager_log_dir()
{
    const QString current_log = source::utils::LogManager::get_log_file();
    QString directory = current_log.isEmpty() ? QString() : QFileInfo(current_log).absolutePath();
    if (directory.isEmpty()) {
        directory = QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("logs"));
    }
    QDir().mkpath(directory);
    return directory;
}

QFileInfoList log_manager_log_files(const QString &directory)
{
    QDir dir(directory);
    return dir.entryInfoList(QStringList() << QStringLiteral("*.log"),
                             QDir::Files | QDir::Readable | QDir::NoSymLinks,
                             QDir::Time);
}

QDateTime log_manager_timestamp_from_file_name(const QString &file_name)
{
    const QRegularExpression expression(QStringLiteral("(\\d{8}_\\d{6})"));
    const QRegularExpressionMatch match = expression.match(file_name);
    if (!match.hasMatch()) {
        return {};
    }
    return QDateTime::fromString(match.captured(1), QStringLiteral("yyyyMMdd_hhmmss"));
}

QDateTime log_manager_timestamp_for_line(const QString &line, const QFileInfo &file_info)
{
    const QString trimmed = line.trimmed();
    if (trimmed.size() >= 23) {
        QDateTime timestamp = QDateTime::fromString(trimmed.left(23), QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"));
        if (timestamp.isValid()) {
            return timestamp;
        }
        timestamp = QDateTime::fromString(trimmed.left(23), QStringLiteral("yyyy-MM-dd'T'hh:mm:ss.zzz"));
        if (timestamp.isValid()) {
            return timestamp;
        }
    }

    const QString first_token = trimmed.section(QLatin1Char(' '), 0, 0);
    QDateTime timestamp = QDateTime::fromString(first_token, Qt::ISODateWithMs);
    if (timestamp.isValid()) {
        return timestamp;
    }
    timestamp = QDateTime::fromString(first_token, Qt::ISODate);
    if (timestamp.isValid()) {
        return timestamp;
    }

    timestamp = log_manager_timestamp_from_file_name(file_info.fileName());
    if (timestamp.isValid()) {
        return timestamp;
    }
    return file_info.lastModified();
}

QString log_manager_origin_for_file(const QString &file_name)
{
    const QString lower = file_name.toLower();
    if (lower.startsWith(QStringLiteral("file_agenda_"))) {
        return get_text(QStringLiteral("Agenda"));
    }
    if (lower.contains(QStringLiteral("startup"))) {
        return get_text(QStringLiteral("Inicialização"));
    }
    if (lower.startsWith(QStringLiteral("file_lumen_")) || lower.startsWith(QStringLiteral("file_eisenhower_"))) {
        return get_text(QStringLiteral("Aplicativo"));
    }
    return get_text(QStringLiteral("Log"));
}

QString log_manager_human_size(qint64 bytes)
{
    static const QStringList units = {QStringLiteral("B"),
                                      QStringLiteral("KB"),
                                      QStringLiteral("MB"),
                                      QStringLiteral("GB")};
    double size = static_cast<double>(bytes);
    int unit = 0;
    while (size >= 1024.0 && unit < units.size() - 1) {
        size /= 1024.0;
        ++unit;
    }
    return QStringLiteral("%1 %2").arg(size, 0, unit == 0 ? 'f' : 'f', unit == 0 ? 0 : 1).arg(units.at(unit));
}

QString log_manager_selected_path(QTableWidget *table)
{
    if (table == nullptr || table->currentRow() < 0) {
        return {};
    }
    QTableWidgetItem *item = table->item(table->currentRow(), LogTableFile);
    if (item == nullptr) {
        return {};
    }
    return item->data(Qt::UserRole).toString();
}

void log_manager_set_headers(QTableWidget *table)
{
    if (table == nullptr) {
        return;
    }
    table->setHorizontalHeaderLabels({get_text(QStringLiteral("Arquivo")),
                                      get_text(QStringLiteral("Origem")),
                                      get_text(QStringLiteral("Tamanho")),
                                      get_text(QStringLiteral("Modificado")),
                                      get_text(QStringLiteral("Registros")),
                                      get_text(QStringLiteral("Caminho"))});
}

void log_manager_remove_entries_for_path(const std::shared_ptr<LogMonitorRuntime> &runtime, const QString &path)
{
    if (!runtime) {
        return;
    }
    runtime->entries.erase(std::remove_if(runtime->entries.begin(),
                                          runtime->entries.end(),
                                          [&path](const LogMonitorEntry &entry) {
                                              return entry.path == path;
                                          }),
                           runtime->entries.end());
}

void log_manager_sort_entries(const std::shared_ptr<LogMonitorRuntime> &runtime)
{
    if (!runtime) {
        return;
    }
    std::stable_sort(runtime->entries.begin(), runtime->entries.end(), [](const LogMonitorEntry &left,
                                                                           const LogMonitorEntry &right) {
        if (left.timestamp != right.timestamp) {
            return left.timestamp < right.timestamp;
        }
        return left.sequence < right.sequence;
    });
}

void log_manager_render_monitor(const std::shared_ptr<LogMonitorRuntime> &runtime)
{
    if (!runtime || runtime->monitor == nullptr) {
        return;
    }

    const bool should_scroll = runtime->auto_scroll == nullptr || runtime->auto_scroll->isChecked();
    QScrollBar *scroll_bar = runtime->monitor->verticalScrollBar();
    const int previous_scroll = scroll_bar != nullptr ? scroll_bar->value() : 0;

    QStringList lines;
    lines.reserve(runtime->entries.size());
    for (const LogMonitorEntry &entry : std::as_const(runtime->entries)) {
        lines << QStringLiteral("[%1] %2").arg(QFileInfo(entry.path).fileName(), entry.line);
    }
    runtime->monitor->setPlainText(lines.join(QLatin1Char('\n')));

    if (scroll_bar != nullptr) {
        if (should_scroll) {
            scroll_bar->setValue(scroll_bar->maximum());
        } else {
            scroll_bar->setValue(qMin(previous_scroll, scroll_bar->maximum()));
        }
    }
}

void log_manager_update_summary(const std::shared_ptr<LogMonitorRuntime> &runtime, int file_count)
{
    if (!runtime || runtime->summary == nullptr) {
        return;
    }
    runtime->summary->setText(
        get_text(QStringLiteral("Pasta de logs: %1\nArquivos: %2 | Registros monitorados: %3"))
            .arg(QDir::toNativeSeparators(runtime->log_dir))
            .arg(file_count)
            .arg(runtime->entries.size()));
}

void log_manager_scan(const std::shared_ptr<LogMonitorRuntime> &runtime, bool force_table_refresh);

void log_manager_retranslate(const std::shared_ptr<LogMonitorRuntime> &runtime)
{
    if (!runtime || runtime->dialog.isNull()) {
        return;
    }

    runtime->dialog->setWindowTitle(get_text(QStringLiteral("Gerenciador de logs")));
    log_manager_set_headers(runtime->table);
    if (runtime->monitor_label != nullptr) {
        runtime->monitor_label->setText(get_text(QStringLiteral("Monitor de logs em tempo real")));
    }
    if (runtime->monitor != nullptr) {
        runtime->monitor->setPlaceholderText(get_text(QStringLiteral("Nenhum registro de log encontrado.")));
    }
    if (runtime->open_log != nullptr) {
        runtime->open_log->setText(get_text(QStringLiteral("Abrir log")));
    }
    if (runtime->open_folder != nullptr) {
        runtime->open_folder->setText(get_text(QStringLiteral("Abrir pasta")));
    }
    if (runtime->refresh != nullptr) {
        runtime->refresh->setText(get_text(QStringLiteral("Atualizar")));
    }
    if (runtime->auto_scroll != nullptr) {
        runtime->auto_scroll->setText(get_text(QStringLiteral("Auto-rolagem")));
    }
    if (runtime->close != nullptr) {
        runtime->close->setText(get_text(QStringLiteral("Fechar")));
    }
    log_manager_scan(runtime, true);
}

void log_manager_update_table(const std::shared_ptr<LogMonitorRuntime> &runtime, const QFileInfoList &files)
{
    if (!runtime || runtime->table == nullptr) {
        return;
    }

    const QString selected_path = log_manager_selected_path(runtime->table);
    QHash<QString, int> record_counts;
    for (const LogMonitorEntry &entry : std::as_const(runtime->entries)) {
        ++record_counts[entry.path];
    }

    runtime->table->setRowCount(files.size());
    for (int row = 0; row < files.size(); ++row) {
        const QFileInfo &info = files.at(row);
        const QString path = info.absoluteFilePath();

        auto *file_item = new QTableWidgetItem(info.fileName());
        file_item->setData(Qt::UserRole, path);
        runtime->table->setItem(row, LogTableFile, file_item);
        runtime->table->setItem(row, LogTableOrigin, new QTableWidgetItem(log_manager_origin_for_file(info.fileName())));
        runtime->table->setItem(row, LogTableSize, new QTableWidgetItem(log_manager_human_size(info.size())));
        runtime->table->setItem(row,
                                LogTableModified,
                                new QTableWidgetItem(info.lastModified().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))));
        runtime->table->setItem(row,
                                LogTableRecords,
                                new QTableWidgetItem(QString::number(record_counts.value(path))));
        runtime->table->setItem(row, LogTablePath, new QTableWidgetItem(QDir::toNativeSeparators(path)));

        if (path == selected_path) {
            runtime->table->selectRow(row);
        }
    }

    if (runtime->table->currentRow() < 0 && files.size() > 0) {
        runtime->table->selectRow(0);
    }
    runtime->table->resizeColumnsToContents();
    if (runtime->table->horizontalHeader() != nullptr) {
        runtime->table->horizontalHeader()->setStretchLastSection(true);
    }
}

bool log_manager_read_new_lines(const std::shared_ptr<LogMonitorRuntime> &runtime,
                                const QFileInfo &file_info,
                                bool read_from_start)
{
    if (!runtime) {
        return false;
    }

    const QString path = file_info.absoluteFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    qint64 offset = read_from_start ? 0 : runtime->offsets.value(path, 0);
    if (offset > file.size()) {
        log_manager_remove_entries_for_path(runtime, path);
        runtime->partial_lines.remove(path);
        offset = 0;
    }
    if (!file.seek(offset)) {
        return false;
    }

    QByteArray data = file.readAll();
    runtime->offsets.insert(path, file.pos());
    if (data.isEmpty()) {
        return false;
    }

    if (!read_from_start && runtime->partial_lines.contains(path)) {
        data.prepend(runtime->partial_lines.take(path));
    }

    const bool complete_last_line = data.endsWith('\n') || data.endsWith('\r');
    QList<QByteArray> chunks = data.split('\n');
    if (!complete_last_line && !chunks.isEmpty()) {
        runtime->partial_lines.insert(path, chunks.takeLast());
    } else {
        runtime->partial_lines.remove(path);
        if (!chunks.isEmpty() && chunks.last().isEmpty()) {
            chunks.removeLast();
        }
    }

    bool added = false;
    for (QByteArray chunk : std::as_const(chunks)) {
        if (chunk.endsWith('\r')) {
            chunk.chop(1);
        }
        const QString line = QString::fromUtf8(chunk).trimmed();
        if (line.isEmpty()) {
            continue;
        }

        LogMonitorEntry entry;
        entry.timestamp = log_manager_timestamp_for_line(line, file_info);
        entry.path = path;
        entry.line = line;
        entry.sequence = ++runtime->sequence;
        runtime->entries.append(entry);
        added = true;
    }
    return added;
}

void log_manager_sync_watchers(const std::shared_ptr<LogMonitorRuntime> &runtime, const QFileInfoList &files)
{
    if (!runtime || runtime->watcher == nullptr) {
        return;
    }

    const QStringList watched_dirs = runtime->watcher->directories();
    if (!watched_dirs.contains(runtime->log_dir)) {
        runtime->watcher->addPath(runtime->log_dir);
    }

    QSet<QString> desired_files;
    for (const QFileInfo &file : files) {
        desired_files.insert(file.absoluteFilePath());
    }

    for (const QString &path : runtime->watcher->files()) {
        if (!desired_files.contains(path)) {
            runtime->watcher->removePath(path);
        }
    }
    const QStringList watched_files = runtime->watcher->files();
    for (const QString &path : std::as_const(desired_files)) {
        if (!watched_files.contains(path) && QFileInfo::exists(path)) {
            runtime->watcher->addPath(path);
        }
    }
}

void log_manager_scan(const std::shared_ptr<LogMonitorRuntime> &runtime, bool force_table_refresh)
{
    if (!runtime || runtime->scanning || runtime->dialog.isNull()) {
        return;
    }

    runtime->scanning = true;
    const QFileInfoList files = log_manager_log_files(runtime->log_dir);
    QSet<QString> current_paths;
    for (const QFileInfo &file : files) {
        current_paths.insert(file.absoluteFilePath());
    }

    const bool files_changed = current_paths != runtime->known_paths;
    if (files_changed) {
        for (const QString &path : std::as_const(runtime->known_paths)) {
            if (!current_paths.contains(path)) {
                runtime->offsets.remove(path);
                runtime->partial_lines.remove(path);
                log_manager_remove_entries_for_path(runtime, path);
            }
        }
        runtime->known_paths = current_paths;
    }

    bool added = false;
    for (const QFileInfo &file : files) {
        const QString path = file.absoluteFilePath();
        const bool first_read = !runtime->offsets.contains(path);
        added = log_manager_read_new_lines(runtime, file, first_read) || added;
    }

    if (added || files_changed) {
        log_manager_sort_entries(runtime);
        log_manager_render_monitor(runtime);
    }

    if (force_table_refresh || added || files_changed) {
        log_manager_update_table(runtime, files);
    }
    log_manager_update_summary(runtime, files.size());
    log_manager_sync_watchers(runtime, files);
    runtime->scanning = false;
}

}  // namespace

void create_spaced_reviews(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    const QString source_module = active_module_key(app);
    const QVariantList unified_records = records_for_app(app);
    const QJsonObject map_backup = read_json_object(mapamental_backup_path());
    const QVariantList source_records = source_module == QStringLiteral("mapamental")
                                            ? mapamental_records_from_backup(map_backup)
                                            : records_for_module(unified_records, source_module);

    if (source_records.isEmpty()) {
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Revisão espaçada")),
                                 get_text(QStringLiteral("Nenhum item encontrado na aba ativa.")));
        return;
    }

    const auto description_for_record = [](const QVariantMap &record) {
        QString description = record.value(QStringLiteral("descricao")).toString().trimmed();
        if (description.isEmpty()) {
            description = record.value(QStringLiteral("description")).toString().trimmed();
        }
        if (description.isEmpty()) {
            description = record.value(QStringLiteral("explicacao")).toString().trimmed();
        }
        if (description.isEmpty()) {
            description = record.value(QStringLiteral("revisao")).toString().trimmed();
        }
        return description;
    };

    RuntimeTranslatedDialog dialog(app);
    dialog.setWindowTitle(get_text(QStringLiteral("Revisão espaçada")));
    dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowMinMaxButtonsHint);
    dialog.setSizeGripEnabled(true);
    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *summary = new QLabel(&dialog);
    summary->setWordWrap(true);
    layout->addWidget(summary);

    auto *selection_group = new QGroupBox(get_text(QStringLiteral("Itens")), &dialog);
    auto *selection_layout = new QVBoxLayout(selection_group);
    selection_layout->setContentsMargins(10, 8, 10, 10);
    selection_layout->setSpacing(7);

    auto *all_items_radio = new QRadioButton(get_text(QStringLiteral("Todos filtrados")), selection_group);
    auto *selected_items_radio = new QRadioButton(get_text(QStringLiteral("Somente selecionados")), selection_group);
    all_items_radio->setChecked(true);

    auto *selection_mode_layout = new QHBoxLayout();
    selection_mode_layout->addWidget(all_items_radio);
    selection_mode_layout->addWidget(selected_items_radio);
    selection_mode_layout->addStretch(1);
    selection_layout->addLayout(selection_mode_layout);

    auto *item_filter = new QLineEdit(selection_group);
    item_filter->setPlaceholderText(get_text(QStringLiteral("Pesquisar item")));
    selection_layout->addWidget(item_filter);

    auto *item_model = new source::models::TaskRecordListModel(selection_group);
    auto *item_proxy = new QSortFilterProxyModel(selection_group);
    item_proxy->setSourceModel(item_model);
    item_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    item_proxy->setFilterRole(source::models::TaskRecordListModel::SearchableRole);

    auto *item_view = new QListView(selection_group);
    item_view->setModel(item_proxy);
    item_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    item_view->setUniformItemSizes(false);
    item_view->setAlternatingRowColors(true);
    item_view->setMinimumHeight(140);
    selection_layout->addWidget(item_view, 1);

    auto *selection_buttons_widget = new QWidget(selection_group);
    selection_buttons_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto *selection_buttons = new QHBoxLayout(selection_buttons_widget);
    selection_buttons->setContentsMargins(0, 6, 0, 0);
    selection_buttons->setSpacing(6);
    auto *select_visible_button = new QPushButton(get_text(QStringLiteral("Selecionar visíveis")), selection_buttons_widget);
    auto *clear_selection_button = new QPushButton(get_text(QStringLiteral("Limpar")), selection_buttons_widget);
    select_visible_button->setMaximumWidth(150);
    clear_selection_button->setMaximumWidth(90);
    selection_buttons->addWidget(select_visible_button);
    selection_buttons->addWidget(clear_selection_button);
    selection_buttons->addStretch(1);
    selection_layout->addWidget(selection_buttons_widget);

    selection_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    layout->addWidget(selection_group, 1);

    const auto adjust_combo_width = [](QComboBox *combo) {
        if (!combo) return;
        const QFontMetrics fm(combo->font());
        int max_width = 0;
        for (int i = 0; i < combo->count(); ++i) {
            max_width = std::max(max_width, fm.horizontalAdvance(combo->itemText(i)));
        }
        const int padding = 38;
        combo->setMinimumWidth(max_width + padding);
    };

    const auto adjust_spin_width = [](QSpinBox *spin) {
        if (!spin) return;
        const QFontMetrics fm(spin->font());
        const QString sample_text = QString::number(spin->maximum()) + QStringLiteral("   ");
        const int w = fm.horizontalAdvance(sample_text) + 36;
        spin->setMinimumWidth(std::max(spin->minimumWidth(), w));
    };

    auto *middle_row = new QHBoxLayout();
    middle_row->setSpacing(8);

    auto *domain_group = new QGroupBox(get_text(QStringLiteral("Intervalo por domínio")), &dialog);
    auto *domain_layout = new QFormLayout(domain_group);
    domain_layout->setContentsMargins(10, 10, 10, 10);
    domain_layout->setHorizontalSpacing(12);
    domain_layout->setVerticalSpacing(6);
    domain_layout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    domain_layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    auto *days_beginner = new QSpinBox(domain_group);
    auto *days_intermediate = new QSpinBox(domain_group);
    auto *days_advanced = new QSpinBox(domain_group);
    for (QSpinBox *spin : {days_beginner, days_intermediate, days_advanced}) {
        spin->setRange(1, 365);
    }
    days_beginner->setValue(1);
    days_intermediate->setValue(3);
    days_advanced->setValue(7);

    auto *lbl_beginner = new QLabel(get_text(QStringLiteral("🥉 Iniciante (dias)")), domain_group);
    auto *lbl_intermediate = new QLabel(get_text(QStringLiteral("🥈 Intermediário (dias)")), domain_group);
    auto *lbl_advanced = new QLabel(get_text(QStringLiteral("🥇 Avançado (dias)")), domain_group);
    domain_layout->addRow(lbl_beginner, days_beginner);
    domain_layout->addRow(lbl_intermediate, days_intermediate);
    domain_layout->addRow(lbl_advanced, days_advanced);

    auto *default_domain = new QComboBox(domain_group);
    default_domain->addItem(get_text(QStringLiteral("🥉 Iniciante")), 0);
    default_domain->addItem(get_text(QStringLiteral("🥈 Intermediário")), 1);
    default_domain->addItem(get_text(QStringLiteral("🥇 Avançado")), 2);
    default_domain->setCurrentIndex(0);
    auto *lbl_default_domain = new QLabel(get_text(QStringLiteral("Domínio padrão (quando ausente)")), domain_group);
    domain_layout->addRow(lbl_default_domain, default_domain);
    middle_row->addWidget(domain_group, 1);

    auto *priority_group = new QGroupBox(get_text(QStringLiteral("Prioridade da revisão")), &dialog);
    auto *priority_layout = new QFormLayout(priority_group);
    priority_layout->setContentsMargins(10, 10, 10, 10);
    priority_layout->setHorizontalSpacing(12);
    priority_layout->setVerticalSpacing(6);
    priority_layout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    priority_layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    auto *priority_mode = new QComboBox(priority_group);
    priority_mode->addItem(get_text(QStringLiteral("Usar prioridade de origem")), QVariant());
    priority_mode->addItem(get_text(QStringLiteral("🔴 Importante e Urgente")), 1);
    priority_mode->addItem(get_text(QStringLiteral("🟠 Importante, mas Não Urgente")), 2);
    priority_mode->addItem(get_text(QStringLiteral("🟡 Não Importante, mas Urgente")), 3);
    priority_mode->addItem(get_text(QStringLiteral("🟢 Não Importante e Não Urgente")), 4);
    auto *lbl_priority_mode = new QLabel(get_text(QStringLiteral("Modo")), priority_group);
    priority_layout->addRow(lbl_priority_mode, priority_mode);

    auto *default_priority = new QComboBox(priority_group);
    default_priority->addItem(get_text(QStringLiteral("🔴 Importante e Urgente")), 1);
    default_priority->addItem(get_text(QStringLiteral("🟠 Importante, mas Não Urgente")), 2);
    default_priority->addItem(get_text(QStringLiteral("🟡 Não Importante, mas Urgente")), 3);
    default_priority->addItem(get_text(QStringLiteral("🟢 Não Importante e Não Urgente")), 4);
    default_priority->setCurrentIndex(1);
    auto *lbl_default_priority = new QLabel(get_text(QStringLiteral("Prioridade padrão (quando ausente)")), priority_group);
    priority_layout->addRow(lbl_default_priority, default_priority);
    middle_row->addWidget(priority_group, 1);

    for (QSpinBox *spin : {days_beginner, days_intermediate, days_advanced}) {
        adjust_spin_width(spin);
    }
    adjust_combo_width(default_domain);
    adjust_combo_width(priority_mode);
    adjust_combo_width(default_priority);

    layout->addLayout(middle_row);

    auto *include_calendar = new QCheckBox(
        get_text(QStringLiteral("Definir datas de revisão (aparecer no calendário/painel)")), &dialog);
    include_calendar->setChecked(true);
    layout->addWidget(include_calendar);

    auto *include_completed =
        new QCheckBox(get_text(QStringLiteral("Incluir itens concluídos")), &dialog);
    include_completed->setChecked(false);
    layout->addWidget(include_completed);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    translate_standard_buttons(app, buttons);
    QPushButton *undo_button = nullptr;
    if (g_spaced_review_undo.has_value()) {
        undo_button = buttons->addButton(get_text(QStringLiteral("Desfazer última execução")),
                                         QDialogButtonBox::ResetRole);
        QObject::connect(undo_button, &QPushButton::clicked, &dialog, [app]() {
            show_spaced_review_undo_dialog(app);
        });
    }
    layout->addWidget(buttons);

    const auto selected_item_indices = [&]() -> QSet<int> {
        QSet<int> selected_indices;
        if (!selected_items_radio->isChecked() || item_view->selectionModel() == nullptr) {
            return selected_indices;
        }
        const QModelIndexList rows = item_view->selectionModel()->selectedRows();
        for (const QModelIndex &proxy_index : rows) {
            const QVariantMap entry =
                item_proxy->data(proxy_index, source::models::TaskRecordListModel::RecordRole).toMap();
            const int record_index = entry.value(QStringLiteral("record_index"), -1).toInt();
            if (record_index >= 0 && record_index < source_records.size()) {
                selected_indices.insert(record_index);
            }
        }
        return selected_indices;
    };

    const auto record_is_considered = [&](int index, const QSet<int> &selected_indices) -> bool {
        if (index < 0 || index >= source_records.size()) {
            return false;
        }
        const QVariantMap record = source_records.at(index).toMap();
        if (!include_completed->isChecked() && record_completed(record)) {
            return false;
        }
        if (record_is_spaced_review(record)) {
            return false;
        }
        const QString title = title_for_record(record);
        if (title.isEmpty()) {
            return false;
        }
        if (selected_items_radio->isChecked()) {
            return selected_indices.contains(index);
        }
        return true;
    };

    const auto update_summary = [&]() {
        const QSet<int> selected_indices = selected_item_indices();
        int considered_count = 0;
        QMap<int, int> domain_counts;
        QMap<int, int> priority_counts;
        int missing_domain = 0;
        int missing_priority = 0;

        for (int i = 0; i < source_records.size(); ++i) {
            if (!record_is_considered(i, selected_indices)) {
                continue;
            }
            ++considered_count;
            const QVariantMap record = source_records.at(i).toMap();

            const QVariant domain_raw = record.value(QStringLiteral("dominio"));
            const bool has_domain = domain_raw.isValid()
                                    && !(domain_raw.typeId() == QMetaType::QString
                                         && domain_raw.toString().trimmed().isEmpty());
            const int domain = normalized_domain_value(domain_raw, 0);
            if (has_domain) {
                domain_counts[domain] += 1;
            } else {
                ++missing_domain;
            }

            QVariant priority_raw = record.value(QStringLiteral("prioridade"));
            if (!priority_raw.isValid()
                || (priority_raw.typeId() == QMetaType::QString && priority_raw.toString().trimmed().isEmpty())) {
                priority_raw = record.value(QStringLiteral("quadrante"));
            }
            if (!priority_raw.isValid()
                || (priority_raw.typeId() == QMetaType::QString && priority_raw.toString().trimmed().isEmpty())) {
                priority_raw = record.value(QStringLiteral("priority"));
            }
            const bool has_priority = priority_raw.isValid()
                                      && !(priority_raw.typeId() == QMetaType::QString
                                           && priority_raw.toString().trimmed().isEmpty());
            const int priority = normalized_priority_value(priority_raw, 2);
            if (has_priority) {
                priority_counts[priority] += 1;
            } else {
                ++missing_priority;
            }
        }

        QStringList summary_lines;
        summary_lines << get_text(QStringLiteral("Fonte: %1")).arg(module_label(source_module));
        summary_lines << get_text(QStringLiteral("Itens encontrados: %1")).arg(considered_count);
        summary_lines
            << get_text(QStringLiteral("Domínio: %1 iniciante, %2 intermediário, %3 avançado%4"))
                   .arg(domain_counts.value(0))
                   .arg(domain_counts.value(1))
                   .arg(domain_counts.value(2))
                   .arg(missing_domain > 0 ? get_text(QStringLiteral(", %1 sem domínio")).arg(missing_domain)
                                           : QString());
        summary_lines
            << get_text(QStringLiteral("Prioridade: %1 🔴, %2 🟠, %3 🟡, %4 🟢%5"))
                   .arg(priority_counts.value(1))
                   .arg(priority_counts.value(2))
                   .arg(priority_counts.value(3))
                   .arg(priority_counts.value(4))
                   .arg(missing_priority > 0 ? get_text(QStringLiteral(", %1 sem prioridade")).arg(missing_priority)
                                             : QString());
        summary->setText(summary_lines.join(QLatin1Char('\n')));
    };

    const auto update_item_model = [&]() {
        const QSet<int> previously_selected = selected_item_indices();
        QVariantList entries;
        for (int i = 0; i < source_records.size(); ++i) {
            const QVariantMap record = source_records.at(i).toMap();
            if (!include_completed->isChecked() && record_completed(record)) {
                continue;
            }
            if (record_is_spaced_review(record)) {
                continue;
            }
            const QString title = title_for_record(record);
            if (title.isEmpty()) {
                continue;
            }

            const QVariant domain_raw = record.value(QStringLiteral("dominio"));
            const bool has_domain = domain_raw.isValid()
                                    && !(domain_raw.typeId() == QMetaType::QString
                                         && domain_raw.toString().trimmed().isEmpty());
            const int domain = has_domain ? normalized_domain_value(domain_raw, -1) : -1;
            QString dom_badge;
            if (domain == 0) {
                dom_badge = QStringLiteral("🥉");
            } else if (domain == 1) {
                dom_badge = QStringLiteral("🥈");
            } else if (domain == 2) {
                dom_badge = QStringLiteral("🥇");
            } else {
                dom_badge = QStringLiteral("⚪");
            }

            QVariant priority_raw = record.value(QStringLiteral("prioridade"));
            if (!priority_raw.isValid()
                || (priority_raw.typeId() == QMetaType::QString && priority_raw.toString().trimmed().isEmpty())) {
                priority_raw = record.value(QStringLiteral("quadrante"));
            }
            if (!priority_raw.isValid()
                || (priority_raw.typeId() == QMetaType::QString && priority_raw.toString().trimmed().isEmpty())) {
                priority_raw = record.value(QStringLiteral("priority"));
            }
            const bool has_priority = priority_raw.isValid()
                                      && !(priority_raw.typeId() == QMetaType::QString
                                           && priority_raw.toString().trimmed().isEmpty());
            const int priority = has_priority ? normalized_priority_value(priority_raw, 0) : 0;
            QString prio_badge;
            if (priority == 1) {
                prio_badge = QStringLiteral("🔴");
            } else if (priority == 2) {
                prio_badge = QStringLiteral("🟠");
            } else if (priority == 3) {
                prio_badge = QStringLiteral("🟡");
            } else if (priority == 4) {
                prio_badge = QStringLiteral("🟢");
            } else {
                prio_badge = QStringLiteral("⚪");
            }

            const QString date = date_for_record(record);
            const QString desc = description_for_record(record);

            QString display = QStringLiteral("[%1 %2] %3").arg(dom_badge, prio_badge, title);
            if (!date.isEmpty()) {
                display += QStringLiteral(" (%1)").arg(date);
            }
            if (record_completed(record)) {
                display += QStringLiteral(" [%1]").arg(get_text(QStringLiteral("concluída")));
            }

            QVariantMap entry;
            entry.insert(QStringLiteral("display"), display);
            entry.insert(QStringLiteral("searchable"), QStringLiteral("%1 %2 %3").arg(title, desc, date));
            entry.insert(QStringLiteral("record_index"), i);
            entries.append(entry);
        }

        item_model->set_records(entries);
        item_proxy->setFilterFixedString(item_filter->text());

        if (item_view->selectionModel() != nullptr && !previously_selected.isEmpty()) {
            QItemSelection restored_selection;
            for (int row = 0; row < item_model->rowCount(); ++row) {
                const QVariantMap entry = item_model->record_at(row);
                const int record_index = entry.value(QStringLiteral("record_index"), -1).toInt();
                if (!previously_selected.contains(record_index)) {
                    continue;
                }
                const QModelIndex source_index = item_model->index(row, 0);
                const QModelIndex proxy_index = item_proxy->mapFromSource(source_index);
                if (proxy_index.isValid()) {
                    restored_selection.select(proxy_index, proxy_index);
                }
            }
            const QSignalBlocker blocker(item_view->selectionModel());
            item_view->selectionModel()->clearSelection();
            item_view->selectionModel()->select(restored_selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        update_summary();
    };

    const auto update_selection_controls = [&]() {
        const bool enabled = selected_items_radio->isChecked();
        item_filter->setEnabled(enabled);
        item_view->setEnabled(enabled);
        select_visible_button->setEnabled(enabled);
        clear_selection_button->setEnabled(enabled);
        update_summary();
    };

    QObject::connect(item_filter, &QLineEdit::textChanged, item_proxy, [item_proxy, update_summary](const QString &text) {
        item_proxy->setFilterFixedString(text);
        update_summary();
    });
    QObject::connect(item_view->selectionModel(),
                     &QItemSelectionModel::selectionChanged,
                     &dialog,
                     [&](const QItemSelection &, const QItemSelection &) { update_summary(); });
    QObject::connect(all_items_radio, &QRadioButton::toggled, &dialog, [&](bool) { update_selection_controls(); });
    QObject::connect(selected_items_radio, &QRadioButton::toggled, &dialog, [&](bool) { update_selection_controls(); });
    QObject::connect(select_visible_button, &QPushButton::clicked, &dialog, [&]() {
        selected_items_radio->setChecked(true);
        if (item_view->selectionModel() == nullptr) {
            return;
        }
        for (int row = 0; row < item_proxy->rowCount(); ++row) {
            const QModelIndex index = item_proxy->index(row, 0);
            item_view->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        update_summary();
    });
    QObject::connect(clear_selection_button, &QPushButton::clicked, &dialog, [&]() {
        if (item_view->selectionModel() != nullptr) {
            item_view->selectionModel()->clearSelection();
        }
        update_summary();
    });
    QObject::connect(include_completed, &QCheckBox::toggled, &dialog, [&](bool) { update_item_model(); });

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (selected_items_radio->isChecked() && selected_item_indices().isEmpty()) {
            QMessageBox::warning(&dialog,
                                 get_text(QStringLiteral("Revisão espaçada")),
                                 get_text(QStringLiteral("Selecione pelo menos uma tarefa ou conceito.")));
            return;
        }
        dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    const auto retranslate_ui = [&]() {
        dialog.setWindowTitle(get_text(QStringLiteral("Revisão espaçada")));
        selection_group->setTitle(get_text(QStringLiteral("Itens")));
        all_items_radio->setText(get_text(QStringLiteral("Todos filtrados")));
        selected_items_radio->setText(get_text(QStringLiteral("Somente selecionados")));
        item_filter->setPlaceholderText(get_text(QStringLiteral("Pesquisar item")));
        select_visible_button->setText(get_text(QStringLiteral("Selecionar visíveis")));
        clear_selection_button->setText(get_text(QStringLiteral("Limpar")));

        domain_group->setTitle(get_text(QStringLiteral("Intervalo por domínio")));
        lbl_beginner->setText(get_text(QStringLiteral("🥉 Iniciante (dias)")));
        lbl_intermediate->setText(get_text(QStringLiteral("🥈 Intermediário (dias)")));
        lbl_advanced->setText(get_text(QStringLiteral("🥇 Avançado (dias)")));
        lbl_default_domain->setText(get_text(QStringLiteral("Domínio padrão (quando ausente)")));
        default_domain->setItemText(0, get_text(QStringLiteral("🥉 Iniciante")));
        default_domain->setItemText(1, get_text(QStringLiteral("🥈 Intermediário")));
        default_domain->setItemText(2, get_text(QStringLiteral("🥇 Avançado")));

        priority_group->setTitle(get_text(QStringLiteral("Prioridade da revisão")));
        lbl_priority_mode->setText(get_text(QStringLiteral("Modo")));
        priority_mode->setItemText(0, get_text(QStringLiteral("Usar prioridade de origem")));
        priority_mode->setItemText(1, get_text(QStringLiteral("🔴 Importante e Urgente")));
        priority_mode->setItemText(2, get_text(QStringLiteral("🟠 Importante, mas Não Urgente")));
        priority_mode->setItemText(3, get_text(QStringLiteral("🟡 Não Importante, mas Urgente")));
        priority_mode->setItemText(4, get_text(QStringLiteral("🟢 Não Importante e Não Urgente")));
        lbl_default_priority->setText(get_text(QStringLiteral("Prioridade padrão (quando ausente)")));
        default_priority->setItemText(0, get_text(QStringLiteral("🔴 Importante e Urgente")));
        default_priority->setItemText(1, get_text(QStringLiteral("🟠 Importante, mas Não Urgente")));
        default_priority->setItemText(2, get_text(QStringLiteral("🟡 Não Importante, mas Urgente")));
        default_priority->setItemText(3, get_text(QStringLiteral("🟢 Não Importante e Não Urgente")));

        include_calendar->setText(get_text(QStringLiteral("Definir datas de revisão (aparecer no calendário/painel)")));
        include_completed->setText(get_text(QStringLiteral("Incluir itens concluídos")));
        translate_standard_buttons(app, buttons);
        if (undo_button != nullptr) {
            undo_button->setText(get_text(QStringLiteral("Desfazer última execução")));
        }
        for (QSpinBox *spin : {days_beginner, days_intermediate, days_advanced}) {
            adjust_spin_width(spin);
        }
        adjust_combo_width(default_domain);
        adjust_combo_width(priority_mode);
        adjust_combo_width(default_priority);

        update_item_model();
        update_selection_controls();
    };

    dialog.set_retranslate_handler([&]() { retranslate_ui(); });

    update_item_model();
    update_selection_controls();
    dialog.setMinimumSize(780, 700);
    dialog.resize(820, 740);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const int fallback_domain = default_domain->currentData().toInt();
    const int fallback_priority = default_priority->currentData().toInt();
    const QVariant priority_mode_value = priority_mode->currentData();
    const bool force_priority = priority_mode_value.isValid();
    const int forced_priority = force_priority ? priority_mode_value.toInt() : fallback_priority;
    const bool calendar_enabled = include_calendar->isChecked();

    const QDate today = QDate::currentDate();
    QVector<SpacedReviewDraft> drafts;
    drafts.reserve(source_records.size());

    const QSet<int> selected_indices = selected_item_indices();

    for (int i = 0; i < source_records.size(); ++i) {
        if (!record_is_considered(i, selected_indices)) {
            continue;
        }
        const QVariantMap record = source_records.at(i).toMap();
        const QString title = title_for_record(record);
        if (title.isEmpty()) {
            continue;
        }

        const QVariant domain_raw = record.value(QStringLiteral("dominio"));
        const int domain = normalized_domain_value(domain_raw, fallback_domain);
        const int days = domain == 0 ? days_beginner->value() : (domain == 1 ? days_intermediate->value()
                                                                             : days_advanced->value());
        const QDate due = calendar_enabled ? today.addDays(days) : QDate();

        QVariant priority_raw = record.value(QStringLiteral("prioridade"));
        if (!priority_raw.isValid()
            || (priority_raw.typeId() == QMetaType::QString && priority_raw.toString().trimmed().isEmpty())) {
            priority_raw = record.value(QStringLiteral("quadrante"));
        }
        if (!priority_raw.isValid()
            || (priority_raw.typeId() == QMetaType::QString && priority_raw.toString().trimmed().isEmpty())) {
            priority_raw = record.value(QStringLiteral("priority"));
        }
        const int priority = force_priority
                                 ? forced_priority
                               : normalized_priority_value(priority_raw, fallback_priority);

        const QString source_id = record_source_id(record, source_module);
        SpacedReviewDraft draft;
        draft.title = get_text(QStringLiteral("Revisão espaçada: %1")).arg(title);
        draft.source_title = title;
        draft.description = description_for_record(record);
        draft.date_iso = due.isValid() ? due.toString(Qt::ISODate) : QString();
        draft.source_module = source_module;
        draft.source_id = source_id;
        draft.priority = priority;
        draft.domain = domain;
        draft.interval_days = days;
        draft.file_path = file_path_for_record(record);
        drafts.append(draft);
    }

    if (drafts.isEmpty()) {
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Revisão espaçada")),
                                 get_text(QStringLiteral("Nenhuma revisão nova foi gerada.")));
        return;
    }

    QSet<QString> default_targets = {QStringLiteral("eisenhower")};
    if (source_module == QStringLiteral("pomodoro")
        || source_module == QStringLiteral("feynman")
        || source_module == QStringLiteral("mapamental")) {
        default_targets.insert(source_module);
    }

    show_spaced_review_window(app, drafts, default_targets);
}

void show_plugin_manager(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    if (app->_plugin_manager_dialog != nullptr && app->_plugin_manager_dialog->isVisible()) {
        app->_plugin_manager_dialog->show();
        app->_plugin_manager_dialog->raise();
        app->_plugin_manager_dialog->activateWindow();
        return;
    }

    auto runtime = std::make_shared<PluginManagerRuntime>();
    runtime->app = app;

    auto *dialog = new RuntimeTranslatedDialog();
    runtime->dialog = dialog;
    configure_runtime_dialog(app, dialog, QSize(1080, 420));
    auto *layout = new QVBoxLayout(dialog);
    runtime->table = new QTableWidget(dialog);
    runtime->table->setColumnCount(5);
    runtime->table->setSelectionBehavior(QAbstractItemView::SelectRows);
    runtime->table->setSelectionMode(QAbstractItemView::SingleSelection);
    runtime->table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    runtime->table->verticalHeader()->setVisible(false);
    layout->addWidget(runtime->table, 1);

    auto *row = new QHBoxLayout();
    runtime->open_folder = new QPushButton(dialog);
    runtime->close = new QPushButton(dialog);
    row->addWidget(runtime->open_folder);
    row->addStretch(1);
    row->addWidget(runtime->close);
    layout->addLayout(row);

    QObject::connect(runtime->open_folder, &QPushButton::clicked, dialog, [runtime]() {
        if (!runtime || runtime->table == nullptr) {
            return;
        }
        const int selected = runtime->table->currentRow();
        if (selected < 0 || runtime->table->item(selected, 3) == nullptr) {
            return;
        }
        const QString path = runtime->table->item(selected, 3)->text();
        if (!path.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
        }
    });
    QObject::connect(runtime->close, &QPushButton::clicked, dialog, &QDialog::close);
    QObject::connect(dialog, &QObject::destroyed, app, [app]() { app->_plugin_manager_dialog = nullptr; });
    if (app->gerenciador_traducao != nullptr) {
        QObject::connect(app->gerenciador_traducao,
                         &source::language::GerenciadorTraducao::idioma_alterado,
                         dialog,
                         [runtime](const QString &) { plugin_manager_retranslate(runtime); });
    }
    dialog->set_retranslate_handler([runtime]() { plugin_manager_retranslate(runtime); });
    app->_plugin_manager_dialog = dialog;
    plugin_manager_retranslate(runtime);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void show_log_manager(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    if (app->_log_manager_dialog != nullptr && app->_log_manager_dialog->isVisible()) {
        app->_log_manager_dialog->show();
        app->_log_manager_dialog->raise();
        app->_log_manager_dialog->activateWindow();
        return;
    }

    auto runtime = std::make_shared<LogMonitorRuntime>();
    runtime->log_dir = log_manager_log_dir();

    auto *dialog = new RuntimeTranslatedDialog();
    runtime->dialog = dialog;
    configure_runtime_dialog(app, dialog, QSize(1180, 720));

    auto *layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    runtime->summary = new QLabel(dialog);
    runtime->summary->setWordWrap(true);
    layout->addWidget(runtime->summary);

    auto *splitter = new QSplitter(Qt::Vertical, dialog);
    runtime->table = new QTableWidget(splitter);
    runtime->table->setColumnCount(LogTableColumnCount);
    runtime->table->setSelectionBehavior(QAbstractItemView::SelectRows);
    runtime->table->setSelectionMode(QAbstractItemView::SingleSelection);
    runtime->table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    runtime->table->verticalHeader()->setVisible(false);
    splitter->addWidget(runtime->table);

    auto *monitor_container = new QWidget(splitter);
    auto *monitor_layout = new QVBoxLayout(monitor_container);
    monitor_layout->setContentsMargins(0, 0, 0, 0);
    monitor_layout->setSpacing(6);
    runtime->monitor_label = new QLabel(monitor_container);
    monitor_layout->addWidget(runtime->monitor_label);

    runtime->monitor = new QPlainTextEdit(monitor_container);
    runtime->monitor->setReadOnly(true);
    runtime->monitor->setLineWrapMode(QPlainTextEdit::NoWrap);
    QFont monitor_font = runtime->monitor->font();
    monitor_font.setFamily(QStringLiteral("Consolas"));
    runtime->monitor->setFont(monitor_font);
    monitor_layout->addWidget(runtime->monitor, 1);
    splitter->addWidget(monitor_container);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    layout->addWidget(splitter, 1);

    auto *row = new QHBoxLayout();
    runtime->open_log = new QPushButton(dialog);
    runtime->open_folder = new QPushButton(dialog);
    runtime->refresh = new QPushButton(dialog);
    runtime->auto_scroll = new QCheckBox(dialog);
    runtime->auto_scroll->setChecked(true);
    runtime->close = new QPushButton(dialog);
    row->addWidget(runtime->open_log);
    row->addWidget(runtime->open_folder);
    row->addWidget(runtime->refresh);
    row->addWidget(runtime->auto_scroll);
    row->addStretch(1);
    row->addWidget(runtime->close);
    layout->addLayout(row);

    runtime->watcher = new QFileSystemWatcher(dialog);
    runtime->timer = new QTimer(dialog);
    runtime->timer->setInterval(1000);

    QObject::connect(runtime->open_log, &QPushButton::clicked, dialog, [runtime]() {
        const QString selected_path = log_manager_selected_path(runtime->table);
        if (!selected_path.isEmpty() && QFileInfo::exists(selected_path)) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(selected_path));
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(runtime->log_dir));
    });
    QObject::connect(runtime->open_folder, &QPushButton::clicked, dialog, [runtime]() {
        const QString selected_path = log_manager_selected_path(runtime->table);
        const QString folder = !selected_path.isEmpty() && QFileInfo::exists(selected_path)
                                   ? QFileInfo(selected_path).absolutePath()
                                   : runtime->log_dir;
        QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
    });
    QObject::connect(runtime->refresh, &QPushButton::clicked, dialog, [runtime]() {
        log_manager_scan(runtime, true);
    });
    QObject::connect(runtime->close, &QPushButton::clicked, dialog, &QDialog::close);
    QObject::connect(runtime->watcher, &QFileSystemWatcher::directoryChanged, dialog, [runtime](const QString &) {
        log_manager_scan(runtime, true);
    });
    QObject::connect(runtime->watcher, &QFileSystemWatcher::fileChanged, dialog, [runtime](const QString &) {
        log_manager_scan(runtime, true);
    });
    QObject::connect(runtime->timer, &QTimer::timeout, dialog, [runtime]() {
        log_manager_scan(runtime, false);
    });
    QObject::connect(dialog, &QObject::destroyed, app, [app]() { app->_log_manager_dialog = nullptr; });
    if (app->gerenciador_traducao != nullptr) {
        QObject::connect(app->gerenciador_traducao,
                         &source::language::GerenciadorTraducao::idioma_alterado,
                         dialog,
                         [runtime](const QString &) { log_manager_retranslate(runtime); });
    }
    dialog->set_retranslate_handler([runtime]() { log_manager_retranslate(runtime); });
    app->_log_manager_dialog = dialog;

    log_manager_retranslate(runtime);
    runtime->timer->start();
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}


}  // namespace source::InterfaceCore::ProductivityFeatures
