#include "source/services/incore_14_EditTaskDialog.hpp"

#include <QApplication>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDateEdit>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPushButton>
#include <QResizeEvent>
#include <QSpinBox>
#include <QTextEdit>
#include <QTime>
#include <QTimeEdit>
#include <QUrl>
#include <QVBoxLayout>

#include "source/services/incore_01_initUI.hpp"
#include "source/services/incore_13_prioridade_display.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"

namespace source::InterfaceCore::incore_14_EditTaskDialog {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

static void clear_layout_items(QLayout *layout)
{
    if (layout == nullptr) {
        return;
    }
    while (auto *item = layout->takeAt(0)) {
        if (auto *child_layout = item->layout()) {
            clear_layout_items(child_layout);
        }
        delete item;
    }
}

int compute_widget_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name)
{
    if (app != nullptr && app->widget_heights.contains(name)) {
        bool ok = false;
        const int value = app->widget_heights.value(name).toInt(&ok);
        if (ok && value > 0) {
            return value;
        }
    }

    if (app != nullptr && app->widget_height_override > 0) {
        return app->widget_height_override;
    }

    return widget != nullptr ? qMax(26, int(widget->fontMetrics().height() * 1.2)) : 28;
}

void apply_widget_min_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name)
{
    if (widget != nullptr) {
        widget->setMinimumHeight(compute_widget_height(app, widget, name));
    }
}

QLocale effective_locale(source::EisenhowerMatrixApp *app)
{
    if (app != nullptr && app->date_input != nullptr) {
        return app->date_input->locale();
    }

    if (app != nullptr && app->gerenciador_traducao != nullptr) {
        const QString idioma = app->gerenciador_traducao->obter_idioma_atual();
        if (idioma.startsWith(QStringLiteral("pt"))) {
            return QLocale(QLocale::Portuguese, QLocale::Brazil);
        }
        if (idioma.startsWith(QStringLiteral("en"))) {
            return QLocale(QLocale::English, QLocale::UnitedStates);
        }
    }

    return QLocale::system();
}

void apply_locale_to_dateedit(source::EisenhowerMatrixApp *app, QDateEdit *date_edit)
{
    if (date_edit == nullptr) {
        return;
    }

    const QLocale locale = effective_locale(app);
    date_edit->setLocale(locale);

    QCalendarWidget *calendar = date_edit->calendarWidget();
    if (calendar == nullptr) {
        calendar = new QCalendarWidget(date_edit);
        date_edit->setCalendarWidget(calendar);
    }
    calendar->setLocale(locale);

    if (auto *qt_app = qobject_cast<QApplication *>(QCoreApplication::instance())) {
        const QColor fill_color = qt_app->palette().color(QPalette::Window);
        QPalette palette = date_edit->palette();
        palette.setColor(QPalette::Base, fill_color);
        palette.setColor(QPalette::AlternateBase, fill_color);
        date_edit->setPalette(palette);

        QPalette calendar_palette = calendar->palette();
        calendar_palette.setColor(QPalette::Base, fill_color);
        calendar_palette.setColor(QPalette::AlternateBase, fill_color);
        calendar->setPalette(calendar_palette);
    }
}

