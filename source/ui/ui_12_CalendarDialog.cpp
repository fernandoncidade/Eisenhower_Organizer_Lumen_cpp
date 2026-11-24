#include "source/ui/ui_12_CalendarDialog.hpp"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDateEdit>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPalette>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>

#include "source/ui/CalendarDialog/agregador_01_CalendarDialog.hpp"
#include "source/ui/ui_10_BadgeCalendarWidget.hpp"
#include "source/ui/ui_11_NativeLineEditFrame.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"
#include "source/utils/LogManager.hpp"

namespace source::GerenciamentoUI {
namespace {

constexpr int kTasksPanelHorizontalInset = 2;
constexpr int kTasksPanelTopInset = 12;
constexpr int kTasksPanelBottomInset = 4;

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

void apply_window_bg(QWidget *widget, const QColor &color)
{
    if (widget == nullptr) {
        return;
    }

    QPalette pal = widget->palette();
    pal.setColor(QPalette::Window, color);
    widget->setAutoFillBackground(true);
    widget->setPalette(pal);
}

}  // namespace

CalendarDialog::CalendarDialog(source::EisenhowerMatrixApp *app_in) : QDialog(app_in), app(app_in)
{
    try {
        setModal(true);
        setWindowTitle(get_text(QStringLiteral("Calendário de Tarefas")));
        _date_format = app->date_input != nullptr ? app->date_input->displayFormat() : QStringLiteral("dd/MM/yyyy");
        _highlighted_dates.clear();

        auto *main_layout = new QVBoxLayout(this);

        const QColor base_color = app->palette().color(QPalette::Window);
        const QColor header_color = QColor(base_color).lighter(120);
        apply_window_bg(this, header_color);

        calendar = new BadgeCalendarWidget(this);
        const QDate initial_date = app->date_input != nullptr ? app->date_input->date() : QDate::currentDate();
        calendar->setSelectedDate(initial_date);
        calendar->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(calendar, &QWidget::customContextMenuRequested, this, &CalendarDialog::_on_calendar_context_menu);

        const QSize hint = calendar->minimumSizeHint();
        const QSize fallback(300, 260);

        calendar_group = new QGroupBox(get_text(QStringLiteral("Calendário")), this);
        calendar_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        apply_window_bg(calendar_group, header_color);
        auto *calendar_group_layout = new QVBoxLayout(calendar_group);
        calendar_group_layout->setContentsMargins(4, 12, 4, 4);
        calendar_group_layout->setSpacing(0);
        calendar->setMinimumSize(hint.expandedTo(fallback));
        calendar_group_layout->addWidget(calendar);

        _apply_locale_to_calendar();
        main_layout->addWidget(calendar_group);

        auto *controls_layout = new QHBoxLayout();
        filter_label = new QLabel(get_text(QStringLiteral("Exibir por")), this);
        controls_layout->addWidget(filter_label);

        filter_combo = new QComboBox(this);
        filter_combo->addItem(get_text(QStringLiteral("Dia")), QStringLiteral("day"));
        filter_combo->addItem(get_text(QStringLiteral("Semana")), QStringLiteral("week"));
        filter_combo->addItem(get_text(QStringLiteral("Mês")), QStringLiteral("month"));
        controls_layout->addWidget(filter_combo);
        controls_layout->addStretch();

        show_label = new QLabel(get_text(QStringLiteral("Mostrar:")), this);
        controls_layout->addWidget(show_label);

        show_priority_checkbox = new QCheckBox(get_text(QStringLiteral("Prioridade")), this);
        show_priority_checkbox->setChecked(true);
        connect(show_priority_checkbox, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState) {
            update_task_list();
        });
        controls_layout->addWidget(show_priority_checkbox);

        show_domain_checkbox = new QCheckBox(get_text(QStringLiteral("Domínio")), this);
        show_domain_checkbox->setChecked(true);
        connect(show_domain_checkbox, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState) {
            update_task_list();
        });
        controls_layout->addWidget(show_domain_checkbox);
        main_layout->addLayout(controls_layout);

