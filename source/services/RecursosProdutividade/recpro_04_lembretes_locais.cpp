#include "source/services/RecursosProdutividade/recpro_04_lembretes_locais.hpp"
#include "source/services/RecursosProdutividade/recpro_01_utilitarios_compartilhados.hpp"

namespace source::InterfaceCore::ProductivityFeatures {

void schedule_local_reminders(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    const bool tray_available = QSystemTrayIcon::isSystemTrayAvailable();

    const QVariantList unified_records = records_for_app(app);
    const QJsonObject map_backup = read_json_object(mapamental_backup_path());
    const QStringList available_modules = available_productivity_modules(app);
    const QSet<QString> all_modules = module_set_from_list(available_modules);
    const QVector<TaskRecordGroup> groups =
        collect_productivity_task_groups(app, unified_records, map_backup, all_modules);
    if (groups.isEmpty()) {
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Lembretes locais")),
                                 get_text(QStringLiteral("Nenhuma tarefa encontrada.")));
        return;
    }

    RuntimeTranslatedDialog dialog(app);
    dialog.setWindowTitle(get_text(QStringLiteral("Ativar lembretes locais")));
    dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    dialog.setMinimumHeight(680);
    dialog.resize(720, 720);
    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    auto *summary = new QLabel(&dialog);
    summary->setWordWrap(true);
    layout->addWidget(summary);

    auto *methods_group = new QGroupBox(get_text(QStringLiteral("Origem")), &dialog);
    methods_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto *methods_layout = new QGridLayout(methods_group);
    methods_layout->setContentsMargins(8, 6, 8, 6);
    methods_layout->setHorizontalSpacing(10);
    methods_layout->setVerticalSpacing(2);
    QMap<QString, QCheckBox *> module_checks =
        create_productivity_module_checks(methods_group, all_modules, available_modules);
    const QVector<QWidget *> methods_widgets = widgets_for_productivity_modules(module_checks, available_modules);
    auto methods_grid = std::make_unique<ResponsiveCheckGrid>();
    methods_grid->container = methods_group;
    methods_grid->layout = methods_layout;
    methods_grid->widgets = methods_widgets;
    auto *methods_grid_filter = new ResponsiveGridResizeFilter(methods_grid.get(), methods_group);
    methods_group->installEventFilter(methods_grid_filter);
    update_responsive_grid(methods_grid.get());

    auto *options_group = new QGroupBox(get_text(QStringLiteral("Avisos")), &dialog);
    auto *options_form = new QFormLayout(options_group);
    options_form->setContentsMargins(8, 6, 8, 6);
    options_form->setSpacing(4);
    options_form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    auto *max_reminders = new QSpinBox(options_group);
    max_reminders->setRange(1, 50);
    max_reminders->setValue(1);
    auto *lbl_max_reminders = new QLabel(get_text(QStringLiteral("Máximo")), options_group);
    auto *include_completed = new QCheckBox(get_text(QStringLiteral("Incluir itens concluídos")), options_group);
    auto *show_in_app = new QCheckBox(get_text(QStringLiteral("Alertar no app")), options_group);
    show_in_app->setChecked(true);
    auto *show_tray = new QCheckBox(get_text(QStringLiteral("Bandeja do sistema")), options_group);
    show_tray->setChecked(tray_available);
    show_tray->setEnabled(tray_available);
    auto *use_default_time = new QCheckBox(get_text(QStringLiteral("Horário padrão se faltar")), options_group);
    use_default_time->setChecked(true);
    auto *lbl_default_time = new QLabel(get_text(QStringLiteral("Horário padrão")), options_group);
    auto *default_time = new QTimeEdit(QTime(9, 0), options_group);
    default_time->setDisplayFormat(QStringLiteral("HH:mm"));
    options_form->addRow(lbl_max_reminders, max_reminders);
    options_form->addRow(include_completed);
    options_form->addRow(show_in_app);
    options_form->addRow(show_tray);
    options_form->addRow(use_default_time);
    options_form->addRow(lbl_default_time, default_time);

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
    item_view->setMinimumHeight(124);
    selection_layout->addWidget(item_view, 1);

    auto *selection_buttons_widget = new QWidget(selection_group);
    selection_buttons_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto *selection_buttons = new QHBoxLayout(selection_buttons_widget);
    selection_buttons->setContentsMargins(0, 12, 0, 0);
    selection_buttons->setSpacing(6);
    auto *select_visible_button = new QPushButton(get_text(QStringLiteral("Selecionar visíveis")), selection_buttons_widget);
    auto *clear_selection_button = new QPushButton(get_text(QStringLiteral("Limpar")), selection_buttons_widget);
    select_visible_button->setMaximumWidth(150);
    clear_selection_button->setMaximumWidth(90);
    selection_buttons->addWidget(select_visible_button);
    selection_buttons->addWidget(clear_selection_button);
    selection_buttons->addStretch(1);
    selection_layout->addWidget(selection_buttons_widget);
    selection_group->setMinimumHeight(260);
    selection_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    auto *active_group = new QGroupBox(get_text(QStringLiteral("Lembretes ativos")), &dialog);
    auto *active_layout = new QVBoxLayout(active_group);
    active_layout->setContentsMargins(8, 6, 8, 6);
    active_layout->setSpacing(4);
    auto *active_reminders_text = new QPlainTextEdit(active_group);
    active_reminders_text->setReadOnly(true);
    active_reminders_text->setPlainText(active_local_reminders_text());
    active_reminders_text->setMinimumHeight(54);
    active_reminders_text->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    active_layout->addWidget(active_reminders_text, 1);
    auto *active_buttons = new QHBoxLayout();
    auto *test_button = new QPushButton(get_text(QStringLiteral("Testar")), active_group);
    test_button->setMaximumWidth(90);
    active_buttons->addWidget(test_button);
    active_buttons->addStretch(1);
    active_layout->addLayout(active_buttons);
    active_group->setMinimumHeight(120);
    active_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    QObject::connect(test_button, &QPushButton::clicked, &dialog, [app, show_in_app, show_tray, tray_available, &dialog]() {
        QSystemTrayIcon *tray = nullptr;
        if (tray_available && show_tray->isChecked()) {
            tray = new QSystemTrayIcon(local_reminder_icon(app), app);
            tray->setToolTip(get_text(QStringLiteral("Lembretes locais")));
            tray->show();
        }
        deliver_local_reminder(app,
                               tray,
                               get_text(QStringLiteral("Lembrete de tarefa")),
                               get_text(QStringLiteral("Teste de lembrete local.")),
                               tray != nullptr,
                               show_in_app->isChecked(),
                               &dialog);
        if (tray != nullptr) {
            QTimer::singleShot(30000, tray, &QObject::deleteLater);
        }
    });