EditTaskDialog::EditTaskDialog(source::EisenhowerMatrixApp *app,
                               QWidget *parent,
                               const QString &initial_text,
                               const QString &initial_file,
                               const QString &initial_description,
                               const QString &initial_date_iso,
                               const QString &initial_time_str,
                               const QVariant &initial_priority,
                               const QVariant &initial_domain,
                               int initial_pomodoros,
                               double initial_duration,
                               int initial_ciclos_totais)
    : QDialog(parent != nullptr ? parent : app), app_(app)
{
    setModal(true);
    setWindowTitle(get_text(QStringLiteral("✏️ Editar Tarefa")));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    root_layout_ = new QVBoxLayout(this);
    root_layout_->setContentsMargins(12, 12, 12, 12);
    root_layout_->setSpacing(8);
    root_layout_->setSizeConstraint(QLayout::SetNoConstraint);

    // 1. Contêiner dos campos superiores (Tarefa, Prioridade, Domínio, Pomodoros, Arquivo)
    top_fields_container_ = new QWidget(this);
    auto *top_layout = new QVBoxLayout(top_fields_container_);
    top_layout->setContentsMargins(0, 0, 0, 0);
    top_layout->setSpacing(4);

    label_tarefa_ = new QLabel(get_text(QStringLiteral("Tarefa:")), top_fields_container_);
    label_tarefa_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label_tarefa_->setFixedHeight(label_tarefa_->fontMetrics().height() + 4);
    top_layout->addWidget(label_tarefa_);

    title_ = new QLineEdit(top_fields_container_);
    title_->setText(initial_text);
    title_->setFixedHeight(compute_widget_height(app_, title_, QStringLiteral("dialog_title_edit")));
    top_layout->addWidget(title_);

    label_prioridade_ = new QLabel(get_text(QStringLiteral("Prioridade:")), top_fields_container_);
    label_prioridade_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label_prioridade_->setFixedHeight(label_prioridade_->fontMetrics().height() + 4);
    top_layout->addWidget(label_prioridade_);

    priority_combo_ = new QComboBox(top_fields_container_);
    priority_labels_ = {
        get_text(QStringLiteral("🔴 Importante e Urgente")),
        get_text(QStringLiteral("🟠 Importante, mas Não Urgente")),
        get_text(QStringLiteral("🟡 Não Importante, mas Urgente")),
        get_text(QStringLiteral("🟢 Não Importante e Não Urgente")),
    };
    priority_combo_->addItems(priority_labels_);

    int priority_index = 0;
    bool ok = false;
    const int prio = initial_priority.toInt(&ok);
    if (ok && prio >= 1 && prio <= 4) {
        priority_index = prio - 1;
    } else {
        const QString normalized = source::InterfaceCore::prioridade_para_texto(initial_priority, app_);
        const int idx = priority_labels_.indexOf(normalized);
        if (idx >= 0) {
            priority_index = idx;
        }
    }
    priority_combo_->setCurrentIndex(priority_index);
    priority_combo_->setFixedHeight(compute_widget_height(app_, priority_combo_, QStringLiteral("dialog_priority_combo")));
    top_layout->addWidget(priority_combo_);

    label_dominio_ = new QLabel(get_text(QStringLiteral("Nível de Domínio:")), top_fields_container_);
    label_dominio_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label_dominio_->setFixedHeight(label_dominio_->fontMetrics().height() + 4);
    top_layout->addWidget(label_dominio_);

    domain_combo_ = new QComboBox(top_fields_container_);
    domain_combo_->addItems({
        source::InterfaceCore::dominio_para_texto(0, false),
        source::InterfaceCore::dominio_para_texto(1, false),
        source::InterfaceCore::dominio_para_texto(2, false),
    });
    bool domain_ok = false;
    const int initial_domain_value = qBound(0, initial_domain.toInt(&domain_ok), 2);
    if (domain_ok) {
        domain_combo_->setCurrentIndex(initial_domain_value);
    } else {
        const QString normalized_domain = source::InterfaceCore::dominio_para_texto(initial_domain, false);
        const int domain_index = domain_combo_->findText(normalized_domain);
        domain_combo_->setCurrentIndex(domain_index >= 0 ? domain_index : 0);
    }
    domain_combo_->setFixedHeight(compute_widget_height(app_, domain_combo_, QStringLiteral("dialog_domain_combo")));
    top_layout->addWidget(domain_combo_);

    label_pomodoro_ = new QLabel(get_text(QStringLiteral("Quantidade de Pomodoros (🍅):")), top_fields_container_);
    label_pomodoro_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label_pomodoro_->setFixedHeight(label_pomodoro_->fontMetrics().height() + 4);
    top_layout->addWidget(label_pomodoro_);

    pomodoros_spin_ = new QSpinBox(top_fields_container_);
    pomodoros_spin_->setRange(0, 999);
    pomodoros_spin_->setValue(qMax(0, initial_pomodoros));
    pomodoros_spin_->setFixedHeight(compute_widget_height(app_, pomodoros_spin_, QStringLiteral("dialog_pomodoros_spin")));
    top_layout->addWidget(pomodoros_spin_);

    label_ciclos_ = new QLabel(get_text(QStringLiteral("Meta de Ciclos Pomodoro (🎯):")), top_fields_container_);
    label_ciclos_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label_ciclos_->setFixedHeight(label_ciclos_->fontMetrics().height() + 4);
    top_layout->addWidget(label_ciclos_);

    const int default_ciclos = app_ != nullptr ? app_->default_pomodoro_cycle_count() : 4;
    ciclos_totais_spin_ = new QSpinBox(top_fields_container_);
    ciclos_totais_spin_->setRange(1, 999);
    ciclos_totais_spin_->setValue(initial_ciclos_totais > 0 ? initial_ciclos_totais : default_ciclos);
    ciclos_totais_spin_->setFixedHeight(compute_widget_height(app_, ciclos_totais_spin_, QStringLiteral("dialog_ciclos_totais_spin")));
    top_layout->addWidget(ciclos_totais_spin_);

    label_duracao_ = new QLabel(get_text(QStringLiteral("Duração:")), top_fields_container_);
    label_duracao_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label_duracao_->setFixedHeight(label_duracao_->fontMetrics().height() + 4);
    top_layout->addWidget(label_duracao_);

    duration_spin_ = new QDoubleSpinBox(top_fields_container_);
    duration_spin_->setRange(0.0, 1000.0);
    duration_spin_->setValue(initial_duration > 0.0 ? initial_duration : 1.0);
    duration_spin_->setSingleStep(0.5);
    duration_spin_->setSuffix(get_text(QStringLiteral(" dias")));
    duration_spin_->setFixedHeight(compute_widget_height(app_, duration_spin_, QStringLiteral("dialog_duration_spin")));
    top_layout->addWidget(duration_spin_);

    label_arquivo_ = new QLabel(get_text(QStringLiteral("Arquivo vinculado (arraste ou escolha):")), top_fields_container_);
    label_arquivo_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label_arquivo_->setFixedHeight(label_arquivo_->fontMetrics().height() + 4);
    top_layout->addWidget(label_arquivo_);

    auto *file_row = new QHBoxLayout();
    file_row->setContentsMargins(0, 0, 0, 0);
    file_row->setSpacing(6);

    file_field_ = new incore_01_initUI::FileDropLineEdit(app_, top_fields_container_);
    file_field_->setText(QFileInfo(initial_file).fileName().isEmpty() ? initial_file : QFileInfo(initial_file).fileName());
    file_field_->setToolTip(initial_file);
    file_field_->setFixedHeight(compute_widget_height(app_, file_field_, QStringLiteral("dialog_file_field")));
    file_row->addWidget(file_field_, 1);

    btn_choose_ = new QPushButton(get_text(QStringLiteral("Escolher...")), top_fields_container_);
    btn_open_ = new QPushButton(get_text(QStringLiteral("Abrir")), top_fields_container_);
    btn_clear_ = new QPushButton(get_text(QStringLiteral("Remover")), top_fields_container_);
    btn_choose_->setFixedHeight(compute_widget_height(app_, btn_choose_, QStringLiteral("dialog_btn_choose")));
    btn_open_->setFixedHeight(compute_widget_height(app_, btn_open_, QStringLiteral("dialog_btn_open")));
    btn_clear_->setFixedHeight(compute_widget_height(app_, btn_clear_, QStringLiteral("dialog_btn_clear")));
    btn_open_->setEnabled(!initial_file.isEmpty());

    connect(btn_choose_, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getOpenFileName(this, get_text(QStringLiteral("Escolher arquivo")), QString(), QStringLiteral("*"));
        if (!path.isEmpty()) {
            file_field_->setText(QFileInfo(path).fileName());
            file_field_->setToolTip(path);
            if (btn_open_ != nullptr) btn_open_->setEnabled(true);
        }
    });
    connect(btn_open_, &QPushButton::clicked, this, [this]() {
        const QString path = file_field_->toolTip();
        if (!path.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
    });
    connect(btn_clear_, &QPushButton::clicked, this, [this]() {
        file_field_->clear();
        file_field_->setToolTip(QString());
        if (btn_open_ != nullptr) btn_open_->setEnabled(false);
    });

    file_row->addWidget(btn_choose_);
    file_row->addWidget(btn_open_);
    file_row->addWidget(btn_clear_);
    top_layout->addLayout(file_row);

    top_layout->activate();
    top_fields_height_ = qMax(310, top_layout->sizeHint().height());
    top_fields_container_->setFixedHeight(top_fields_height_);
    top_fields_container_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // 2. Contêiner da descrição ("Descrição (opcional):")
    desc_container_ = new QWidget(this);
    auto *desc_layout = new QVBoxLayout(desc_container_);
    desc_layout->setContentsMargins(0, 0, 0, 0);
    desc_layout->setSpacing(4);

    label_desc_ = new QLabel(get_text(QStringLiteral("Descrição (opcional):")), desc_container_);
    label_desc_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label_desc_->setFixedHeight(label_desc_->fontMetrics().height() + 4);
    desc_layout->addWidget(label_desc_);

    desc_ = new QTextEdit(desc_container_);
    desc_->setPlainText(initial_description);
    desc_->setMinimumHeight(60);
    desc_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    desc_layout->addWidget(desc_, 1);
    desc_container_->setMinimumHeight(60 + label_desc_->fontMetrics().height() + 8);

    // 3. Contêiner dos campos inferiores (Data e horário, Cancelar e OK)
    bottom_fields_container_ = new QWidget(this);
    auto *bottom_layout = new QVBoxLayout(bottom_fields_container_);
    bottom_layout->setContentsMargins(0, 0, 0, 0);
    bottom_layout->setSpacing(4);

    label_data_hora_ = new QLabel(get_text(QStringLiteral("Data e horário (opcional):")), bottom_fields_container_);
    label_data_hora_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    label_data_hora_->setFixedHeight(label_data_hora_->fontMetrics().height() + 4);
    bottom_layout->addWidget(label_data_hora_);

    auto *date_row = new QHBoxLayout();
    date_row->setContentsMargins(0, 0, 0, 0);
    date_row->setSpacing(6);

    date_enabled_ = new QCheckBox(get_text(QStringLiteral("Vincular data")), bottom_fields_container_);
    date_enabled_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    date_enabled_->setFixedHeight(compute_widget_height(app_, date_enabled_, QStringLiteral("dialog_date_enabled")));
    date_edit_ = new QDateEdit(bottom_fields_container_);
    date_edit_->setCalendarPopup(true);
    date_edit_->setDisplayFormat(app_ != nullptr && app_->date_input != nullptr ? app_->date_input->displayFormat()
                                                                                : QStringLiteral("dd/MM/yyyy"));
    date_edit_->setDate(QDate::currentDate());
    date_edit_->setFixedHeight(compute_widget_height(app_, date_edit_, QStringLiteral("dialog_date_edit")));
    apply_locale_to_dateedit(app_, date_edit_);

    const QDate init_date = QDate::fromString(initial_date_iso, Qt::ISODate);
    date_enabled_->setChecked(init_date.isValid());
    if (init_date.isValid()) {
        date_edit_->setDate(init_date);
    }
    date_edit_->setEnabled(date_enabled_->isChecked());
    connect(date_enabled_, &QCheckBox::toggled, date_edit_, &QWidget::setEnabled);

    date_row->addWidget(date_enabled_);
    date_row->addWidget(date_edit_, 1);
    bottom_layout->addLayout(date_row);

    auto *time_row = new QHBoxLayout();
    time_row->setContentsMargins(0, 0, 0, 0);
    time_row->setSpacing(6);

    time_enabled_ = new QCheckBox(get_text(QStringLiteral("Vincular horário")), bottom_fields_container_);
    time_enabled_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    time_enabled_->setFixedHeight(compute_widget_height(app_, time_enabled_, QStringLiteral("dialog_time_enabled")));
    time_edit_ = new QTimeEdit(bottom_fields_container_);
    time_edit_->setDisplayFormat(QStringLiteral("HH:mm"));
    time_edit_->setTime(QTime::currentTime());
    time_edit_->setFixedHeight(compute_widget_height(app_, time_edit_, QStringLiteral("dialog_time_edit")));

    const QTime init_time = QTime::fromString(initial_time_str, QStringLiteral("HH:mm"));
    time_enabled_->setChecked(init_date.isValid() && init_time.isValid());
    if (init_time.isValid()) {
        time_edit_->setTime(init_time);
    }

    auto sync_time_enabled = [this]() {
        time_enabled_->setEnabled(date_enabled_->isChecked());
        if (!date_enabled_->isChecked()) {
            time_enabled_->setChecked(false);
        }
        time_edit_->setEnabled(date_enabled_->isChecked() && time_enabled_->isChecked());
    };
    connect(date_enabled_, &QCheckBox::toggled, this, [sync_time_enabled]() { sync_time_enabled(); });
    connect(time_enabled_, &QCheckBox::toggled, this, [sync_time_enabled]() { sync_time_enabled(); });
    sync_time_enabled();

    time_row->addWidget(time_enabled_);
    time_row->addWidget(time_edit_, 1);
    bottom_layout->addLayout(time_row);

    auto *buttons = new QHBoxLayout();
    buttons->setContentsMargins(0, 6, 0, 0);
    buttons->setSpacing(8);
    buttons->addStretch(1);

    btn_cancel_ = new QPushButton(get_text(QStringLiteral("Cancelar")), bottom_fields_container_);
    btn_ok_ = new QPushButton(get_text(QStringLiteral("OK")), bottom_fields_container_);
    btn_cancel_->setFixedSize(85, compute_widget_height(app_, btn_cancel_, QStringLiteral("dialog_btn_cancel")));
    btn_ok_->setFixedSize(85, compute_widget_height(app_, btn_ok_, QStringLiteral("dialog_btn_ok")));

    connect(btn_cancel_, &QPushButton::clicked, this, &QDialog::reject);
    connect(btn_ok_, &QPushButton::clicked, this, &QDialog::accept);

    buttons->addWidget(btn_cancel_);
    buttons->addWidget(btn_ok_);
    bottom_layout->addLayout(buttons);

    bottom_layout->activate();
    bottom_fields_height_ = qMax(120, bottom_layout->sizeHint().height());
    bottom_fields_container_->setFixedHeight(bottom_fields_height_);
    bottom_fields_container_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    const int compact_min_h = top_fields_height_ + bottom_fields_height_ + 60 + 18 + 4 + 16 + 24;
    setMinimumSize(440, compact_min_h);
    resize(480, 600);
    _update_layout_mode(width(), height());
}

