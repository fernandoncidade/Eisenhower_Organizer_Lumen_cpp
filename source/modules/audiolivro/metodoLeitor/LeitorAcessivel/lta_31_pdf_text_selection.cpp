#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_31_pdf_text_selection.hpp"

namespace source::plugin::Audiolivro {

PDFTextSelection::PDFTextSelection(QObject *parent)
    : QObject(parent)
{
}

void PDFTextSelection::clear()
{
    selected_chars_.clear();
}

QColor get_selection_color()
{
    return QColor(80, 150, 255, 90);
}

void paint_selection(QPainter *painter, const QList<CharInfo> &chars, double zoom, int offset_x, int offset_y)
{
    if (painter == nullptr) {
        return;
    }
    painter->save();
    painter->setBrush(get_selection_color());
    painter->setPen(Qt::NoPen);
    for (const CharInfo &info : chars) {
        QRectF rect = info.rect;
        rect.setX(rect.x() * zoom + offset_x);
        rect.setY(rect.y() * zoom + offset_y);
        rect.setWidth(rect.width() * zoom);
        rect.setHeight(rect.height() * zoom);
        painter->drawRect(rect);
    }
    painter->restore();
}

void paint_caret(QPainter *painter, const QRectF &caret, double zoom, int offset_x, int offset_y)
{
    if (painter == nullptr || caret.isNull()) {
        return;
    }
    painter->save();
    painter->setPen(QPen(QColor(40, 120, 255), 1));
    painter->drawLine(QPointF(caret.x() * zoom + offset_x, caret.y() * zoom + offset_y),
                      QPointF(caret.x() * zoom + offset_x, (caret.y() + caret.height()) * zoom + offset_y));
    painter->restore();
}

}  // namespace source::plugin::Audiolivro
