#pragma once

#include <QPoint>
#include <QRect>
#include <QWidget>

namespace source::plugin::Audiolivro {

class FocusRuler : public QWidget
{
    Q_OBJECT

public:
    enum ResizeEdge {
        NoEdge = 0,
        LeftEdge = 0x1,
        TopEdge = 0x2,
        RightEdge = 0x4,
        BottomEdge = 0x8
    };
    Q_DECLARE_FLAGS(ResizeEdges, ResizeEdge)

    explicit FocusRuler(QWidget *parent = nullptr);

signals:
    void closed();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void enterEvent(QEnterEvent *event) override;

private:
    ResizeEdges get_resize_edge(const QPoint &pos) const;
    void update_cursor(const QPoint &pos);

    bool dragging_ = false;
    bool resizing_ = false;
    QPoint offset_;
    QPoint global_offset_;
    QRect start_geometry_;
    ResizeEdges resize_edge_ = NoEdge;
    int resize_margin_ = 8;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FocusRuler::ResizeEdges)

}  // namespace source::plugin::Audiolivro
