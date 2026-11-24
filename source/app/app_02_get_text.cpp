#include "source/app/app_02_get_text.hpp"
#include "source/modules/gantt/gantt_plugin_interface.hpp"
#include "source/utils/TaskProgressHelper.hpp"

namespace source::app {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}


QScreen *effective_screen_for_widget(const QWidget *widget)
{
    if (widget != nullptr) {
        if (QWindow *handle = widget->windowHandle(); handle != nullptr && handle->screen() != nullptr) {
            return handle->screen();
        }
        if (widget->screen() != nullptr) {
            return widget->screen();
        }
        if (widget->window() != nullptr && widget->window()->screen() != nullptr) {
            return widget->window()->screen();
        }
    }

    return QApplication::primaryScreen();
}

QMargins top_level_frame_margins(const QWidget *widget)
{
    if (widget == nullptr) {
        return {};
    }

    const QRect frame = widget->frameGeometry();
    const QRect client = widget->geometry();
    return QMargins(qMax(0, client.left() - frame.left()),
                    qMax(0, client.top() - frame.top()),
                    qMax(0, frame.right() - client.right()),
                    qMax(0, frame.bottom() - client.bottom()));
}

QSize max_client_size_for_screen(const QWidget *widget, const QRect &available)
{
    const QMargins margins = top_level_frame_margins(widget);
    return QSize(qMax(320, available.width() - margins.left() - margins.right()),
                 qMax(240, available.height() - margins.top() - margins.bottom()));
}

QSize initial_window_minimum_for_screen(const QSize &max_client_size)
{
    return QSize(qMin(kInitialWindowMinWidth, max_client_size.width()),
                 qMin(kInitialWindowMinHeight, max_client_size.height()));
}

void relax_module_page(QWidget *page)
{
    if (page == nullptr) {
        return;
    }

    page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    page->setMinimumHeight(0);
    if (page->layout() != nullptr) {
        page->layout()->setSizeConstraint(QLayout::SetNoConstraint);
        page->layout()->invalidate();
    }
    page->updateGeometry();
}

QString eisenhower_module_tab_text()
{
    return get_text(QStringLiteral("🔠 Eisenhower"));
}

QString feynman_module_tab_text()
{
    return get_text(QStringLiteral("🪢 Feynman"));
}

QString pomodoro_module_tab_text()
{
    return get_text(QStringLiteral("🍅 Pomodoro"));
}

QString mapamental_module_tab_text()
{
    return get_text(QStringLiteral("🧠 Mapa Mental"));
}

QString mapamental_module_tab_text(const QString &display_name)
{
    const QString name = display_name.trimmed().isEmpty() ? get_text(QStringLiteral("Mapa Mental")) : display_name.trimmed();
    return QStringLiteral("🧠 %1").arg(name);
}

QString audiolivro_module_tab_text(const QString &display_name)
{
    const QString name = display_name.trimmed().isEmpty() ? get_text(QStringLiteral("Audiolivro")) : display_name.trimmed();
    return QStringLiteral("🎧 %1").arg(name);
}

int count_selectable_items(const QListWidget *list)
{
    if (list == nullptr) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < list->count(); ++i) {
        if (QListWidgetItem *item = list->item(i); item != nullptr && (item->flags() & Qt::ItemIsSelectable)) {
            ++count;
        }
    }
    return count;
}

QString selected_task_key(const QListWidgetItem *item)
{
    if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
        return {};
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
    if (!task_id.isEmpty()) {
        return task_id;
    }

    const QString text = data.value(QStringLiteral("text")).toString().trimmed();
    const QString date = data.value(QStringLiteral("date")).toString().trimmed();
    const QString time = data.value(QStringLiteral("time")).toString().trimmed();
    const QString priority = data.value(QStringLiteral("priority")).toString().trimmed();
    const QString status = data.value(QStringLiteral("status")).toString().trimmed();
    const QString file_path = data.value(QStringLiteral("file_path")).toString().trimmed();

    return QStringLiteral("%1|%2|%3|%4|%5|%6")
        .arg(text, date, time, priority, status, file_path);
}

