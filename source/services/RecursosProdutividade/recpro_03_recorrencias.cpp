#include "source/services/RecursosProdutividade/recpro_03_recorrencias.hpp"
#include "source/services/RecursosProdutividade/recpro_01_utilitarios_compartilhados.hpp"

#include <QFontMetrics>
#include <QFrame>
#include <QListWidget>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QToolButton>

namespace {

class PreviewListToolButton : public QToolButton {
public:
    explicit PreviewListToolButton(const QString &full_text, QWidget *parent = nullptr)
        : QToolButton(parent), full_text_(full_text)
    {
        setMinimumWidth(0);
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        setPopupMode(QToolButton::InstantPopup);
        setToolButtonStyle(Qt::ToolButtonTextOnly);
        update_elided_text();
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QToolButton::resizeEvent(event);
        update_elided_text();
    }

    void update_elided_text() {
        const int available_width = qMax(10, width() - 16);
        const QFontMetrics fm(font());
        setText(fm.elidedText(full_text_, Qt::ElideRight, available_width));
    }

private:
    QString full_text_;
};

} // namespace

namespace source::InterfaceCore::ProductivityFeatures {

void materialize_recurring_tasks(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    QVariantList unified_records = records_for_app(app);
    QJsonObject map_backup = read_json_object(mapamental_backup_path());
    const QStringList available_modules = available_productivity_modules(app);
    const QSet<QString> all_modules = module_set_from_list(available_modules);
    QVector<TaskRecordGroup> groups =
        collect_productivity_task_groups(app, unified_records, map_backup, all_modules);
    if (groups.isEmpty()) {
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Recorrências")),
                                 get_text(QStringLiteral("Nenhum item encontrado nos métodos selecionados.")));
        return;
    }

    RuntimeTranslatedDialog dialog(app);
    dialog.setWindowTitle(get_text(QStringLiteral("Gerar próximas recorrências")));
    dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    auto *layout = new QVBoxLayout(&dialog);
    layout->setSizeConstraint(QLayout::SetNoConstraint);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *summary = new QLabel(&dialog);
    summary->setWordWrap(true);
    layout->addWidget(summary);

    auto *guide = new QLabel(get_text(QStringLiteral(
                                 "Origem: os métodos marcados abaixo serão lidos. Destino: as novas ocorrências serão criadas nos mesmos métodos em que cada item já existe; se o item for compartilhado, a ocorrência também será compartilhada nos métodos marcados.")),
                             &dialog);
    guide->setWordWrap(true);
    layout->addWidget(guide);

    auto *methods_group = new QGroupBox(get_text(QStringLiteral("Origem e destino dos métodos")), &dialog);
    methods_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *methods_layout = new QGridLayout(methods_group);
    methods_layout->setContentsMargins(8, 6, 8, 6);
    methods_layout->setHorizontalSpacing(10);
    methods_layout->setVerticalSpacing(2);
    QSet<QString> checked_by_default = all_modules;
    QMap<QString, QCheckBox *> module_checks =
        create_productivity_module_checks(methods_group, checked_by_default, available_modules);
    const QVector<QWidget *> methods_widgets = widgets_for_productivity_modules(module_checks, available_modules);
    auto methods_grid = std::make_unique<ResponsiveCheckGrid>();
    methods_grid->container = methods_group;
    methods_grid->layout = methods_layout;
    methods_grid->widgets = methods_widgets;
    auto *methods_grid_filter = new ResponsiveGridResizeFilter(methods_grid.get(), methods_group);
    methods_group->installEventFilter(methods_grid_filter);
    update_responsive_grid(methods_grid.get());

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
    item_view->setMinimumHeight(118);
    selection_layout->addWidget(item_view, 1);

    auto *selection_buttons_widget = new QWidget(selection_group);
    selection_buttons_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto *selection_buttons = new QHBoxLayout(selection_buttons_widget);
    selection_buttons->setContentsMargins(0, 8, 0, 0);
    selection_buttons->setSpacing(6);
    auto *select_visible_button = new QPushButton(get_text(QStringLiteral("Selecionar visíveis")), selection_buttons_widget);
    auto *clear_selection_button = new QPushButton(get_text(QStringLiteral("Limpar")), selection_buttons_widget);
    select_visible_button->setMaximumWidth(150);
    clear_selection_button->setMaximumWidth(90);
    selection_buttons->addWidget(select_visible_button);
    selection_buttons->addWidget(clear_selection_button);
    selection_buttons->addStretch(1);
    selection_layout->addWidget(selection_buttons_widget);
    selection_group->setMinimumHeight(200);
    selection_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    auto *settings_group = new QGroupBox(get_text(QStringLiteral("Configurações")), &dialog);
    auto *form = new QFormLayout(settings_group);
    auto *horizon_days = new QSpinBox(settings_group);
    horizon_days->setRange(1, 365);
    horizon_days->setValue(30);
    auto *max_per_item = new QSpinBox(settings_group);
    max_per_item->setRange(1, 60);
    max_per_item->setValue(8);
    auto *include_completed = new QCheckBox(get_text(QStringLiteral("Incluir itens concluídos")), settings_group);
    auto *avoid_duplicates = new QCheckBox(get_text(QStringLiteral("Evitar duplicadas existentes")), settings_group);
    avoid_duplicates->setChecked(true);
    auto *apply_default = new QCheckBox(
        get_text(QStringLiteral("Aplicar frequência padrão a itens sem recorrência definida")), settings_group);
    auto *default_recurrence = new QComboBox(settings_group);
    default_recurrence->addItem(get_text(QStringLiteral("Diária")), QStringLiteral("daily"));
    default_recurrence->addItem(get_text(QStringLiteral("Semanal")), QStringLiteral("weekly"));
    default_recurrence->addItem(get_text(QStringLiteral("Mensal")), QStringLiteral("monthly"));
    default_recurrence->setCurrentIndex(1);
    default_recurrence->setEnabled(false);
    QObject::connect(apply_default, &QCheckBox::toggled, default_recurrence, &QComboBox::setEnabled);

    auto *lbl_horizon = new QLabel(get_text(QStringLiteral("Alcance (dias)")), settings_group);
    auto *lbl_max = new QLabel(get_text(QStringLiteral("Máximo por item")), settings_group);
    auto *lbl_default_recurrence = new QLabel(get_text(QStringLiteral("Frequência padrão")), settings_group);
    form->addRow(lbl_horizon, horizon_days);
    form->addRow(lbl_max, max_per_item);
    form->addRow(include_completed);
    form->addRow(avoid_duplicates);
    form->addRow(apply_default);
    form->addRow(lbl_default_recurrence, default_recurrence);

