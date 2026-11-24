#pragma once

#include <algorithm>
#include <optional>
#include <tuple>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QApplication>
#include <QBrush>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDateEdit>
#include <QDialog>
#include <QDir>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QMenu>
#include <QPalette>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScopedValueRollback>
#include <QSet>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStringList>
#include <QStyle>
#include <QStyleOptionButton>
#include <QTabBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QTime>
#include <QTimeEdit>
#include <QUuid>
#include <QVBoxLayout>
#include <QVariantList>

#include "source/modules/pomodoro/Pomodoro/tmp_03_GerenciadorTarefas.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_05_GerenciadorTempo.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_02_PomodoroTimer.hpp"
#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_25_prioridade_display.hpp"
#include "source/utils/AccentTabStyle.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"
#include "source/utils/UnifiedTaskStore.hpp"

namespace source::plugin::Pomodoro::GerenciadorTarefasInternal {

inline source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

inline QString normalized_ui_layout_key(const QString &value)
{
    return layout::normalized_ui_layout_key(value);
}

inline bool is_robust_ui_layout(const QString &value)
{
    return layout::is_robust_ui_layout(value);
}

inline QString normalized_panel_layout_key(const QString &value)
{
    return layout::normalized_panel_layout_key(value);
}

constexpr int kTaskIdRole = Qt::UserRole;
constexpr int kGroupHeaderRole = Qt::UserRole + 1;
constexpr int kGroupKeyRole = Qt::UserRole + 2;

class IndicatorOnlyCheckBox : public QCheckBox {
public:
    explicit IndicatorOnlyCheckBox(QWidget *parent = nullptr) : QCheckBox(parent) {}

protected:
    bool hitButton(const QPoint &pos) const override
    {
        try {
            QStyleOptionButton option;
            initStyleOption(&option);
            const QRect indicator_rect = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option, this);
            const int spacing = style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing, &option, this);
            const QString text = this->text();
            const int text_width = fontMetrics().horizontalAdvance(text);
            const int text_height = fontMetrics().height();
            const int text_x = indicator_rect.right() + qMax(0, spacing);
            const int text_y = (height() - text_height) / 2;
            const QRect text_rect(text_x, text_y, qMax(0, text_width), qMax(0, text_height));
            return indicator_rect.united(text_rect).contains(pos);
        } catch (...) {
            return QCheckBox::hitButton(pos);
        }
    }
};

class AutoOpenTabWidget : public QTabWidget {
public:
    explicit AutoOpenTabWidget(GerenciadorTarefas *owner, QWidget *parent = nullptr)
        : QTabWidget(parent), owner_(owner)
    {
        setAcceptDrops(true);
    }

protected:
    void dragEnterEvent(QDragEnterEvent *event) override
    {
        if (event != nullptr) {
            event->acceptProposedAction();
        }
    }

    void dragMoveEvent(QDragMoveEvent *event) override
    {
        try {
            if (event == nullptr) {
                return;
            }

            const QPoint tab_pos = tabBar()->mapFrom(this, event->position().toPoint());
            const int idx = tabBar()->tabAt(tab_pos);
            if (idx >= 0 && idx != currentIndex()) {
                setCurrentIndex(idx);
            }
            event->acceptProposedAction();
        } catch (...) {
            QTabWidget::dragMoveEvent(event);
        }
    }

    void dropEvent(QDropEvent *event) override
    {
        try {
            if (event == nullptr || owner_ == nullptr) {
                return;
            }

            const QPoint tab_pos = tabBar()->mapFrom(this, event->position().toPoint());
            const int idx = tabBar()->tabAt(tab_pos);
            if (idx < 0) {
                event->ignore();
                return;
            }

            setCurrentIndex(idx);
            auto *column = dynamic_cast<KanbanColumn *>(widget(idx));
            if (column == nullptr || column->lista == nullptr) {
                event->ignore();
                return;
            }

            auto *source_list = qobject_cast<QListWidget *>(event->source());
            auto *target_list = column->lista;
            if (source_list == nullptr || source_list == target_list) {
                event->ignore();
                return;
            }

            QList<QListWidgetItem *> items = source_list->selectedItems();
            if (items.isEmpty() && source_list->currentItem() != nullptr) {
                items.append(source_list->currentItem());
            }

            for (QListWidgetItem *item : items) {
                const int row = source_list->row(item);
                if (row >= 0) {
                    if (auto *moved = source_list->takeItem(row)) {
                        target_list->addItem(moved);
                    }
                }
            }

            owner_->_sincronizar_status_pos_movimento(target_list);
            owner_->atualizar_listas();
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } catch (...) {
            QTabWidget::dropEvent(event);
        }
    }

private:
    QPointer<GerenciadorTarefas> owner_;
};

static inline void clear_layout_items(QLayout *layout)
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

}  // namespace source::plugin::Pomodoro::GerenciadorTarefasInternal

namespace source::plugin::Pomodoro::GerenciadorTarefasInternal {

class EditTaskDialog : public QDialog {
public:
    struct Values {
        QString title;
        QString file_path;
        QString description;
        QString date_iso;
        QString time_str;
        int pomodoros = 0;
        double duracao = 1.0;
        QString prioridade;
        int dominio = 0;
        int ciclos_totais = 4;
    };