void collect_selected_task_keys(const QListWidget *list, QSet<QString> &selected_keys)
{
    if (list == nullptr) {
        return;
    }

    for (QListWidgetItem *item : list->selectedItems()) {
        const QString key = selected_task_key(item);
        if (!key.isEmpty()) {
            selected_keys.insert(key);
        }
    }
}

QString module_task_counter_text(const QString &module_name,
                                 int selected_count,
                                 int total_count)
{
    return get_text(QStringLiteral("%1: Tarefas selecionadas: %2 | Total de tarefas: %3"))
        .arg(module_name, QString::number(selected_count), QString::number(total_count));
}

QStringList legacy_appdata_task_paths()
{
    const QString persistent_dir = source::utils::obter_caminho_persistente();
    return {
        QDir(persistent_dir).filePath(QStringLiteral("eisenhower_tarefas.json")),
        QDir(persistent_dir).filePath(QStringLiteral("tasks.json")),
        QDir(persistent_dir).filePath(QStringLiteral("tarefas_pomodoro.json")),
        QDir(persistent_dir).filePath(QStringLiteral("conceitos_feynman.json")),
    };
}

void remove_legacy_task_datastores()
{
    for (const QString &path : legacy_appdata_task_paths()) {
        if (path.trimmed().isEmpty()) {
            continue;
        }

        QFile::remove(path);
        QFile::remove(path + QStringLiteral(".bak"));
    }
}

QString task_layout_key(source::EisenhowerMatrixApp::TaskLayoutMode mode)
{
    switch (mode) {
    case source::EisenhowerMatrixApp::TaskLayoutMode::Quadrants:
        return QStringLiteral("quadrants");
    case source::EisenhowerMatrixApp::TaskLayoutMode::Matrix2x2:
        return QStringLiteral("matrix_2x2");
    case source::EisenhowerMatrixApp::TaskLayoutMode::TabsHorizontal:
        return QStringLiteral("tabs_horizontal");
    case source::EisenhowerMatrixApp::TaskLayoutMode::Tabs:
    default:
        return QStringLiteral("tabs");
    }
}

PomodoroUiLayoutParts normalized_pomodoro_ui_layout_parts(const QString &raw_key)
{
    QString key = raw_key.trimmed().toLower();
    QStringList parts;
    parts.reserve(2);
    QString current;

    for (QChar ch : key) {
        if (ch == QLatin1Char(':') || ch == QLatin1Char('/') || ch == QLatin1Char('-') || ch == QLatin1Char('_')
            || ch == QLatin1Char('|')) {
            if (!current.isEmpty()) {
                parts.append(current);
                current.clear();
            }
            continue;
        }
        current.append(ch);
    }

    if (!current.isEmpty()) {
        parts.append(current);
    }

    QString mode = parts.value(0);
    if (mode != QStringLiteral("robust")) {
        mode = QStringLiteral("simple");
    }

    QString panel = parts.value(1);
    if (panel != QStringLiteral("vertical") && panel != QStringLiteral("horizontal")) {
        panel = QStringLiteral("tabs");
    }

    return {mode, panel};
}

QString normalized_pomodoro_ui_layout_key(const QString &raw_key)
{
    const PomodoroUiLayoutParts parts = normalized_pomodoro_ui_layout_parts(raw_key);
    return QStringLiteral("%1:%2").arg(parts.mode, parts.panel);
}

QString normalized_feynman_ui_layout_key(const QString &raw_key)
{
    const QString key = raw_key.trimmed().toLower();
    if (key == QStringLiteral("horizontal") || key == QStringLiteral("vertical")) {
        return key;
    }
    return QStringLiteral("tabs");
}