    auto *when_group = new QGroupBox(get_text(QStringLiteral("Quando aplicar")), &dialog);
    auto *when_layout = new QHBoxLayout(when_group);
    when_layout->setContentsMargins(10, 8, 10, 8);
    when_layout->setSpacing(12);
    auto *when_form = new QFormLayout();
    when_form->setContentsMargins(0, 0, 0, 0);
    when_form->setSpacing(6);
    const QString date_format = QLocale().dateFormat(QLocale::ShortFormat);
    auto *application_start_date = new QDateEdit(QDate::currentDate(), when_group);
    application_start_date->setCalendarPopup(true);
    application_start_date->setDisplayFormat(date_format);
    auto *application_end_date = new QDateEdit(QDate::currentDate().addDays(horizon_days->value()), when_group);
    application_end_date->setCalendarPopup(true);
    application_end_date->setDisplayFormat(date_format);
    auto *application_start_time = new QTimeEdit(QTime(0, 0), when_group);
    application_start_time->setDisplayFormat(QStringLiteral("HH:mm"));
    auto *application_end_time = new QTimeEdit(QTime(23, 59), when_group);
    application_end_time->setDisplayFormat(QStringLiteral("HH:mm"));
    auto *missing_base_date = new QDateEdit(QDate::currentDate(), when_group);
    missing_base_date->setCalendarPopup(true);
    missing_base_date->setDisplayFormat(date_format);
    auto *missing_base_time = new QComboBox(when_group);
    for (int minutes = 0; minutes < 24 * 60; minutes += 15) {
        const QTime time(minutes / 60, minutes % 60);
        missing_base_time->addItem(time.toString(QStringLiteral("HH:mm")), time.toString(QStringLiteral("HH:mm")));
    }
    missing_base_time->setEditable(true);
    missing_base_time->setMaxVisibleItems(12);
    const auto recurrence_field_width = [&]() {
        int width = 132;
        const QWidget *fields[] = {application_start_date,
                                   application_end_date,
                                   application_start_time,
                                   application_end_time,
                                   missing_base_date,
                                   missing_base_time};
        for (const QWidget *field : fields) {
            if (field != nullptr) {
                width = qMax(width, field->sizeHint().width() + 10);
            }
        }
        return width;
    }();
    for (QWidget *field : {static_cast<QWidget *>(application_start_date),
                           static_cast<QWidget *>(application_end_date),
                           static_cast<QWidget *>(application_start_time),
                           static_cast<QWidget *>(application_end_time),
                           static_cast<QWidget *>(missing_base_date),
                           static_cast<QWidget *>(missing_base_time)}) {
        field->setMinimumWidth(recurrence_field_width);
        field->setMaximumWidth(recurrence_field_width);
    }
    auto *use_start_time_when_missing =
        new QCheckBox(get_text(QStringLiteral("Usar hora inicial quando o item não tiver horário")), when_group);
    auto *lbl_start_date = new QLabel(get_text(QStringLiteral("Data inicial")), when_group);
    auto *lbl_start_time = new QLabel(get_text(QStringLiteral("Hora inicial")), when_group);
    auto *lbl_end_date = new QLabel(get_text(QStringLiteral("Data final")), when_group);
    auto *lbl_end_time = new QLabel(get_text(QStringLiteral("Hora final")), when_group);
    auto *lbl_missing_date = new QLabel(get_text(QStringLiteral("Data para itens sem data")), when_group);
    auto *lbl_missing_time = new QLabel(get_text(QStringLiteral("Hora para itens sem horário")), when_group);
    when_form->addRow(lbl_start_date, application_start_date);
    when_form->addRow(lbl_start_time, application_start_time);
    when_form->addRow(lbl_end_date, application_end_date);
    when_form->addRow(lbl_end_time, application_end_time);
    when_form->addRow(use_start_time_when_missing);
    when_form->addRow(lbl_missing_date, missing_base_date);
    when_form->addRow(lbl_missing_time, missing_base_time);
    when_layout->addLayout(when_form, 0);

    auto *calendar_column = new QVBoxLayout();
    calendar_column->setContentsMargins(0, 0, 0, 0);
    calendar_column->setSpacing(6);
    auto *application_calendar_strip =
        create_multi_month_calendar_strip(when_group, application_start_date->date(), 180);
    auto *application_calendar_filter =
        new CalendarStripResizeFilter(application_calendar_strip, application_calendar_strip->container);
    application_calendar_strip->container->installEventFilter(application_calendar_filter);
    calendar_column->addWidget(application_calendar_strip->container);
    auto *calendar_buttons = new QHBoxLayout();
    auto *set_application_start_button = new QPushButton(get_text(QStringLiteral("Início")), when_group);
    auto *set_application_end_button = new QPushButton(get_text(QStringLiteral("Fim")), when_group);
    set_application_start_button->setMaximumWidth(88);
    set_application_end_button->setMaximumWidth(88);
    calendar_buttons->addWidget(set_application_start_button);
    calendar_buttons->addWidget(set_application_end_button);
    calendar_buttons->addStretch(1);
    calendar_column->addLayout(calendar_buttons);
    when_layout->addLayout(calendar_column, 1);
    when_group->setMinimumSize(when_group->sizeHint());

    auto *preview_group = new QGroupBox(get_text(QStringLiteral("Prévia do que será criado")), &dialog);
    auto *preview_layout = new QVBoxLayout(preview_group);
    auto *preview_table = new QTableWidget(preview_group);
    preview_table->setColumnCount(5);
    preview_table->setHorizontalHeaderLabels({get_text(QStringLiteral("Item")),
                                              get_text(QStringLiteral("Origem encontrada")),
                                              get_text(QStringLiteral("Criar em")),
                                              get_text(QStringLiteral("Recorrência usada")),
                                              get_text(QStringLiteral("Próximas datas"))});
    preview_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    preview_table->setSelectionMode(QAbstractItemView::NoSelection);
    preview_table->setAlternatingRowColors(true);
    preview_table->verticalHeader()->setVisible(false);
    preview_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    preview_table->horizontalHeader()->setStretchLastSection(true);
    preview_table->setColumnWidth(0, 180);
    preview_table->setColumnWidth(1, 140);
    preview_table->setColumnWidth(2, 120);
    preview_table->setColumnWidth(3, 140);
    preview_table->setColumnWidth(4, 180);
    preview_table->setMinimumHeight(150);
    preview_layout->addWidget(preview_table);

    auto *left_column_widget = new QWidget(&dialog);
    auto *left_column = new QVBoxLayout(left_column_widget);
    left_column->setContentsMargins(0, 0, 0, 0);
    left_column->setSpacing(layout->spacing());
    left_column->addWidget(methods_group);
    left_column->addWidget(selection_group, 1);
    left_column->addWidget(settings_group);

    auto *right_column_widget = new QWidget(&dialog);
    auto *right_column = new QVBoxLayout(right_column_widget);
    right_column->setContentsMargins(0, 0, 0, 0);
    right_column->setSpacing(layout->spacing());
    right_column->addWidget(when_group);
    right_column->addWidget(preview_group, 1);

    left_column_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    right_column_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    const int left_column_min_width = left_column_widget->sizeHint().width();
    const int right_column_min_width = right_column_widget->sizeHint().width();
    left_column_widget->setMinimumWidth(left_column_min_width);
    right_column_widget->setMinimumWidth(right_column_min_width);
    when_group->setMinimumWidth(right_column_min_width);

    auto *columns_row = new QHBoxLayout();
    columns_row->setSpacing(layout->spacing());
    columns_row->addWidget(left_column_widget, 1);
    columns_row->addWidget(right_column_widget, 1);
    layout->addLayout(columns_row, 1);

    const auto set_combo_current_data = [](QComboBox *combo, const QString &data) {
        if (combo == nullptr) {
            return;
        }
        const int index = combo->findData(data);
        if (index >= 0) {
            combo->setCurrentIndex(index);
        }
    };
    const auto set_time_combo_current_time = [&](QComboBox *combo, const QTime &time) {
        if (combo == nullptr || !time.isValid()) {
            return;
        }
        const QString value = time.toString(QStringLiteral("HH:mm"));
        int index = combo->findData(value);
        if (index < 0) {
            combo->addItem(value, value);
            index = combo->findData(value);
        }
        if (index >= 0) {
            combo->setCurrentIndex(index);
        }
    };
    const auto time_combo_time = [](const QComboBox *combo) {
        if (combo == nullptr) {
            return QTime();
        }
        QTime time = parse_time(combo->currentData().toString());
        if (!time.isValid()) {
            time = parse_time(combo->currentText());
        }
        return time;
    };
    const auto selected_group_indices = [&]() {
        QSet<int> selected_indices;
        if (!selected_items_radio->isChecked() || item_view->selectionModel() == nullptr) {
            return selected_indices;
        }

        const QModelIndexList rows = item_view->selectionModel()->selectedRows();
        for (const QModelIndex &proxy_index : rows) {
            const QVariantMap entry =
                item_proxy->data(proxy_index, source::models::TaskRecordListModel::RecordRole).toMap();
            const int group_index = entry.value(QStringLiteral("group_index"), -1).toInt();
            if (group_index >= 0 && group_index < groups.size()) {
                selected_indices.insert(group_index);
            }
        }
        return selected_indices;
    };

