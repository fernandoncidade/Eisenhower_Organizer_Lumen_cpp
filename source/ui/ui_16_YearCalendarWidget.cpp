#include "source/ui/ui_16_YearCalendarWidget.hpp"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>

#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/services/incore_13_prioridade_display.hpp"
#include "source/eisenhower_app.hpp"
#include "source/ui/agregador_01_ui.hpp"
#include "source/utils/LogManager.hpp"

namespace source::GerenciamentoUI {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

bool event_is_context_menu(QEvent *event)
{
    return event != nullptr && event->type() == QEvent::ContextMenu;
}

bool event_is_double_click(QEvent *event)
{
    return event != nullptr && event->type() == QEvent::MouseButtonDblClick;
}

}  // namespace

YearCalendarWidget::YearCalendarWidget(QWidget *parent)
    : QWidget(parent), _locale(QLocale::system()), _year(QDate::currentDate().year()),
      _selected_month(QDate::currentDate().month())
{
    auto *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(6, 6, 6, 6);
    main_layout->setSpacing(6);

    auto *header_layout = new QHBoxLayout();
    header_layout->setContentsMargins(0, 0, 0, 0);
    header_layout->setSpacing(6);

    prev_year_button = new QToolButton(this);
    prev_year_button->setText(QStringLiteral("<"));
    connect(prev_year_button, &QToolButton::clicked, this, [this]() { setYear(_year - 1); });
    header_layout->addWidget(prev_year_button);

    year_label = new QLabel(this);
    year_label->setAlignment(Qt::AlignCenter);
    year_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    header_layout->addWidget(year_label, 1);

    next_year_button = new QToolButton(this);
    next_year_button->setText(QStringLiteral(">"));
    connect(next_year_button, &QToolButton::clicked, this, [this]() { setYear(_year + 1); });
    header_layout->addWidget(next_year_button);

    main_layout->addLayout(header_layout);

    auto *month_grid = new QGridLayout();
    month_grid->setContentsMargins(0, 0, 0, 0);
    month_grid->setHorizontalSpacing(6);
    month_grid->setVerticalSpacing(6);
    main_layout->addLayout(month_grid);

    for (int month = 1; month <= 12; ++month) {
        auto *button = new QToolButton(this);
        button->setCheckable(true);
        button->setAutoExclusive(true);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        button->setMinimumHeight(56);
        button->setProperty("month", month);
        button->installEventFilter(this);
        connect(button, &QToolButton::clicked, this, [this, month]() { _on_month_clicked(month); });
        _month_buttons.append(button);
        month_grid->addWidget(button, (month - 1) / 4, (month - 1) % 4);
    }

    _refresh_header();
    _refresh_month_buttons();
    _apply_palette_style();
    _install_theme_sync();
}

void YearCalendarWidget::setLocale(const QLocale &locale)
{
    _locale = locale;
    _refresh_month_buttons();
}

void YearCalendarWidget::setYear(int year_value)
{
    const int target_year = year_value;
    if (_year == target_year) {
        return;
    }
    _year = target_year;
    _refresh_header();
    _refresh_month_buttons();
    emit monthSelected(selectedDate());
}

int YearCalendarWidget::year() const
{
    return _year;
}

void YearCalendarWidget::setSelectedDate(const QDate &qdate)
{
    if (!qdate.isValid()) {
        return;
    }
    if (_year == qdate.year() && _selected_month == qdate.month()) {
        return;
    }
    _year = qdate.year();
    _selected_month = qdate.month();
    _refresh_header();
    _refresh_month_buttons();
}

QDate YearCalendarWidget::selectedDate() const
{
    return QDate(_year, _selected_month, 1);
}

void YearCalendarWidget::set_month_counts(const QMap<QPair<int, int>, int> &month_counts)
{
    if (_month_counts == month_counts) {
        return;
    }
    _month_counts = month_counts;
    _refresh_month_buttons();
}

void YearCalendarWidget::set_month_tooltip_map(const QMap<QPair<int, int>, QStringList> &month_tooltips)
{
    if (_month_tooltips == month_tooltips) {
        return;
    }
    _month_tooltips = month_tooltips;
    _refresh_month_buttons();
}