    auto *timing_group = new QGroupBox(get_text(QStringLiteral("Quando alertar")), &dialog);
    auto *form = new QFormLayout(timing_group);
    form->setContentsMargins(8, 6, 8, 6);
    form->setSpacing(4);
    form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    auto *use_item_due_time = new QRadioButton(get_text(QStringLiteral("Usar data e horário de cada tarefa")), timing_group);
    auto *use_custom_due_time = new QRadioButton(get_text(QStringLiteral("Definir um único horário personalizado")), timing_group);
    use_item_due_time->setChecked(true);
    const QDateTime suggested_reminder = suggested_custom_reminder_datetime();
    const QString date_format = QLocale().dateFormat(QLocale::ShortFormat);
    auto *lbl_custom_date = new QLabel(get_text(QStringLiteral("Data")), timing_group);
    auto *custom_date = new QDateEdit(suggested_reminder.date(), timing_group);
    custom_date->setCalendarPopup(true);
    custom_date->setDisplayFormat(date_format);
    auto *lbl_custom_time = new QLabel(get_text(QStringLiteral("Hora")), timing_group);
    auto *custom_time = new QTimeEdit(suggested_reminder.time(), timing_group);
    custom_time->setDisplayFormat(QStringLiteral("HH:mm"));
    auto *lbl_start_date = new QLabel(get_text(QStringLiteral("Início")), timing_group);
    auto *start_date = new QDateEdit(QDate::currentDate(), timing_group);
    start_date->setCalendarPopup(true);
    start_date->setDisplayFormat(date_format);
    auto *lbl_end_date = new QLabel(get_text(QStringLiteral("Fim")), timing_group);
    auto *end_date = new QDateEdit(QDate::currentDate().addDays(6), timing_group);
    end_date->setCalendarPopup(true);
    end_date->setDisplayFormat(date_format);
    auto *lbl_start_time = new QLabel(get_text(QStringLiteral("Hora inicial")), timing_group);
    auto *start_time = new QTimeEdit(QTime(0, 0), timing_group);
    start_time->setDisplayFormat(QStringLiteral("HH:mm"));
    auto *lbl_end_time = new QLabel(get_text(QStringLiteral("Hora final")), timing_group);
    auto *end_time = new QTimeEdit(QTime(23, 59), timing_group);
    end_time->setDisplayFormat(QStringLiteral("HH:mm"));
    const auto reminder_field_width = [&]() {
        constexpr int kMinimumFieldWidth = 140;
        int width = kMinimumFieldWidth;
        const QWidget *fields[] = {custom_date,
                                   custom_time,
                                   start_date,
                                   end_date,
                                   start_time,
                                   end_time,
                                   max_reminders,
                                   default_time};
        for (const QWidget *field : fields) {
            if (field != nullptr) {
                width = qMax(width, field->sizeHint().width() + 10);
            }
        }
        return width;
    }();
    const auto apply_reminder_field_width = [reminder_field_width](QWidget *field) {
        if (field == nullptr) {
            return;
        }
        field->setMinimumWidth(reminder_field_width);
        field->setMaximumWidth(reminder_field_width);
    };
    apply_reminder_field_width(custom_date);
    apply_reminder_field_width(custom_time);
    apply_reminder_field_width(start_date);
    apply_reminder_field_width(end_date);
    apply_reminder_field_width(start_time);
    apply_reminder_field_width(end_time);
    apply_reminder_field_width(max_reminders);
    apply_reminder_field_width(default_time);
    auto *lbl_quick_period = new QLabel(get_text(QStringLiteral("Período")), timing_group);
    auto *quick_period = new QWidget(timing_group);
    quick_period->setMinimumWidth(286);
    quick_period->setMaximumWidth(320);
    auto *quick_period_layout = new QHBoxLayout(quick_period);
    quick_period_layout->setContentsMargins(0, 0, 0, 0);
    auto *period_amount = new QSpinBox(quick_period);
    period_amount->setRange(1, 60);
    period_amount->setValue(7);
    period_amount->setMinimumWidth(72);
    period_amount->setMaximumWidth(82);
    auto *period_unit = new QComboBox(quick_period);
    period_unit->addItem(get_text(QStringLiteral("Dias")), QStringLiteral("days"));
    period_unit->addItem(get_text(QStringLiteral("Semanas")), QStringLiteral("weeks"));
    period_unit->addItem(get_text(QStringLiteral("Meses")), QStringLiteral("months"));
    period_unit->setMinimumWidth(112);
    period_unit->setMaximumWidth(126);
    auto *apply_period = new QPushButton(get_text(QStringLiteral("Aplicar")), quick_period);
    apply_period->setMinimumWidth(84);
    apply_period->setMaximumWidth(96);
    quick_period_layout->addWidget(period_amount);
    quick_period_layout->addWidget(period_unit);
    quick_period_layout->addWidget(apply_period);
    quick_period_layout->addStretch(1);