    const auto group_matches_current_filters = [&](int group_index) {
        if (group_index < 0 || group_index >= groups.size()) {
            return false;
        }
        const QSet<QString> selected_modules = checked_productivity_module_set(module_checks);
        if (selected_modules.isEmpty()) {
            return false;
        }
        const TaskRecordGroup &group = groups.at(group_index);
        if (!group_has_selected_module(group, selected_modules)) {
            return false;
        }
        return include_completed->isChecked() || group_has_open_record(group);
    };

    const auto group_is_considered = [&](int group_index, const QSet<int> &selected_indices) {
        if (!group_matches_current_filters(group_index)) {
            return false;
        }
        return !selected_items_radio->isChecked() || selected_indices.contains(group_index);
    };

    const auto recurrence_date_options = [&](const QDate &base_date,
                                             const QString &recurrence,
                                             const QDateTime &window_start,
                                             const QDateTime &window_end,
                                             const QTime &occurrence_time,
                                             bool include_base_date,
                                             int max_per_item_count) {
        if (!base_date.isValid() || recurrence.trimmed().isEmpty()) {
            return QStringList{get_text(QStringLiteral("Sem data base"))};
        }
        if (!window_start.isValid() || !window_end.isValid() || window_end < window_start) {
            return QStringList{get_text(QStringLiteral("Período inválido"))};
        }

        QStringList dates;
        QDate next = include_base_date ? base_date : next_recurrence_date(base_date, recurrence);
        int guard = 0;
        while (next.isValid() && next <= window_end.date() && dates.size() < max_per_item_count && guard++ < 120) {
            if (recurrence_occurrence_in_window(next, occurrence_time, window_start, window_end)) {
                dates << (occurrence_time.isValid()
                              ? QStringLiteral("%1 %2").arg(next.toString(QStringLiteral("dd/MM/yyyy")),
                                                            occurrence_time.toString(QStringLiteral("HH:mm")))
                              : next.toString(QStringLiteral("dd/MM/yyyy")));
            }
            next = next_recurrence_date(next, recurrence);
        }
        if (dates.isEmpty()) {
            return QStringList{get_text(QStringLiteral("Nenhuma data no período"))};
        }
        return dates;
    };
    struct RecurrenceBaseDate {
        QDate date;
        bool include_base_date = false;
    };
    const auto first_group_date_for_modules = [](const TaskRecordGroup &group, const QStringList &modules) {
        QSet<QString> module_set;
        for (const QString &module : modules) {
            const QString normalized = normalize_module_key(module);
            if (!normalized.isEmpty()) {
                module_set.insert(normalized);
            }
        }

        for (const QVariantMap &record : group.records) {
            if (!module_set.isEmpty() && !module_set.contains(record_module(record))) {
                continue;
            }

            const QDate date = QDate::fromString(date_for_record(record), Qt::ISODate);
            if (date.isValid()) {
                return date;
            }
        }
        return QDate();
    };
    const auto recurrence_base_date_for_group = [&](const TaskRecordGroup &group, const QStringList &modules) {
        const QDate date = first_group_date_for_modules(group, modules);
        if (date.isValid()) {
            return RecurrenceBaseDate{date, false};
        }
        if (use_start_time_when_missing->isChecked() && missing_base_date->date().isValid()) {
            return RecurrenceBaseDate{missing_base_date->date(), true};
        }
        return RecurrenceBaseDate{};
    };
    const auto first_group_time_for_modules = [](const TaskRecordGroup &group, const QStringList &modules) {
        QSet<QString> module_set;
        for (const QString &module : modules) {
            const QString normalized = normalize_module_key(module);
            if (!normalized.isEmpty()) {
                module_set.insert(normalized);
            }
        }

        for (const QVariantMap &record : group.records) {
            if (!module_set.isEmpty() && !module_set.contains(record_module(record))) {
                continue;
            }

            const QTime time = parse_time(time_for_record(record));
            if (time.isValid()) {
                return time;
            }
        }
        return QTime();
    };
    const auto occurrence_time_for_group = [&](const TaskRecordGroup &group, const QStringList &modules) {
        QTime occurrence_time = first_group_time_for_modules(group, modules);
        if (!occurrence_time.isValid() && use_start_time_when_missing->isChecked()) {
            occurrence_time = time_combo_time(missing_base_time);
        }
        return occurrence_time;
    };