void EditTaskDialog::_update_layout_mode(int width, int height)
{
    Q_UNUSED(height);

    LayoutMode target_mode = current_layout_mode_;
    if (isMaximized()) {
        target_mode = LayoutMode::Wide;
    } else if (current_layout_mode_ == LayoutMode::Wide) {
        if (width < 680) {
            target_mode = LayoutMode::Compact;
        }
    } else {
        if (width >= 760) {
            target_mode = LayoutMode::Wide;
        } else {
            target_mode = LayoutMode::Compact;
        }
    }

    if (target_mode == current_layout_mode_) {
        return;
    }
    current_layout_mode_ = target_mode;
    if (target_mode == LayoutMode::Wide) {
        _apply_wide_layout();
    } else {
        _apply_compact_layout();
    }
}

void EditTaskDialog::_apply_compact_layout()
{
    setUpdatesEnabled(false);
    clear_layout_items(root_layout_);

    top_fields_container_->setMinimumWidth(0);
    top_fields_container_->setMaximumWidth(QWIDGETSIZE_MAX);
    top_fields_container_->setFixedHeight(top_fields_height_);
    top_fields_container_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    bottom_fields_container_->setMinimumWidth(0);
    bottom_fields_container_->setMaximumWidth(QWIDGETSIZE_MAX);
    bottom_fields_container_->setFixedHeight(bottom_fields_height_);
    bottom_fields_container_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    root_layout_->addWidget(top_fields_container_);
    root_layout_->addWidget(desc_container_, 1);
    root_layout_->addWidget(bottom_fields_container_);

    const int compact_min_h = top_fields_height_ + bottom_fields_height_ + 60 + 18 + 4 + 16 + 24;
    setMinimumSize(440, compact_min_h);
    if (height() < compact_min_h) {
        resize(width(), compact_min_h);
    }

    root_layout_->activate();
    setUpdatesEnabled(true);
}