QStringList default_module_tab_order()
{
    return {QStringLiteral("eisenhower"),
            QStringLiteral("pomodoro"),
            QStringLiteral("feynman"),
            QStringLiteral("mapamental"),
            QStringLiteral("audiolivro")};
}

QString normalized_module_tab_key(const QString &raw_key)
{
    const QString key = raw_key.trimmed().toLower();
    return default_module_tab_order().contains(key) ? key : QString();
}

QMap<QString, bool> default_module_tab_visibility()
{
    QMap<QString, bool> visibility;
    for (const QString &key : default_module_tab_order()) {
        visibility.insert(key, true);
    }
    return visibility;
}

QStringList normalized_module_tab_order(const QStringList &raw_order)
{
    const QStringList defaults = default_module_tab_order();
    QStringList normalized;
    normalized.reserve(defaults.size());

    const auto append_key = [&normalized, &defaults](const QString &raw_key) {
        const QString key = raw_key.trimmed().toLower();
        if (!key.isEmpty() && defaults.contains(key) && !normalized.contains(key)) {
            normalized.append(key);
        }
    };

    for (const QString &key : raw_order) {
        append_key(key);
    }
    for (const QString &key : defaults) {
        append_key(key);
    }

    return normalized;
}

QStringList json_string_array_to_list(const QJsonValue &value)
{
    QStringList result;
    if (!value.isArray()) {
        return result;
    }

    const QJsonArray array = value.toArray();
    result.reserve(array.size());
    for (const QJsonValue &entry : array) {
        const QString key = entry.toString().trimmed();
        if (!key.isEmpty()) {
            result.append(key);
        }
    }

    return result;
}

QMap<QString, bool> normalized_module_tab_visibility(const QMap<QString, bool> &raw_visibility)
{
    QMap<QString, bool> visibility = default_module_tab_visibility();
    for (auto it = raw_visibility.constBegin(); it != raw_visibility.constEnd(); ++it) {
        const QString key = normalized_module_tab_key(it.key());
        if (!key.isEmpty()) {
            visibility.insert(key, it.value());
        }
    }
    return visibility;
}

source::EisenhowerMatrixApp::TaskSortCriterion task_sort_criterion_from_mode(
    source::EisenhowerMatrixApp::TaskSortMode mode)
{
    switch (mode) {
    case source::EisenhowerMatrixApp::TaskSortMode::DateAscending:
    case source::EisenhowerMatrixApp::TaskSortMode::DateDescending:
        return source::EisenhowerMatrixApp::TaskSortCriterion::Date;
    case source::EisenhowerMatrixApp::TaskSortMode::TimeAscending:
    case source::EisenhowerMatrixApp::TaskSortMode::TimeDescending:
        return source::EisenhowerMatrixApp::TaskSortCriterion::Time;
    case source::EisenhowerMatrixApp::TaskSortMode::AlphabeticalAscending:
    case source::EisenhowerMatrixApp::TaskSortMode::AlphabeticalDescending:
    case source::EisenhowerMatrixApp::TaskSortMode::None:
    default:
        return source::EisenhowerMatrixApp::TaskSortCriterion::Alphabetical;
    }
}

Qt::SortOrder task_sort_order_from_mode(source::EisenhowerMatrixApp::TaskSortMode mode)
{
    switch (mode) {
    case source::EisenhowerMatrixApp::TaskSortMode::DateDescending:
    case source::EisenhowerMatrixApp::TaskSortMode::TimeDescending:
    case source::EisenhowerMatrixApp::TaskSortMode::AlphabeticalDescending:
        return Qt::DescendingOrder;
    case source::EisenhowerMatrixApp::TaskSortMode::DateAscending:
    case source::EisenhowerMatrixApp::TaskSortMode::TimeAscending:
    case source::EisenhowerMatrixApp::TaskSortMode::AlphabeticalAscending:
    case source::EisenhowerMatrixApp::TaskSortMode::None:
    default:
        return Qt::AscendingOrder;
    }
}

