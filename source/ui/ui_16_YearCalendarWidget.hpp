#pragma once

#include <QDate>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QStringList>
#include <QVariantMap>
#include <QWidget>

class QLabel;
class QToolButton;
class QEvent;
class QObject;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI {

class YearCalendarWidget : public QWidget {
    Q_OBJECT

public:
    explicit YearCalendarWidget(QWidget *parent = nullptr);

    source::EisenhowerMatrixApp *app = nullptr;
    QObject *owner = nullptr;

    void setLocale(const QLocale &locale);
    void setYear(int year);
    int year() const;
    void setSelectedDate(const QDate &qdate);
    QDate selectedDate() const;
    void set_month_counts(const QMap<QPair<int, int>, int> &month_counts);
    void set_month_tooltip_map(const QMap<QPair<int, int>, QStringList> &month_tooltips);
    void set_month_data(const QMap<QPair<int, int>, int> &month_counts,
                        const QMap<QPair<int, int>, QStringList> &month_tooltips);
    const QMap<QPair<int, int>, int> &month_counts() const;
    const QMap<QPair<int, int>, QStringList> &month_tooltips() const;

signals:
    void monthSelected(const QDate &date);
    void monthDoubleClicked(const QDate &date);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    QList<QVariantMap> _month_tasks(int month) const;
    QString _format_task_label(const QVariantMap &task, bool show_priority, bool show_domain) const;
    void _edit_task_from_data(const QVariantMap &task);
    bool _remove_task_from_data(const QVariantMap &task, bool confirm = true);
    int _remove_all_tasks_for_month(int month);
    void _show_month_context_menu(int month, QWidget *watched, QEvent *event);
    void _on_month_clicked(int month);
    void _refresh_header();
    void _refresh_month_buttons();
    void _apply_palette_style();
    void _install_theme_sync();

    QLocale _locale;
    int _year = 0;
    int _selected_month = 1;
    QMap<QPair<int, int>, int> _month_counts;
    QMap<QPair<int, int>, QStringList> _month_tooltips;
    QObject *_theme_sync_filter = nullptr;
    bool _styling_in_progress = false;

    QLabel *year_label = nullptr;
    QToolButton *prev_year_button = nullptr;
    QToolButton *next_year_button = nullptr;
    QList<QToolButton *> _month_buttons;
};

}  // namespace source::GerenciamentoUI
