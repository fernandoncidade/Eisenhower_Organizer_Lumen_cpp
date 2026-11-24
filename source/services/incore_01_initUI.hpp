#pragma once

#include <QGroupBox>
#include <QLineEdit>
#include <QColor>
#include <QList>
#include <QPalette>
#include <QProxyStyle>
#include <QTabBar>
#include <QTabWidget>
#include <QTimeEdit>
#include <QDateEdit>

class QContextMenuEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QPaintEvent;
class QListWidget;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::incore_01_initUI {

QString get_text(const QString &text);

class RotatableQuadrantGroupBox : public QGroupBox {
    Q_OBJECT
public:
    explicit RotatableQuadrantGroupBox(QWidget *parent = nullptr);

    bool is_horizontal_collapsed() const;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;
};

class CustomTimeEdit : public QTimeEdit {
public:
    explicit CustomTimeEdit(QWidget *parent = nullptr);

protected:
    void stepBy(int steps) override;
};

class FileDropLineEdit : public QLineEdit {
public:
    explicit FileDropLineEdit(source::EisenhowerMatrixApp *app, QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    source::EisenhowerMatrixApp *app_ = nullptr;
};

class LocalizedDateEdit : public QDateEdit {
public:
    explicit LocalizedDateEdit(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
};

class LocalizedTimeEdit : public CustomTimeEdit {
public:
    explicit LocalizedTimeEdit(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
};

class LightActiveTabBar : public QTabBar {
public:
    explicit LightActiveTabBar(QWidget *parent = nullptr);

    void set_quadrant_style(QProxyStyle *style);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize tabSizeHint(int index) const override;

private:
    QProxyStyle *quadrant_style_ = nullptr;
};

class QuadrantTabStyle : public QProxyStyle {
public:
    using QProxyStyle::QProxyStyle;

    void set_tab_accent_colors(const QList<QColor> &colors);

protected:
    void drawPrimitive(PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget = nullptr) const override;
    void drawControl(ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const override;
    int pixelMetric(PixelMetric metric,
                    const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override;
    QRect subElementRect(SubElement element,
                         const QStyleOption *option,
                         const QWidget *widget = nullptr) const override;
    int styleHint(StyleHint hint,
                  const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr,
                  QStyleHintReturn *returnData = nullptr) const override;

private:
    bool is_light(const QColor &color) const;
    QColor hover_color(const QPalette &palette) const;
    QColor text_color(const QColor &background) const;
    QColor quadrant_accent_color(const QStyleOption *option, const QWidget *widget) const;
    QColor inactive_tab_color(const QStyleOption *option, const QWidget *widget) const;
    QColor hovered_tab_color(const QStyleOption *option, const QWidget *widget) const;
    QColor selected_quadrant_color(const QStyleOption *option, const QWidget *widget) const;
    QPalette selected_tab_palette(const QPalette &base_palette, const QColor &fill_color) const;
    QList<QColor> tab_accent_colors_;
};

class AutoSwitchTabWidget : public QTabWidget {
public:
    explicit AutoSwitchTabWidget(QWidget *parent = nullptr);

    void set_app(source::EisenhowerMatrixApp *app);
    void set_drop_targets(const QList<QListWidget *> &targets);
    void set_tab_accent_colors(const QList<QColor> &colors);
    void refresh_quadrant_style();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QPoint event_pos(QEvent *event) const;
    bool is_completed_list(QListWidget *list) const;
    QListWidget *target_list_for_index(int index, QListWidget *source_list) const;
    void handle_tab_drop(QDropEvent *event, int index);

    source::EisenhowerMatrixApp *app_ = nullptr;
    LightActiveTabBar *tab_bar_ = nullptr;
    QProxyStyle *pane_style_ = nullptr;
    QuadrantTabStyle *quadrant_style_ = nullptr;
    QList<QListWidget *> drop_targets_;
    QList<QColor> tab_accent_colors_;
};

void init_ui(source::EisenhowerMatrixApp *app);

}  // namespace source::InterfaceCore::incore_01_initUI