QString task_sort_criterion_key(source::EisenhowerMatrixApp::TaskSortCriterion criterion)
{
    switch (criterion) {
    case source::EisenhowerMatrixApp::TaskSortCriterion::Date:
        return QStringLiteral("date");
    case source::EisenhowerMatrixApp::TaskSortCriterion::Alphabetical:
        return QStringLiteral("alphabetical");
    case source::EisenhowerMatrixApp::TaskSortCriterion::Time:
    default:
        return QStringLiteral("time");
    }
}

QString task_sort_order_key(Qt::SortOrder order)
{
    return order == Qt::DescendingOrder ? QStringLiteral("desc") : QStringLiteral("asc");
}

void clear_layout(QLayout *layout)
{
    if (layout == nullptr) {
        return;
    }

    while (QLayoutItem *item = layout->takeAt(0)) {
        delete item;
    }
}

QString normalized_text(const QString &value)
{
    QString text = value.trimmed();
    text.replace(QStringLiteral("🔴"), QString());
    text.replace(QStringLiteral("🟠"), QString());
    text.replace(QStringLiteral("🟡"), QString());
    text.replace(QStringLiteral("🟢"), QString());
    text.remove(QChar(0x0332));
    return text.simplified().trimmed();
}

QString base_task_text(QListWidgetItem *item)
{
    if (item == nullptr) {
        return {};
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString text = data.value(QStringLiteral("text")).toString().trimmed();
    if (!text.isEmpty()) {
        return text;
    }

    return source::InterfaceCore::MostrarMenuContextoModules::base_text_from_item(item).trimmed();
}

QString priority_label_without_emoji(const QVariant &priority, source::EisenhowerMatrixApp *app)
{
    QString label = priority.toString().trimmed();
    bool ok = false;
    const int index = priority.toInt(&ok);
    if (ok && app != nullptr) {
        label = source::InterfaceCore::prioridade_para_texto(index, app);
    }

    return normalized_text(label);
}

int quadrant_from_priority(const QVariant &priority, source::EisenhowerMatrixApp *app)
{
    bool ok = false;
    const int numeric = priority.toInt(&ok);
    if (ok && numeric >= 1 && numeric <= 4) {
        return numeric;
    }

    const QString label = priority_label_without_emoji(priority, app);
    if (label.contains(QStringLiteral("Importante e Urgente"), Qt::CaseInsensitive)) {
        return 1;
    }
    if (label.contains(QStringLiteral("Importante, mas Não Urgente"), Qt::CaseInsensitive)) {
        return 2;
    }
    if (label.contains(QStringLiteral("Não Importante, mas Urgente"), Qt::CaseInsensitive)) {
        return 3;
    }
    if (label.contains(QStringLiteral("Não Importante e Não Urgente"), Qt::CaseInsensitive)) {
        return 4;
    }

    return 1;
}

int normalized_domain_value(const QVariant &domain)
{
    bool ok = false;
    const int numeric = domain.toInt(&ok);
    if (ok) {
        return qBound(0, numeric, 2);
    }

    const QString label = source::InterfaceCore::dominio_para_texto(domain, false);
    if (label.startsWith(QStringLiteral("🥇"))) {
        return 2;
    }
    if (label.startsWith(QStringLiteral("🥈"))) {
        return 1;
    }
    return 0;
}

bool payload_completed(const QVariantMap &task)
{
    if (task.value(QStringLiteral("completed")).toBool()) {
        return true;
    }

    const QString status = task.value(QStringLiteral("status")).toString().trimmed();
    return status.compare(QStringLiteral("Done"), Qt::CaseInsensitive) == 0
        || status.compare(QStringLiteral("Completed"), Qt::CaseInsensitive) == 0;
}

QList<QListWidget *> all_task_lists(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return {};
    }

    return {app->quadrant1_list,
            app->quadrant2_list,
            app->quadrant3_list,
            app->quadrant4_list,
            app->quadrant1_in_progress_list,
            app->quadrant2_in_progress_list,
            app->quadrant3_in_progress_list,
            app->quadrant4_in_progress_list,
            app->quadrant1_completed_list,
            app->quadrant2_completed_list,
            app->quadrant3_completed_list,
            app->quadrant4_completed_list};
}

