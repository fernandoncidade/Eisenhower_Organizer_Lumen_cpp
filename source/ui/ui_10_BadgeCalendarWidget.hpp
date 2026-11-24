#pragma once

#include <QCalendarWidget>
#include <QDate>
#include <QMap>
#include <QStringList>
#include <QVariantMap>

class QEvent;
class QPainter;
class QRect;
class QTimer;

namespace source::GerenciamentoUI {

class BadgeCalendarWidget : public QCalendarWidget {
    Q_OBJECT

public:
    explicit BadgeCalendarWidget(QWidget *parent = nullptr);

    QMap<QString, int> _task_counts_by_date;
    QMap<int, int> _task_counts_by_week;
    QMap<QPair<int, int>, int> _task_counts_by_month;
    QMap<QString, QStringList> _tasks_by_date;
    QMap<QString, QList<QVariantMap>> _tasks_by_date_data;
    QMap<QString, int> _task_counts;
    QMap<int, int> _week_counts;
    QMap<QPair<int, int>, int> _month_counts;

    void set_task_tooltip_map(const QMap<QString, QStringList> &tasks_by_date);
    void set_tasks_by_date_data(const QMap<QString, QList<QVariantMap>> &tasks_by_date_data);
    void set_task_counts(const QStringList &task_dates_iso);
    void refresh_all_cells();
    int get_week_count(int week_number) const;
    int get_month_count(int year, int month) const;
    QDate _date_at_pos(const QPoint &pos) const;

protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;
    bool event(QEvent *event) override;

private:
    void _draw_badge(QPainter *painter, const QRect &cell_rect, int count) const;
    void _draw_today_highlight(QPainter *painter, const QRect &rect, const QDate &date) const;
    void _schedule_today_refresh();

    QTimer *today_refresh_timer_ = nullptr;
};

}  // namespace source::GerenciamentoUI