    QSet<QString> saved_recurrence_selected_source_ids;
    const auto save_recurrence_dialog_settings = [&]() {
        QJsonObject settings;
        QJsonArray modules;
        for (const QString &module : ordered_modules_from_set(checked_productivity_module_set(module_checks))) {
            modules.append(module);
        }
        settings.insert(QStringLiteral("modules"), modules);
        settings.insert(QStringLiteral("selection_mode"),
                        selected_items_radio->isChecked() ? QStringLiteral("selected") : QStringLiteral("all"));
        settings.insert(QStringLiteral("item_filter"), item_filter->text());

        QJsonArray selected_ids;
        const QSet<int> selected_indices = selected_group_indices();
        for (int index : selected_indices) {
            if (index >= 0 && index < groups.size()) {
                selected_ids.append(source_id_for_group(groups.at(index)));
            }
        }
        settings.insert(QStringLiteral("selected_source_ids"), selected_ids);

        settings.insert(QStringLiteral("horizon_days"), horizon_days->value());
        settings.insert(QStringLiteral("max_per_item"), max_per_item->value());
        settings.insert(QStringLiteral("include_completed"), include_completed->isChecked());
        settings.insert(QStringLiteral("avoid_duplicates"), avoid_duplicates->isChecked());
        settings.insert(QStringLiteral("apply_default"), apply_default->isChecked());
        settings.insert(QStringLiteral("default_recurrence"), default_recurrence->currentData().toString());
        settings.insert(QStringLiteral("application_start_date"), application_start_date->date().toString(Qt::ISODate));
        settings.insert(QStringLiteral("application_end_date"), application_end_date->date().toString(Qt::ISODate));
        settings.insert(QStringLiteral("application_start_time"),
                        application_start_time->time().toString(QStringLiteral("HH:mm")));
        settings.insert(QStringLiteral("application_end_time"),
                        application_end_time->time().toString(QStringLiteral("HH:mm")));
        settings.insert(QStringLiteral("use_start_time_when_missing"), use_start_time_when_missing->isChecked());
        settings.insert(QStringLiteral("missing_base_date"), missing_base_date->date().toString(Qt::ISODate));
        const QTime missing_time = time_combo_time(missing_base_time);
        settings.insert(QStringLiteral("missing_base_time"),
                        missing_time.isValid() ? missing_time.toString(QStringLiteral("HH:mm"))
                                               : missing_base_time->currentText().trimmed());
        write_json_object(recurring_tasks_settings_path(), settings);
    };
    const auto load_recurrence_dialog_settings = [&]() {
        const QJsonObject settings = read_json_object(recurring_tasks_settings_path());
        if (settings.isEmpty()) {
            return;
        }

        const QJsonArray modules = settings.value(QStringLiteral("modules")).toArray();
        if (!modules.isEmpty()) {
            for (QCheckBox *check : module_checks) {
                if (check != nullptr) {
                    check->setChecked(false);
                }
            }
            for (const QJsonValue &value : modules) {
                QCheckBox *check = module_checks.value(normalize_module_key(value.toString()));
                if (check != nullptr) {
                    check->setChecked(true);
                }
            }
            ensure_any_productivity_module_checked(module_checks);
        }

        selected_items_radio->setChecked(settings.value(QStringLiteral("selection_mode")).toString() == QStringLiteral("selected"));
        all_items_radio->setChecked(!selected_items_radio->isChecked());
        item_filter->setText(settings.value(QStringLiteral("item_filter")).toString());
        item_proxy->setFilterFixedString(item_filter->text());

        saved_recurrence_selected_source_ids.clear();
        const QJsonArray selected_ids = settings.value(QStringLiteral("selected_source_ids")).toArray();
        for (const QJsonValue &value : selected_ids) {
            const QString id = value.toString().trimmed();
            if (!id.isEmpty()) {
                saved_recurrence_selected_source_ids.insert(id);
            }
        }

        horizon_days->setValue(qBound(horizon_days->minimum(),
                                      settings.value(QStringLiteral("horizon_days")).toInt(horizon_days->value()),
                                      horizon_days->maximum()));
        max_per_item->setValue(qBound(max_per_item->minimum(),
                                      settings.value(QStringLiteral("max_per_item")).toInt(max_per_item->value()),
                                      max_per_item->maximum()));
        include_completed->setChecked(settings.value(QStringLiteral("include_completed")).toBool(include_completed->isChecked()));
        avoid_duplicates->setChecked(settings.value(QStringLiteral("avoid_duplicates")).toBool(avoid_duplicates->isChecked()));
        apply_default->setChecked(settings.value(QStringLiteral("apply_default")).toBool(apply_default->isChecked()));
        set_combo_current_data(default_recurrence, settings.value(QStringLiteral("default_recurrence")).toString());

        const QDate start = QDate::fromString(settings.value(QStringLiteral("application_start_date")).toString(), Qt::ISODate);
        const QDate end = QDate::fromString(settings.value(QStringLiteral("application_end_date")).toString(), Qt::ISODate);
        if (start.isValid()) {
            application_start_date->setDate(start);
            set_calendar_strip_selected_date(application_calendar_strip, start);
        }
        if (end.isValid()) {
            application_end_date->setDate(end);
        }
        const QTime start_time = parse_time(settings.value(QStringLiteral("application_start_time")).toString());
        const QTime end_time = parse_time(settings.value(QStringLiteral("application_end_time")).toString());
        if (start_time.isValid()) {
            application_start_time->setTime(start_time);
        }
        if (end_time.isValid()) {
            application_end_time->setTime(end_time);
        }
        use_start_time_when_missing->setChecked(
            settings.value(QStringLiteral("use_start_time_when_missing")).toBool(use_start_time_when_missing->isChecked()));
        const QDate missing_date =
            QDate::fromString(settings.value(QStringLiteral("missing_base_date")).toString(), Qt::ISODate);
        missing_base_date->setDate(missing_date.isValid() ? missing_date : application_start_date->date());
        const QTime missing_time = parse_time(settings.value(QStringLiteral("missing_base_time")).toString());
        set_time_combo_current_time(missing_base_time,
                                    missing_time.isValid() ? missing_time : application_start_time->time());
    };