    EditTaskDialog(GerenciadorTarefas *owner,
                   const QString &initial_text,
                   const QString &initial_file,
                   const QString &initial_description,
                   const QString &initial_date,
                   const QString &initial_time,
                   int initial_pomodoros,
                   const QString &initial_priority,
                   int initial_dominio,
                   double initial_duracao = 1.0,
                   int initial_ciclos_totais = -1)
        : QDialog(owner), owner_(owner)
    {
        setModal(true);
        setWindowTitle(QCoreApplication::translate("PomodoroPlugin", "✏️ Editar Tarefa"));
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

        label_tarefa_ = new QLabel(QCoreApplication::translate("PomodoroPlugin", "Tarefa:"), top_fields_container_);
        label_tarefa_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        label_tarefa_->setFixedHeight(label_tarefa_->fontMetrics().height() + 4);
        top_layout->addWidget(label_tarefa_);

        title_ = new QLineEdit(top_fields_container_);
        title_->setText(initial_text);
        title_->setFixedHeight(compute_widget_height(title_));
        top_layout->addWidget(title_);

        label_prioridade_ = new QLabel(QCoreApplication::translate("PomodoroPlugin", "Prioridade:"), top_fields_container_);
        label_prioridade_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        label_prioridade_->setFixedHeight(label_prioridade_->fontMetrics().height() + 4);
        top_layout->addWidget(label_prioridade_);

        combo_prioridade_ = new QComboBox(top_fields_container_);
        combo_prioridade_->addItems({
            QCoreApplication::translate("PomodoroPlugin", "🔴 Importante e Urgente"),
            QCoreApplication::translate("PomodoroPlugin", "🟠 Importante, mas Nao Urgente"),
            QCoreApplication::translate("PomodoroPlugin", "🟡 Nao Importante, mas Urgente"),
            QCoreApplication::translate("PomodoroPlugin", "🟢 Nao Importante e Nao Urgente"),
        });
        const QMap<QString, int> priority_index = {
            {QStringLiteral("Importante e Urgente"), 0},
            {QStringLiteral("Importante, mas Nao Urgente"), 1},
            {QStringLiteral("Importante, mas Não Urgente"), 1},
            {QStringLiteral("Nao Importante, mas Urgente"), 2},
            {QStringLiteral("Não Importante, mas Urgente"), 2},
            {QStringLiteral("Nao Importante e Nao Urgente"), 3},
            {QStringLiteral("Não Importante e Não Urgente"), 3},
        };
        combo_prioridade_->setCurrentIndex(
            priority_index.value(Tarefa::normalizar_prioridade(initial_priority), 0));
        combo_prioridade_->setFixedHeight(compute_widget_height(combo_prioridade_));
        top_layout->addWidget(combo_prioridade_);

        label_dominio_ = new QLabel(QCoreApplication::translate("PomodoroPlugin", "Nivel de Dominio:"), top_fields_container_);
        label_dominio_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        label_dominio_->setFixedHeight(label_dominio_->fontMetrics().height() + 4);
        top_layout->addWidget(label_dominio_);

        combo_dominio_ = new QComboBox(top_fields_container_);
        combo_dominio_->addItems({
            QCoreApplication::translate("PomodoroPlugin", "🥉 Iniciante (nao consigo explicar)"),
            QCoreApplication::translate("PomodoroPlugin", "🥈 Intermediario (explico com dificuldade)"),
            QCoreApplication::translate("PomodoroPlugin", "🥇 Avancado (explico facilmente)"),
        });
        combo_dominio_->setCurrentIndex(Tarefa::normalizar_dominio(initial_dominio));
        combo_dominio_->setFixedHeight(compute_widget_height(combo_dominio_));
        top_layout->addWidget(combo_dominio_);

        label_pomodoro_ = new QLabel(QCoreApplication::translate("PomodoroPlugin", "Quantidade de Pomodoros (🍅):"), top_fields_container_);
        label_pomodoro_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        label_pomodoro_->setFixedHeight(label_pomodoro_->fontMetrics().height() + 4);
        top_layout->addWidget(label_pomodoro_);

        spin_pomodoros_ = new QSpinBox(top_fields_container_);
        spin_pomodoros_->setRange(0, 999);
        spin_pomodoros_->setValue(qMax(0, initial_pomodoros));
        spin_pomodoros_->setFixedHeight(compute_widget_height(spin_pomodoros_));
        top_layout->addWidget(spin_pomodoros_);

        const int default_ciclos = owner_ != nullptr ? owner_->obter_ciclos_permitidos_padrao() : source::plugin::Pomodoro::PomodoroTimer::obter_ciclos_permitidos_persistidos();
        label_ciclos_ = new QLabel(QCoreApplication::translate("PomodoroPlugin", "Meta de Ciclos Pomodoro (🎯):"), top_fields_container_);
        label_ciclos_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        label_ciclos_->setFixedHeight(label_ciclos_->fontMetrics().height() + 4);
        top_layout->addWidget(label_ciclos_);

        spin_ciclos_totais_ = new QSpinBox(top_fields_container_);
        spin_ciclos_totais_->setRange(1, 999);
        spin_ciclos_totais_->setValue(initial_ciclos_totais > 0 ? initial_ciclos_totais : default_ciclos);
        spin_ciclos_totais_->setFixedHeight(compute_widget_height(spin_ciclos_totais_));
        top_layout->addWidget(spin_ciclos_totais_);

        label_duracao_ = new QLabel(QCoreApplication::translate("PomodoroPlugin", "Duração:"), top_fields_container_);
        label_duracao_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        label_duracao_->setFixedHeight(label_duracao_->fontMetrics().height() + 4);
        top_layout->addWidget(label_duracao_);

        spin_duracao_ = new QDoubleSpinBox(top_fields_container_);
        spin_duracao_->setRange(0.0, 1000.0);
        spin_duracao_->setSingleStep(0.5);
        spin_duracao_->setValue(initial_duracao > 0.0 ? initial_duracao : 1.0);
        spin_duracao_->setSuffix(QCoreApplication::translate("PomodoroPlugin", " dias"));
        spin_duracao_->setFixedHeight(compute_widget_height(spin_duracao_));
        top_layout->addWidget(spin_duracao_);

        label_arquivo_ = new QLabel(QCoreApplication::translate("PomodoroPlugin", "Arquivo vinculado (arraste ou escolha):"), top_fields_container_);
        label_arquivo_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        label_arquivo_->setFixedHeight(label_arquivo_->fontMetrics().height() + 4);
        top_layout->addWidget(label_arquivo_);

        auto *file_row = new QHBoxLayout();
        file_row->setContentsMargins(0, 0, 0, 0);
        file_row->setSpacing(6);

        file_field_ = new FileDropLineEdit(owner_, top_fields_container_);
        file_field_->setText(initial_file.isEmpty() ? QString() : QFileInfo(initial_file).fileName());
        file_field_->setToolTip(initial_file);
        file_field_->setFixedHeight(compute_widget_height(file_field_));
        connect(file_field_, &FileDropLineEdit::fileDropped, this, [this](const QString &path) {
            file_field_->setText(QFileInfo(path).fileName());
            file_field_->setToolTip(path);
            if (btn_open_ != nullptr) btn_open_->setEnabled(true);
        });
        file_row->addWidget(file_field_, 1);

        btn_choose_ = new QPushButton(QCoreApplication::translate("PomodoroPlugin", "Escolher..."), top_fields_container_);
        btn_open_ = new QPushButton(QCoreApplication::translate("PomodoroPlugin", "Abrir"), top_fields_container_);
        btn_clear_ = new QPushButton(QCoreApplication::translate("PomodoroPlugin", "Remover"), top_fields_container_);
        btn_choose_->setFixedHeight(compute_widget_height(btn_choose_));
        btn_open_->setFixedHeight(compute_widget_height(btn_open_));
        btn_clear_->setFixedHeight(compute_widget_height(btn_clear_));
        btn_open_->setEnabled(!initial_file.isEmpty());

        connect(btn_choose_, &QPushButton::clicked, this, [this]() {
            const QString path = QFileDialog::getOpenFileName(this, QCoreApplication::translate("PomodoroPlugin", "Escolher arquivo"), QString(), QStringLiteral("*"));
            if (!path.isEmpty()) {
                file_field_->setText(QFileInfo(path).fileName());
                file_field_->setToolTip(path);
                if (btn_open_ != nullptr) btn_open_->setEnabled(true);
            }
        });
        connect(btn_open_, &QPushButton::clicked, this, [this]() {
            compat::open_local_path(file_field_->toolTip(), this);
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

        // 2. Contêiner da descrição
        desc_container_ = new QWidget(this);
        auto *desc_layout = new QVBoxLayout(desc_container_);
        desc_layout->setContentsMargins(0, 0, 0, 0);
        desc_layout->setSpacing(4);

        label_desc_ = new QLabel(QCoreApplication::translate("PomodoroPlugin", "Descricao (opcional):"), desc_container_);
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

        label_data_hora_ = new QLabel(QCoreApplication::translate("PomodoroPlugin", "Data e horário (opcional):"), bottom_fields_container_);
        label_data_hora_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        label_data_hora_->setFixedHeight(label_data_hora_->fontMetrics().height() + 4);
        bottom_layout->addWidget(label_data_hora_);

        auto *date_row = new QHBoxLayout();
        date_row->setContentsMargins(0, 0, 0, 0);
        date_row->setSpacing(6);

        cb_date_ = new QCheckBox(QCoreApplication::translate("PomodoroPlugin", "Vincular data"), bottom_fields_container_);
        cb_date_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        cb_date_->setFixedHeight(compute_widget_height(cb_date_));
        date_edit_ = new QDateEdit(bottom_fields_container_);
        date_edit_->setCalendarPopup(true);
        date_edit_->setLocale(compat::effective_locale(this));
        date_edit_->setDisplayFormat(compat::effective_date_format(this));
        date_edit_->setDate(QDate::currentDate());
        date_edit_->setFixedHeight(compute_widget_height(date_edit_));

        const QDate init_date = QDate::fromString(initial_date, Qt::ISODate);
        cb_date_->setChecked(init_date.isValid());
        if (init_date.isValid()) {
            date_edit_->setDate(init_date);
        }
        date_edit_->setEnabled(cb_date_->isChecked());
        connect(cb_date_, &QCheckBox::toggled, date_edit_, &QWidget::setEnabled);

        date_row->addWidget(cb_date_);
        date_row->addWidget(date_edit_, 1);
        bottom_layout->addLayout(date_row);

        auto *time_row = new QHBoxLayout();
        time_row->setContentsMargins(0, 0, 0, 0);
        time_row->setSpacing(6);

        cb_time_ = new QCheckBox(QCoreApplication::translate("PomodoroPlugin", "Vincular horario"), bottom_fields_container_);
        cb_time_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        cb_time_->setFixedHeight(compute_widget_height(cb_time_));
        time_edit_ = new QTimeEdit(bottom_fields_container_);
        time_edit_->setDisplayFormat(QStringLiteral("HH:mm"));
        time_edit_->setTime(QTime::currentTime());
        time_edit_->setFixedHeight(compute_widget_height(time_edit_));

        const QTime init_time = QTime::fromString(initial_time, QStringLiteral("HH:mm"));
        cb_time_->setChecked(init_date.isValid() && init_time.isValid());
        if (init_time.isValid()) {
            time_edit_->setTime(init_time);
        }

        const auto sync_time_enabled = [this]() {
            cb_time_->setEnabled(cb_date_->isChecked());
            if (!cb_date_->isChecked()) {
                cb_time_->setChecked(false);
            }
            time_edit_->setEnabled(cb_date_->isChecked() && cb_time_->isChecked());
        };
        connect(cb_date_, &QCheckBox::toggled, this, [sync_time_enabled](bool) { sync_time_enabled(); });
        connect(cb_time_, &QCheckBox::toggled, this, [sync_time_enabled](bool) { sync_time_enabled(); });
        sync_time_enabled();

        time_row->addWidget(cb_time_);
        time_row->addWidget(time_edit_, 1);
        bottom_layout->addLayout(time_row);

        auto *buttons = new QHBoxLayout();
        buttons->setContentsMargins(0, 6, 0, 0);
        buttons->setSpacing(8);
        buttons->addStretch(1);

        btn_cancel_ = new QPushButton(QCoreApplication::translate("PomodoroPlugin", "Cancelar"), bottom_fields_container_);
        btn_ok_ = new QPushButton(QCoreApplication::translate("PomodoroPlugin", "OK"), bottom_fields_container_);
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

        const int compact_min_h = top_fields_height_ + bottom_fields_height_ + 60 + 18 + 4 + 16 + 24;
        setMinimumSize(440, compact_min_h);
        resize(480, 600);
        _update_layout_mode(width(), height());
    }

    Values values() const
    {
        Values data;
        data.title = title_ != nullptr ? title_->text().trimmed() : QString();
        data.file_path = file_field_ != nullptr ? file_field_->toolTip().trimmed() : QString();
        data.description = desc_ != nullptr ? desc_->toPlainText().trimmed() : QString();
        data.prioridade = combo_prioridade_ != nullptr ? combo_prioridade_->currentText() : QString();
        data.dominio = combo_dominio_ != nullptr ? combo_dominio_->currentIndex() : 0;
        const int default_ciclos = owner_ != nullptr ? owner_->obter_ciclos_permitidos_padrao() : source::plugin::Pomodoro::PomodoroTimer::obter_ciclos_permitidos_persistidos();
        data.pomodoros = spin_pomodoros_ != nullptr ? spin_pomodoros_->value() : 0;
        data.ciclos_totais = spin_ciclos_totais_ != nullptr ? spin_ciclos_totais_->value() : default_ciclos;
        data.duracao = spin_duracao_ != nullptr ? spin_duracao_->value() : 1.0;

        if (cb_date_ != nullptr && cb_date_->isChecked() && date_edit_ != nullptr) {
            data.date_iso = date_edit_->date().toString(Qt::ISODate);

            if (cb_time_ != nullptr && cb_time_->isChecked() && time_edit_ != nullptr) {
                data.time_str = time_edit_->time().toString(QStringLiteral("HH:mm"));
            }
        }

        return data;
    }

    void retranslateUi()
    {
        setWindowTitle(QCoreApplication::translate("PomodoroPlugin", "✏️ Editar Tarefa"));

        if (label_tarefa_ != nullptr) {
            label_tarefa_->setText(QCoreApplication::translate("PomodoroPlugin", "Tarefa:"));
        }
        if (label_prioridade_ != nullptr) {
            label_prioridade_->setText(QCoreApplication::translate("PomodoroPlugin", "Prioridade:"));
        }
        if (combo_prioridade_ != nullptr) {
            const int curr_prio = combo_prioridade_->currentIndex();
            combo_prioridade_->blockSignals(true);
            combo_prioridade_->clear();
            combo_prioridade_->addItems({
                QCoreApplication::translate("PomodoroPlugin", "🔴 Importante e Urgente"),
                QCoreApplication::translate("PomodoroPlugin", "🟠 Importante, mas Nao Urgente"),
                QCoreApplication::translate("PomodoroPlugin", "🟡 Nao Importante, mas Urgente"),
                QCoreApplication::translate("PomodoroPlugin", "🟢 Nao Importante e Nao Urgente"),
            });
            if (curr_prio >= 0 && curr_prio < combo_prioridade_->count()) {
                combo_prioridade_->setCurrentIndex(curr_prio);
            }
            combo_prioridade_->blockSignals(false);
        }
        if (label_dominio_ != nullptr) {
            label_dominio_->setText(QCoreApplication::translate("PomodoroPlugin", "Nivel de Dominio:"));
        }
        if (combo_dominio_ != nullptr) {
            const int curr_domain = combo_dominio_->currentIndex();
            combo_dominio_->blockSignals(true);
            combo_dominio_->clear();
            combo_dominio_->addItems({
                QCoreApplication::translate("PomodoroPlugin", "🥉 Iniciante (nao consigo explicar)"),
                QCoreApplication::translate("PomodoroPlugin", "🥈 Intermediario (explico com dificuldade)"),
                QCoreApplication::translate("PomodoroPlugin", "🥇 Avancado (explico facilmente)"),
            });
            if (curr_domain >= 0 && curr_domain < combo_dominio_->count()) {
                combo_dominio_->setCurrentIndex(curr_domain);
            }
            combo_dominio_->blockSignals(false);
        }
        if (label_pomodoro_ != nullptr) {
            label_pomodoro_->setText(QCoreApplication::translate("PomodoroPlugin", "Quantidade de Pomodoros (🍅):"));
        }
        if (label_ciclos_ != nullptr) {
            label_ciclos_->setText(QCoreApplication::translate("PomodoroPlugin", "Meta de Ciclos Pomodoro (🎯):"));
        }
        if (label_duracao_ != nullptr) {
            label_duracao_->setText(QCoreApplication::translate("PomodoroPlugin", "Duração:"));
        }
        if (spin_duracao_ != nullptr) {
            spin_duracao_->setSuffix(QCoreApplication::translate("PomodoroPlugin", " dias"));
        }
        if (label_arquivo_ != nullptr) {
            label_arquivo_->setText(QCoreApplication::translate("PomodoroPlugin", "Arquivo vinculado (arraste ou escolha):"));
        }
        if (btn_choose_ != nullptr) {
            btn_choose_->setText(QCoreApplication::translate("PomodoroPlugin", "Escolher..."));
        }
        if (btn_open_ != nullptr) {
            btn_open_->setText(QCoreApplication::translate("PomodoroPlugin", "Abrir"));
        }
        if (btn_clear_ != nullptr) {
            btn_clear_->setText(QCoreApplication::translate("PomodoroPlugin", "Remover"));
        }
        if (label_desc_ != nullptr) {
            label_desc_->setText(QCoreApplication::translate("PomodoroPlugin", "Descricao (opcional):"));
        }
        if (label_data_hora_ != nullptr) {
            label_data_hora_->setText(QCoreApplication::translate("PomodoroPlugin", "Data e horário (opcional):"));
        }
        if (cb_date_ != nullptr) {
            cb_date_->setText(QCoreApplication::translate("PomodoroPlugin", "Vincular data"));
        }
        if (cb_time_ != nullptr) {
            cb_time_->setText(QCoreApplication::translate("PomodoroPlugin", "Vincular horario"));
        }
        if (date_edit_ != nullptr) {
            date_edit_->setLocale(compat::effective_locale(this));
        }
        if (btn_cancel_ != nullptr) {
            btn_cancel_->setText(QCoreApplication::translate("PomodoroPlugin", "Cancelar"));
        }
        if (btn_ok_ != nullptr) {
            btn_ok_->setText(QCoreApplication::translate("PomodoroPlugin", "OK"));
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

    QPointer<GerenciadorTarefas> owner_;
    QLineEdit *title_ = nullptr;
    QComboBox *combo_prioridade_ = nullptr;
    QComboBox *combo_dominio_ = nullptr;
    FileDropLineEdit *file_field_ = nullptr;
    QTextEdit *desc_ = nullptr;
    QCheckBox *cb_date_ = nullptr;
    QDateEdit *date_edit_ = nullptr;
    QCheckBox *cb_time_ = nullptr;
    QTimeEdit *time_edit_ = nullptr;
    QSpinBox *spin_pomodoros_ = nullptr;
    QSpinBox *spin_ciclos_totais_ = nullptr;
    QDoubleSpinBox *spin_duracao_ = nullptr;

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

inline QDate parse_task_date(const QString &value)
{
    if (value.isEmpty()) {
        return {};
    }

    QDate date = QDate::fromString(value, QStringLiteral("yyyy-MM-dd"));
    if (!date.isValid()) {
        date = QDate::fromString(value, Qt::ISODate);
    }
    return date;
}

inline QTime parse_task_time(const QString &value)
{
    if (value.isEmpty()) {
        return {};
    }

    QTime time = QTime::fromString(value, QStringLiteral("HH:mm"));
    if (!time.isValid()) {
        time = QTime::fromString(value, QStringLiteral("HH:mm:ss"));
    }
    return time;
}

inline QString human_date_for_display(const QWidget *context, const QString &date_value)
{
    const QDate date = parse_task_date(date_value);
    return date.isValid() ? date.toString(compat::effective_date_format(context)) : date_value.trimmed();
}

inline QString visible_task_title(const QWidget *context,
                           const QString &base_text,
                           const QString &date_value,
                           const QString &time_value)
{
    QString display_text = base_text.trimmed();
    if (compat::host_bridge() == nullptr || !compat::host_bridge()->reserved_space_enabled()) {
        return display_text;
    }

    const QString human_date = human_date_for_display(context, date_value);
    if (human_date.isEmpty()) {
        return display_text;
    }

    const QString cleaned_time = time_value.trimmed();
    return cleaned_time.isEmpty() ? QStringLiteral("%1 — %2").arg(display_text, human_date)
                                  : QStringLiteral("%1 — %2 %3").arg(display_text, human_date, cleaned_time);
}

inline QString watermark_task_line(const QWidget *context, const QString &date_value, const QString &time_value)
{
    if (compat::host_bridge() == nullptr || !compat::host_bridge()->watermark_enabled()) {
        return {};
    }

    QStringList parts;
    const QString human_date = human_date_for_display(context, date_value);
    if (!human_date.trimmed().isEmpty()) {
        parts << QStringLiteral("%1: %2")
                     .arg(QCoreApplication::translate("PomodoroPlugin", "Data"), human_date.trimmed());
    }

    const QString cleaned_time = time_value.trimmed();
    if (!cleaned_time.isEmpty()) {
        parts << QStringLiteral("%1: %2")
                     .arg(QCoreApplication::translate("PomodoroPlugin", "Horario"), cleaned_time);
    }

    return parts.join(QStringLiteral("  |  "));
}

inline QString task_group_key(const QString &date_value, const QString &time_value)
{
    const QString cleaned_time = time_value.trimmed();
    if (cleaned_time.isEmpty()) {
        return {};
    }

    const QString cleaned_date = date_value.trimmed().isEmpty() ? QStringLiteral("9999-99-99") : date_value.trimmed();
    return QStringLiteral("%1|%2").arg(cleaned_date, cleaned_time);
}

inline bool is_group_header(QListWidgetItem *item)
{
    return item != nullptr && item->data(kGroupHeaderRole).toString() == QStringLiteral("group_header");
}

inline int priority_rank(const QString &value)
{
    static const QMap<QString, int> rank = {
        {QStringLiteral("Importante e Urgente"), 1},
        {QStringLiteral("Importante, mas Não Urgente"), 2},
        {QStringLiteral("Não Importante, mas Urgente"), 3},
        {QStringLiteral("Não Importante e Não Urgente"), 4},
    };
    return rank.value(Tarefa::normalizar_prioridade(value), 1);
}

inline QString task_status_for_list(const GerenciadorTarefas *self, const QListWidget *list)
{
    if (self == nullptr || list == nullptr) {
        return QStringLiteral("Todo");
    }

    if (self->col_todo != nullptr && self->col_todo->lista == list) {
        return QStringLiteral("Todo");
    }

    if (self->col_doing != nullptr && self->col_doing->lista == list) {
        return QStringLiteral("Doing");
    }

    return QStringLiteral("Done");
}

inline QString priority_from_item_text(const QString &line)
{
    if (line.startsWith(QStringLiteral("🔴"))) {
        return QStringLiteral("Importante e Urgente");
    }
    if (line.startsWith(QStringLiteral("🟠"))) {
        return QStringLiteral("Importante, mas Não Urgente");
    }
    if (line.startsWith(QStringLiteral("🟡"))) {
        return QStringLiteral("Não Importante, mas Urgente");
    }
    if (line.startsWith(QStringLiteral("🟢"))) {
        return QStringLiteral("Não Importante e Não Urgente");
    }
    return QString();
}

inline QString legacy_tasks_path()
{
    return QDir(compat::plugin_persistent_dir()).filePath(QStringLiteral("tarefas_pomodoro.json"));
}

inline QString fold_status_text(QString value)
{
    value = value.trimmed().toCaseFolded();
    value.replace(QStringLiteral("não"), QStringLiteral("nao"));
    value.replace(QStringLiteral("ã"), QStringLiteral("a"));
    value.replace(QStringLiteral("á"), QStringLiteral("a"));
    value.replace(QStringLiteral("â"), QStringLiteral("a"));
    value.replace(QStringLiteral("é"), QStringLiteral("e"));
    value.replace(QStringLiteral("ê"), QStringLiteral("e"));
    value.replace(QStringLiteral("í"), QStringLiteral("i"));
    value.replace(QStringLiteral("ó"), QStringLiteral("o"));
    value.replace(QStringLiteral("ô"), QStringLiteral("o"));
    value.replace(QStringLiteral("ú"), QStringLiteral("u"));
    value.replace(QStringLiteral("ç"), QStringLiteral("c"));
    value.replace(QLatin1Char(','), QLatin1Char(' '));
    value.replace(QLatin1Char('-'), QLatin1Char(' '));
    return value.simplified();
}

inline QString normalize_task_status(const QString &status, bool completed_hint = false)
{
    const QString folded = fold_status_text(status);
    if (folded.isEmpty()) {
        return completed_hint ? QStringLiteral("Done") : QStringLiteral("Todo");
    }

    if (folded == QStringLiteral("done") || folded == QStringLiteral("completed")
        || folded.contains(QStringLiteral("conclu")) || folded.contains(QStringLiteral("finaliz"))) {
        return QStringLiteral("Done");
    }

    if (folded == QStringLiteral("doing") || folded.contains(QStringLiteral("in progress"))
        || folded.contains(QStringLiteral("progress")) || folded.contains(QStringLiteral("progresso"))
        || folded.contains(QStringLiteral("andamento"))) {
        return QStringLiteral("Doing");
    }

    if (folded == QStringLiteral("todo") || folded == QStringLiteral("to do")
        || folded == QStringLiteral("afazer") || folded == QStringLiteral("a fazer")
        || folded == QStringLiteral("pending") || folded == QStringLiteral("pendente")) {
        return QStringLiteral("Todo");
    }

    return completed_hint ? QStringLiteral("Done") : QStringLiteral("Todo");
}

inline bool is_done_status(const QString &status)
{
    return status.compare(QStringLiteral("Done"), Qt::CaseInsensitive) == 0
        || status.compare(QStringLiteral("Completed"), Qt::CaseInsensitive) == 0;
}

inline QStringList steps_from_variant_list(const QVariantList &steps)
{
    QStringList result;
    result.reserve(steps.size());
    for (const QVariant &value : steps) {
        result.append(value.toString());
    }
    return result;
}

inline QVariantMap unified_record_from_task(const Tarefa &task)
{
    const int prioridade = priority_rank(task.prioridade);
    const QString status = normalize_task_status(task.status);
    const bool in_prog = (status == QStringLiteral("Doing"));
    QVariantList etapas;
    etapas.reserve(task.etapas.size());
    for (const QString &etapa : task.etapas) {
        etapas.append(etapa);
    }

    const double dur = task.duracao > 0.0 ? task.duracao : 1.0;
    return source::utils::UnifiedTaskStore::normalize_record({
        {QStringLiteral("id"), task.id},
        {QStringLiteral("modulo"), QStringLiteral("pomodoro")},
        {QStringLiteral("tipo"), QStringLiteral("tarefa")},
        {QStringLiteral("titulo"), task.titulo},
        {QStringLiteral("descricao"), task.descricao},
        {QStringLiteral("prioridade"), prioridade},
        {QStringLiteral("quadrante"), prioridade},
        {QStringLiteral("categoria"), QStringLiteral("kanban")},
        {QStringLiteral("dominio"), task.dominio},
        {QStringLiteral("status"), status},
        {QStringLiteral("concluida"), is_done_status(status)},
        {QStringLiteral("in_progress"), in_prog},
        {QStringLiteral("em_progresso"), in_prog},
        {QStringLiteral("em_andamento"), in_prog},
        {QStringLiteral("data"), task.date},
        {QStringLiteral("hora"), task.time},
        {QStringLiteral("arquivo_vinculado"), task.file_path},
        {QStringLiteral("cor_texto"), task.cor_texto},
        {QStringLiteral("integracao_origem"), task.integracao_origem},
        {QStringLiteral("data_criacao"), task.data_criacao},
        {QStringLiteral("etapas"), etapas},
        {QStringLiteral("font_config"), task.font_config},
        {QStringLiteral("sincronizada_com_eisenhower"), task.sincronizada_com_eisenhower},
        {QStringLiteral("duracao"), dur},
        {QStringLiteral("duration"), dur},
        {QStringLiteral("duracao_dias"), dur},
        {QStringLiteral("pomodoros"), task.pomodoros_completados},
        {QStringLiteral("pomodoros_completados"), task.pomodoros_completados},
        {QStringLiteral("ciclos_totais"), task.ciclos_totais > 0 ? task.ciclos_totais : 4},
        {QStringLiteral("ciclos_permitidos"), task.ciclos_totais > 0 ? task.ciclos_totais : 4},
        {QStringLiteral("total_ciclos"), task.ciclos_totais > 0 ? task.ciclos_totais : 4},
        {QStringLiteral("ciclos_customizados"), task.ciclos_customizados},
        {QStringLiteral("percentual"), task.percentual_conclusao()},
    });
}

inline std::optional<Tarefa> task_from_unified_record(const QVariantMap &record)
{
    const QVariantMap normalized = source::utils::UnifiedTaskStore::normalize_record(record);
    if (normalized.value(QStringLiteral("modulo")).toString().compare(QStringLiteral("pomodoro"), Qt::CaseInsensitive)
        != 0) {
        return std::nullopt;
    }

    const QString title = normalized.value(QStringLiteral("titulo")).toString().trimmed();
    if (title.isEmpty()) {
        return std::nullopt;
    }

    const QString prioridade =
        Tarefa::normalizar_prioridade(GerenciadorTarefasFns::prioridade_para_texto(normalized.value(QStringLiteral("prioridade"))));
    QString status = normalize_task_status(normalized.value(QStringLiteral("status")).toString(),
                                           normalized.value(QStringLiteral("concluida")).toBool());
    if (status != QStringLiteral("Done") && (normalized.value(QStringLiteral("in_progress")).toBool()
                                             || normalized.value(QStringLiteral("em_progresso")).toBool()
                                             || normalized.value(QStringLiteral("em_andamento")).toBool())) {
        status = QStringLiteral("Doing");
    }

    Tarefa task(title,
                normalized.value(QStringLiteral("descricao")).toString(),
                steps_from_variant_list(normalized.value(QStringLiteral("etapas")).toList()),
                prioridade,
                status,
                normalized.value(QStringLiteral("id")).toString(),
                normalized.value(QStringLiteral("arquivo_vinculado")).toString(),
                normalized.value(QStringLiteral("data")).toString(),
                normalized.value(QStringLiteral("hora")).toString(),
                Tarefa::normalizar_dominio(normalized.value(QStringLiteral("dominio"))),
                normalized.value(QStringLiteral("cor_texto")).toString(),
                normalized.value(QStringLiteral("integracao_origem")).toString());
    if (normalized.contains(QStringLiteral("pomodoros"))) {
        task.pomodoros_completados = normalized.value(QStringLiteral("pomodoros")).toInt();
    } else if (normalized.contains(QStringLiteral("pomodoros_completados"))) {
        task.pomodoros_completados = normalized.value(QStringLiteral("pomodoros_completados")).toInt();
    }
    const int c_tot = normalized.value(QStringLiteral("ciclos_totais"),
                                       normalized.value(QStringLiteral("ciclos_permitidos"),
                                                        normalized.value(QStringLiteral("total_ciclos"), 4))).toInt();
    task.ciclos_totais = c_tot > 0 ? c_tot : 4;
    task.ciclos_customizados = normalized.value(QStringLiteral("ciclos_customizados"), false).toBool();
    double dur = 1.0;
    if (normalized.contains(QStringLiteral("duracao")) && normalized.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        dur = normalized.value(QStringLiteral("duracao")).toDouble();
    } else if (normalized.contains(QStringLiteral("duration")) && normalized.value(QStringLiteral("duration")).toDouble() > 0.0) {
        dur = normalized.value(QStringLiteral("duration")).toDouble();
    } else if (normalized.contains(QStringLiteral("duracao_dias")) && normalized.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
        dur = normalized.value(QStringLiteral("duracao_dias")).toDouble();
    }
    task.duracao = dur;
    task.sincronizada_com_eisenhower = normalized.value(QStringLiteral("sincronizada_com_eisenhower")).toBool();
    task.font_config = normalized.value(QStringLiteral("font_config")).toMap();
    return task;
}

inline QVector<Tarefa> tasks_from_records(const QVariantList &records)
{
    QVector<Tarefa> tasks;
    tasks.reserve(records.size());
    for (const QVariant &value : records) {
        const auto maybe_task = task_from_unified_record(value.toMap());
        if (maybe_task.has_value()) {
            tasks.append(maybe_task.value());
        }
    }
    return tasks;
}

inline QVariantMap sort_payload_from_task(const Tarefa &task)
{
    return {
        {QStringLiteral("id"), task.id},
        {QStringLiteral("text"), task.titulo},
        {QStringLiteral("date"), task.date},
        {QStringLiteral("time"), task.time},
    };
}

inline bool has_host_task_sort()
{
    return compat::host_bridge() != nullptr && compat::host_bridge()->has_active_task_sort();
}

inline bool host_task_sort_less(const Tarefa *lhs, const Tarefa *rhs)
{
    if (lhs == nullptr || rhs == nullptr || compat::host_bridge() == nullptr) {
        return false;
    }

    return compat::host_bridge()->task_sort_less(sort_payload_from_task(*lhs), sort_payload_from_task(*rhs));
}

inline QVector<Tarefa> load_tasks_from_path(const QString &path)
{
    const QVariantList records = source::utils::UnifiedTaskStore::load_records(path);
    const bool sync_enabled = compat::host_bridge() == nullptr || compat::host_bridge()->task_sync_enabled();
    QVariantList pomodoro_records = sync_enabled
        ? source::utils::UnifiedTaskStore::project_records_for_module(records, QStringLiteral("pomodoro"))
        : source::utils::UnifiedTaskStore::filter_records_by_module(records, QStringLiteral("pomodoro"));
    if (pomodoro_records.isEmpty() && sync_enabled) {
        pomodoro_records = source::utils::UnifiedTaskStore::filter_records_by_module(records, QStringLiteral("pomodoro"));
    }
    return tasks_from_records(pomodoro_records);
}

inline QVariantList records_from_tasks(const QVector<Tarefa> &tasks)
{
    QVariantList records;
    records.reserve(tasks.size());
    for (const Tarefa &task : tasks) {
        records.append(unified_record_from_task(task));
    }
    return records;
}

}  // namespace source::plugin::Pomodoro::GerenciadorTarefasInternal