void EditTaskDialog::_apply_wide_layout()
{
    setUpdatesEnabled(false);
    clear_layout_items(root_layout_);

    const int frozen_width = 410;
    top_fields_container_->setFixedSize(frozen_width, top_fields_height_);
    top_fields_container_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    bottom_fields_container_->setFixedSize(frozen_width, bottom_fields_height_);
    bottom_fields_container_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto *h_layout = new QHBoxLayout();
    h_layout->setContentsMargins(0, 0, 0, 0);
    h_layout->setSpacing(12);

    h_layout->addWidget(desc_container_, 1);

    auto *right_vbox = new QVBoxLayout();
    right_vbox->setContentsMargins(0, 0, 0, 0);
    right_vbox->setSpacing(6);
    right_vbox->addStretch(1);
    right_vbox->addWidget(top_fields_container_);
    right_vbox->addWidget(bottom_fields_container_);
    right_vbox->setAlignment(Qt::AlignRight | Qt::AlignBottom);

    h_layout->addLayout(right_vbox, 0);
    root_layout_->addLayout(h_layout);

    const int wide_min_h = top_fields_height_ + bottom_fields_height_ + 6 + 24;
    setMinimumSize(440, wide_min_h);

    root_layout_->activate();
    setUpdatesEnabled(true);
}

void EditTaskDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (event != nullptr) {
        _update_layout_mode(event->size().width(), event->size().height());
    }
}

void EditTaskDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event != nullptr) {
        if (event->type() == QEvent::WindowStateChange) {
            _update_layout_mode(width(), height());
        } else if (event->type() == QEvent::LanguageChange) {
            retranslateUi();
        }
    }
}

void EditTaskDialog::retranslateUi()
{
    setWindowTitle(get_text(QStringLiteral("✏️ Editar Tarefa")));

    if (label_tarefa_ != nullptr) {
        label_tarefa_->setText(get_text(QStringLiteral("Tarefa:")));
    }
    if (label_prioridade_ != nullptr) {
        label_prioridade_->setText(get_text(QStringLiteral("Prioridade:")));
    }
    if (priority_combo_ != nullptr) {
        const int curr_prio = priority_combo_->currentIndex();
        priority_combo_->blockSignals(true);
        priority_combo_->clear();
        priority_labels_ = {
            get_text(QStringLiteral("🔴 Importante e Urgente")),
            get_text(QStringLiteral("🟠 Importante, mas Não Urgente")),
            get_text(QStringLiteral("🟡 Não Importante, mas Urgente")),
            get_text(QStringLiteral("🟢 Não Importante e Não Urgente")),
        };
        priority_combo_->addItems(priority_labels_);
        if (curr_prio >= 0 && curr_prio < priority_combo_->count()) {
            priority_combo_->setCurrentIndex(curr_prio);
        }
        priority_combo_->blockSignals(false);
    }
    if (label_dominio_ != nullptr) {
        label_dominio_->setText(get_text(QStringLiteral("Nível de Domínio:")));
    }
    if (domain_combo_ != nullptr) {
        const int curr_domain = domain_combo_->currentIndex();
        domain_combo_->blockSignals(true);
        domain_combo_->clear();
        domain_combo_->addItems({
            source::InterfaceCore::dominio_para_texto(0, false),
            source::InterfaceCore::dominio_para_texto(1, false),
            source::InterfaceCore::dominio_para_texto(2, false),
        });
        if (curr_domain >= 0 && curr_domain < domain_combo_->count()) {
            domain_combo_->setCurrentIndex(curr_domain);
        }
        domain_combo_->blockSignals(false);
    }
    if (label_pomodoro_ != nullptr) {
        label_pomodoro_->setText(get_text(QStringLiteral("Quantidade de Pomodoros (🍅):")));
    }
    if (label_ciclos_ != nullptr) {
        label_ciclos_->setText(get_text(QStringLiteral("Meta de Ciclos Pomodoro (🎯):")));
    }
    if (label_duracao_ != nullptr) {
        label_duracao_->setText(get_text(QStringLiteral("Duração:")));
    }
    if (duration_spin_ != nullptr) {
        duration_spin_->setSuffix(get_text(QStringLiteral(" dias")));
    }
    if (label_arquivo_ != nullptr) {
        label_arquivo_->setText(get_text(QStringLiteral("Arquivo vinculado (arraste ou escolha):")));
    }
    if (btn_choose_ != nullptr) {
        btn_choose_->setText(get_text(QStringLiteral("Escolher...")));
    }
    if (btn_open_ != nullptr) {
        btn_open_->setText(get_text(QStringLiteral("Abrir")));
    }
    if (btn_clear_ != nullptr) {
        btn_clear_->setText(get_text(QStringLiteral("Remover")));
    }
    if (label_desc_ != nullptr) {
        label_desc_->setText(get_text(QStringLiteral("Descrição (opcional):")));
    }
    if (label_data_hora_ != nullptr) {
        label_data_hora_->setText(get_text(QStringLiteral("Data e horário (opcional):")));
    }
    if (date_enabled_ != nullptr) {
        date_enabled_->setText(get_text(QStringLiteral("Vincular data")));
    }
    if (time_enabled_ != nullptr) {
        time_enabled_->setText(get_text(QStringLiteral("Vincular horário")));
    }
    if (date_edit_ != nullptr) {
        apply_locale_to_dateedit(app_, date_edit_);
    }
    if (btn_cancel_ != nullptr) {
        btn_cancel_->setText(get_text(QStringLiteral("Cancelar")));
    }
    if (btn_ok_ != nullptr) {
        btn_ok_->setText(get_text(QStringLiteral("OK")));
    }
}

