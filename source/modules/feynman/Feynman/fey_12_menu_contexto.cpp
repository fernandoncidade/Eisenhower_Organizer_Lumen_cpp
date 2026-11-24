#include "source/modules/feynman/Feynman/fey_12_menu_contexto.hpp"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QResizeEvent>
#include <QTextEdit>
#include <QTime>
#include <QTimeEdit>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QUuid>
#include <QtGlobal>

#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"

namespace source::plugin::Feynman {
namespace {

class FeynmanFileDropLineEdit final : public QLineEdit {
public:
	explicit FeynmanFileDropLineEdit(QWidget *parent = nullptr) : QLineEdit(parent)
	{
		setAcceptDrops(true);
	}

protected:
	void dragEnterEvent(QDragEnterEvent *event) override
	{
		if (event != nullptr && event->mimeData()->hasUrls()) {
			event->acceptProposedAction();
			return;
		}

		QLineEdit::dragEnterEvent(event);
	}

	void dragMoveEvent(QDragMoveEvent *event) override
	{
		if (event != nullptr && event->mimeData()->hasUrls()) {
			event->acceptProposedAction();
			return;
		}

		QLineEdit::dragMoveEvent(event);
	}

	void dropEvent(QDropEvent *event) override
	{
		if (event != nullptr && event->mimeData()->hasUrls()) {
			const QList<QUrl> urls = event->mimeData()->urls();
			if (!urls.isEmpty()) {
				const QString path = urls.constFirst().toLocalFile().trimmed();
				if (!path.isEmpty()) {
					setText(QFileInfo(path).fileName());
					setToolTip(path);
				}
			}
			event->acceptProposedAction();
			return;
		}

		QLineEdit::dropEvent(event);
	}
};

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

static inline int compute_widget_height(const QWidget *widget)
{
	return widget != nullptr ? qMax(26, int(widget->fontMetrics().height() * 1.2)) : 28;
}

class FeynmanEditTaskDialog final : public QDialog {
public:
	struct Values {
		QString title;
		QString description;
		QString file_path;
		QString date_iso;
		QString time_str;
		double duration = 1.0;
		int priority = kDefaultPriority;
		int domain = kDefaultDomain;
		int pomodoros = 0;
		int ciclos_totais = 4;
	};