QString placeholder_text_for_list(source::EisenhowerMatrixApp *app, const QListWidget *list)
{
    if (app == nullptr || list == nullptr) {
        return {};
    }

    if (list == app->quadrant1_list) {
        return get_text(QStringLiteral("1º Quadrante"));
    }
    if (list == app->quadrant2_list) {
        return get_text(QStringLiteral("2º Quadrante"));
    }
    if (list == app->quadrant3_list) {
        return get_text(QStringLiteral("3º Quadrante"));
    }
    if (list == app->quadrant4_list) {
        return get_text(QStringLiteral("4º Quadrante"));
    }
    if (app->is_in_progress_list(list)) {
        return get_text(QStringLiteral("Nenhuma Tarefa em Progresso"));
    }
    return get_text(QStringLiteral("Nenhuma Tarefa Concluída"));
}

QString human_date_for_app(const source::EisenhowerMatrixApp *app, const QString &date_iso)
{
    const QString cleaned = date_iso.trimmed();
    if (cleaned.isEmpty()) {
        return {};
    }

    const QDate date = QDate::fromString(cleaned, Qt::ISODate);
    if (!date.isValid()) {
        return cleaned;
    }

    return (app != nullptr && app->date_input != nullptr) ? date.toString(app->date_input->displayFormat()) : cleaned;
}

QString visible_task_text(const source::EisenhowerMatrixApp *app,
                          const QString &base_text,
                          const QString &date_iso,
                          const QString &time_str)
{
    QString display_text = base_text.trimmed();
    if (app == nullptr || !app->reserved_space_eisenhower_enabled()) {
        return display_text;
    }

    const QString human_date = human_date_for_app(app, date_iso);
    if (human_date.isEmpty()) {
        return display_text;
    }

    const QString cleaned_time = time_str.trimmed();
    return cleaned_time.isEmpty() ? QStringLiteral("%1 — %2").arg(display_text, human_date)
                                  : QStringLiteral("%1 — %2 %3").arg(display_text, human_date, cleaned_time);
}

QString compose_task_display_text(const source::EisenhowerMatrixApp *app,
                                  const QString &base_text,
                                  const QString &date_iso,
                                  const QString &time_str)
{
    return visible_task_text(app, base_text, date_iso, time_str);
}

QDate task_date_value(const QVariantMap &data)
{
    return QDate::fromString(data.value(QStringLiteral("date")).toString().trimmed(), Qt::ISODate);
}

QTime task_time_value(const QVariantMap &data)
{
    return QTime::fromString(data.value(QStringLiteral("time")).toString().trimmed(), QStringLiteral("HH:mm"));
}

QVariantMap normalized_task_sort_payload(const QVariantMap &payload)
{
    QVariantMap normalized = payload;

    const QString text = payload.value(QStringLiteral("text")).toString().trimmed().isEmpty()
                             ? payload.value(QStringLiteral("titulo")).toString().trimmed()
                             : payload.value(QStringLiteral("text")).toString().trimmed();
    if (!text.isEmpty()) {
        normalized.insert(QStringLiteral("text"), text);
    }

    const QString date_value = payload.value(QStringLiteral("date")).toString().trimmed().isEmpty()
                                   ? payload.value(QStringLiteral("data")).toString().trimmed()
                                   : payload.value(QStringLiteral("date")).toString().trimmed();
    if (!date_value.isEmpty()) {
        normalized.insert(QStringLiteral("date"), date_value);
    }

    const QString time_value = payload.value(QStringLiteral("time")).toString().trimmed().isEmpty()
                                   ? payload.value(QStringLiteral("hora")).toString().trimmed()
                                   : payload.value(QStringLiteral("time")).toString().trimmed();
    if (!time_value.isEmpty()) {
        normalized.insert(QStringLiteral("time"), time_value);
    }

    return normalized;
}

