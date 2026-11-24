#pragma once

#include <QObject>
#include <QPoint>

namespace source::plugin::Audiolivro {

class PDFView;

class PDFMouseHandler : public QObject
{
    Q_OBJECT

public:
    explicit PDFMouseHandler(PDFView *view, QObject *parent = nullptr);
    bool copy_selection();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    PDFView *view_ = nullptr;
    bool hand_dragging_ = false;
    QPoint last_pos_;
};

}  // namespace source::plugin::Audiolivro