	FeynmanEditTaskDialog(const QWidget *context, const QVariantMap &conceito, QWidget *parent = nullptr)
		: QDialog(parent), context_(context)
	{
		setModal(true);
		setWindowTitle(compat::tr(context_, QStringLiteral("✏️ Editar Tarefa"), QStringLiteral("✏️ Edit Task")));
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

		root_layout_ = new QVBoxLayout(this);
		root_layout_->setContentsMargins(12, 12, 12, 12);
		root_layout_->setSpacing(8);
		root_layout_->setSizeConstraint(QLayout::SetNoConstraint);

		// 1. Contêiner dos campos superiores (Tarefa, Prioridade, Domínio, Pomodoros, Ciclos, Duração, Arquivo)
		top_fields_container_ = new QWidget(this);
		auto *top_layout = new QVBoxLayout(top_fields_container_);
		top_layout->setContentsMargins(0, 0, 0, 0);
		top_layout->setSpacing(4);

		label_tarefa_ = new QLabel(compat::tr(context_, QStringLiteral("Tarefa:"), QStringLiteral("Task:")), top_fields_container_);
		label_tarefa_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		label_tarefa_->setFixedHeight(label_tarefa_->fontMetrics().height() + 4);
		top_layout->addWidget(label_tarefa_);

		title_ = new QLineEdit(top_fields_container_);
		title_->setText(conceito.value(QStringLiteral("titulo")).toString().trimmed());
		title_->setFixedHeight(compute_widget_height(title_));
		top_layout->addWidget(title_);

		label_prioridade_ = new QLabel(compat::tr(context_, QStringLiteral("Prioridade:"), QStringLiteral("Priority:")), top_fields_container_);
		label_prioridade_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		label_prioridade_->setFixedHeight(label_prioridade_->fontMetrics().height() + 4);
		top_layout->addWidget(label_prioridade_);

		priority_combo_ = new QComboBox(top_fields_container_);
		priority_combo_->addItem(priority_label(context_, 1), 1);
		priority_combo_->addItem(priority_label(context_, 2), 2);
		priority_combo_->addItem(priority_label(context_, 3), 3);
		priority_combo_->addItem(priority_label(context_, 4), 4);
		const int priority_index =
			priority_combo_->findData(normalized_priority_value(conceito.value(QStringLiteral("prioridade"))));
		priority_combo_->setCurrentIndex(priority_index >= 0 ? priority_index : 0);
		priority_combo_->setFixedHeight(compute_widget_height(priority_combo_));
		top_layout->addWidget(priority_combo_);

		label_dominio_ = new QLabel(compat::tr(context_, QStringLiteral("Nível de Domínio:"), QStringLiteral("Mastery Level:")), top_fields_container_);
		label_dominio_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		label_dominio_->setFixedHeight(label_dominio_->fontMetrics().height() + 4);
		top_layout->addWidget(label_dominio_);

		domain_combo_ = new QComboBox(top_fields_container_);
		domain_combo_->addItem(domain_label(context_, 0, false), 0);
		domain_combo_->addItem(domain_label(context_, 1, false), 1);
		domain_combo_->addItem(domain_label(context_, 2, false), 2);
		const int domain_index =
			domain_combo_->findData(normalized_domain_value(conceito.value(QStringLiteral("dominio"))));
		domain_combo_->setCurrentIndex(domain_index >= 0 ? domain_index : 0);
		domain_combo_->setFixedHeight(compute_widget_height(domain_combo_));
		top_layout->addWidget(domain_combo_);

		label_pomodoro_ = new QLabel(compat::tr(context_, QStringLiteral("Quantidade de Pomodoros (🍅):"), QStringLiteral("Quantity of Pomodoros (🍅):")), top_fields_container_);
		label_pomodoro_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		label_pomodoro_->setFixedHeight(label_pomodoro_->fontMetrics().height() + 4);
		top_layout->addWidget(label_pomodoro_);

		pomodoros_spin_ = new QSpinBox(top_fields_container_);
		pomodoros_spin_->setRange(0, 999);
		int initial_pomodoros = 0;
		if (conceito.contains(QStringLiteral("pomodoros"))) {
			initial_pomodoros = conceito.value(QStringLiteral("pomodoros")).toInt();
		} else if (conceito.contains(QStringLiteral("pomodoros_completados"))) {
			initial_pomodoros = conceito.value(QStringLiteral("pomodoros_completados")).toInt();
		}
		pomodoros_spin_->setValue(initial_pomodoros);
		pomodoros_spin_->setFixedHeight(compute_widget_height(pomodoros_spin_));
		top_layout->addWidget(pomodoros_spin_);

		label_ciclos_ = new QLabel(compat::tr(context_, QStringLiteral("Meta de Ciclos Pomodoro (🎯):"), QStringLiteral("Target Pomodoro Cycles (🎯):")), top_fields_container_);
		label_ciclos_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		label_ciclos_->setFixedHeight(label_ciclos_->fontMetrics().height() + 4);
		top_layout->addWidget(label_ciclos_);

		ciclos_totais_spin_ = new QSpinBox(top_fields_container_);
		ciclos_totais_spin_->setRange(1, 999);
		const int default_ciclos = compat::host_bridge() != nullptr ? compat::host_bridge()->default_pomodoro_cycle_count() : 4;
		const bool is_in_prog = is_concept_in_progress(conceito);
		const bool is_custom = conceito.value(QStringLiteral("ciclos_customizados"), false).toBool()
		                       || (conceito.contains(QStringLiteral("ciclos_totais"))
		                           && conceito.value(QStringLiteral("ciclos_totais")).toInt() > 0
		                           && conceito.value(QStringLiteral("ciclos_totais")).toInt() != default_ciclos);
		const int saved_c_tot = conceito.value(QStringLiteral("ciclos_totais"),
		                        conceito.value(QStringLiteral("ciclos_permitidos"),
		                        conceito.value(QStringLiteral("total_ciclos"), default_ciclos))).toInt();
		int initial_ciclos = default_ciclos;
		if (is_in_prog) {
			initial_ciclos = (is_custom && saved_c_tot > 0) ? saved_c_tot : default_ciclos;
		} else {
			initial_ciclos = saved_c_tot > 0 ? saved_c_tot : default_ciclos;
		}
		if (initial_ciclos <= 0) {
			initial_ciclos = default_ciclos;
		}
		ciclos_totais_spin_->setValue(initial_ciclos);
		ciclos_totais_spin_->setFixedHeight(compute_widget_height(ciclos_totais_spin_));
		top_layout->addWidget(ciclos_totais_spin_);

		label_duracao_ = new QLabel(compat::tr(context_, QStringLiteral("Duração:"), QStringLiteral("Duration:")), top_fields_container_);
		label_duracao_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		label_duracao_->setFixedHeight(label_duracao_->fontMetrics().height() + 4);
		top_layout->addWidget(label_duracao_);

		duration_spin_ = new QDoubleSpinBox(top_fields_container_);
		duration_spin_->setRange(0.0, 1000.0);
		duration_spin_->setSingleStep(0.5);
		double initial_duration = 1.0;
		if (conceito.contains(QStringLiteral("duracao")) && conceito.value(QStringLiteral("duracao")).toDouble() > 0.0) {
			initial_duration = conceito.value(QStringLiteral("duracao")).toDouble();
		} else if (conceito.contains(QStringLiteral("duration")) && conceito.value(QStringLiteral("duration")).toDouble() > 0.0) {
			initial_duration = conceito.value(QStringLiteral("duration")).toDouble();
		}
		duration_spin_->setValue(initial_duration);
		duration_spin_->setSuffix(compat::is_english(context_) ? QStringLiteral(" days") : QStringLiteral(" dias"));
		duration_spin_->setFixedHeight(compute_widget_height(duration_spin_));
		top_layout->addWidget(duration_spin_);

		label_arquivo_ = new QLabel(compat::tr(context_, QStringLiteral("Arquivo vinculado (arraste ou escolha):"), QStringLiteral("Linked file (drag or choose):")), top_fields_container_);
		label_arquivo_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		label_arquivo_->setFixedHeight(label_arquivo_->fontMetrics().height() + 4);
		top_layout->addWidget(label_arquivo_);

		auto *file_row = new QHBoxLayout();
		file_row->setContentsMargins(0, 0, 0, 0);
		file_row->setSpacing(6);

		file_field_ = new FeynmanFileDropLineEdit(top_fields_container_);
		const QString initial_file = conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();
		file_field_->setText(initial_file.isEmpty() ? QString() : QFileInfo(initial_file).fileName());
		file_field_->setToolTip(initial_file);
		file_field_->setFixedHeight(compute_widget_height(file_field_));
		file_row->addWidget(file_field_, 1);

		btn_choose_ = new QPushButton(compat::tr(context_, QStringLiteral("Escolher..."), QStringLiteral("Choose...")), top_fields_container_);
		btn_open_ = new QPushButton(compat::tr(context_, QStringLiteral("Abrir"), QStringLiteral("Open")), top_fields_container_);
		btn_clear_ = new QPushButton(compat::tr(context_, QStringLiteral("Remover"), QStringLiteral("Remove")), top_fields_container_);
		btn_choose_->setFixedHeight(compute_widget_height(btn_choose_));
		btn_open_->setFixedHeight(compute_widget_height(btn_open_));
		btn_clear_->setFixedHeight(compute_widget_height(btn_clear_));

		const auto sync_open_button = [this]() {
			if (btn_open_ != nullptr) {
				btn_open_->setEnabled(file_field_ != nullptr && !file_field_->toolTip().trimmed().isEmpty());
			}
		};

		connect(btn_choose_, &QPushButton::clicked, this, [this, sync_open_button]() {
			const QString path = QFileDialog::getOpenFileName(
				this,
				compat::tr(context_, QStringLiteral("Selecionar arquivo para vincular"), QStringLiteral("Select a file to link")),
				QString(),
				compat::tr(context_, QStringLiteral("Todos os arquivos (*)"), QStringLiteral("All files (*)")));
			if (!path.isEmpty()) {
				file_field_->setText(QFileInfo(path).fileName());
				file_field_->setToolTip(path);
				sync_open_button();
			}
		});
		connect(btn_open_, &QPushButton::clicked, this, [this]() {
			compat::open_local_path(file_field_ != nullptr ? file_field_->toolTip() : QString(), this);
		});
		connect(btn_clear_, &QPushButton::clicked, this, [this, sync_open_button]() {
			if (file_field_ != nullptr) {
				file_field_->clear();
				file_field_->setToolTip(QString());
			}
			sync_open_button();
		});
		connect(file_field_, &QLineEdit::textChanged, this, [sync_open_button](const QString &) { sync_open_button(); });
		sync_open_button();

		file_row->addWidget(btn_choose_);
		file_row->addWidget(btn_open_);
		file_row->addWidget(btn_clear_);
		top_layout->addLayout(file_row);

		top_layout->activate();
		top_fields_height_ = qMax(310, top_layout->sizeHint().height());
		top_fields_container_->setFixedHeight(top_fields_height_);
		top_fields_container_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

		// 2. Contêiner da descrição
		desc_container_ = new QWidget(this);
		auto *desc_layout = new QVBoxLayout(desc_container_);
		desc_layout->setContentsMargins(0, 0, 0, 0);
		desc_layout->setSpacing(4);

		label_desc_ = new QLabel(compat::tr(context_, QStringLiteral("Descrição (opcional):"), QStringLiteral("Description (optional):")), desc_container_);
		label_desc_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		label_desc_->setFixedHeight(label_desc_->fontMetrics().height() + 4);
		desc_layout->addWidget(label_desc_);

		desc_ = new QTextEdit(desc_container_);
		desc_->setPlainText(concept_description_text(context_, conceito));
		desc_->setMinimumHeight(60);
		desc_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		desc_layout->addWidget(desc_, 1);
		desc_container_->setMinimumHeight(60 + label_desc_->fontMetrics().height() + 8);

		// 3. Contêiner dos campos inferiores (Data e horário, Cancelar e OK)
		bottom_fields_container_ = new QWidget(this);
		auto *bottom_layout = new QVBoxLayout(bottom_fields_container_);
		bottom_layout->setContentsMargins(0, 0, 0, 0);
		bottom_layout->setSpacing(4);

		label_data_hora_ = new QLabel(compat::tr(context_, QStringLiteral("Data e horário (opcional):"), QStringLiteral("Date and time (optional):")), bottom_fields_container_);
		label_data_hora_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		label_data_hora_->setFixedHeight(label_data_hora_->fontMetrics().height() + 4);
		bottom_layout->addWidget(label_data_hora_);

		auto *date_row = new QHBoxLayout();
		date_row->setContentsMargins(0, 0, 0, 0);
		date_row->setSpacing(6);

		date_enabled_ = new QCheckBox(compat::tr(context_, QStringLiteral("Vincular data"), QStringLiteral("Link date")), bottom_fields_container_);
		date_enabled_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		date_enabled_->setFixedHeight(compute_widget_height(date_enabled_));
		date_edit_ = new QDateEdit(bottom_fields_container_);
		date_edit_->setCalendarPopup(true);
		date_edit_->setLocale(effective_locale(context_));
		date_edit_->setDisplayFormat(effective_date_format(context_));
		date_edit_->setDate(QDate::currentDate());
		date_edit_->setFixedHeight(compute_widget_height(date_edit_));

		const QDate initial_date =
			QDate::fromString(conceito.value(QStringLiteral("integracao_date")).toString().trimmed(), Qt::ISODate);
		date_enabled_->setChecked(initial_date.isValid());
		if (initial_date.isValid()) {
			date_edit_->setDate(initial_date);
		}
		date_edit_->setEnabled(date_enabled_->isChecked());
		connect(date_enabled_, &QCheckBox::toggled, date_edit_, &QWidget::setEnabled);

		date_row->addWidget(date_enabled_);
		date_row->addWidget(date_edit_, 1);
		bottom_layout->addLayout(date_row);

		auto *time_row = new QHBoxLayout();
		time_row->setContentsMargins(0, 0, 0, 0);
		time_row->setSpacing(6);

		time_enabled_ = new QCheckBox(compat::tr(context_, QStringLiteral("Vincular horário"), QStringLiteral("Link time")), bottom_fields_container_);
		time_enabled_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		time_enabled_->setFixedHeight(compute_widget_height(time_enabled_));
		time_edit_ = new QTimeEdit(bottom_fields_container_);
		time_edit_->setDisplayFormat(QStringLiteral("HH:mm"));
		time_edit_->setTime(QTime::currentTime());
		time_edit_->setFixedHeight(compute_widget_height(time_edit_));

		const QTime initial_time =
			QTime::fromString(conceito.value(QStringLiteral("integracao_time")).toString().trimmed(), QStringLiteral("HH:mm"));
		time_enabled_->setChecked(initial_date.isValid() && initial_time.isValid());
		if (initial_time.isValid()) {
			time_edit_->setTime(initial_time);
		}

		const auto sync_time_enabled = [this]() {
			if (time_enabled_ == nullptr || time_edit_ == nullptr || date_enabled_ == nullptr) {
				return;
			}

			time_enabled_->setEnabled(date_enabled_->isChecked());
			if (!date_enabled_->isChecked()) {
				time_enabled_->setChecked(false);
			}
			time_edit_->setEnabled(date_enabled_->isChecked() && time_enabled_->isChecked());
		};
		connect(date_enabled_, &QCheckBox::toggled, this, [sync_time_enabled](bool) { sync_time_enabled(); });
		connect(time_enabled_, &QCheckBox::toggled, this, [sync_time_enabled](bool) { sync_time_enabled(); });
		sync_time_enabled();

		time_row->addWidget(time_enabled_);
		time_row->addWidget(time_edit_, 1);
		bottom_layout->addLayout(time_row);

		auto *buttons = new QHBoxLayout();
		buttons->setContentsMargins(0, 6, 0, 0);
		buttons->setSpacing(8);
		buttons->addStretch(1);

		btn_cancel_ = new QPushButton(compat::tr(context_, QStringLiteral("Cancelar"), QStringLiteral("Cancel")), bottom_fields_container_);
		btn_ok_ = new QPushButton(compat::tr(context_, QStringLiteral("OK"), QStringLiteral("OK")), bottom_fields_container_);
		btn_cancel_->setFixedSize(85, compute_widget_height(btn_cancel_));
		btn_ok_->setFixedSize(85, compute_widget_height(btn_ok_));

		connect(btn_cancel_, &QPushButton::clicked, this, &QDialog::reject);
		connect(btn_ok_, &QPushButton::clicked, this, &QDialog::accept);

		buttons->addWidget(btn_cancel_);
		buttons->addWidget(btn_ok_);
		bottom_layout->addLayout(buttons);

		bottom_layout->activate();
		bottom_fields_height_ = qMax(120, bottom_layout->sizeHint().height());
		bottom_fields_container_->setFixedHeight(bottom_fields_height_);
		bottom_fields_container_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

		for (QWidget *widget : {static_cast<QWidget *>(title_),
								static_cast<QWidget *>(file_field_),
								static_cast<QWidget *>(priority_combo_),
								static_cast<QWidget *>(domain_combo_),
								static_cast<QWidget *>(duration_spin_),
								static_cast<QWidget *>(pomodoros_spin_),
								static_cast<QWidget *>(ciclos_totais_spin_),
								static_cast<QWidget *>(desc_),
								static_cast<QWidget *>(date_edit_),
								static_cast<QWidget *>(time_edit_)}) {
			apply_window_as_base(widget);
		}

		const int compact_min_h = top_fields_height_ + bottom_fields_height_ + 60 + 18 + 4 + 16 + 24;
		setMinimumSize(440, compact_min_h);
		resize(480, 600);
		_update_layout_mode(width(), height());
	}