        tasks_group = new QGroupBox(get_text(QStringLiteral("Lista Global de Tarefas:")), this);
        tasks_group->setFlat(false);
        tasks_group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        apply_window_bg(tasks_group, header_color);
        auto *tasks_group_layout = new QVBoxLayout(tasks_group);
        tasks_group_layout->setContentsMargins(kTasksPanelHorizontalInset,
                                               kTasksPanelTopInset,
                                               kTasksPanelHorizontalInset,
                                               kTasksPanelBottomInset);
        tasks_group_layout->setSpacing(6);

        tasks_list = new QListWidget(tasks_group);
        tasks_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
        tasks_list->setSelectionBehavior(QAbstractItemView::SelectItems);
        tasks_list->setSelectionRectVisible(true);
        tasks_list->setDragEnabled(false);
        tasks_list->setDragDropMode(QAbstractItemView::NoDragDrop);
        tasks_list->setMouseTracking(true);
        tasks_list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        tasks_list->setContextMenuPolicy(Qt::CustomContextMenu);
        source::utils::install_linked_title_underline_delegate(tasks_list);
        connect(tasks_list, &QWidget::customContextMenuRequested, this, [this](const QPoint &pt) {
            if (tasks_list == nullptr) {
                return;
            }
            QListWidgetItem *item = tasks_list->itemAt(pt);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
                return;
            }
            app->show_context_menu(pt, tasks_list);
        });
        connect(tasks_list, &QListWidget::itemDoubleClicked, this, &CalendarDialog::_open_linked_file);

        QPalette list_pal = tasks_list->palette();
        list_pal.setColor(QPalette::Base, base_color);
        list_pal.setColor(QPalette::AlternateBase, base_color);
        tasks_list->setPalette(list_pal);
        tasks_list->setAutoFillBackground(true);
        if (tasks_list->viewport() != nullptr) {
            tasks_list->viewport()->setAutoFillBackground(true);
        }

        tasks_group_layout->addWidget(tasks_list, 1);
        main_layout->addWidget(tasks_group, 1);

        connect(calendar, &QCalendarWidget::selectionChanged, this, &CalendarDialog::update_task_list);
        connect(filter_combo, &QComboBox::currentIndexChanged, this, [this]() { update_task_list(); });

        if (app->gerenciador_traducao != nullptr) {
            connect(app->gerenciador_traducao,
                    &source::language::GerenciadorTraducao::idioma_alterado,
                    this,
                    [this](const QString &) { _on_language_changed(); });
        }

        update_task_list();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao inicializar CalendarDialog."));
    }
}

void CalendarDialog::closeEvent(QCloseEvent *event)
{
    CalendarDialogModules::closeEvent(this, event);
    QDialog::closeEvent(event);
}

QList<QVariantMap> CalendarDialog::_collect_tasks() const
{
    return CalendarDialogModules::_collect_tasks(this);
}

void CalendarDialog::_open_linked_file(QListWidgetItem *item)
{
    CalendarDialogModules::_open_linked_file(this, item);
}

void CalendarDialog::update_task_list()
{
    CalendarDialogModules::update_task_list(this);
}

void CalendarDialog::_on_calendar_context_menu(const QPoint &pos)
{
    CalendarDialogModules::_on_calendar_context_menu(this, pos);
}

void CalendarDialog::_prompt_edit_task_for_date(const QDate &qdate)
{
    CalendarDialogModules::_prompt_edit_task_for_date(this, qdate);
}

void CalendarDialog::_prompt_remove_task_for_date(const QDate &qdate)
{
    CalendarDialogModules::_prompt_remove_task_for_date(this, qdate);
}

void CalendarDialog::_update_badge_counts()
{
    CalendarDialogModules::_update_badge_counts(this);
}

void CalendarDialog::_on_language_changed()
{
    CalendarDialogModules::_on_language_changed(this);
}

void CalendarDialog::_apply_locale_to_calendar()
{
    CalendarDialogModules::_apply_locale_to_calendar(this);
}

QSet<QString> CalendarDialog::_get_task_dates() const
{
    return CalendarDialogModules::_get_task_dates(this);
}

void CalendarDialog::_apply_highlighted_dates()
{
    CalendarDialogModules::_apply_highlighted_dates(this);
}

}  // namespace source::GerenciamentoUI