void YearCalendarWidget::set_month_data(const QMap<QPair<int, int>, int> &month_counts,
                                        const QMap<QPair<int, int>, QStringList> &month_tooltips)
{
    if (_month_counts == month_counts && _month_tooltips == month_tooltips) {
        return;
    }
    _month_counts = month_counts;
    _month_tooltips = month_tooltips;
    _refresh_month_buttons();
}

const QMap<QPair<int, int>, int> &YearCalendarWidget::month_counts() const
{
    return _month_counts;
}

const QMap<QPair<int, int>, QStringList> &YearCalendarWidget::month_tooltips() const
{
    return _month_tooltips;
}

bool YearCalendarWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (QToolButton *button = qobject_cast<QToolButton *>(watched)) {
        const int month = button->property("month").toInt();
        if (month >= 1 && month <= 12 && event_is_context_menu(event)) {
            _show_month_context_menu(month, button, event);
            return true;
        }
        if (month >= 1 && month <= 12 && event_is_double_click(event)) {
            _on_month_clicked(month);
            emit monthDoubleClicked(selectedDate());
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void YearCalendarWidget::changeEvent(QEvent *event)
{
    try {
        if (event != nullptr
            && (event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange
                || event->type() == QEvent::ThemeChange || event->type() == QEvent::StyleChange)) {
            _apply_palette_style();
        }
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao processar changeEvent no calendário anual."));
    }

    QWidget::changeEvent(event);
}

QList<QVariantMap> YearCalendarWidget::_month_tasks(int month) const
{
    QList<QVariantMap> tasks;
    if (app == nullptr) {
        return tasks;
    }

    const QList<QVariantMap> all_tasks = task_utils::collect_tasks_from_app(app);
    for (const QVariantMap &task : all_tasks) {
        const QString ds = task.value(QStringLiteral("date")).toString();
        const QDate qd = QDate::fromString(ds, Qt::ISODate);
        if (qd.isValid() && qd.year() == _year && qd.month() == month) {
            tasks.append(task);
        }
    }

    std::sort(tasks.begin(), tasks.end(), [](const QVariantMap &a, const QVariantMap &b) {
        const QString a_date = a.value(QStringLiteral("date")).toString();
        const QString b_date = b.value(QStringLiteral("date")).toString();
        if (a_date != b_date) {
            return a_date < b_date;
        }

        const QString a_time = a.value(QStringLiteral("time")).toString();
        const QString b_time = b.value(QStringLiteral("time")).toString();
        if (a_time != b_time) {
            return a_time < b_time;
        }

        return a.value(QStringLiteral("text")).toString() < b.value(QStringLiteral("text")).toString();
    });
    return tasks;
}

QString YearCalendarWidget::_format_task_label(const QVariantMap &task, bool show_priority, bool show_domain) const
{
    QString text = task.value(QStringLiteral("text")).toString().trimmed();
    if (text.isEmpty()) {
        text = get_text(QStringLiteral("(Sem título)"));
    }

    QString date_str = task.value(QStringLiteral("date")).toString();
    const QString time_str = task.value(QStringLiteral("time")).toString();
    QString emoji;
    if (show_priority) {
        emoji = task_utils::priority_emoji(task.value(QStringLiteral("priority")), app);
    }
    const QString domain_badge =
        show_domain ? source::InterfaceCore::dominio_insignia(task.value(QStringLiteral("dominio"))) : QString();
    if (!domain_badge.isEmpty()) {
        text = QStringLiteral("%1 %2").arg(domain_badge, text);
    }

    if (!date_str.isEmpty()) {
        const QDate qd = QDate::fromString(date_str, Qt::ISODate);
        if (qd.isValid()) {
            date_str = qd.toString(QStringLiteral("dd/MM/yyyy"));
        }
    }

    QString label = text;
    if (!time_str.isEmpty()) {
        label = QStringLiteral("%1 %2 — %3").arg(date_str, time_str, label).trimmed();
    } else if (!date_str.isEmpty()) {
        label = QStringLiteral("%1 — %2").arg(date_str, label).trimmed();
    }
    if (!emoji.isEmpty()) {
        label = QStringLiteral("%1 %2").arg(emoji, label);
    }
    return label;
}

void YearCalendarWidget::_edit_task_from_data(const QVariantMap &task)
{
    if (app == nullptr) {
        return;
    }
    const auto found = task_utils::find_source_item_for_calendar(app, task);
    if (found.first == nullptr || found.second == nullptr) {
        QMessageBox::warning(this, get_text(QStringLiteral("✏️ Editar Tarefa")), get_text(QStringLiteral("Tarefa não encontrada.")));
        return;
    }

    app->edit_task_datetime(found.second, found.first);
    if (auto *panel = qobject_cast<CalendarPanel *>(owner)) {
        panel->update_task_list();
    }
}

bool YearCalendarWidget::_remove_task_from_data(const QVariantMap &task, bool confirm)
{
    if (app == nullptr) {
        return false;
    }
    const auto found = task_utils::find_source_item_for_calendar(app, task);
    if (found.first == nullptr || found.second == nullptr) {
        QMessageBox::warning(this, get_text(QStringLiteral("🗑️ Remover Tarefa")), get_text(QStringLiteral("Tarefa não encontrada.")));
        return false;
    }
    return app->remove_task(found.second, found.first, confirm);
}

int YearCalendarWidget::_remove_all_tasks_for_month(int month)
{
    const QList<QVariantMap> tasks = _month_tasks(month);
    if (tasks.isEmpty()) {
        return 0;
    }

    const auto confirm = QMessageBox::question(
        this,
        get_text(QStringLiteral("Remover Todas")),
        get_text(QStringLiteral("Deseja remover todas as tarefas deste mês?")),
        QMessageBox::Yes | QMessageBox::No);
    if (confirm != QMessageBox::Yes) {
        return 0;
    }

    int removed = 0;
    int failed = 0;
    for (const QVariantMap &task : tasks) {
        if (_remove_task_from_data(task, false)) {
            ++removed;
        } else {
            ++failed;
        }
    }

    if (auto *panel = qobject_cast<CalendarPanel *>(owner)) {
        panel->update_task_list();
    }

    if (removed > 0 && failed == 0) {
        QMessageBox::information(this, get_text(QStringLiteral("🗑️ Remover Tarefa")),
                                 get_text(QStringLiteral("Tarefa(s) removida(s) com sucesso.")));
    } else if (removed > 0) {
        QMessageBox::warning(this, get_text(QStringLiteral("🗑️ Remover Tarefa")),
                             get_text(QStringLiteral("Algumas tarefas não puderam ser removidas.")));
    } else {
        QMessageBox::warning(this, get_text(QStringLiteral("🗑️ Remover Tarefa")),
                             get_text(QStringLiteral("Não foi possível remover as tarefas deste mês.")));
    }

    return removed;
}

void YearCalendarWidget::_show_month_context_menu(int month, QWidget *watched, QEvent *event)
{
    const QList<QVariantMap> tasks = _month_tasks(month);
    bool show_priority = true;
    bool show_domain = true;
    if (owner != nullptr) {
        const QVariant checked = owner->property("show_priority_checked");
        if (checked.isValid()) {
            show_priority = checked.toBool();
        }
        const QVariant domain_checked = owner->property("show_domain_checked");
        if (domain_checked.isValid()) {
            show_domain = domain_checked.toBool();
        }
    }

    QMenu menu(this);
    QMenu *edit_menu = menu.addMenu(get_text(QStringLiteral("✏️ Editar Tarefa")));
    QMenu *remove_menu = menu.addMenu(get_text(QStringLiteral("🗑️ Remover Tarefa")));

    if (tasks.isEmpty()) {
        edit_menu->setEnabled(false);
        remove_menu->setEnabled(false);
    } else {
        for (const QVariantMap &task : tasks) {
            QAction *act = edit_menu->addAction(_format_task_label(task, show_priority, show_domain));
            connect(act, &QAction::triggered, this, [this, task]() { _edit_task_from_data(task); });
        }

        for (const QVariantMap &task : tasks) {
            QAction *act = remove_menu->addAction(_format_task_label(task, show_priority, show_domain));
            connect(act, &QAction::triggered, this, [this, task]() { _remove_task_from_data(task, true); });
        }

        remove_menu->addSeparator();
        QAction *remove_all = remove_menu->addAction(get_text(QStringLiteral("Remover Todas")));
        connect(remove_all, &QAction::triggered, this, [this, month]() { _remove_all_tasks_for_month(month); });
    }

    QPoint global_pos = watched->mapToGlobal(watched->rect().center());
    if (event != nullptr && event->type() == QEvent::ContextMenu) {
        if (auto *ctx = static_cast<QContextMenuEvent *>(event)) {
            global_pos = ctx->globalPos();
        }
    }
    menu.exec(global_pos);
}

void YearCalendarWidget::_on_month_clicked(int month)
{
    _selected_month = month;
    _refresh_month_buttons();
    emit monthSelected(selectedDate());
}

void YearCalendarWidget::_refresh_header()
{
    if (year_label->text() != QString::number(_year)) {
        year_label->setText(QString::number(_year));
    }
}

void YearCalendarWidget::_refresh_month_buttons()
{
    for (int idx = 1; idx <= _month_buttons.size(); ++idx) {
        QToolButton *button = _month_buttons.at(idx - 1);
        QString month_name = _locale.standaloneMonthName(idx, QLocale::ShortFormat);
        if (month_name.isEmpty()) {
            month_name = _locale.monthName(idx, QLocale::ShortFormat);
        }
        if (month_name.isEmpty()) {
            month_name = QDate(_year, idx, 1).toString(QStringLiteral("MMM"));
        }

        const int count = _month_counts.value(qMakePair(_year, idx), 0);
        QString label = month_name.left(1).toUpper() + month_name.mid(1);
        if (count > 0) {
            label = QStringLiteral("%1\n%2").arg(label).arg(count);
        }
        if (button->text() != label) {
            button->setText(label);
        }

        button->setChecked(idx == _selected_month);
        const bool has_tasks = count > 0;
        if (button->property("hasTasks").toBool() != has_tasks) {
            button->setProperty("hasTasks", has_tasks);
            button->style()->unpolish(button);
            button->style()->polish(button);
        }

        const QString tooltip_text = _month_tooltips.value(qMakePair(_year, idx)).join(QLatin1Char('\n'));
        if (button->toolTip() != tooltip_text) {
            button->setToolTip(tooltip_text);
        }
    }
}

void YearCalendarWidget::_apply_palette_style()
{
    if (_styling_in_progress) {
        return;
    }

    _styling_in_progress = true;
    const QPalette pal = palette();
    const QColor base_color = pal.base().color();
    const bool is_light = base_color.lightness() >= 128;
    const QColor border_color = is_light ? base_color.darker(135) : base_color.lighter(150);
    const QString border = border_color.name();
    const QString base_bg = base_color.name();
    const QString text = pal.text().color().name();
    const QString highlight = pal.highlight().color().name();
    const QString highlight_text = pal.highlightedText().color().name();

    setStyleSheet(QStringLiteral(
                      "QToolButton { border: 1px solid %1; border-radius: 8px; padding: 6px; background: %2; color: %3; font-weight: 600; }"
                      "QToolButton:hover { background: %2; border: 1px solid %4; }"
                      "QToolButton[hasTasks=\"true\"] { border: 1px solid %4; }"
                      "QToolButton:checked { background: %4; color: %5; }")
                      .arg(border, base_bg, text, highlight, highlight_text));

    for (QToolButton *button : _month_buttons) {
        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }
    _styling_in_progress = false;
}

void YearCalendarWidget::_install_theme_sync()
{
    class ThemeSyncFilter : public QObject {
    public:
        explicit ThemeSyncFilter(YearCalendarWidget *owner) : QObject(owner), owner_(owner) {}

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override
        {
            Q_UNUSED(obj);
            if (event != nullptr
                && (event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::ThemeChange)) {
                owner_->_apply_palette_style();
            }
            return QObject::eventFilter(obj, event);
        }

    private:
        YearCalendarWidget *owner_;
    };

    if (auto *qt_app = qobject_cast<QApplication *>(QCoreApplication::instance())) {
        _theme_sync_filter = new ThemeSyncFilter(this);
        qt_app->installEventFilter(_theme_sync_filter);
    }
}

}  // namespace source::GerenciamentoUI