	Values values() const
	{
		Values values;
		values.title = title_ != nullptr ? title_->text().trimmed() : QString();
		values.description = desc_ != nullptr ? desc_->toPlainText().trimmed() : QString();
		values.file_path = file_field_ != nullptr ? file_field_->toolTip().trimmed() : QString();
		values.priority = normalized_priority_value(priority_combo_ != nullptr ? priority_combo_->currentData() : QVariant());
		values.domain = normalized_domain_value(domain_combo_ != nullptr ? domain_combo_->currentData() : QVariant());
		values.duration = duration_spin_ != nullptr ? duration_spin_->value() : 1.0;
		values.pomodoros = pomodoros_spin_ != nullptr ? pomodoros_spin_->value() : 0;
		const int default_ciclos = compat::host_bridge() != nullptr ? compat::host_bridge()->default_pomodoro_cycle_count() : 4;
		values.ciclos_totais = ciclos_totais_spin_ != nullptr ? ciclos_totais_spin_->value() : default_ciclos;

		if (date_enabled_ != nullptr && date_enabled_->isChecked() && date_edit_ != nullptr) {
			values.date_iso = date_edit_->date().toString(Qt::ISODate);
			if (time_enabled_ != nullptr && time_enabled_->isChecked() && time_edit_ != nullptr) {
				values.time_str = time_edit_->time().toString(QStringLiteral("HH:mm"));
			}
		}

		return values;
	}

	void retranslateUi()
	{
		setWindowTitle(compat::tr(context_, QStringLiteral("✏️ Editar Tarefa"), QStringLiteral("✏️ Edit Task")));

		if (label_tarefa_ != nullptr) {
			label_tarefa_->setText(compat::tr(context_, QStringLiteral("Tarefa:"), QStringLiteral("Task:")));
		}
		if (label_prioridade_ != nullptr) {
			label_prioridade_->setText(compat::tr(context_, QStringLiteral("Prioridade:"), QStringLiteral("Priority:")));
		}
		if (priority_combo_ != nullptr) {
			const int curr_data = priority_combo_->currentData().toInt();
			priority_combo_->blockSignals(true);
			priority_combo_->clear();
			priority_combo_->addItem(priority_label(context_, 1), 1);
			priority_combo_->addItem(priority_label(context_, 2), 2);
			priority_combo_->addItem(priority_label(context_, 3), 3);
			priority_combo_->addItem(priority_label(context_, 4), 4);
			const int idx = priority_combo_->findData(curr_data);
			if (idx >= 0) {
				priority_combo_->setCurrentIndex(idx);
			}
			priority_combo_->blockSignals(false);
		}
		if (label_dominio_ != nullptr) {
			label_dominio_->setText(compat::tr(context_, QStringLiteral("Nível de Domínio:"), QStringLiteral("Mastery Level:")));
		}
		if (domain_combo_ != nullptr) {
			const int curr_data = domain_combo_->currentData().toInt();
			domain_combo_->blockSignals(true);
			domain_combo_->clear();
			domain_combo_->addItem(domain_label(context_, 0, false), 0);
			domain_combo_->addItem(domain_label(context_, 1, false), 1);
			domain_combo_->addItem(domain_label(context_, 2, false), 2);
			const int idx = domain_combo_->findData(curr_data);
			if (idx >= 0) {
				domain_combo_->setCurrentIndex(idx);
			}
			domain_combo_->blockSignals(false);
		}
		if (label_pomodoro_ != nullptr) {
			label_pomodoro_->setText(compat::tr(context_, QStringLiteral("Quantidade de Pomodoros (🍅):"), QStringLiteral("Quantity of Pomodoros (🍅):")));
		}
		if (label_ciclos_ != nullptr) {
			label_ciclos_->setText(compat::tr(context_, QStringLiteral("Meta de Ciclos Pomodoro (🎯):"), QStringLiteral("Target Pomodoro Cycles (🎯):")));
		}
		if (label_duracao_ != nullptr) {
			label_duracao_->setText(compat::tr(context_, QStringLiteral("Duração:"), QStringLiteral("Duration:")));
		}
		if (duration_spin_ != nullptr) {
			duration_spin_->setSuffix(compat::is_english(context_) ? QStringLiteral(" days") : QStringLiteral(" dias"));
		}
		if (label_arquivo_ != nullptr) {
			label_arquivo_->setText(compat::tr(context_, QStringLiteral("Arquivo vinculado (arraste ou escolha):"), QStringLiteral("Linked file (drag or choose):")));
		}
		if (btn_choose_ != nullptr) {
			btn_choose_->setText(compat::tr(context_, QStringLiteral("Escolher..."), QStringLiteral("Choose...")));
		}
		if (btn_open_ != nullptr) {
			btn_open_->setText(compat::tr(context_, QStringLiteral("Abrir"), QStringLiteral("Open")));
		}
		if (btn_clear_ != nullptr) {
			btn_clear_->setText(compat::tr(context_, QStringLiteral("Remover"), QStringLiteral("Remove")));
		}
		if (label_desc_ != nullptr) {
			label_desc_->setText(compat::tr(context_, QStringLiteral("Descrição (opcional):"), QStringLiteral("Description (optional):")));
		}
		if (label_data_hora_ != nullptr) {
			label_data_hora_->setText(compat::tr(context_, QStringLiteral("Data e horário (opcional):"), QStringLiteral("Date and time (optional):")));
		}
		if (date_enabled_ != nullptr) {
			date_enabled_->setText(compat::tr(context_, QStringLiteral("Vincular data"), QStringLiteral("Link date")));
		}
		if (time_enabled_ != nullptr) {
			time_enabled_->setText(compat::tr(context_, QStringLiteral("Vincular horário"), QStringLiteral("Link time")));
		}
		if (date_edit_ != nullptr) {
			date_edit_->setLocale(effective_locale(context_));
		}
		if (btn_cancel_ != nullptr) {
			btn_cancel_->setText(compat::tr(context_, QStringLiteral("Cancelar"), QStringLiteral("Cancel")));
		}
		if (btn_ok_ != nullptr) {
			btn_ok_->setText(compat::tr(context_, QStringLiteral("OK"), QStringLiteral("OK")));
		}
	}

protected:
	void resizeEvent(QResizeEvent *event) override
	{
		QDialog::resizeEvent(event);
		if (event != nullptr) {
			_update_layout_mode(event->size().width(), event->size().height());
		}
	}