    QPointer<QFrame> preview_list_popup;
    const auto update_summary = [&]() {
        if (!preview_list_popup.isNull()) {
            preview_list_popup->close();
            preview_list_popup->deleteLater();
            preview_list_popup.clear();
        }

        const QSet<QString> selected = checked_productivity_module_set(module_checks);
        const QString selected_text =
            selected.isEmpty() ? get_text(QStringLiteral("Nenhum método selecionado"))
                               : module_list_label(ordered_modules_from_set(selected));
        const QSet<int> selected_indices = selected_group_indices();
        int considered_count = 0;
        int recurring_count = 0;
        for (int i = 0; i < groups.size(); ++i) {
            if (!group_is_considered(i, selected_indices)) {
                continue;
            }
            ++considered_count;
            if (!group_recurrence(groups.at(i)).isEmpty()) {
                ++recurring_count;
            }
        }
        summary->setText(get_text(QStringLiteral(
                             "%1 item(ns) serão considerados. %2 já têm recorrência definida. Origem/destino selecionados: %3."))
                             .arg(considered_count)
                             .arg(recurring_count)
                             .arg(selected_text));

        preview_table->setRowCount(0);
        int shown = 0;
        const QFontMetrics preview_metrics(preview_table->font());
        const auto header_width = [&](int column, const QString &fallback) {
            const QString header_text = preview_table->horizontalHeaderItem(column) != nullptr
                                            ? preview_table->horizontalHeaderItem(column)->text()
                                            : fallback;
            return preview_metrics.horizontalAdvance(header_text) + 56;
        };
        int source_column_width = header_width(1, get_text(QStringLiteral("Origem encontrada")));
        int destination_column_width = header_width(2, get_text(QStringLiteral("Criar em")));
        int dates_column_width = header_width(4, get_text(QStringLiteral("Próximas datas")));
        const auto module_labels_for_keys = [](const QStringList &modules) {
            QStringList labels;
            for (const QString &module : modules) {
                labels.append(module_label(module));
            }
            return labels;
        };
        const auto set_preview_list_cell =
            [&](int row, int column, const QStringList &values, int minimum_width, int &column_width) {
                const QStringList clean_values = [&]() {
                    QStringList cleaned;
                    for (const QString &value : values) {
                        const QString text = value.trimmed();
                        if (!text.isEmpty() && !cleaned.contains(text)) {
                            cleaned.append(text);
                        }
                    }
                    return cleaned;
                }();
                const QString collapsed_text =
                    clean_values.isEmpty()
                        ? QString()
                        : clean_values.first() + (clean_values.size() > 1 ? QStringLiteral(" ...") : QString());
                const QString tooltip = clean_values.join(QLatin1Char('\n'));
                column_width = qMax(column_width, preview_metrics.horizontalAdvance(collapsed_text) + 56);

                if (clean_values.size() <= 1) {
                    auto *item = new QTableWidgetItem(collapsed_text);
                    item->setToolTip(tooltip);
                    preview_table->setItem(row, column, item);
                    return;
                }

                auto *item = new QTableWidgetItem(QString());
                item->setToolTip(tooltip);
                preview_table->setItem(row, column, item);

                auto *button = new PreviewListToolButton(collapsed_text, preview_table);
                button->setToolTip(tooltip);
                QObject::connect(button, &QToolButton::clicked, button, [&, button, clean_values, minimum_width]() {
                    if (!preview_list_popup.isNull()) {
                        preview_list_popup->close();
                        preview_list_popup->deleteLater();
                        preview_list_popup.clear();
                    }

                    auto *popup = new QFrame(nullptr, Qt::Popup | Qt::FramelessWindowHint);
                    popup->setFrameShape(QFrame::StyledPanel);
                    popup->setAttribute(Qt::WA_DeleteOnClose);
                    auto *popup_layout = new QVBoxLayout(popup);
                    popup_layout->setContentsMargins(2, 2, 2, 2);
                    popup_layout->setSpacing(0);

                    auto *list = new QListWidget(popup);
                    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                    list->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                    list->setSelectionMode(QAbstractItemView::NoSelection);
                    list->addItems(clean_values);
                    popup_layout->addWidget(list);

                    const QFontMetrics popup_metrics(list->font());
                    int content_width = qMax(button->width(), minimum_width);
                    for (const QString &value : clean_values) {
                        content_width = qMax(content_width, popup_metrics.horizontalAdvance(value) + 44);
                    }
                    content_width = qMin(content_width, 520);

                    const QPoint top_left = button->mapToGlobal(QPoint(0, button->height()));
                    QScreen *screen = QGuiApplication::screenAt(top_left);
                    if (screen == nullptr) {
                        screen = QGuiApplication::primaryScreen();
                    }
                    const QRect available = screen != nullptr ? screen->availableGeometry() : QRect();
                    const int available_below =
                        available.isValid() ? qMax(48, available.bottom() - top_left.y() + 1) : 180;
                    const int row_height = qMax(list->sizeHintForRow(0), popup_metrics.height() + 8);
                    const int desired_height =
                        qMin(clean_values.size(), 8) * row_height + popup_layout->contentsMargins().top()
                        + popup_layout->contentsMargins().bottom() + 4;
                    const int popup_height = qMin(qMax(48, desired_height), available_below);

                    popup->setGeometry(QRect(top_left, QSize(content_width, popup_height)));
                    preview_list_popup = popup;
                    QObject::connect(popup, &QObject::destroyed, &dialog, [&]() {
                        preview_list_popup.clear();
                    });
                    popup->show();
                    popup->raise();
                    list->setFocus(Qt::PopupFocusReason);
                });
                preview_table->setCellWidget(row, column, button);
            };
        const QDateTime window_start =
            recurrence_window_start(application_start_date->date(), application_start_time->time());
        const QDateTime window_end =
            recurrence_window_end(application_end_date->date(), application_end_time->time());
        const QString fallback_recurrence =
            apply_default->isChecked() ? default_recurrence->currentData().toString() : QString();
        for (int i = 0; i < groups.size(); ++i) {
            if (!group_is_considered(i, selected_indices)) {
                continue;
            }
            const TaskRecordGroup &group = groups.at(i);

            QString recurrence = group_recurrence(group);
            QString recurrence_label = recurrence_user_label(recurrence);
            if (recurrence.isEmpty() && !fallback_recurrence.isEmpty()) {
                recurrence = fallback_recurrence;
                recurrence_label = get_text(QStringLiteral("Padrão: %1")).arg(recurrence_user_label(recurrence));
            }
            if (recurrence.isEmpty()) {
                continue;
            }

            const QStringList destination_modules = ordered_modules_for_group(group, selected);
            if (destination_modules.isEmpty()) {
                continue;
            }

            const int row = preview_table->rowCount();
            preview_table->insertRow(row);
            preview_table->setItem(row, 0, new QTableWidgetItem(title_for_record(group.primary)));
            const QStringList source_modules = ordered_modules_for_group(group, selected);
            set_preview_list_cell(row,
                                  1,
                                  module_labels_for_keys(source_modules),
                                  140,
                                  source_column_width);
            set_preview_list_cell(row,
                                  2,
                                  module_labels_for_keys(destination_modules),
                                  120,
                                  destination_column_width);
            preview_table->setItem(row, 3, new QTableWidgetItem(recurrence_label));
            const QTime occurrence_time = occurrence_time_for_group(group, source_modules);
            const RecurrenceBaseDate base_date = recurrence_base_date_for_group(group, source_modules);
            const QStringList recurrence_dates = recurrence_date_options(base_date.date,
                                                                         recurrence,
                                                                         window_start,
                                                                         window_end,
                                                                         occurrence_time,
                                                                         base_date.include_base_date,
                                                                         max_per_item->value());
            set_preview_list_cell(row, 4, recurrence_dates, 150, dates_column_width);
            ++shown;
        }
        if (shown > 0) {
            preview_table->setColumnWidth(1, qMax(preview_table->columnWidth(1), source_column_width));
            preview_table->setColumnWidth(2, qMax(preview_table->columnWidth(2), destination_column_width));
            preview_table->setColumnWidth(4, qMax(preview_table->columnWidth(4), dates_column_width));
        } else {
            preview_table->insertRow(0);
            preview_table->setSpan(0, 0, 1, preview_table->columnCount());
            preview_table->setItem(0,
                                   0,
                                   new QTableWidgetItem(get_text(QStringLiteral(
                                       "Nenhuma tarefa pronta para gerar ocorrências com as configurações atuais."))));
        }
    };