bool task_matches_payload(QListWidgetItem *item, const QVariantMap &payload)
{
    if (item == nullptr) {
        return false;
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString payload_id = payload.value(QStringLiteral("id")).toString().trimmed();
    const QString current_id = data.value(QStringLiteral("id")).toString().trimmed();

    if (!payload_id.isEmpty() && !current_id.isEmpty()) {
        return payload_id == current_id;
    }

    return normalized_text(data.value(QStringLiteral("text")).toString()) == normalized_text(payload.value(QStringLiteral("titulo")).toString())
        && normalized_text(data.value(QStringLiteral("date")).toString()) == normalized_text(payload.value(QStringLiteral("date")).toString())
        && normalized_text(data.value(QStringLiteral("time")).toString()) == normalized_text(payload.value(QStringLiteral("time")).toString())
        && normalized_text(data.value(QStringLiteral("file_path")).toString()) == normalized_text(payload.value(QStringLiteral("file_path")).toString())
        && normalized_text(data.value(QStringLiteral("description")).toString()) == normalized_text(payload.value(QStringLiteral("description")).toString())
        && quadrant_from_priority(data.value(QStringLiteral("priority")), nullptr)
               == quadrant_from_priority(payload.value(QStringLiteral("prioridade")), nullptr);
}

bool task_matches_sync_payload(QListWidgetItem *item, const QVariantMap &payload)
{
    if (item == nullptr) {
        return false;
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString payload_id = payload.value(QStringLiteral("id")).toString().trimmed();
    const QString current_id = data.value(QStringLiteral("id")).toString().trimmed();

    if (!payload_id.isEmpty() && !current_id.isEmpty()) {
        return payload_id == current_id;
    }

    const QString payload_title = normalized_text(payload.value(QStringLiteral("titulo")).toString());
    const QString current_title = normalized_text(base_task_text(item));
    const QString payload_date = normalized_text(payload.value(QStringLiteral("date")).toString());
    const QString current_date = normalized_text(data.value(QStringLiteral("date")).toString());

    if (payload_title.isEmpty() || current_title.isEmpty() || payload_date.isEmpty() || current_date.isEmpty()) {
        return false;
    }

    return payload_title == current_title && payload_date == current_date;
}

QPair<QString, QString> display_and_tooltip_for_task(source::EisenhowerMatrixApp *app,
                                                     const QVariantMap &payload,
                                                     int quadrant)
{
    const QString title = payload.value(QStringLiteral("titulo")).toString().trimmed();
    const QString domain_badge = payload.contains(QStringLiteral("dominio"))
                                     ? source::InterfaceCore::dominio_insignia(payload.value(QStringLiteral("dominio")))
                                     : QString();
    const QString decorated_title =
        domain_badge.isEmpty() ? title : QStringLiteral("%1 %2").arg(domain_badge, title);
    const QString date_value = payload.value(QStringLiteral("date")).toString().trimmed();
    const QString time_value = payload.value(QStringLiteral("time")).toString().trimmed();
    QString display_text = compose_task_display_text(app, decorated_title, date_value, time_value);
    QStringList tooltip_lines;

    if (!date_value.isEmpty()) {
        const QString human_date = human_date_for_app(app, date_value);
        if (!human_date.isEmpty()) {
            tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Data")), human_date);
        }
    }

    if (!time_value.isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Horário")), time_value);
    }

    const QString file_path = payload.value(QStringLiteral("file_path")).toString().trimmed();
    if (!file_path.isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Arquivo")), file_path);
    }

    const QString description = payload.value(QStringLiteral("description")).toString().trimmed();
    if (!description.isEmpty()) {
        tooltip_lines << QStringLiteral("%1: %2").arg(get_text(QStringLiteral("Descrição")), description);
    }

    tooltip_lines << QStringLiteral("%1: %2")
                         .arg(get_text(QStringLiteral("Prioridade")),
                              priority_label_without_emoji(payload.value(QStringLiteral("prioridade"),
                                                                         payload.value(QStringLiteral("priority"))),
                                                           app));
    if (payload.contains(QStringLiteral("dominio"))) {
        tooltip_lines << QStringLiteral("%1: %2")
                             .arg(get_text(QStringLiteral("Nível de Domínio")),
                                  source::InterfaceCore::dominio_para_texto(payload.value(QStringLiteral("dominio")), false));
    }

    double duration = 1.0;
    if (payload.contains(QStringLiteral("duracao")) && payload.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        duration = payload.value(QStringLiteral("duracao")).toDouble();
    } else if (payload.contains(QStringLiteral("duration")) && payload.value(QStringLiteral("duration")).toDouble() > 0.0) {
        duration = payload.value(QStringLiteral("duration")).toDouble();
    } else if (payload.contains(QStringLiteral("duracao_dias")) && payload.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
        duration = payload.value(QStringLiteral("duracao_dias")).toDouble();
    }
    tooltip_lines << format_duration_tooltip_line(duration);

    const int pomodoros = payload.value(QStringLiteral("pomodoros"), payload.value(QStringLiteral("pomodoros_completados"), 0)).toInt();
    const int safe_c_tot = payload.value(QStringLiteral("ciclos_totais"), payload.value(QStringLiteral("ciclos_permitidos"), payload.value(QStringLiteral("total_ciclos"), 4))).toInt();
    const int actual_c_tot = safe_c_tot > 0 ? safe_c_tot : 4;
    const bool is_done = payload.value(QStringLiteral("concluida")).toBool() || payload.value(QStringLiteral("status")).toString() == QStringLiteral("Done");
    if (pomodoros > 0 || actual_c_tot > 0) {
        const double pom_pct = source::utils::calculate_task_completion_percentage(pomodoros, actual_c_tot, is_done);
        const QString pom_line_fmt = get_text(QStringLiteral("Ciclos Pomodoro: %1/%2 (%3%)"));
        tooltip_lines << pom_line_fmt
                             .arg(QString::number(pomodoros),
                                  QString::number(actual_c_tot),
                                  QString::number(pom_pct, 'f', 0));
    }



    Q_UNUSED(quadrant);
    return {display_text, tooltip_lines.join(QLatin1Char('\n'))};
}

QString format_duration_text(double duration_days)
{
    if (qFuzzyCompare(duration_days, 1.0)) {
        const QString one_day = get_text(QStringLiteral("1 dia"));
        if (one_day != QStringLiteral("1 dia")) {
            return one_day;
        }
        return QStringLiteral("1 %1").arg(get_text(QStringLiteral("dia")));
    }
    const double rounded = std::round(duration_days * 10.0) / 10.0;
    if (rounded == static_cast<int>(rounded)) {
        const QString multi_days = get_text(QStringLiteral("%1 dias"));
        if (multi_days.contains(QStringLiteral("%1"))) {
            return multi_days.arg(static_cast<int>(rounded));
        }
        return QStringLiteral("%1 %2").arg(static_cast<int>(rounded)).arg(get_text(QStringLiteral("dias")));
    }
    const QString multi_days = get_text(QStringLiteral("%1 dias"));
    if (multi_days.contains(QStringLiteral("%1"))) {
        return multi_days.arg(QString::number(duration_days, 'f', 1));
    }
    return QStringLiteral("%1 %2").arg(QString::number(duration_days, 'f', 1)).arg(get_text(QStringLiteral("dias")));
}

QString format_duration_tooltip_line(double duration_days)
{
    const QString label = get_text(QStringLiteral("Duração:"));
    if (label.endsWith(QLatin1Char(':'))) {
        return QStringLiteral("%1 %2").arg(label, format_duration_text(duration_days));
    }
    return QStringLiteral("%1: %2").arg(label, format_duration_text(duration_days));
}

QStringList pomodoro_plugin_filenames()
{
    return source::utils::PluginRegistry::binary_names(QStringLiteral("PomodoroPlugin"));
}

QStringList pomodoro_plugin_candidates()
{
    return source::utils::PluginRegistry::candidate_paths(QStringLiteral("pomodoro"),
                                                          QStringLiteral("PomodoroPlugin"));
}

QStringList feynman_plugin_filenames()
{
    return source::utils::PluginRegistry::binary_names(QStringLiteral("FeynmanPlugin"));
}

QStringList feynman_plugin_candidates()
{
    return source::utils::PluginRegistry::candidate_paths(QStringLiteral("feynman"),
                                                          QStringLiteral("FeynmanPlugin"));
}

QStringList mapamental_plugin_filenames()
{
    return source::utils::PluginRegistry::binary_names(QStringLiteral("MapaMentalPlugin"));
}

QStringList mapamental_plugin_candidates()
{
    return source::utils::PluginRegistry::candidate_paths(QStringLiteral("mapamental"),
                                                          QStringLiteral("MapaMentalPlugin"));
}

QStringList modelo_plugin_filenames()
{
    return source::utils::PluginRegistry::binary_names(QStringLiteral("ModeloPlugin"));
}

QStringList modelo_plugin_candidates()
{
    return source::utils::PluginRegistry::candidate_paths(QStringLiteral("modelo"),
                                                          QStringLiteral("ModeloPlugin"));
}

QStringList audiolivro_plugin_filenames()
{
    return source::utils::PluginRegistry::binary_names(QStringLiteral("AudiolivroPlugin"));
}

QStringList audiolivro_plugin_candidates()
{
    return source::utils::PluginRegistry::candidate_paths(QStringLiteral("audiolivro"),
                                                          QStringLiteral("AudiolivroPlugin"));
}

QStringList agenda_plugin_filenames()
{
    return source::utils::PluginRegistry::binary_names(QStringLiteral("AgendaPlugin"));
}

QStringList agenda_plugin_candidates()
{
    return source::utils::PluginRegistry::candidate_paths(QStringLiteral("agenda"),
                                                          QStringLiteral("AgendaPlugin"));
}

QStringList gantt_plugin_filenames()
{
    return source::utils::PluginRegistry::binary_names(QStringLiteral("GanttPlugin"));
}

QStringList gantt_plugin_candidates()
{
    return source::utils::PluginRegistry::candidate_paths(QStringLiteral("gantt"),
                                                          QStringLiteral("GanttPlugin"));
}

void prepend_process_path(const QStringList &paths)
{
#if defined(Q_OS_WIN)
    QStringList current = qEnvironmentVariable("PATH").split(QLatin1Char(';'), Qt::SkipEmptyParts);
    QStringList normalized_current;
    normalized_current.reserve(current.size());
    for (const QString &entry : current) {
        normalized_current.append(QDir::toNativeSeparators(QFileInfo(entry).absoluteFilePath()));
    }

    QStringList merged = current;
    for (const QString &path : paths) {
        const QString normalized = QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath());
        if (!normalized.isEmpty() && !normalized_current.contains(normalized, Qt::CaseInsensitive)) {
            merged.prepend(normalized);
            normalized_current.prepend(normalized);
        }
    }

    qputenv("PATH", merged.join(QLatin1Char(';')).toUtf8());
#else
    Q_UNUSED(paths);
#endif
}

}  // namespace source::app