    form->addRow(use_item_due_time);
    form->addRow(use_custom_due_time);
    form->addRow(lbl_custom_date, custom_date);
    form->addRow(lbl_custom_time, custom_time);
    form->addRow(lbl_start_date, start_date);
    form->addRow(lbl_end_date, end_date);
    form->addRow(lbl_quick_period, quick_period);
    form->addRow(lbl_start_time, start_time);
    form->addRow(lbl_end_time, end_time);

    auto *calendar_group = new QGroupBox(get_text(QStringLiteral("Calendário")), &dialog);
    auto *calendar_layout = new QVBoxLayout(calendar_group);
    calendar_layout->setContentsMargins(8, 6, 8, 6);
    calendar_layout->setSpacing(4);
    auto *calendar_strip = create_multi_month_calendar_strip(calendar_group, start_date->date(), 170);
    auto *calendar_filter = new CalendarStripResizeFilter(calendar_strip, calendar_strip->container);
    calendar_strip->container->installEventFilter(calendar_filter);
    calendar_layout->addWidget(calendar_strip->container);
    auto *calendar_buttons = new QHBoxLayout();
    auto *set_start_button = new QPushButton(get_text(QStringLiteral("Início")), calendar_group);
    auto *set_end_button = new QPushButton(get_text(QStringLiteral("Fim")), calendar_group);
    set_start_button->setMaximumWidth(86);
    set_end_button->setMaximumWidth(86);
    calendar_buttons->addWidget(set_start_button);
    calendar_buttons->addWidget(set_end_button);
    calendar_buttons->addStretch(1);
    calendar_layout->addLayout(calendar_buttons);
    calendar_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    auto *left_column_widget = new QWidget(&dialog);
    auto *left_column = new QVBoxLayout(left_column_widget);
    left_column->setContentsMargins(0, 0, 0, 0);
    left_column->setSpacing(6);
    left_column->addWidget(methods_group);
    left_column->addWidget(selection_group, 1);
    left_column->addWidget(active_group, 1);

    auto *right_column_widget = new QWidget(&dialog);
    auto *right_column = new QVBoxLayout(right_column_widget);
    right_column->setContentsMargins(0, 0, 0, 0);
    right_column->setSpacing(6);
    right_column->addWidget(timing_group);
    right_column->addWidget(options_group);
    right_column->addWidget(calendar_group, 1);

    left_column_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    right_column_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    const int columns_min_width =
        qMax(left_column_widget->sizeHint().width(), right_column_widget->sizeHint().width());
    left_column_widget->setMinimumWidth(columns_min_width);
    right_column_widget->setMinimumWidth(columns_min_width);

    auto *main_row = new QHBoxLayout();
    main_row->setSpacing(8);
    main_row->addWidget(left_column_widget, 1);
    main_row->addWidget(right_column_widget, 1);
    layout->addLayout(main_row, 1);

    if (!tray_available) {
        auto *tray_warning = new QLabel(get_text(QStringLiteral(
            "A bandeja do sistema não está disponível; os lembretes serão mostrados dentro do aplicativo.")),
                                        &dialog);
        tray_warning->setWordWrap(true);
        layout->addWidget(tray_warning);
    }

    const auto set_date_range = [&](QDate start, QDate end) {
        if (!start.isValid()) {
            start = QDate::currentDate();
        }
        if (!end.isValid()) {
            end = start;
        }
        if (end < start) {
            qSwap(start, end);
        }
        start_date->setDate(start);
        end_date->setDate(end);
        set_calendar_strip_selected_date(calendar_strip, start);
    };

    const auto apply_quick_period = [&]() {
        const QDate start = start_date->date();
        QDate end = start;
        const int amount = period_amount->value();
        const QString unit = period_unit->currentData().toString();
        if (unit == QStringLiteral("weeks")) {
            end = start.addDays(amount * 7 - 1);
        } else if (unit == QStringLiteral("months")) {
            end = start.addMonths(amount).addDays(-1);
        } else {
            end = start.addDays(amount - 1);
        }
        set_date_range(start, end);
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
        const TaskRecordGroup &group = groups.at(group_index);
        if (!group_has_selected_module(group, checked_productivity_module_set(module_checks))) {
            return false;
        }
        return include_completed->isChecked() || group_has_open_record(group);
    };

    const auto due_for_group = [&](const TaskRecordGroup &group) -> std::optional<QDateTime> {
        if (use_custom_due_time->isChecked()) {
            const QDateTime due(custom_date->date(), custom_time->time());
            return due.isValid() ? std::optional<QDateTime>(due) : std::nullopt;
        }
        std::optional<QDateTime> due = reminder_due_for_group(group, use_default_time->isChecked(), default_time->time());
        if (!due.has_value()) {
            return std::nullopt;
        }
        QDateTime dt = due.value();
        const QDateTime now = QDateTime::currentDateTime();
        if (dt.date() == now.date() && dt.addSecs(kReminderPastGraceSeconds) < now) {
            dt = now.addSecs(60);
        }
        return dt;
    };

    const auto group_is_considered = [&](int group_index, const QSet<int> &selected_indices) {
        if (!group_matches_current_filters(group_index)) {
            return false;
        }
        return !selected_items_radio->isChecked() || selected_indices.contains(group_index);
    };

    const auto candidate_count_for_current_settings = [&]() {
        QSet<qint64> due_groups;
        const QDateTime now = QDateTime::currentDateTime();
        const QSet<int> selected_indices = selected_group_indices();
        for (int i = 0; i < groups.size(); ++i) {
            if (!group_is_considered(i, selected_indices)) {
                continue;
            }
            const std::optional<QDateTime> due = due_for_group(groups.at(i));
            if (!due.has_value()) {
                continue;
            }
            if (use_custom_due_time->isChecked()) {
                if (reminder_due_is_schedulable(due.value(), now)) {
                    due_groups.insert(due.value().toMSecsSinceEpoch());
                }
            } else if (reminder_due_in_window(due.value(),
                                              now,
                                              start_date->date(),
                                              end_date->date(),
                                              start_time->time(),
                                              end_time->time())) {
                due_groups.insert(due.value().toMSecsSinceEpoch());
            }
        }
        return due_groups.size() * max_reminders->value();
    };