    const auto update_item_model = [&]() {
        const QSet<int> previously_selected = selected_group_indices();
        QVariantList entries;
        for (int i = 0; i < groups.size(); ++i) {
            if (!group_matches_current_filters(i)) {
                continue;
            }
            QVariantMap entry = display_entry_for_group(groups.at(i));
            entry.insert(QStringLiteral("group_index"), i);
            entries.append(entry);
        }

        item_model->set_records(entries);
        item_proxy->setFilterFixedString(item_filter->text());
        if (item_view->selectionModel() != nullptr && !previously_selected.isEmpty()) {
            QItemSelection restored_selection;
            for (int row = 0; row < item_model->rowCount(); ++row) {
                const QVariantMap entry = item_model->record_at(row);
                const int group_index = entry.value(QStringLiteral("group_index"), -1).toInt();
                if (!previously_selected.contains(group_index)) {
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

    const auto update_missing_datetime_controls = [&]() {
        const bool enabled = use_start_time_when_missing->isChecked();
        missing_base_date->setEnabled(enabled);
        missing_base_time->setEnabled(enabled);
    };

    const auto restore_saved_recurrence_item_selection = [&]() {
        if (saved_recurrence_selected_source_ids.isEmpty() || item_view->selectionModel() == nullptr) {
            return;
        }
        QItemSelection restored_selection;
        for (int row = 0; row < item_model->rowCount(); ++row) {
            const QVariantMap entry = item_model->record_at(row);
            const int group_index = entry.value(QStringLiteral("group_index"), -1).toInt();
            if (group_index < 0 || group_index >= groups.size()) {
                continue;
            }
            if (!saved_recurrence_selected_source_ids.contains(source_id_for_group(groups.at(group_index)))) {
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
    };

    load_recurrence_dialog_settings();
    update_item_model();
    restore_saved_recurrence_item_selection();
    update_missing_datetime_controls();
    update_selection_controls();

    for (QCheckBox *check : module_checks) {
        QObject::connect(check, &QCheckBox::toggled, &dialog, [&](bool) { update_item_model(); });
    }
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
        QItemSelection visible_selection;
        for (int row = 0; row < item_proxy->rowCount(); ++row) {
            const QModelIndex index = item_proxy->index(row, 0);
            if (index.isValid()) {
                visible_selection.select(index, index);
            }
        }
        {
            const QSignalBlocker blocker(item_view->selectionModel());
            item_view->selectionModel()->select(visible_selection,
                                                QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        update_summary();
    });
    QObject::connect(clear_selection_button, &QPushButton::clicked, &dialog, [&]() {
        if (item_view->selectionModel() != nullptr) {
            const QSignalBlocker blocker(item_view->selectionModel());
            item_view->selectionModel()->clearSelection();
        }
        update_summary();
    });
    bool syncing_application_range = false;
    const auto sync_horizon_from_application_dates = [&]() {
        if (syncing_application_range) {
            return;
        }
        syncing_application_range = true;
        const int days = qMax(1, application_start_date->date().daysTo(application_end_date->date()));
        horizon_days->setValue(qBound(horizon_days->minimum(), days, horizon_days->maximum()));
        syncing_application_range = false;
        update_summary();
    };
    QDate previous_application_start_date = application_start_date->date();
    QTime previous_application_start_time = application_start_time->time();

    QObject::connect(horizon_days, qOverload<int>(&QSpinBox::valueChanged), &dialog, [&](int days) {
        if (syncing_application_range) {
            return;
        }
        syncing_application_range = true;
        application_end_date->setDate(application_start_date->date().addDays(days));
        set_calendar_strip_selected_date(application_calendar_strip, application_end_date->date());
        syncing_application_range = false;
        update_summary();
    });
    QObject::connect(application_start_date, &QDateEdit::dateChanged, &dialog, [&](const QDate &date) {
        if (missing_base_date->date() == previous_application_start_date) {
            const QSignalBlocker blocker(missing_base_date);
            missing_base_date->setDate(date);
        }
        previous_application_start_date = date;
        if (application_end_date->date() < date) {
            application_end_date->setDate(date);
        }
        set_calendar_strip_selected_date(application_calendar_strip, date);
        sync_horizon_from_application_dates();
    });
    QObject::connect(application_end_date, &QDateEdit::dateChanged, &dialog, [&](const QDate &date) {
        if (date < application_start_date->date()) {
            application_start_date->setDate(date);
        }
        sync_horizon_from_application_dates();
    });
    QObject::connect(application_start_time, &QTimeEdit::timeChanged, &dialog, [&](const QTime &time) {
        if (time_combo_time(missing_base_time) == previous_application_start_time) {
            const QSignalBlocker blocker(missing_base_time);
            set_time_combo_current_time(missing_base_time, time);
        }
        previous_application_start_time = time;
        update_summary();
    });
    QObject::connect(application_end_time, &QTimeEdit::timeChanged, &dialog, [&](const QTime &) { update_summary(); });
    QObject::connect(missing_base_date, &QDateEdit::dateChanged, &dialog, [&](const QDate &) { update_summary(); });
    QObject::connect(missing_base_time, qOverload<int>(&QComboBox::currentIndexChanged), &dialog, [&](int) {
        update_summary();
    });
    QObject::connect(missing_base_time, &QComboBox::editTextChanged, &dialog, [&](const QString &) { update_summary(); });
    QObject::connect(use_start_time_when_missing, &QCheckBox::toggled, &dialog, [&](bool) {
        update_missing_datetime_controls();
        update_summary();
    });
    QObject::connect(set_application_start_button, &QPushButton::clicked, &dialog, [&]() {
        application_start_date->setDate(calendar_strip_selected_date(application_calendar_strip));
        update_summary();
    });
    QObject::connect(set_application_end_button, &QPushButton::clicked, &dialog, [&]() {
        application_end_date->setDate(calendar_strip_selected_date(application_calendar_strip));
        update_summary();
    });
    QObject::connect(max_per_item, qOverload<int>(&QSpinBox::valueChanged), &dialog, [&](int) { update_summary(); });
    QObject::connect(include_completed, &QCheckBox::toggled, &dialog, [&](bool) { update_item_model(); });
    QObject::connect(apply_default, &QCheckBox::toggled, &dialog, [&](bool) { update_summary(); });
    QObject::connect(default_recurrence, qOverload<int>(&QComboBox::currentIndexChanged), &dialog, [&](int) {
        update_summary();
    });
    const auto restore_recurrence_defaults = [&]() {
        for (QCheckBox *check : module_checks) {
            if (check != nullptr) {
                check->setChecked(true);
            }
        }

        all_items_radio->setChecked(true);
        item_filter->clear();
        saved_recurrence_selected_source_ids.clear();
        if (item_view->selectionModel() != nullptr) {
            item_view->selectionModel()->clearSelection();
        }

        syncing_application_range = true;
        const QDate today = QDate::currentDate();
        application_start_date->setDate(today);
        application_end_date->setDate(today.addDays(30));
        horizon_days->setValue(30);
        syncing_application_range = false;
        set_calendar_strip_selected_date(application_calendar_strip, today);

        application_start_time->setTime(QTime(0, 0));
        application_end_time->setTime(QTime(23, 59));
        missing_base_date->setDate(today);
        set_combo_current_data(missing_base_time, QStringLiteral("00:00"));
        max_per_item->setValue(8);
        include_completed->setChecked(false);
        avoid_duplicates->setChecked(true);
        apply_default->setChecked(false);
        set_combo_current_data(default_recurrence, QStringLiteral("weekly"));
        use_start_time_when_missing->setChecked(false);

        update_item_model();
        update_missing_datetime_controls();
        update_responsive_grid(methods_grid.get());
        update_calendar_strip(application_calendar_strip);
    };

    const auto generate_occurrences = [&]() {
        const QSet<QString> selected_modules = checked_productivity_module_set(module_checks);
        if (selected_modules.isEmpty()) {
            QMessageBox::warning(&dialog,
                                 get_text(QStringLiteral("Recorrências")),
                                 get_text(QStringLiteral("Selecione pelo menos um método.")));
            return;
        }
        const QSet<int> selected_indices_for_accept = selected_group_indices();
        if (selected_items_radio->isChecked() && selected_indices_for_accept.isEmpty()) {
            QMessageBox::warning(&dialog,
                                 get_text(QStringLiteral("Recorrências")),
                                 get_text(QStringLiteral("Selecione pelo menos uma tarefa ou conceito.")));
            return;
        }

        QSet<QString> existing = existing_recurrence_keys(unified_records, map_backup);
        const QDateTime selected_window_start =
            recurrence_window_start(application_start_date->date(), application_start_time->time());
        const QDateTime selected_window_end =
            recurrence_window_end(application_end_date->date(), application_end_time->time());
        if (!selected_window_start.isValid() || !selected_window_end.isValid() || selected_window_end < selected_window_start) {
            QMessageBox::warning(&dialog,
                                 get_text(QStringLiteral("Recorrências")),
                                 get_text(QStringLiteral("O período de aplicação é inválido.")));
            return;
        }
        const QString fallback_recurrence =
            apply_default->isChecked() ? default_recurrence->currentData().toString() : QString();
        const bool use_start_time_for_missing_occurrences = use_start_time_when_missing->isChecked();

        QVariantList additions;
        QJsonObject updated_map = map_backup;
        QJsonArray nodes = updated_map.value(QStringLiteral("nodes")).toArray();
        int nodes_added = 0;
        QMap<QString, int> created_counts;

        for (int i = 0; i < groups.size(); ++i) {
            if (!group_is_considered(i, selected_indices_for_accept)) {
                continue;
            }
            const TaskRecordGroup &group = groups.at(i);

            QString recurrence = group_recurrence(group);
            if (recurrence.isEmpty()) {
                recurrence = fallback_recurrence;
            }
            if (recurrence.isEmpty()) {
                continue;
            }

            QStringList modules = ordered_modules_for_group(group, selected_modules);
            if (modules.isEmpty()) {
                continue;
            }
            const RecurrenceBaseDate base_date = recurrence_base_date_for_group(group, modules);
            if (!base_date.date.isValid()) {
                continue;
            }

            const QDate until = group_recurrence_until(group);
            const QString source_id = source_id_for_group(group);
            const QString source_modules = module_list_label(ordered_modules_for_group(group));
            QDate next = base_date.include_base_date ? base_date.date : next_recurrence_date(base_date.date, recurrence);
            int generated_for_item = 0;
            int guard = 0;
            QTime occurrence_time = first_group_time_for_modules(group, modules);
            if (!occurrence_time.isValid() && use_start_time_for_missing_occurrences) {
                occurrence_time = time_combo_time(missing_base_time);
            }
            while (next.isValid() && next <= selected_window_end.date() && generated_for_item < max_per_item->value()
                   && guard++ < 120) {
                if (until.isValid() && next > until) {
                    break;
                }

                if (recurrence_occurrence_in_window(next, occurrence_time, selected_window_start, selected_window_end)) {
                    const QString occurrence_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                    for (const QString &module : modules) {
                        const QVariantMap source_record = record_for_group_module(group, module);
                        const QString key = recurrence_duplicate_key(module,
                                                                     source_id,
                                                                     title_for_record(source_record),
                                                                     next.toString(Qt::ISODate));
                        if (avoid_duplicates->isChecked() && existing.contains(key)) {
                            continue;
                        }

                        QString occurrence_time_override;
                        if (!parse_time(time_for_record(source_record)).isValid() && use_start_time_for_missing_occurrences) {
                            const QTime fallback_time = time_combo_time(missing_base_time);
                            if (fallback_time.isValid()) {
                                occurrence_time_override = fallback_time.toString(QStringLiteral("HH:mm"));
                            }
                        }
                        additions.append(recurring_record_copy(source_record,
                                                              module,
                                                              next,
                                                              occurrence_id,
                                                              source_id,
                                                              occurrence_time_override));
                        existing.insert(key);
                        created_counts[module] += 1;

                        if (module == QStringLiteral("mapamental")) {
                            nodes.append(recurring_map_node_for_record(source_record,
                                                                       next,
                                                                       occurrence_id,
                                                                       source_id,
                                                                       source_modules,
                                                                       recurrence,
                                                                       nodes.size(),
                                                                       occurrence_time_override));
                            ++nodes_added;
                        }
                    }
                    ++generated_for_item;
                }

                next = next_recurrence_date(next, recurrence);
            }
        }

        if (additions.isEmpty() && nodes_added == 0) {
            QMessageBox::information(&dialog,
                                     get_text(QStringLiteral("Recorrências")),
                                     get_text(QStringLiteral("Nenhuma ocorrência nova foi gerada.")));
            return;
        }

        RecurringTasksUndoSnapshot undo_snapshot;
        undo_snapshot.records_before = unified_records;
        undo_snapshot.mapamental_backup_before = map_backup;
        for (auto it = created_counts.constBegin(); it != created_counts.constEnd(); ++it) {
            if (it.value() > 0) {
                undo_snapshot.affected_modules.insert(it.key());
            }
        }
        undo_snapshot.created_at = QDateTime::currentDateTime();
        g_recurring_tasks_undo = undo_snapshot;
        save_recurring_tasks_undo_snapshot(undo_snapshot);

        bool saved_records = true;
        if (!additions.isEmpty()) {
            QVariantList merged = unified_records;
            for (const QVariant &record : additions) {
                merged.append(record);
            }
            if (!fallback_recurrence.isEmpty()) {
                for (int i = 0; i < groups.size(); ++i) {
                    if (!group_is_considered(i, selected_indices_for_accept)) continue;
                    const TaskRecordGroup &group = groups.at(i);
                    if (!group_recurrence(group).isEmpty()) continue;
                    const QString source_id = source_id_for_group(group);
                    for (int r = 0; r < merged.size(); ++r) {
                        QVariantMap rec = merged.at(r).toMap();
                        if (record_source_id(rec, QStringLiteral("productivity")) == source_id || rec.value(QStringLiteral("id")).toString().trimmed() == source_id) {
                            rec.insert(QStringLiteral("recurrence"), fallback_recurrence);
                            rec.insert(QStringLiteral("recorrencia"), fallback_recurrence);
                            merged[r] = rec;
                        }
                    }
                }
            }
            saved_records = source::utils::UnifiedTaskStore::save_records(app->tasks_path, merged);
        }

        bool saved_map = true;
        if (nodes_added > 0) {
            updated_map.insert(QStringLiteral("version"), 2);
            updated_map.insert(QStringLiteral("saved_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
            updated_map.insert(QStringLiteral("nodes"), nodes);
            if (!updated_map.contains(QStringLiteral("connections"))) {
                updated_map.insert(QStringLiteral("connections"), QJsonArray{});
            }
            saved_map = write_json_object(mapamental_backup_path(), updated_map);
        }

        if (saved_records) {
            app->reload_all_modules_from_shared_store();
        }
        if (saved_map && nodes_added > 0 && app->has_mapamental_plugin()) {
            app->reload_mapamental_from_backup();
        }
        if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
            app->calendar_pane->calendar_panel->update_task_list();
        }

        save_recurrence_dialog_settings();

        unified_records = records_for_app(app);
        map_backup = read_json_object(mapamental_backup_path());
        groups = collect_productivity_task_groups(app, unified_records, map_backup, all_modules);
        update_item_model();
        update_summary();

        QStringList lines;
        int total_created = 0;
        for (const QString &module : productivity_modules_order()) {
            const int count = created_counts.value(module);
            if (count > 0) {
                total_created += count;
                lines << get_text(QStringLiteral("%1: %2")).arg(module_label(module)).arg(count);
            }
        }

        const QString header = (saved_records && saved_map)
                                   ? get_text(QStringLiteral("%1 ocorrência(s) gerada(s).")).arg(total_created)
                                   : get_text(QStringLiteral("Ocorrências geradas com avisos."));
        QMessageBox result_box(&dialog);
        result_box.setIcon(QMessageBox::Information);
        result_box.setWindowTitle(get_text(QStringLiteral("Recorrências")));
        result_box.setText(lines.isEmpty() ? header : QStringLiteral("%1\n%2").arg(header, lines.join(QLatin1Char('\n'))));
        QPushButton *restore_button = result_box.addButton(get_text(QStringLiteral("Restaurar tarefas anteriores")),
                                                           QMessageBox::ActionRole);
        result_box.addButton(QMessageBox::Ok);
        translate_standard_buttons(app, &result_box);
        result_box.exec();
        if (result_box.clickedButton() == restore_button) {
            if (show_recurring_tasks_restore_dialog(app)) {
                unified_records = records_for_app(app);
                map_backup = read_json_object(mapamental_backup_path());
                groups = collect_productivity_task_groups(app, unified_records, map_backup, all_modules);
                update_item_model();
                update_summary();
            }
        }
    };

    const auto save_definitions = [&]() {
        save_recurrence_dialog_settings();
        const QString fallback_recurrence =
            apply_default->isChecked() ? default_recurrence->currentData().toString() : QString();
        if (!fallback_recurrence.isEmpty()) {
            QVariantList records = source::utils::UnifiedTaskStore::load_records(app->tasks_path);
            const QSet<int> selected_indices = selected_group_indices();
            bool modified_any = false;
            for (int i = 0; i < groups.size(); ++i) {
                if (!group_is_considered(i, selected_indices)) continue;
                const TaskRecordGroup &group = groups.at(i);
                if (!group_recurrence(group).isEmpty()) continue;
                const QString source_id = source_id_for_group(group);
                for (int r = 0; r < records.size(); ++r) {
                    QVariantMap rec = records.at(r).toMap();
                    if (record_source_id(rec, QStringLiteral("productivity")) == source_id || rec.value(QStringLiteral("id")).toString().trimmed() == source_id) {
                        rec.insert(QStringLiteral("recurrence"), fallback_recurrence);
                        rec.insert(QStringLiteral("recorrencia"), fallback_recurrence);
                        records[r] = rec;
                        modified_any = true;
                    }
                }
            }
            if (modified_any) {
                source::utils::UnifiedTaskStore::save_records(app->tasks_path, records);
                app->reload_all_modules_from_shared_store();
                unified_records = records_for_app(app);
                map_backup = read_json_object(mapamental_backup_path());
                groups = collect_productivity_task_groups(app, unified_records, map_backup, all_modules);
                update_item_model();
                update_summary();
            }
        }
        QMessageBox::information(&dialog,
                                 get_text(QStringLiteral("Recorrências")),
                                 get_text(QStringLiteral("Definições registradas.")));
    };

    auto *buttons_widget = new QWidget(&dialog);
    auto *buttons_layout = new QHBoxLayout(buttons_widget);
    buttons_layout->setContentsMargins(0, 0, 0, 0);
    buttons_layout->setSpacing(6);

    auto *restore_defaults_button = new QPushButton(get_text(QStringLiteral("Restaurar padrão")), buttons_widget);
    auto *restore_original_button = new QPushButton(get_text(QStringLiteral("Restaurar tarefas anteriores")), buttons_widget);
    auto *save_settings_button = new QPushButton(get_text(QStringLiteral("Salvar definições")), buttons_widget);
    auto *apply_button = new QPushButton(get_text(QStringLiteral("Gerar ocorrências")), buttons_widget);
    auto *close_button = new QPushButton(get_text(QStringLiteral("Fechar")), buttons_widget);

    buttons_layout->addWidget(restore_defaults_button);
    buttons_layout->addWidget(restore_original_button);
    buttons_layout->addStretch(1);
    buttons_layout->addWidget(save_settings_button);
    buttons_layout->addWidget(apply_button);
    buttons_layout->addWidget(close_button);
    layout->addWidget(buttons_widget);

    QObject::connect(apply_button, &QPushButton::clicked, &dialog, generate_occurrences);
    QObject::connect(restore_original_button, &QPushButton::clicked, &dialog, [&]() {
        if (show_recurring_tasks_restore_dialog(app)) {
            unified_records = records_for_app(app);
            map_backup = read_json_object(mapamental_backup_path());
            groups = collect_productivity_task_groups(app, unified_records, map_backup, all_modules);
            update_item_model();
            update_summary();
        }
    });
    QObject::connect(restore_defaults_button, &QPushButton::clicked, &dialog, restore_recurrence_defaults);
    QObject::connect(save_settings_button, &QPushButton::clicked, &dialog, save_definitions);
    QObject::connect(close_button, &QPushButton::clicked, &dialog, &QDialog::reject);

    const auto retranslate_ui = [&]() {
        dialog.setWindowTitle(get_text(QStringLiteral("Gerar próximas recorrências")));
        guide->setText(get_text(QStringLiteral(
            "Origem: os métodos marcados abaixo serão lidos. Destino: as novas ocorrências serão criadas nos mesmos métodos em que cada item já existe; se o item for compartilhado, a ocorrência também será compartilhada nos métodos marcados.")));
        methods_group->setTitle(get_text(QStringLiteral("Origem e destino dos métodos")));
        selection_group->setTitle(get_text(QStringLiteral("Itens")));
        all_items_radio->setText(get_text(QStringLiteral("Todos filtrados")));
        selected_items_radio->setText(get_text(QStringLiteral("Somente selecionados")));
        item_filter->setPlaceholderText(get_text(QStringLiteral("Pesquisar item")));
        select_visible_button->setText(get_text(QStringLiteral("Selecionar visíveis")));
        clear_selection_button->setText(get_text(QStringLiteral("Limpar")));
        settings_group->setTitle(get_text(QStringLiteral("Configurações")));
        lbl_horizon->setText(get_text(QStringLiteral("Alcance (dias)")));
        lbl_max->setText(get_text(QStringLiteral("Máximo por item")));
        include_completed->setText(get_text(QStringLiteral("Incluir itens concluídos")));
        avoid_duplicates->setText(get_text(QStringLiteral("Evitar duplicadas existentes")));
        apply_default->setText(get_text(QStringLiteral("Aplicar frequência padrão a itens sem recorrência definida")));
        lbl_default_recurrence->setText(get_text(QStringLiteral("Frequência padrão")));
        default_recurrence->setItemText(0, get_text(QStringLiteral("Diária")));
        default_recurrence->setItemText(1, get_text(QStringLiteral("Semanal")));
        default_recurrence->setItemText(2, get_text(QStringLiteral("Mensal")));
        when_group->setTitle(get_text(QStringLiteral("Quando aplicar")));
        lbl_start_date->setText(get_text(QStringLiteral("Data inicial")));
        lbl_start_time->setText(get_text(QStringLiteral("Hora inicial")));
        lbl_end_date->setText(get_text(QStringLiteral("Data final")));
        lbl_end_time->setText(get_text(QStringLiteral("Hora final")));
        use_start_time_when_missing->setText(get_text(QStringLiteral("Usar hora inicial quando o item não tiver horário")));
        lbl_missing_date->setText(get_text(QStringLiteral("Data para itens sem data")));
        lbl_missing_time->setText(get_text(QStringLiteral("Hora para itens sem horário")));
        set_application_start_button->setText(get_text(QStringLiteral("Início")));
        set_application_end_button->setText(get_text(QStringLiteral("Fim")));
        preview_group->setTitle(get_text(QStringLiteral("Prévia do que será criado")));
        preview_table->setHorizontalHeaderLabels({get_text(QStringLiteral("Item")),
                                                  get_text(QStringLiteral("Origem encontrada")),
                                                  get_text(QStringLiteral("Criar em")),
                                                  get_text(QStringLiteral("Recorrência usada")),
                                                  get_text(QStringLiteral("Próximas datas"))});
        restore_defaults_button->setText(get_text(QStringLiteral("Restaurar padrão")));
        apply_button->setText(get_text(QStringLiteral("Gerar ocorrências")));
        restore_original_button->setText(get_text(QStringLiteral("Restaurar tarefas anteriores")));
        save_settings_button->setText(get_text(QStringLiteral("Salvar definições")));
        close_button->setText(get_text(QStringLiteral("Fechar")));
        const QString date_fmt = QLocale().dateFormat(QLocale::ShortFormat);
        application_start_date->setDisplayFormat(date_fmt);
        application_end_date->setDisplayFormat(date_fmt);
        missing_base_date->setDisplayFormat(date_fmt);
        update_summary();
    };
    dialog.set_retranslate_handler([&]() { retranslate_ui(); });
    retranslate_ui();

    layout->activate();
    compact_calendar_strip_to_base(application_calendar_strip);
    layout->activate();
    const QSize recurrence_initial_minimum_size = dialog.minimumSizeHint();
    const QSize recurrence_initial_size = dialog.sizeHint().expandedTo(recurrence_initial_minimum_size);
    dialog.setMinimumSize(recurrence_initial_size);
    dialog.resize(recurrence_initial_size);
    auto *restore_geometry_filter = new RestoreNormalGeometryFilter(
        &dialog,
        recurrence_initial_size,
        recurrence_initial_size,
        [&]() {
            compact_calendar_strip_to_base(application_calendar_strip);
        },
        [&]() {
            update_responsive_grid(methods_grid.get());
            update_calendar_strip(application_calendar_strip);
        },
        false,
        &dialog);
    dialog.installEventFilter(restore_geometry_filter);

    update_calendar_strip(application_calendar_strip);

    dialog.exec();
}


}  // namespace source::InterfaceCore::ProductivityFeatures
