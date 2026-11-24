#pragma once

#include <QWidget>

class QVBoxLayout;
class QEvent;
class QPaintEvent;

namespace source::GerenciamentoUI {

class NativeLineEditFrame : public QWidget {
    Q_OBJECT

public:
    explicit NativeLineEditFrame(QWidget *child, QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    int _frame_width() const;
    void _update_margins_from_style();

    QWidget *_child = nullptr;
    QVBoxLayout *_layout = nullptr;
};

}  // namespace source::GerenciamentoUI