    const auto update_summary = [&]() {
        const QSet<QString> selected_modules = checked_productivity_module_set(module_checks);
        const QString origin_label = selected_modules.isEmpty()
                                         ? get_text(QStringLiteral("Nenhum método selecionado"))
                                         : module_list_label(ordered_modules_from_set(selected_modules));
        const int available_items = item_proxy->rowCount();
        const int selection_count = selected_group_indices().size();
        const QString target_label =
            selected_items_radio->isChecked()
                ? get_text(QStringLiteral("%1 item(ns) selecionado(s)")).arg(selection_count)
                : get_text(QStringLiteral("%1 item(ns) dos métodos selecionados")).arg(available_items);
        const QString timing_label =
            use_custom_due_time->isChecked() ? get_text(QStringLiteral("data/hora escolhida"))
                                             : get_text(QStringLiteral("data/hora dos itens"));
        summary->setText(get_text(QStringLiteral("%1; %2 lembrete(s) possíveis usando %3. Origem: %4."))
                             .arg(target_label)
                             .arg(candidate_count_for_current_settings())
                             .arg(timing_label, origin_label));
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
            for (int row = 0; row < item_model->rowCount(); ++row) {
                const QVariantMap entry = item_model->record_at(row);
                const int group_index = entry.value(QStringLiteral("group_index"), -1).toInt();
                if (!previously_selected.contains(group_index)) {
                    continue;
                }
                const QModelIndex source_index = item_model->index(row, 0);
                const QModelIndex proxy_index = item_proxy->mapFromSource(source_index);
                if (proxy_index.isValid()) {
                    item_view->selectionModel()->select(proxy_index,
                                                        QItemSelectionModel::Select | QItemSelectionModel::Rows);
                }
            }
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

    const auto update_timing_controls = [&]() {
        const bool item_time_mode = use_item_due_time->isChecked();
        custom_date->setEnabled(!item_time_mode);
        custom_time->setEnabled(!item_time_mode);
        start_date->setEnabled(item_time_mode);
        end_date->setEnabled(item_time_mode);
        quick_period->setEnabled(item_time_mode);
        period_amount->setEnabled(item_time_mode);
        period_unit->setEnabled(item_time_mode);
        apply_period->setEnabled(item_time_mode);
        start_time->setEnabled(item_time_mode);
        end_time->setEnabled(item_time_mode);
        calendar_group->setEnabled(item_time_mode);
        use_default_time->setEnabled(item_time_mode);
        default_time->setEnabled(item_time_mode && use_default_time->isChecked());
        update_summary();
    };

    QSet<QString> saved_local_selected_source_ids;
    const auto set_combo_current_data = [](QComboBox *combo, const QString &data) {
        if (combo == nullptr) {
            return;
        }
        const int index = combo->findData(data);
        if (index >= 0) {
            combo->setCurrentIndex(index);
        }
    };
    const auto save_local_reminder_settings = [&]() {
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

        settings.insert(QStringLiteral("max_reminders"), max_reminders->value());
        settings.insert(QStringLiteral("include_completed"), include_completed->isChecked());
        settings.insert(QStringLiteral("show_in_app"), show_in_app->isChecked());
        settings.insert(QStringLiteral("show_tray"), show_tray->isChecked());
        settings.insert(QStringLiteral("use_default_time"), use_default_time->isChecked());
        settings.insert(QStringLiteral("default_time"), default_time->time().toString(QStringLiteral("HH:mm")));
        settings.insert(QStringLiteral("timing_mode"),
                        use_custom_due_time->isChecked() ? QStringLiteral("custom") : QStringLiteral("item"));
        settings.insert(QStringLiteral("custom_date"), custom_date->date().toString(Qt::ISODate));
        settings.insert(QStringLiteral("custom_time"), custom_time->time().toString(QStringLiteral("HH:mm")));
        settings.insert(QStringLiteral("start_date"), start_date->date().toString(Qt::ISODate));
        settings.insert(QStringLiteral("end_date"), end_date->date().toString(Qt::ISODate));
        settings.insert(QStringLiteral("start_time"), start_time->time().toString(QStringLiteral("HH:mm")));
        settings.insert(QStringLiteral("end_time"), end_time->time().toString(QStringLiteral("HH:mm")));
        settings.insert(QStringLiteral("period_amount"), period_amount->value());
        settings.insert(QStringLiteral("period_unit"), period_unit->currentData().toString());
        write_json_object(local_reminder_settings_path(), settings);
    };
    const auto load_local_reminder_settings = [&]() {
        const QJsonObject settings = read_json_object(local_reminder_settings_path());
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

        saved_local_selected_source_ids.clear();
        const QJsonArray selected_ids = settings.value(QStringLiteral("selected_source_ids")).toArray();
        for (const QJsonValue &value : selected_ids) {
            const QString id = value.toString().trimmed();
            if (!id.isEmpty()) {
                saved_local_selected_source_ids.insert(id);
            }
        }

        max_reminders->setValue(qBound(max_reminders->minimum(),
                                       settings.value(QStringLiteral("max_reminders")).toInt(max_reminders->value()),
                                       max_reminders->maximum()));
        include_completed->setChecked(settings.value(QStringLiteral("include_completed")).toBool(include_completed->isChecked()));
        show_in_app->setChecked(settings.value(QStringLiteral("show_in_app")).toBool(show_in_app->isChecked()));
        show_tray->setChecked(tray_available && settings.value(QStringLiteral("show_tray")).toBool(show_tray->isChecked()));
        use_default_time->setChecked(settings.value(QStringLiteral("use_default_time")).toBool(use_default_time->isChecked()));

        const QTime loaded_default_time = parse_time(settings.value(QStringLiteral("default_time")).toString());
        if (loaded_default_time.isValid()) {
            default_time->setTime(loaded_default_time);
        }
        if (settings.value(QStringLiteral("timing_mode")).toString() == QStringLiteral("custom")) {
            use_custom_due_time->setChecked(true);
        } else {
            use_item_due_time->setChecked(true);
        }

        const QDate loaded_custom_date = QDate::fromString(settings.value(QStringLiteral("custom_date")).toString(), Qt::ISODate);
        const QDate loaded_start_date = QDate::fromString(settings.value(QStringLiteral("start_date")).toString(), Qt::ISODate);
        const QDate loaded_end_date = QDate::fromString(settings.value(QStringLiteral("end_date")).toString(), Qt::ISODate);
        if (loaded_custom_date.isValid()) {
            custom_date->setDate(loaded_custom_date);
        }
        if (loaded_start_date.isValid() || loaded_end_date.isValid()) {
            set_date_range(loaded_start_date.isValid() ? loaded_start_date : start_date->date(),
                           loaded_end_date.isValid() ? loaded_end_date : end_date->date());
        }

        const QTime loaded_custom_time = parse_time(settings.value(QStringLiteral("custom_time")).toString());
        const QTime loaded_start_time = parse_time(settings.value(QStringLiteral("start_time")).toString());
        const QTime loaded_end_time = parse_time(settings.value(QStringLiteral("end_time")).toString());
        if (loaded_custom_time.isValid()) {
            custom_time->setTime(loaded_custom_time);
        }
        if (loaded_start_time.isValid()) {
            start_time->setTime(loaded_start_time);
        }
        if (loaded_end_time.isValid()) {
            end_time->setTime(loaded_end_time);
        }
        period_amount->setValue(qBound(period_amount->minimum(),
                                       settings.value(QStringLiteral("period_amount")).toInt(period_amount->value()),
                                       period_amount->maximum()));
        set_combo_current_data(period_unit, settings.value(QStringLiteral("period_unit")).toString());
    };
    const auto restore_saved_local_item_selection = [&]() {
        if (saved_local_selected_source_ids.isEmpty() || item_view->selectionModel() == nullptr) {
            return;
        }
        item_view->selectionModel()->clearSelection();
        for (int row = 0; row < item_model->rowCount(); ++row) {
            const QVariantMap entry = item_model->record_at(row);
            const int group_index = entry.value(QStringLiteral("group_index"), -1).toInt();
            if (group_index < 0 || group_index >= groups.size()) {
                continue;
            }
            if (!saved_local_selected_source_ids.contains(source_id_for_group(groups.at(group_index)))) {
                continue;
            }
            const QModelIndex source_index = item_model->index(row, 0);
            const QModelIndex proxy_index = item_proxy->mapFromSource(source_index);
            if (proxy_index.isValid()) {
                item_view->selectionModel()->select(proxy_index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    };
    load_local_reminder_settings();

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
    const auto restore_local_defaults = [&]() {
        for (QCheckBox *check : module_checks) {
            if (check != nullptr) {
                check->setChecked(true);
            }
        }
        all_items_radio->setChecked(true);
        item_filter->clear();
        if (item_view->selectionModel() != nullptr) {
            item_view->selectionModel()->clearSelection();
        }
        use_item_due_time->setChecked(true);
        const QDateTime suggested = suggested_custom_reminder_datetime();
        custom_date->setDate(suggested.date());
        custom_time->setTime(suggested.time());
        set_date_range(QDate::currentDate(), QDate::currentDate().addDays(6));
        start_time->setTime(QTime(0, 0));
        end_time->setTime(QTime(23, 59));
        period_amount->setValue(7);
        period_unit->setCurrentIndex(0);
        max_reminders->setValue(1);
        include_completed->setChecked(false);
        show_in_app->setChecked(true);
        show_tray->setChecked(tray_available);
        use_default_time->setChecked(true);
        default_time->setTime(QTime(9, 0));
        active_reminders_text->setPlainText(active_local_reminders_text());
        update_item_model();
        update_selection_controls();
        update_timing_controls();
    };

    QObject::connect(start_date, &QDateEdit::dateChanged, &dialog, [&](const QDate &date) {
        if (end_date->date() < date) {
            end_date->setDate(date);
        }
        set_calendar_strip_selected_date(calendar_strip, date);
        update_summary();
    });
    QObject::connect(end_date, &QDateEdit::dateChanged, &dialog, [&](const QDate &date) {
        if (date < start_date->date()) {
            start_date->setDate(date);
        }
        update_summary();
    });
    QObject::connect(start_time, &QTimeEdit::timeChanged, &dialog, [&](const QTime &) { update_summary(); });
    QObject::connect(end_time, &QTimeEdit::timeChanged, &dialog, [&](const QTime &) { update_summary(); });
    QObject::connect(custom_date, &QDateEdit::dateChanged, &dialog, [&](const QDate &) { update_summary(); });
    QObject::connect(custom_time, &QTimeEdit::timeChanged, &dialog, [&](const QTime &) { update_summary(); });
    QObject::connect(default_time, &QTimeEdit::timeChanged, &dialog, [&](const QTime &) { update_summary(); });
    QObject::connect(max_reminders, qOverload<int>(&QSpinBox::valueChanged), &dialog, [&](int) { update_summary(); });
    QObject::connect(include_completed, &QCheckBox::toggled, &dialog, [&](bool) { update_item_model(); });
    QObject::connect(use_default_time, &QCheckBox::toggled, &dialog, [&](bool) { update_timing_controls(); });
    QObject::connect(use_item_due_time, &QRadioButton::toggled, &dialog, [&](bool) { update_timing_controls(); });
    QObject::connect(use_custom_due_time, &QRadioButton::toggled, &dialog, [&](bool) { update_timing_controls(); });
    QObject::connect(period_amount, qOverload<int>(&QSpinBox::valueChanged), &dialog, [&](int) { apply_quick_period(); });
    QObject::connect(period_unit, qOverload<int>(&QComboBox::currentIndexChanged), &dialog, [&](int) {
        apply_quick_period();
    });
    QObject::connect(apply_period, &QPushButton::clicked, &dialog, [&]() {
        apply_quick_period();
        update_summary();
    });
    QObject::connect(set_start_button, &QPushButton::clicked, &dialog, [&]() {
        set_date_range(calendar_strip_selected_date(calendar_strip), end_date->date());
        update_summary();
    });
    QObject::connect(set_end_button, &QPushButton::clicked, &dialog, [&]() {
        set_date_range(start_date->date(), calendar_strip_selected_date(calendar_strip));
        update_summary();
    });
    update_item_model();
    restore_saved_local_item_selection();
    update_selection_controls();
    update_timing_controls();

    const auto apply_local_reminders = [&]() {
        const bool show_in_app_messages = show_in_app->isChecked();
        const bool show_tray_messages = show_tray->isChecked() && tray_available;
        const QSet<QString> selected_modules = checked_productivity_module_set(module_checks);
        if (selected_modules.isEmpty()) {
            QMessageBox::warning(&dialog,
                                 get_text(QStringLiteral("Lembretes locais")),
                                 get_text(QStringLiteral("Selecione pelo menos um método.")));
            return;
        }
        if (!show_in_app_messages && !show_tray_messages) {
            QMessageBox::warning(&dialog,
                                 get_text(QStringLiteral("Lembretes locais")),
                                 get_text(QStringLiteral("Ative pelo menos uma forma de aviso.")));
            return;
        }
        const QSet<int> selected_indices_for_accept = selected_group_indices();
        if (selected_items_radio->isChecked() && selected_indices_for_accept.isEmpty()) {
            QMessageBox::warning(&dialog,
                                 get_text(QStringLiteral("Lembretes locais")),
                                 get_text(QStringLiteral("Selecione pelo menos uma tarefa ou conceito.")));
            return;
        }

        struct ReminderCandidate {
            QDateTime due;
            QString title;
            QString modules;
        };
        struct ReminderSchedule {
            QDateTime due;
            QStringList item_lines;
            QString title;
            QString modules;
        };

        const QDateTime now = QDateTime::currentDateTime();
        const bool custom_time_mode = use_custom_due_time->isChecked();
        QDateTime selected_custom_due(custom_date->date(), custom_time->time());
        if (custom_time_mode) {
            if (selected_custom_due.date() == now.date() && selected_custom_due.addSecs(kReminderPastGraceSeconds) < now) {
                selected_custom_due = now.addSecs(60);
            }
            if (!reminder_due_is_schedulable(selected_custom_due, now)) {
                QMessageBox::warning(&dialog,
                                     get_text(QStringLiteral("Lembretes locais")),
                                     get_text(QStringLiteral("A data/hora de lembrete escolhida já passou.")));
                return;
            }
        }

        const QDate selected_start_date = start_date->date();
        const QDate selected_end_date = end_date->date();
        const QTime selected_start_time = start_time->time();
        const QTime selected_end_time = end_time->time();
        QVector<ReminderCandidate> candidates;
        for (int i = 0; i < groups.size(); ++i) {
            if (!group_is_considered(i, selected_indices_for_accept)) {
                continue;
            }
            const TaskRecordGroup &group = groups.at(i);
            const std::optional<QDateTime> due = custom_time_mode
                                                     ? std::optional<QDateTime>(selected_custom_due)
                                                     : reminder_due_for_group(group,
                                                                              use_default_time->isChecked(),
                                                                              default_time->time());
            if (!due.has_value()) {
                continue;
            }
            QDateTime due_dt = due.value();
            if (due_dt.date() == now.date() && due_dt.addSecs(kReminderPastGraceSeconds) < now) {
                due_dt = now.addSecs(60);
            }

            if (custom_time_mode) {
                if (!reminder_due_is_schedulable(due_dt, now)) {
                    continue;
                }
            } else {
                if (!reminder_due_in_window(due_dt,
                                            now,
                                            selected_start_date,
                                            selected_end_date,
                                            selected_start_time,
                                            selected_end_time)) {
                    continue;
                }
            }

            candidates.append({due_dt,
                               title_for_record(group.primary),
                               module_list_label(ordered_modules_for_group(group, selected_modules))});
        }

        std::sort(candidates.begin(), candidates.end(), [](const ReminderCandidate &left, const ReminderCandidate &right) {
            if (left.due == right.due) {
                return left.title.localeAwareCompare(right.title) < 0;
            }
            return left.due < right.due;
        });
        if (candidates.isEmpty()) {
            QMessageBox::information(&dialog,
                                     get_text(QStringLiteral("Lembretes locais")),
                                     get_text(QStringLiteral("Nenhum lembrete foi agendado com as configurações selecionadas.")));
            return;
        }

        QVector<ReminderSchedule> schedules;
        for (const ReminderCandidate &candidate : std::as_const(candidates)) {
            if (schedules.isEmpty() || schedules.last().due != candidate.due) {
                schedules.append({candidate.due, {}, {}, {}});
            }

            ReminderSchedule &schedule = schedules.last();
            const QString item_line = QStringLiteral("[%1] %2").arg(candidate.modules, candidate.title);
            if (!schedule.item_lines.contains(item_line)) {
                schedule.item_lines.append(item_line);
            }
        }
        for (ReminderSchedule &schedule : schedules) {
            const int item_count = schedule.item_lines.size();
            schedule.title = item_count == 1
                                 ? schedule.item_lines.first().section(QStringLiteral("] "), 1)
                                 : get_text(QStringLiteral("%1 item(ns) selecionado(s)")).arg(item_count);
            schedule.modules = item_count == 1
                                   ? schedule.item_lines.first().section(QLatin1Char('['), 1, 1).section(QLatin1Char(']'), 0, 0)
                                   : get_text(QStringLiteral("Itens"));
        }

        QSystemTrayIcon *tray = nullptr;
        if (show_tray_messages) {
            tray = new QSystemTrayIcon(local_reminder_icon(app), app);
            tray->setToolTip(get_text(QStringLiteral("Lembretes locais")));
            tray->show();
        }

        auto *runtime = new QObject(app);
        if (tray != nullptr) {
            tray->setParent(runtime);
        }
        auto remaining_timers = std::make_shared<int>(0);
        QStringList scheduled_preview;
        int scheduled = 0;
        const int reminders_per_schedule = max_reminders->value();
        for (const ReminderSchedule &schedule : std::as_const(schedules)) {
            const qint64 ms = now.msecsTo(schedule.due);
            const qint64 grace_ms = static_cast<qint64>(kReminderPastGraceSeconds) * 1000;
            if (ms < -grace_ms || ms > std::numeric_limits<int>::max()) {
                continue;
            }

            const qint64 base_interval_ms = qMax<qint64>(1000, qMax<qint64>(0, ms));
            for (int reminder_index = 0; reminder_index < reminders_per_schedule; ++reminder_index) {
                const qint64 repeated_ms = base_interval_ms + static_cast<qint64>(reminder_index) * 1000;
                if (repeated_ms > std::numeric_limits<int>::max()) {
                    continue;
                }
                const int interval_ms = static_cast<int>(repeated_ms);
                const QString repetition_suffix =
                    reminders_per_schedule > 1
                        ? QStringLiteral(" (%1/%2)").arg(reminder_index + 1).arg(reminders_per_schedule)
                        : QString();
                const QString reminder_title = schedule.title + repetition_suffix;
                const QString reminder_message =
                    QStringLiteral("%1\n%2").arg(schedule.due.toString(QStringLiteral("dd/MM/yyyy HH:mm")),
                                                 schedule.item_lines.join(QLatin1Char('\n')));

                auto *timer = new QTimer(runtime);
                timer->setSingleShot(true);
                timer->setTimerType(Qt::PreciseTimer);
                ++(*remaining_timers);
                const QString reminder_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                QObject::connect(timer,
                                 &QTimer::timeout,
                                 app,
                                 [app,
                                  tray,
                                  reminder_id,
                                  reminder_message,
                                  show_tray_messages,
                                  show_in_app_messages,
                                  timer,
                                  runtime,
                                  remaining_timers]() {
                    deliver_local_reminder(app,
                                           tray,
                                           get_text(QStringLiteral("Lembrete de tarefa")),
                                           reminder_message,
                                           show_tray_messages,
                                           show_in_app_messages);
                    unregister_active_local_reminder(reminder_id);
                    timer->deleteLater();
                    --(*remaining_timers);
                    if (*remaining_timers <= 0) {
                        runtime->deleteLater();
                    }
                });
                timer->start(interval_ms);
                register_active_local_reminder(reminder_id, runtime, timer, schedule.due, reminder_title, schedule.modules);
                if (scheduled_preview.size() < 5) {
                    scheduled_preview << QStringLiteral("%1 - %2")
                                             .arg(schedule.due.toString(QStringLiteral("dd/MM/yyyy HH:mm")),
                                                  reminder_title);
                }
                ++scheduled;
            }
        }

        save_local_reminder_settings();
        active_reminders_text->setPlainText(active_local_reminders_text());

        if (scheduled == 0) {
            runtime->deleteLater();
            QMessageBox::information(&dialog,
                                     get_text(QStringLiteral("Lembretes locais")),
                                     get_text(QStringLiteral("Nenhum lembrete foi agendado com as configurações selecionadas.")));
            return;
        }

        QString message =
            custom_time_mode
                ? get_text(QStringLiteral("%1 lembrete(s) agendado(s) para %2."))
                      .arg(scheduled)
                      .arg(selected_custom_due.toString(QStringLiteral("dd/MM/yyyy HH:mm")))
                : get_text(QStringLiteral("%1 lembrete(s) agendado(s) entre %2 e %3."))
                      .arg(scheduled)
                      .arg(selected_start_date.toString(QStringLiteral("dd/MM/yyyy")))
                      .arg(selected_end_date.toString(QStringLiteral("dd/MM/yyyy")));
        if (!scheduled_preview.isEmpty()) {
            message += QStringLiteral("\n\n%1\n%2")
                           .arg(get_text(QStringLiteral("Próximos lembretes agendados:")),
                                scheduled_preview.join(QLatin1Char('\n')));
        }

        QMessageBox::information(&dialog, get_text(QStringLiteral("Lembretes locais")), message);
    };

    auto *bottom_buttons_widget = new QWidget(&dialog);
    auto *bottom_buttons_layout = new QHBoxLayout(bottom_buttons_widget);
    bottom_buttons_layout->setContentsMargins(0, 0, 0, 0);
    bottom_buttons_layout->setSpacing(6);

    auto *restore_defaults_button = new QPushButton(get_text(QStringLiteral("Restaurar padrão")), bottom_buttons_widget);
    auto *remove_active_button = new QPushButton(get_text(QStringLiteral("Limpar ativos")), bottom_buttons_widget);
    auto *save_settings_button = new QPushButton(get_text(QStringLiteral("Salvar definições")), bottom_buttons_widget);
    auto *apply_button = new QPushButton(get_text(QStringLiteral("Aplicar lembretes")), bottom_buttons_widget);
    auto *close_button = new QPushButton(get_text(QStringLiteral("Fechar")), bottom_buttons_widget);

    bottom_buttons_layout->addWidget(restore_defaults_button);
    bottom_buttons_layout->addWidget(remove_active_button);
    bottom_buttons_layout->addStretch(1);
    bottom_buttons_layout->addWidget(save_settings_button);
    bottom_buttons_layout->addWidget(apply_button);
    bottom_buttons_layout->addWidget(close_button);
    layout->addWidget(bottom_buttons_widget);

    QObject::connect(restore_defaults_button, &QPushButton::clicked, &dialog, restore_local_defaults);
    QObject::connect(remove_active_button, &QPushButton::clicked, &dialog, [&]() {
        const int removed = cancel_active_local_reminders();
        active_reminders_text->setPlainText(active_local_reminders_text());
        QMessageBox::information(&dialog,
                                 get_text(QStringLiteral("Lembretes locais")),
                                 removed > 0
                                     ? get_text(QStringLiteral("%1 lembrete(s) ativo(s) removido(s).")).arg(removed)
                                     : get_text(QStringLiteral("Nenhum lembrete ativo para remover.")));
    });
    QObject::connect(save_settings_button, &QPushButton::clicked, &dialog, [&]() {
        save_local_reminder_settings();
        QMessageBox::information(&dialog,
                                 get_text(QStringLiteral("Lembretes locais")),
                                 get_text(QStringLiteral("Definições registradas.")));
    });
    QObject::connect(apply_button, &QPushButton::clicked, &dialog, apply_local_reminders);
    QObject::connect(close_button, &QPushButton::clicked, &dialog, &QDialog::reject);

    const auto retranslate_ui = [&]() {
        dialog.setWindowTitle(get_text(QStringLiteral("Ativar lembretes locais")));
        methods_group->setTitle(get_text(QStringLiteral("Origem")));
        selection_group->setTitle(get_text(QStringLiteral("Itens")));
        all_items_radio->setText(get_text(QStringLiteral("Todos filtrados")));
        selected_items_radio->setText(get_text(QStringLiteral("Somente selecionados")));
        item_filter->setPlaceholderText(get_text(QStringLiteral("Pesquisar item")));
        select_visible_button->setText(get_text(QStringLiteral("Selecionar visíveis")));
        clear_selection_button->setText(get_text(QStringLiteral("Limpar")));
        active_group->setTitle(get_text(QStringLiteral("Lembretes ativos")));
        test_button->setText(get_text(QStringLiteral("Testar")));
        timing_group->setTitle(get_text(QStringLiteral("Quando alertar")));
        use_item_due_time->setText(get_text(QStringLiteral("Usar data e horário de cada tarefa")));
        use_custom_due_time->setText(get_text(QStringLiteral("Definir um único horário personalizado")));
        lbl_custom_date->setText(get_text(QStringLiteral("Data")));
        lbl_custom_time->setText(get_text(QStringLiteral("Hora")));
        lbl_start_date->setText(get_text(QStringLiteral("Início")));
        lbl_end_date->setText(get_text(QStringLiteral("Fim")));
        lbl_quick_period->setText(get_text(QStringLiteral("Período")));
        period_unit->setItemText(0, get_text(QStringLiteral("Dias")));
        period_unit->setItemText(1, get_text(QStringLiteral("Semanas")));
        period_unit->setItemText(2, get_text(QStringLiteral("Meses")));
        apply_period->setText(get_text(QStringLiteral("Aplicar")));
        lbl_start_time->setText(get_text(QStringLiteral("Hora inicial")));
        lbl_end_time->setText(get_text(QStringLiteral("Hora final")));
        options_group->setTitle(get_text(QStringLiteral("Avisos")));
        lbl_max_reminders->setText(get_text(QStringLiteral("Máximo")));
        include_completed->setText(get_text(QStringLiteral("Incluir itens concluídos")));
        show_in_app->setText(get_text(QStringLiteral("Alertar no app")));
        show_tray->setText(get_text(QStringLiteral("Bandeja do sistema")));
        use_default_time->setText(get_text(QStringLiteral("Horário padrão se faltar")));
        lbl_default_time->setText(get_text(QStringLiteral("Horário padrão")));
        calendar_group->setTitle(get_text(QStringLiteral("Calendário")));
        set_start_button->setText(get_text(QStringLiteral("Início")));
        set_end_button->setText(get_text(QStringLiteral("Fim")));
        restore_defaults_button->setText(get_text(QStringLiteral("Restaurar padrão")));
        remove_active_button->setText(get_text(QStringLiteral("Limpar ativos")));
        save_settings_button->setText(get_text(QStringLiteral("Salvar definições")));
        apply_button->setText(get_text(QStringLiteral("Aplicar lembretes")));
        close_button->setText(get_text(QStringLiteral("Fechar")));
        const QString date_fmt = QLocale().dateFormat(QLocale::ShortFormat);
        custom_date->setDisplayFormat(date_fmt);
        start_date->setDisplayFormat(date_fmt);
        end_date->setDisplayFormat(date_fmt);
        update_summary();
    };
    dialog.set_retranslate_handler([&]() { retranslate_ui(); });
    retranslate_ui();

    layout->activate();
    const int content_width = qMax(dialog.minimumSizeHint().width(), dialog.sizeHint().width());
    dialog.setMinimumWidth(content_width);
    dialog.resize(content_width, dialog.height());
    update_calendar_strip(calendar_strip);
    const QSize local_reminder_initial_size = dialog.size();
    const QSize local_reminder_minimum_size(dialog.minimumWidth(), dialog.minimumHeight());
    auto *restore_geometry_filter = new RestoreNormalGeometryFilter(
        &dialog,
        local_reminder_initial_size,
        local_reminder_minimum_size,
        [&]() {
            compact_calendar_strip_to_base(calendar_strip);
        },
        [&]() {
            update_responsive_grid(methods_grid.get());
            update_calendar_strip(calendar_strip);
        },
        false,
        &dialog);
    dialog.installEventFilter(restore_geometry_filter);

    dialog.exec();
}


}  // namespace source::InterfaceCore::ProductivityFeatures
