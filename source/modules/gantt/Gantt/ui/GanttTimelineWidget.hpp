#pragma once

#include <QDateTime>
#include <QHash>
#include <QList>
#include <QPainter>
#include <QScrollArea>
#include <QWidget>
#include "../domain/models/Schedule.hpp"

namespace source::gantt {

enum class GanttZoom {
    Day,
    Week,
    Month
};

class GanttTimelineCanvas;

class GanttTimelineWidget : public QScrollArea {
    Q_OBJECT

public:
    explicit GanttTimelineWidget(QWidget *parent = nullptr);

    void set_schedule(const Schedule &schedule);
    void set_zoom(GanttZoom zoom);
    void set_reference_date(const QDateTime &ref_date);
    void set_show_baseline(bool show);
    void retranslate_ui();

    GanttZoom current_zoom() const { return zoom_; }
    bool is_showing_baseline() const { return show_baseline_; }

signals:
    void item_clicked(const QString &item_id);
    void item_double_clicked(const QString &item_id);
    void request_delete_item(const QString &item_id);

protected:
    void changeEvent(QEvent *event) override;

private:
    GanttTimelineCanvas *canvas_ = nullptr;
    GanttZoom zoom_ = GanttZoom::Day;
    bool show_baseline_ = true;
};

class GanttTimelineCanvas : public QWidget {
    Q_OBJECT

public:
    explicit GanttTimelineCanvas(QWidget *parent = nullptr);

    void set_data(const Schedule &schedule, GanttZoom zoom, bool show_baseline, const QDateTime &ref_date);
    void retranslate_ui();

signals:
    void item_clicked(const QString &item_id);
    void item_double_clicked(const QString &item_id);
    void request_delete_item(const QString &item_id);

protected:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    Schedule schedule_;
    GanttZoom zoom_ = GanttZoom::Day;
    bool show_baseline_ = true;
    QDateTime reference_date_;

    QDateTime min_date_;
    QDateTime max_date_;
    int row_height_ = 28;
    int header_height_ = 44;
    double pixels_per_hour_ = 4.0;

    struct ItemLayout {
        QString id;
        QRectF bar_rect;
        QRectF baseline_rect;
        bool is_milestone;
        bool critical;
        double progress;
    };
    QHash<QString, ItemLayout> layouts_;

    double date_to_x(const QDateTime &dt) const;
    void update_geometry_params();
    void draw_header(QPainter &p);
    void draw_grid(QPainter &p);
    void draw_bars(QPainter &p);
    void draw_dependencies(QPainter &p);
    void draw_reference_lines(QPainter &p);
};

} // namespace source::gantt