	void changeEvent(QEvent *event) override
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

private:
	enum class LayoutMode { None, Compact, Wide };
	void _update_layout_mode(int width, int height)
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

	void _apply_compact_layout()
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

	void _apply_wide_layout()
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

	const QWidget *context_ = nullptr;
	QLineEdit *title_ = nullptr;
	QComboBox *priority_combo_ = nullptr;
	QComboBox *domain_combo_ = nullptr;
	QDoubleSpinBox *duration_spin_ = nullptr;
	QSpinBox *pomodoros_spin_ = nullptr;
	QSpinBox *ciclos_totais_spin_ = nullptr;
	FeynmanFileDropLineEdit *file_field_ = nullptr;
	QCheckBox *date_enabled_ = nullptr;
	QDateEdit *date_edit_ = nullptr;
	QCheckBox *time_enabled_ = nullptr;
	QTimeEdit *time_edit_ = nullptr;
	QTextEdit *desc_ = nullptr;

	QLabel *label_tarefa_ = nullptr;
	QLabel *label_prioridade_ = nullptr;
	QLabel *label_dominio_ = nullptr;
	QLabel *label_pomodoro_ = nullptr;
	QLabel *label_ciclos_ = nullptr;
	QLabel *label_duracao_ = nullptr;
	QLabel *label_arquivo_ = nullptr;
	QPushButton *btn_choose_ = nullptr;
	QPushButton *btn_open_ = nullptr;
	QPushButton *btn_clear_ = nullptr;
	QLabel *label_desc_ = nullptr;
	QLabel *label_data_hora_ = nullptr;
	QPushButton *btn_cancel_ = nullptr;
	QPushButton *btn_ok_ = nullptr;

