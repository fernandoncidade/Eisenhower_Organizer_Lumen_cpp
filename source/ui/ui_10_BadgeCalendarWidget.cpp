#include "source/ui/ui_10_BadgeCalendarWidget.hpp"

#include <QAbstractItemView>
#include <QDateTime>
#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOptionFocusRect>
#include <QTimer>
#include <QToolTip>
#include <limits>

#include "source/utils/LogManager.hpp"

namespace source::GerenciamentoUI {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

}  // namespace

BadgeCalendarWidget::BadgeCalendarWidget(QWidget *parent) : QCalendarWidget(parent)
{
    try {
        setMouseTracking(true);
        today_refresh_timer_ = new QTimer(this);
        today_refresh_timer_->setSingleShot(true);
        QObject::connect(today_refresh_timer_, &QTimer::timeout, this, [this]() {
            refresh_all_cells();
            _schedule_today_refresh();
        });
        _schedule_today_refresh();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao inicializar BadgeCalendarWidget."));
    }
}

void BadgeCalendarWidget::set_task_tooltip_map(const QMap<QString, QStringList> &tasks_by_date)
{
    _tasks_by_date = tasks_by_date;
}

void BadgeCalendarWidget::set_tasks_by_date_data(const QMap<QString, QList<QVariantMap>> &tasks_by_date_data)
{
    _tasks_by_date_data = tasks_by_date_data;
}

void BadgeCalendarWidget::set_task_counts(const QStringList &task_dates_iso)
{
    try {
        _task_counts_by_date.clear();
        _task_counts_by_week.clear();
        _task_counts_by_month.clear();

        for (const QString &date_str : task_dates_iso) {
            const QDate date = QDate::fromString(date_str, Qt::ISODate);
            if (!date.isValid()) {
                continue;
            }

            _task_counts_by_date[date_str] = _task_counts_by_date.value(date_str) + 1;
            _task_counts_by_week[date.weekNumber()] = _task_counts_by_week.value(date.weekNumber()) + 1;
            const QPair<int, int> month_key(date.year(), date.month());
            _task_counts_by_month[month_key] = _task_counts_by_month.value(month_key) + 1;
        }

        _task_counts = _task_counts_by_date;
        _week_counts = _task_counts_by_week;
        _month_counts = _task_counts_by_month;
        updateCells();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao definir contadores de tarefas."));
    }
}

void BadgeCalendarWidget::refresh_all_cells()
{
    updateCells();
}

void BadgeCalendarWidget::paintCell(QPainter *painter, const QRect &rect, QDate date) const
{
    try {
        QCalendarWidget::paintCell(painter, rect, date);
        _draw_today_highlight(painter, rect, date);
        const QString date_str = date.toString(Qt::ISODate);
        const int count = _task_counts_by_date.value(date_str, 0);
        if (count > 0) {
            _draw_badge(painter, rect, count);
        }
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao pintar célula do calendário."));
    }
}

void BadgeCalendarWidget::_draw_badge(QPainter *painter, const QRect &cell_rect, int count) const
{
    try {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        const QString text = QString::number(count);
        QFont font = painter->font();
        font.setPointSize(qMax(7, font.pointSize() - 2));
        font.setBold(true);
        painter->setFont(font);

        const QFontMetrics fm(font);
        const int text_width = fm.horizontalAdvance(text);
        const int badge_size = qMin(qMax(16, text_width + 8), cell_rect.width() / 2);
        const QRect badge_rect(cell_rect.right() - badge_size - 2, cell_rect.top() + 2, badge_size, badge_size);

        QColor badge_color = palette().highlight().color();
        badge_color.setAlpha(200);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(badge_color));
        painter->drawEllipse(badge_rect);

        painter->setPen(QPen(palette().highlightedText().color()));
        painter->drawText(badge_rect, Qt::AlignCenter, text);
        painter->restore();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao desenhar badge."));
    }
}

void BadgeCalendarWidget::_draw_today_highlight(QPainter *painter, const QRect &rect, const QDate &date) const
{
    try {
        if (!date.isValid() || date != QDate::currentDate()) {
            return;
        }

        QRect focus_rect = rect.adjusted(2, 2, -2, -2);
        if (focus_rect.width() <= 4 || focus_rect.height() <= 4) {
            focus_rect = rect.adjusted(1, 1, -1, -1);
        }

        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.rect = focus_rect;
        option.state = QStyle::State_Item | QStyle::State_KeyboardFocusChange;
        option.backgroundColor = palette().color(QPalette::Base);

        painter->save();
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter, this);
        painter->restore();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao desenhar destaque do dia atual."));
    }
}

void BadgeCalendarWidget::_schedule_today_refresh()
{
    try {
        if (today_refresh_timer_ == nullptr) {
            return;
        }

        const QDateTime now = QDateTime::currentDateTime();
        const QDateTime next_midnight(now.date().addDays(1), QTime(0, 0, 1));
        qint64 delay_ms = now.msecsTo(next_midnight);
        if (delay_ms <= 0) {
            delay_ms = 60000;
        }

        today_refresh_timer_->start(static_cast<int>(qMin<qint64>(delay_ms, std::numeric_limits<int>::max())));
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao agendar atualização do dia atual no calendário."));
    }
}

QDate BadgeCalendarWidget::_date_at_pos(const QPoint &pos) const
{
    try {
        if (QAbstractItemView *view = findChild<QAbstractItemView *>()) {
            const QPoint view_pos = view->mapFrom(const_cast<BadgeCalendarWidget *>(this), pos);
            const QModelIndex idx = view->indexAt(view_pos);
            if (idx.isValid()) {
                const int roles[] = {
                    int(Qt::UserRole),
                    int(Qt::UserRole) + 1,
                    int(Qt::UserRole) + 2,
                    int(Qt::DisplayRole),
                };
                for (int role : roles) {
                    const QVariant value = idx.data(role);
                    if (value.canConvert<QDate>()) {
                        const QDate qd = value.toDate();
                        if (qd.isValid()) {
                            return qd;
                        }
                    }
                }

                bool ok = false;
                const int day = idx.data(Qt::DisplayRole).toInt(&ok);
                if (ok && day > 0) {
                    const QDate qd(yearShown(), monthShown(), day);
                    if (qd.isValid()) {
                        return qd;
                    }
                }
            }
        }
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao obter data pelo mouse no calendário."));
    }

    return QDate();
}

bool BadgeCalendarWidget::event(QEvent *event)
{
    try {
        if (event->type() == QEvent::ToolTip) {
            auto *help_event = static_cast<QHelpEvent *>(event);
            const QDate date = _date_at_pos(help_event->pos());
            if (date.isValid()) {
                const QStringList lines = _tasks_by_date.value(date.toString(Qt::ISODate));
                if (!lines.isEmpty()) {
                    QToolTip::showText(help_event->globalPos(), lines.join(QLatin1Char('\n')), this);
                    return true;
                }
            }

            QToolTip::hideText();
            event->ignore();
            return true;
        }
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao exibir tooltip no calendário."));
    }

    return QCalendarWidget::event(event);
}

int BadgeCalendarWidget::get_week_count(int week_number) const
{
    return _task_counts_by_week.value(week_number, 0);
}

int BadgeCalendarWidget::get_month_count(int year, int month) const
{
    return _task_counts_by_month.value(qMakePair(year, month), 0);
}

}  // namespace source::GerenciamentoUI
