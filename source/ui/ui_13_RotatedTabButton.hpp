#pragma once

#include <QStringList>
#include <QWidget>
#include <functional>

class QMouseEvent;
class QPaintEvent;

namespace source::GerenciamentoUI {

class RotatedTabButton : public QWidget {
    Q_OBJECT

public:
    enum class RotationDirection {
        None,
        Clockwise90,
        CounterClockwise90,
    };

    explicit RotatedTabButton(const QString &text, QWidget *parent = nullptr);

    void setText(const QString &text);
    QString text() const;
    void setSizeReferenceTexts(const QStringList &texts);
    void setClickHandler(std::function<void()> handler);
    void setRotationDirection(RotationDirection direction);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void _recompute_size();

    QString _text;
    QStringList _size_reference_texts;
    std::function<void()> _click_handler;
    RotationDirection _rotation_direction = RotationDirection::Clockwise90;
};

}  // namespace source::GerenciamentoUI
