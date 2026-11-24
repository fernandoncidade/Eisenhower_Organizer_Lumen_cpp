#pragma once

#include <QColor>
#include <QObject>
#include <QPainter>
#include <QRectF>

namespace source::plugin::Audiolivro {

struct CharInfo {
    int page_index = 0;
    QChar character;
    QRectF rect;
};

class PDFTextSelection : public QObject
{
    Q_OBJECT

public:
    explicit PDFTextSelection(QObject *parent = nullptr);
    void clear();
    QList<CharInfo> selected_chars() const { return selected_chars_; }

private:
    QList<CharInfo> selected_chars_;
};

QColor get_selection_color();
void paint_selection(QPainter *painter, const QList<CharInfo> &chars, double zoom, int offset_x, int offset_y);
void paint_caret(QPainter *painter, const QRectF &caret, double zoom, int offset_x, int offset_y);

}  // namespace source::plugin::Audiolivro