	LayoutMode current_layout_mode_ = LayoutMode::None;
	QWidget *top_fields_container_ = nullptr;
	QWidget *desc_container_ = nullptr;
	QWidget *bottom_fields_container_ = nullptr;
	QVBoxLayout *root_layout_ = nullptr;
	int top_fields_height_ = 0;
	int bottom_fields_height_ = 0;
};

}  // namespace

void MetodoFeynman::mostrar_menu_contexto(const QPoint &position)
{
	QListWidgetItem *item = lista_conceitos->itemAt(position);
	if (item == nullptr || is_group_header(item) || !(item->flags() & Qt::ItemIsSelectable)) {
		return;
	}

	QList<QListWidgetItem *> selected_items = lista_conceitos->selectedItems();
	if (selected_items.isEmpty() || !selected_items.contains(item)) {
		lista_conceitos->clearSelection();
		item->setSelected(true);
		lista_conceitos->setCurrentItem(item);
		selected_items = {item};
	}

	const QVariantMap conceito = normalized_concept(obter_conceito_do_item(item));
	if (conceito.isEmpty()) {
		return;
	}
	const QString caminho_vinculado = conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed();

	QMenu menu(this);
	QAction *acao_editar =
		menu.addAction(compat::tr(this, QStringLiteral("✏️ Editar Tarefa"), QStringLiteral("✏️ Edit Task")));
	connect(acao_editar, &QAction::triggered, this, [this, item]() { editar_tarefa(item); });

	QAction *acao_fonte = menu.addAction(
		compat::tr(this, QStringLiteral("🔤 Configurações de Fonte"), QStringLiteral("🔤 Font Settings")));
	connect(acao_fonte, &QAction::triggered, this, [this, selected_items]() {
		QList<QListWidgetItem *> target_items = selected_items;
		if (source::utils::open_task_font_settings_dialog(this, target_items, source::utils::PluginFontContext::Feynman)) {
			if (QWidget *top = this->window()) {
				for (QListWidget *list : top->findChildren<QListWidget *>()) {
					if (list == nullptr) continue;
					for (int i = 0; i < list->count(); ++i) {
						QListWidgetItem *sel_item = list->item(i);
						if (sel_item == nullptr) continue;
						const int index = obter_indice_conceito_do_item(sel_item);
						if (index >= 0 && index < conceitos_.size()) {
							QVariantMap conceito = normalized_concept(conceitos_.at(index).toMap());
							conceito.insert(QStringLiteral("font_config"), sel_item->data(source::utils::kCustomFontSettingsRole));
							conceitos_[index] = conceito;
							if (has_conceito_atual_
								&& conceito_atual_.value(QStringLiteral("titulo")).toString().compare(
									   conceito.value(QStringLiteral("titulo")).toString(), Qt::CaseInsensitive)
									   == 0) {
								conceito_atual_ = conceito;
							}
						}
					}
				}
			}
			salvar_conceitos();
			atualizar_lista();
		}
	});

	QAction *acao_colorir = menu.addAction(
		compat::tr(this, QStringLiteral("🎨 Colorir tarefas"), QStringLiteral("🎨 Color Tasks")));
	connect(acao_colorir, &QAction::triggered, this, [this, selected_items]() {
		QColor initial_color;
		for (QListWidgetItem *sel_item : selected_items) {
			const int index = obter_indice_conceito_do_item(sel_item);
			if (index >= 0 && index < conceitos_.size()) {
				const QVariantMap conceito = normalized_concept(conceitos_.at(index).toMap());
				const QString col_str = conceito.value(QStringLiteral("cor_texto")).toString().trimmed();
				if (QColor(col_str).isValid()) {
					initial_color = QColor(col_str);
					break;
				}
			}
		}
		const QColor selected_color = source::utils::choose_task_color(this, initial_color, source::utils::PluginFontContext::Feynman);
		if (!selected_color.isValid()) {
			return;
		}
		const QString color_name = selected_color.name(QColor::HexRgb);
		for (QListWidgetItem *sel_item : selected_items) {
			const int index = obter_indice_conceito_do_item(sel_item);
			if (index >= 0 && index < conceitos_.size()) {
				QVariantMap conceito = normalized_concept(conceitos_.at(index).toMap());
				conceito.insert(QStringLiteral("cor_texto"), color_name);
				conceitos_[index] = conceito;
				if (has_conceito_atual_
					&& conceito_atual_.value(QStringLiteral("titulo")).toString().compare(
						   conceito.value(QStringLiteral("titulo")).toString(), Qt::CaseInsensitive)
						   == 0) {
					conceito_atual_ = conceito;
				}
			}
		}
		salvar_conceitos();
		atualizar_lista();
	});

	QAction *acao_recorrencia = menu.addAction(
		compat::tr(this, QStringLiteral("🔁 Recorrência"), QStringLiteral("🔁 Recurrence")));
	connect(acao_recorrencia, &QAction::triggered, this, [this, item]() {
		const int index = obter_indice_conceito_do_item(item);
		if (index < 0 || index >= conceitos_.size()) {
			return;
		}

		QVariantMap conceito = normalized_concept(conceitos_.at(index).toMap());

		QDialog dialog(this);
		dialog.setWindowTitle(compat::tr(this, QStringLiteral("Recorrência"), QStringLiteral("Recurrence")));
		auto *layout = new QFormLayout(&dialog);
		auto *combo = new QComboBox(&dialog);
		combo->addItem(compat::tr(this, QStringLiteral("Sem recorrência"), QStringLiteral("No recurrence")), QString());
		combo->addItem(compat::tr(this, QStringLiteral("Diária"), QStringLiteral("Daily")), QStringLiteral("daily"));
		combo->addItem(compat::tr(this, QStringLiteral("Semanal"), QStringLiteral("Weekly")), QStringLiteral("weekly"));
		combo->addItem(compat::tr(this, QStringLiteral("Mensal"), QStringLiteral("Monthly")), QStringLiteral("monthly"));

		auto *until_enabled =
			new QCheckBox(compat::tr(this, QStringLiteral("Definir data final"), QStringLiteral("Set end date")), &dialog);
		auto *until = new QDateEdit(QDate::currentDate().addMonths(3), &dialog);
		until->setCalendarPopup(true);
		until->setEnabled(false);
		connect(until_enabled, &QCheckBox::toggled, until, &QDateEdit::setEnabled);

		QString current = conceito.value(QStringLiteral("recurrence")).toString().trimmed();
		if (current.isEmpty()) {
			current = conceito.value(QStringLiteral("recorrencia")).toString().trimmed();
		}
		QString until_str = conceito.value(QStringLiteral("recurrence_until")).toString().trimmed();
		if (until_str.isEmpty()) {
			until_str = conceito.value(QStringLiteral("recorrencia_ate")).toString().trimmed();
		}

		const int idx = combo->findData(current);
		combo->setCurrentIndex(idx >= 0 ? idx : 0);
		const QDate current_until = QDate::fromString(until_str, Qt::ISODate);
		if (current_until.isValid()) {
			until_enabled->setChecked(true);
			until->setDate(current_until);
		}

		layout->addRow(compat::tr(this, QStringLiteral("Frequência"), QStringLiteral("Frequency")), combo);
		layout->addRow(until_enabled, until);
		auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
		if (compat::host_bridge() != nullptr) {
			compat::host_bridge()->translate_standard_buttons(buttons);
		}
		layout->addRow(buttons);
		connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
		connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

		if (dialog.exec() != QDialog::Accepted) {
			return;
		}

		const QString chosen_recurrence = combo->currentData().toString();
		const QString chosen_until = until_enabled->isChecked() ? until->date().toString(Qt::ISODate) : QString();
		conceito.insert(QStringLiteral("recurrence"), chosen_recurrence);
		conceito.insert(QStringLiteral("recorrencia"), chosen_recurrence);
		conceito.insert(QStringLiteral("recurrence_until"), chosen_until);
		conceito.insert(QStringLiteral("recorrencia_ate"), chosen_until);
		conceitos_[index] = conceito;
		if (has_conceito_atual_
			&& conceito_atual_.value(QStringLiteral("titulo")).toString().compare(
				   conceito.value(QStringLiteral("titulo")).toString(), Qt::CaseInsensitive)
				   == 0) {
			conceito_atual_ = conceito;
		}
		salvar_conceitos();
		atualizar_lista();

		if (compat::host_bridge() != nullptr) {
			bool metadata_changed = false;
			QVariantMap payload = construir_payload_tarefa_compartilhada(item, &metadata_changed);
			payload.insert(QStringLiteral("recurrence"), chosen_recurrence);
			payload.insert(QStringLiteral("recorrencia"), chosen_recurrence);
			payload.insert(QStringLiteral("recurrence_until"), chosen_until);
			payload.insert(QStringLiteral("recorrencia_ate"), chosen_until);
			compat::host_bridge()->publish_task_to_eisenhower(payload);
			compat::host_bridge()->notify_shared_state_changed(QStringLiteral("feynman"));
		}
	});

	QAction *acao_excluir = menu.addAction(
		selected_items.size() > 1
			? compat::tr(this,
						 QStringLiteral("🗑️ Excluir Tarefas Selecionadas"),
						 QStringLiteral("🗑️ Delete Selected Tasks"))
			: compat::tr(this, QStringLiteral("🗑️ Excluir Tarefa"), QStringLiteral("🗑️ Delete Task")));
	connect(acao_excluir, &QAction::triggered, this, [this]() { deletar_conceito(); });

	menu.addSeparator();

	QMenu *menu_prioridade =
		menu.addMenu(compat::tr(this, QStringLiteral("🔠 Redefinir Prioridade"), QStringLiteral("🔠 Redefine Priority")));
	QAction *acao_prioridade_1 = menu_prioridade->addAction(priority_label(this, 1));
	QAction *acao_prioridade_2 = menu_prioridade->addAction(priority_label(this, 2));
	QAction *acao_prioridade_3 = menu_prioridade->addAction(priority_label(this, 3));
	QAction *acao_prioridade_4 = menu_prioridade->addAction(priority_label(this, 4));

	QMenu *menu_dominio =
		menu.addMenu(compat::tr(this, QStringLiteral("🎯 Redefinir Nível de Domínio"), QStringLiteral("🎯 Redefine Mastery Level")));

	QAction *acao_iniciante = menu_dominio->addAction(
		domain_label(this, 0, true));
	QAction *acao_intermediario = menu_dominio->addAction(
		domain_label(this, 1, true));
	QAction *acao_avancado = menu_dominio->addAction(domain_label(this, 2, true));

	connect(acao_prioridade_1, &QAction::triggered, this, [this, item]() { redefinir_prioridade(item, 1); });
	connect(acao_prioridade_2, &QAction::triggered, this, [this, item]() { redefinir_prioridade(item, 2); });
	connect(acao_prioridade_3, &QAction::triggered, this, [this, item]() { redefinir_prioridade(item, 3); });
	connect(acao_prioridade_4, &QAction::triggered, this, [this, item]() { redefinir_prioridade(item, 4); });
	connect(acao_iniciante, &QAction::triggered, this, [this, item]() { redefinir_dominio(item, 0); });
	connect(acao_intermediario, &QAction::triggered, this, [this, item]() { redefinir_dominio(item, 1); });
	connect(acao_avancado, &QAction::triggered, this, [this, item]() { redefinir_dominio(item, 2); });

	menu.addSeparator();

	bool added_share_action = false;
	if (compat::host_bridge() != nullptr && compat::host_bridge()->can_publish_task_to_pomodoro()) {
		added_share_action = true;
		QAction *acao_compartilhar_pomodoro =
			menu.addAction(compat::tr(this,
									  QStringLiteral("🍅 Compartilhar com Pomodoro"),
									  QStringLiteral("🍅 Share with Pomodoro")));
		connect(acao_compartilhar_pomodoro, &QAction::triggered, this, [this, selected_items]() {
			int sent = 0;
			bool metadata_changed = false;
			QStringList failed_titles;

			for (QListWidgetItem *selected_item : selected_items) {
				const QVariantMap conceito_item = obter_conceito_do_item(selected_item);
				const QString titulo = conceito_item.value(QStringLiteral("titulo")).toString().trimmed();
				const QVariantMap payload = construir_payload_tarefa_compartilhada(selected_item, &metadata_changed);
				if (payload.isEmpty() || compat::host_bridge() == nullptr
					|| !compat::host_bridge()->publish_task_to_pomodoro(payload)) {
					failed_titles.append(titulo.isEmpty() ? selected_item->text().trimmed() : titulo);
					continue;
				}

				++sent;
			}

			if (metadata_changed) {
				salvar_conceitos();
			}

			if (sent > 0) {
				compat::MessageBox::information(
					this,
					compat::tr(this, QStringLiteral("Compartilhamento concluído"), QStringLiteral("Sharing completed")),
					compat::tr(this,
							   QStringLiteral("%1 conceito(s) foram compartilhados com Pomodoro."),
							   QStringLiteral("%1 concept(s) were shared with Pomodoro."))
						.arg(sent));
			}

			if (!failed_titles.isEmpty()) {
				compat::MessageBox::warning(
					this,
					compat::tr(this, QStringLiteral("Erro"), QStringLiteral("Error")),
					compat::tr(this,
							   QStringLiteral("Não foi possível compartilhar o conceito \"%1\" com Pomodoro."),
							   QStringLiteral("The concept \"%1\" could not be shared with Pomodoro."))
						.arg(failed_titles.join(QStringLiteral("\", \""))));
			}
		});
	}

	if (compat::host_bridge() != nullptr && compat::host_bridge()->can_publish_task_to_eisenhower()) {
		added_share_action = true;
		QAction *acao_compartilhar_eisenhower =
			menu.addAction(compat::tr(this,
									  QStringLiteral("🗂️ Compartilhar com Eisenhower"),
									  QStringLiteral("🗂️ Share with Eisenhower")));
		connect(acao_compartilhar_eisenhower, &QAction::triggered, this, [this, selected_items]() {
			int sent = 0;
			bool metadata_changed = false;
			QStringList failed_titles;

			for (QListWidgetItem *selected_item : selected_items) {
				const QVariantMap conceito_item = obter_conceito_do_item(selected_item);
				const QString titulo = conceito_item.value(QStringLiteral("titulo")).toString().trimmed();
				const QVariantMap payload = construir_payload_tarefa_compartilhada(selected_item, &metadata_changed);
				if (payload.isEmpty() || compat::host_bridge() == nullptr
					|| !compat::host_bridge()->publish_task_to_eisenhower(payload)) {
					failed_titles.append(titulo.isEmpty() ? selected_item->text().trimmed() : titulo);
					continue;
				}

				++sent;
			}

			if (metadata_changed) {
				salvar_conceitos();
			}

			if (sent > 0) {
				compat::MessageBox::information(
					this,
					compat::tr(this, QStringLiteral("Compartilhamento concluído"), QStringLiteral("Sharing completed")),
					compat::tr(this,
							   QStringLiteral("%1 conceito(s) foram compartilhados com Eisenhower."),
							   QStringLiteral("%1 concept(s) were shared with Eisenhower."))
						.arg(sent));
			}

			if (!failed_titles.isEmpty()) {
				compat::MessageBox::warning(
					this,
					compat::tr(this, QStringLiteral("Erro"), QStringLiteral("Error")),
					compat::tr(this,
							   QStringLiteral("Não foi possível compartilhar o conceito \"%1\" com Eisenhower."),
							   QStringLiteral("The concept \"%1\" could not be shared with Eisenhower."))
						.arg(failed_titles.join(QStringLiteral("\", \""))));
			}
		});
	}

	if (compat::host_bridge() != nullptr && compat::host_bridge()->can_publish_task_to_gantt() && !selected_items.isEmpty()) {
		added_share_action = true;
		QListWidgetItem *selected_item = selected_items.first();
		if (selected_item != nullptr) {
			QVariantMap conceito = obter_conceito_do_item(selected_item);
			QString task_id = conceito.value(QStringLiteral("id")).toString().trimmed();
			if (task_id.isEmpty()) {
				task_id = conceito.value(QStringLiteral("integracao_task_id")).toString().trimmed();
			}
			if (task_id.isEmpty()) {
				task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
				conceito.insert(QStringLiteral("id"), task_id);
				conceito.insert(QStringLiteral("integracao_task_id"), task_id);
				selected_item->setData(kConceptRole, conceito);
				const int idx = obter_indice_conceito_do_item(selected_item);
				if (idx >= 0 && idx < conceitos_.size()) {
					conceitos_[idx] = conceito;
				}
				salvar_conceitos();
			}
			QString task_title = conceito.value(QStringLiteral("titulo")).toString().trimmed();
			if (task_title.isEmpty()) task_title = selected_item->text().trimmed();

			QString wbs_and_name, sched_name, act_status;
			double act_progress = 0.0;
			const bool is_linked = compat::host_bridge()->get_gantt_summary(
				task_id, &wbs_and_name, &sched_name, &act_progress, &act_status);

			if (is_linked) {
				// 1. Abrir no Cronograma de Gantt (...)
				const QString open_text = !wbs_and_name.trimmed().isEmpty()
					? compat::tr(this,
								 QStringLiteral("🏗️ Abrir no Cronograma de Gantt (%1 - %2%)"),
								 QStringLiteral("🏗️ Open in Gantt Schedule (%1 - %2%)"))
						  .arg(wbs_and_name)
						  .arg(static_cast<int>(act_progress))
					: compat::tr(this,
								 QStringLiteral("🏗️ Abrir no Cronograma de Gantt (%1%)"),
								 QStringLiteral("🏗️ Open in Gantt Schedule (%1%)"))
						  .arg(static_cast<int>(act_progress));
				QAction *acao_abrir_gantt = menu.addAction(open_text);
				connect(acao_abrir_gantt, &QAction::triggered, this, [this, task_id]() {
					compat::host_bridge()->open_gantt_and_highlight_activity(task_id, this);
				});

				// 2. Registrar Avanço de Campo no Cronograma...
				QAction *acao_progresso = menu.addAction(compat::tr(
					this,
					QStringLiteral("⚡ Registrar Avanço de Campo no Cronograma..."),
					QStringLiteral("⚡ Record Field Progress in Schedule...")));
				connect(acao_progresso, &QAction::triggered, this, [this, selected_item, task_id, wbs_and_name, act_progress]() {
					bool ok = false;
					const QString label_text = !wbs_and_name.trimmed().isEmpty()
						? compat::tr(this,
									 QStringLiteral("Informe o percentual executado para '%1':"),
									 QStringLiteral("Enter executed percentage for '%1':"))
							  .arg(wbs_and_name)
						: compat::tr(this,
									 QStringLiteral("Informe o percentual executado:"),
									 QStringLiteral("Enter executed percentage:"));
					const double val = QInputDialog::getDouble(
						this,
						compat::tr(this, QStringLiteral("Avanço Físico de Obra"), QStringLiteral("Field Work Progress")),
						label_text,
						act_progress, 0.0, 100.0, 1, &ok);
					if (ok) {
						const QString note = QInputDialog::getText(
							this,
							compat::tr(this, QStringLiteral("Diário de Obra"), QStringLiteral("Construction Daily Log")),
							compat::tr(this, QStringLiteral("Observações de campo (opcional):"), QStringLiteral("Field notes (optional):")),
							QLineEdit::Normal, QString(), &ok);
						QString err;
						if (compat::host_bridge()->record_field_progress_for_task(task_id, val, note, QStringLiteral("Campo"), &err)) {
							compat::MessageBox::information(
								this,
								compat::tr(this, QStringLiteral("Avanço Registrado"), QStringLiteral("Progress Recorded")),
								compat::tr(this,
										   QStringLiteral("Avanço físico de %1% registrado com sucesso no cronograma da obra!"),
										   QStringLiteral("Physical progress of %1% successfully recorded in the construction schedule!"))
									.arg(val));
							const int idx = obter_indice_conceito_do_item(selected_item);
							if (idx >= 0 && idx < conceitos_.size()) {
								QVariantMap c = normalized_concept(conceitos_.at(idx).toMap());
								c.insert(QStringLiteral("progresso"), val);
								c.insert(QStringLiteral("integracao_completed"), val >= 100.0);
								c.insert(QStringLiteral("integracao_status"),
										 val >= 100.0 ? QStringLiteral("Done")
													  : (val > 0.0 ? QStringLiteral("Doing")
																   : QStringLiteral("Todo")));
								conceitos_[idx] = c;
								salvar_conceitos();
							}
							atualizar_lista();
							compat::host_bridge()->notify_shared_state_changed(QStringLiteral("feynman"));
						} else {
							compat::MessageBox::warning(
								this,
								compat::tr(this, QStringLiteral("Erro"), QStringLiteral("Error")),
								err);
						}
					}
				});

				// 3. Alterar Vínculo no Gantt...
				QAction *acao_alterar_vinculo = menu.addAction(compat::tr(
					this,
					QStringLiteral("🔗 Alterar Vínculo no Gantt..."),
					QStringLiteral("🔗 Change Link in Gantt...")));
				connect(acao_alterar_vinculo, &QAction::triggered, this, [this, task_id, task_title]() {
					if (compat::host_bridge()->open_gantt_link_dialog(task_id, task_title, this)) {
						compat::MessageBox::information(
							this,
							compat::tr(this, QStringLiteral("Cronograma"), QStringLiteral("Schedule")),
							compat::tr(this,
									   QStringLiteral("Vínculo com o cronograma atualizado com sucesso!"),
									   QStringLiteral("Link with schedule updated successfully!")));
						salvar_conceitos();
						atualizar_lista();
						compat::host_bridge()->notify_shared_state_changed(QStringLiteral("feynman"));
					}
				});

				// 4. Desvincular do Cronograma de Gantt...
				QAction *acao_desvincular = menu.addAction(compat::tr(
					this,
					QStringLiteral("🔓 Desvincular do Cronograma de Gantt..."),
					QStringLiteral("🔓 Unlink from Gantt Schedule...")));
				connect(acao_desvincular, &QAction::triggered, this, [this, task_id, wbs_and_name]() {
					QMessageBox confirm_unlink(this);
					confirm_unlink.setIcon(QMessageBox::Question);
					confirm_unlink.setWindowTitle(compat::tr(this, QStringLiteral("Desvincular"), QStringLiteral("Unlink")));
					confirm_unlink.setText(compat::tr(this,
													  QStringLiteral("Deseja desvincular esta tarefa da atividade '%1' do cronograma?"),
													  QStringLiteral("Do you want to unlink this task from activity '%1' of the schedule?"))
											   .arg(wbs_and_name));
					confirm_unlink.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
					confirm_unlink.setDefaultButton(QMessageBox::No);
					compat::translate_standard_buttons(&confirm_unlink, this);
					if (confirm_unlink.exec() == QMessageBox::Yes) {
						compat::host_bridge()->unlink_task(task_id);
						compat::MessageBox::information(
							this,
							compat::tr(this, QStringLiteral("Cronograma"), QStringLiteral("Schedule")),
							compat::tr(this,
									   QStringLiteral("Tarefa desvinculada com sucesso do cronograma."),
									   QStringLiteral("Task unlinked successfully from schedule.")));
						salvar_conceitos();
						atualizar_lista();
						compat::host_bridge()->notify_shared_state_changed(QStringLiteral("feynman"));
					}
				});
			} else {
				// 1. Vincular ao Cronograma de Gantt...
				QAction *acao_vincular_gantt = menu.addAction(compat::tr(
					this,
					QStringLiteral("🔒 Vincular ao Cronograma de Gantt..."),
					QStringLiteral("🔒 Link to Gantt Schedule...")));
				connect(acao_vincular_gantt, &QAction::triggered, this, [this, task_id, task_title]() {
					if (compat::host_bridge()->open_gantt_link_dialog(task_id, task_title, this)) {
						compat::MessageBox::information(
							this,
							compat::tr(this, QStringLiteral("Cronograma"), QStringLiteral("Schedule")),
							compat::tr(this,
									   QStringLiteral("Vínculo com o cronograma de Gantt atualizado com sucesso!"),
									   QStringLiteral("Link with the Gantt schedule updated successfully!")));
						salvar_conceitos();
						atualizar_lista();
						compat::host_bridge()->notify_shared_state_changed(QStringLiteral("feynman"));
					}
				});
			}
		}
	}

	if (added_share_action) {
		menu.addSeparator();
	}

	QAction *acao_vincular = menu.addAction(action_text_with_path(
		compat::tr(this, QStringLiteral("📎 Vincular Arquivo"), QStringLiteral("📎 Link File")), caminho_vinculado));
	QAction *acao_remover_vinculo = menu.addAction(action_text_with_path(
		compat::tr(this, QStringLiteral("🗑️ Remover Arquivo Vinculado"), QStringLiteral("🗑️ Remove Linked File")),
		caminho_vinculado));
	QAction *acao_abrir_arquivo = menu.addAction(action_text_with_path(
		compat::tr(this, QStringLiteral("📂 Abrir Arquivo Vinculado"), QStringLiteral("📂 Open Linked File")),
		caminho_vinculado));

	acao_remover_vinculo->setEnabled(!caminho_vinculado.isEmpty());
	acao_abrir_arquivo->setEnabled(!caminho_vinculado.isEmpty());

	connect(acao_vincular, &QAction::triggered, this, [this, item]() { vincular_arquivo_conceito(item); });
	connect(acao_remover_vinculo, &QAction::triggered, this, [this, item]() { remover_arquivo_vinculado(item); });
	connect(acao_abrir_arquivo, &QAction::triggered, this, [this, item]() { abrir_arquivo_conceito(item); });

	menu.exec(lista_conceitos->mapToGlobal(position));
}

void MetodoFeynman::editar_tarefa(QListWidgetItem *item)
{
	const int index = obter_indice_conceito_do_item(item);
	if (index < 0 || index >= conceitos_.size()) {
		return;
	}

	const QVariantMap original = normalized_concept(conceitos_.at(index).toMap());
	FeynmanEditTaskDialog dialog(this, original, this);
	if (dialog.exec() != QDialog::Accepted) {
		return;
	}

	const FeynmanEditTaskDialog::Values values = dialog.values();
	if (values.title.isEmpty()) {
		compat::MessageBox::feynman_warn_missing_title(this);
		return;
	}

	const int duplicate_index = encontrar_indice_conceito(values.title, Qt::CaseInsensitive);
	if (duplicate_index >= 0 && duplicate_index != index) {
		compat::MessageBox::warning(
			this,
			compat::tr(this, QStringLiteral("Título duplicado"), QStringLiteral("Duplicate title")),
			compat::tr(this,
					   QStringLiteral("Já existe um conceito com o título \"%1\". Escolha um título diferente."),
					   QStringLiteral("There is already a concept with the title \"%1\". Choose a different title."))
				.arg(values.title));
		return;
	}

	QVariantMap updated = original;
	updated.insert(QStringLiteral("titulo"), values.title);
	updated.insert(QStringLiteral("prioridade"), values.priority);
	updated.insert(QStringLiteral("dominio"), values.domain);
	updated.insert(QStringLiteral("duracao"), values.duration);
	updated.insert(QStringLiteral("duration"), values.duration);
	updated.insert(QStringLiteral("pomodoros"), values.pomodoros);
	updated.insert(QStringLiteral("pomodoros_completados"), values.pomodoros);
	const int default_ciclos = compat::host_bridge() != nullptr ? compat::host_bridge()->default_pomodoro_cycle_count() : 4;
	const int final_ciclos = values.ciclos_totais > 0 ? values.ciclos_totais : default_ciclos;
	updated.insert(QStringLiteral("ciclos_totais"), final_ciclos);
	updated.insert(QStringLiteral("ciclos_permitidos"), final_ciclos);
	updated.insert(QStringLiteral("total_ciclos"), final_ciclos);
	updated.insert(QStringLiteral("ciclos_customizados"), true);
	updated.insert(QStringLiteral("arquivo_vinculado"),
				   values.file_path.isEmpty() ? QVariant() : QVariant(values.file_path));

	const QString descricao_original_gerada = composed_description(this,
																   original.value(QStringLiteral("titulo")).toString(),
																   original.value(QStringLiteral("explicacao")).toString(),
																   original.value(QStringLiteral("lacunas")).toString(),
																   original.value(QStringLiteral("revisao")).toString())
												  .trimmed();
	const QString descricao_composta = composed_description(this,
															updated.value(QStringLiteral("titulo")).toString(),
															updated.value(QStringLiteral("explicacao")).toString(),
															updated.value(QStringLiteral("lacunas")).toString(),
															updated.value(QStringLiteral("revisao")).toString())
										   .trimmed();
	const QString descricao_editada = values.description.trimmed();
	QString descricao_final;
	if (has_external_description_base(original)) {
		QString descricao_base = manual_description_text(descricao_editada, descricao_original_gerada);
		if (descricao_base.trimmed().isEmpty()) {
			descricao_base = original.value(QStringLiteral("descricao")).toString();
		}
		descricao_final = description_with_external_base(descricao_base, descricao_composta).trimmed();
	} else {
		descricao_final = merge_description_edit(descricao_original_gerada, descricao_editada, descricao_composta).trimmed();
	}
	const bool descricao_customizada = descricao_final != descricao_composta;
	updated.insert(QStringLiteral("descricao_customizada"), descricao_customizada);
	if (descricao_customizada) {
		updated.insert(QStringLiteral("descricao"), descricao_final);
	} else {
		updated.remove(QStringLiteral("descricao"));
	}

	if (values.date_iso.isEmpty()) {
		updated.remove(QStringLiteral("integracao_date"));
	} else {
		updated.insert(QStringLiteral("integracao_date"), values.date_iso);
	}

	if (values.time_str.isEmpty()) {
		updated.remove(QStringLiteral("integracao_time"));
	} else {
		updated.insert(QStringLiteral("integracao_time"), values.time_str);
	}

	QString task_id = original.value(QStringLiteral("id")).toString().trimmed();
	if (task_id.isEmpty()) {
		task_id = original.value(QStringLiteral("integracao_task_id")).toString().trimmed();
	}
	if (task_id.isEmpty()) {
		task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
		updated.insert(QStringLiteral("id"), task_id);
		updated.insert(QStringLiteral("integracao_task_id"), task_id);
		if (updated.value(QStringLiteral("integracao_origem")).toString().trimmed().isEmpty()) {
			updated.insert(QStringLiteral("integracao_origem"), QStringLiteral("feynman"));
		}
	}

	updated = normalized_concept(updated);
	conceitos_[index] = updated;
	conceito_atual_ = updated;
	has_conceito_atual_ = true;

	atualizar_lista();

	const int refreshed_index = encontrar_indice_conceito(values.title);
	if (refreshed_index >= 0 && refreshed_index < lista_conceitos->count()) {
		if (QListWidgetItem *updated_item = lista_conceitos->item(refreshed_index)) {
			lista_conceitos->setCurrentItem(updated_item);
			updated_item->setSelected(true);
			selecionar_conceito(updated_item);
		}
	}

	compat::MessageBox::feynman_edit_task_success(this);

	QTimer::singleShot(0, this, [this, updated, task_id, values, final_ciclos]() {
		salvar_conceitos();

		if (compat::host_bridge() != nullptr) {
			const QString status = updated.value(QStringLiteral("integracao_status")).toString().trimmed();
			const bool completed = is_concept_completed(updated);
			const bool in_progress = is_concept_in_progress(updated);

			QVariantMap sync_payload;
			sync_payload.insert(QStringLiteral("integration_id"), QUuid::createUuid().toString(QUuid::WithoutBraces));
			sync_payload.insert(QStringLiteral("origin"), QStringLiteral("feynman"));
			sync_payload.insert(QStringLiteral("id"), task_id);
			sync_payload.insert(QStringLiteral("titulo"), values.title);
			sync_payload.insert(QStringLiteral("prioridade"), priority_text_without_emoji(this, values.priority));
			sync_payload.insert(QStringLiteral("dominio"), values.domain);
			sync_payload.insert(QStringLiteral("description"), shared_task_description(this, updated));
			sync_payload.insert(QStringLiteral("file_path"), values.file_path);
			sync_payload.insert(QStringLiteral("date"), values.date_iso);
			sync_payload.insert(QStringLiteral("time"), values.time_str);
			sync_payload.insert(QStringLiteral("cor_texto"), updated.value(QStringLiteral("cor_texto")).toString().trimmed());
			sync_payload.insert(QStringLiteral("font_config"), updated.value(QStringLiteral("font_config")));
			sync_payload.insert(QStringLiteral("duracao"), values.duration);
			sync_payload.insert(QStringLiteral("duration"), values.duration);
			sync_payload.insert(QStringLiteral("pomodoros"), values.pomodoros);
			sync_payload.insert(QStringLiteral("pomodoros_completados"), values.pomodoros);
			sync_payload.insert(QStringLiteral("ciclos_totais"), final_ciclos);
			sync_payload.insert(QStringLiteral("ciclos_permitidos"), final_ciclos);
			sync_payload.insert(QStringLiteral("total_ciclos"), final_ciclos);
			sync_payload.insert(QStringLiteral("ciclos_customizados"), true);
			sync_payload.insert(QStringLiteral("completed"), completed);
			sync_payload.insert(QStringLiteral("in_progress"), in_progress);
			sync_payload.insert(QStringLiteral("status"), status.isEmpty() ? (completed ? QStringLiteral("Done") : (in_progress ? QStringLiteral("Doing") : QStringLiteral("Todo"))) : status);

			compat::host_bridge()->publish_task_to_eisenhower(sync_payload);
			compat::host_bridge()->publish_task_to_pomodoro(sync_payload);
			if (!task_id.isEmpty()) {
				compat::host_bridge()->sync_task_duration(task_id, values.duration);
			}
		}
	});
}

void MetodoFeynman::redefinir_dominio(QListWidgetItem *item, int novo_nivel)
{
	const int index = obter_indice_conceito_do_item(item);
	if (index < 0 || index >= conceitos_.size()) {
		return;
	}

	QVariantMap conceito = normalized_concept(conceitos_.at(index).toMap());
	conceito.insert(QStringLiteral("dominio"), qBound(kDefaultDomain, novo_nivel, 2));
	conceitos_[index] = conceito;

	if (has_conceito_atual_
		&& conceito_atual_.value(QStringLiteral("titulo")).toString().compare(conceito.value(QStringLiteral("titulo")).toString(),
																			  Qt::CaseInsensitive)
			   == 0) {
		conceito_atual_ = conceito;
		const int domain_index = combo_dominio->findData(qBound(kDefaultDomain, novo_nivel, 2));
		combo_dominio->setCurrentIndex(domain_index >= 0 ? domain_index : 0);
	}

	salvar_conceitos();
	atualizar_lista();
}

void MetodoFeynman::redefinir_prioridade(QListWidgetItem *item, int nova_prioridade)
{
	const int index = obter_indice_conceito_do_item(item);
	if (index < 0 || index >= conceitos_.size()) {
		return;
	}

	QVariantMap conceito = normalized_concept(conceitos_.at(index).toMap());
	conceito.insert(QStringLiteral("prioridade"), normalized_priority_value(nova_prioridade));
	conceitos_[index] = conceito;

	if (has_conceito_atual_
		&& conceito_atual_.value(QStringLiteral("titulo")).toString().compare(conceito.value(QStringLiteral("titulo")).toString(),
																			  Qt::CaseInsensitive)
			   == 0) {
		conceito_atual_ = conceito;
		const int priority_index =
			combo_prioridade->findData(normalized_priority_value(conceito.value(QStringLiteral("prioridade"))));
		combo_prioridade->setCurrentIndex(priority_index >= 0 ? priority_index : 0);
	}

	salvar_conceitos();
	atualizar_lista();
}

QVariantMap MetodoFeynman::construir_payload_tarefa_compartilhada(QListWidgetItem *item, bool *metadata_changed)
{
	const int index = obter_indice_conceito_do_item(item);
	if (index < 0 || index >= conceitos_.size()) {
		return {};
	}

	QVariantMap conceito = normalized_concept(conceitos_.at(index).toMap());
	const QString titulo = conceito.value(QStringLiteral("titulo")).toString().trimmed();
	if (titulo.isEmpty()) {
		return {};
	}

	QString task_id = conceito.value(QStringLiteral("integracao_task_id")).toString().trimmed();
	if (task_id.isEmpty()) {
		task_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
		if (conceito.value(QStringLiteral("integracao_origem")).toString().trimmed().isEmpty()) {
			conceito.insert(QStringLiteral("integracao_origem"), QStringLiteral("feynman"));
		}
		conceito.insert(QStringLiteral("integracao_task_id"), task_id);
		conceitos_[index] = normalized_concept(conceito);

		if (has_conceito_atual_
			&& conceito_atual_.value(QStringLiteral("titulo")).toString().compare(titulo, Qt::CaseInsensitive) == 0) {
			conceito_atual_ = conceitos_.at(index).toMap();
		}

		if (metadata_changed != nullptr) {
			*metadata_changed = true;
		}
	}

	const QString status = conceito.value(QStringLiteral("integracao_status")).toString().trimmed();
	const bool completed = conceito.value(QStringLiteral("integracao_completed")).toBool()
						   || status.compare(QStringLiteral("Done"), Qt::CaseInsensitive) == 0
						   || status.compare(QStringLiteral("Completed"), Qt::CaseInsensitive) == 0;

	QVariantMap payload;
	payload.insert(QStringLiteral("integration_id"), QUuid::createUuid().toString(QUuid::WithoutBraces));
	payload.insert(QStringLiteral("origin"), QStringLiteral("feynman"));
	payload.insert(QStringLiteral("id"), task_id);
	payload.insert(QStringLiteral("titulo"), titulo);
	payload.insert(QStringLiteral("prioridade"), priority_text_without_emoji(this, conceito.value(QStringLiteral("prioridade")).toInt()));
	payload.insert(QStringLiteral("dominio"), normalized_domain_value(conceito.value(QStringLiteral("dominio"))));
	payload.insert(QStringLiteral("description"), shared_task_description(this, conceito));
	payload.insert(QStringLiteral("file_path"), conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed());
	payload.insert(QStringLiteral("date"), conceito.value(QStringLiteral("integracao_date")).toString().trimmed());
	payload.insert(QStringLiteral("time"), conceito.value(QStringLiteral("integracao_time")).toString().trimmed());
	payload.insert(QStringLiteral("cor_texto"), conceito.value(QStringLiteral("cor_texto")).toString().trimmed());
	payload.insert(QStringLiteral("font_config"), conceito.value(QStringLiteral("font_config")));
	payload.insert(QStringLiteral("duracao"), conceito.value(QStringLiteral("duracao"), 1.0));
	payload.insert(QStringLiteral("duration"), conceito.value(QStringLiteral("duration"), 1.0));
	const int default_ciclos = compat::host_bridge() != nullptr ? compat::host_bridge()->default_pomodoro_cycle_count() : 4;
	const int c_tot = conceito.value(QStringLiteral("ciclos_totais"),
					  conceito.value(QStringLiteral("ciclos_permitidos"),
					  conceito.value(QStringLiteral("total_ciclos"), default_ciclos))).toInt();
	const int safe_c_tot = c_tot > 0 ? c_tot : default_ciclos;
	const bool custom_ciclos = conceito.value(QStringLiteral("ciclos_customizados"), false).toBool()
	                           || (c_tot > 0 && c_tot != default_ciclos);
	const int pom = conceito.value(QStringLiteral("pomodoros"),
					conceito.value(QStringLiteral("pomodoros_completados"), 0)).toInt();
	const bool in_progress = is_concept_in_progress(conceito);

	payload.insert(QStringLiteral("pomodoros"), pom);
	payload.insert(QStringLiteral("pomodoros_completados"), pom);
	payload.insert(QStringLiteral("ciclos_totais"), safe_c_tot);
	payload.insert(QStringLiteral("ciclos_permitidos"), safe_c_tot);
	payload.insert(QStringLiteral("total_ciclos"), safe_c_tot);
	payload.insert(QStringLiteral("ciclos_customizados"), custom_ciclos);
	payload.insert(QStringLiteral("completed"), completed);
	payload.insert(QStringLiteral("in_progress"), in_progress);
	payload.insert(QStringLiteral("status"), status.isEmpty() ? (completed ? QStringLiteral("Done") : (in_progress ? QStringLiteral("Doing") : QStringLiteral("Todo"))) : status);
	const QString rec = conceito.value(QStringLiteral("recurrence"), conceito.value(QStringLiteral("recorrencia"))).toString().trimmed();
	const QString rec_until = conceito.value(QStringLiteral("recurrence_until"), conceito.value(QStringLiteral("recorrencia_ate"))).toString().trimmed();
	payload.insert(QStringLiteral("recurrence"), rec);
	payload.insert(QStringLiteral("recorrencia"), rec);
	payload.insert(QStringLiteral("recurrence_until"), rec_until);
	payload.insert(QStringLiteral("recorrencia_ate"), rec_until);
	return payload;
}

}  // namespace source::plugin::Feynman

namespace source::plugin::Feynman::MetodoFeynmanFns {
void mostrar_menu_contexto(MetodoFeynman &self, const QPoint &position) { self.mostrar_menu_contexto(position); }
void redefinir_dominio(MetodoFeynman &self, QListWidgetItem *item, int novo_nivel) { self.redefinir_dominio(item, novo_nivel); }
void redefinir_prioridade(MetodoFeynman &self, QListWidgetItem *item, int nova_prioridade) { self.redefinir_prioridade(item, nova_prioridade); }
}