EditTaskDialog::Values EditTaskDialog::get_values() const
{
    Values values;
    values.text = title_ != nullptr ? title_->text().trimmed() : QString();
    values.file_path = file_field_ != nullptr ? file_field_->toolTip().trimmed() : QString();
    values.description = desc_ != nullptr ? desc_->toPlainText().trimmed() : QString();
    if (date_enabled_ != nullptr && date_enabled_->isChecked() && date_edit_ != nullptr) {
        values.date_iso = date_edit_->date().toString(Qt::ISODate);
        if (time_enabled_ != nullptr && time_enabled_->isChecked() && time_edit_ != nullptr) {
            values.time_str = time_edit_->time().toString(QStringLiteral("HH:mm"));
        }
    }
    values.priority = (priority_combo_ != nullptr ? priority_combo_->currentIndex() : 0) + 1;
    values.domain = domain_combo_ != nullptr ? domain_combo_->currentIndex() : 0;
    values.pomodoros = pomodoros_spin_ != nullptr ? pomodoros_spin_->value() : 0;
    values.duration = duration_spin_ != nullptr ? duration_spin_->value() : 1.0;
    const int default_ciclos = app_ != nullptr ? app_->default_pomodoro_cycle_count() : 4;
    values.ciclos_totais = ciclos_totais_spin_ != nullptr ? ciclos_totais_spin_->value() : default_ciclos;
    return values;
}

}  // namespace source::InterfaceCore::incore_14_EditTaskDialog
