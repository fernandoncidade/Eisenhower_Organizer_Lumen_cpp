#pragma once

#include <QColor>
#include <QList>
#include <QObject>
#include <QRectF>
#include <QString>

namespace source::plugin::Audiolivro {

enum class PDFHighlightMode {
    Word,
    Sentence,
    Paragraph
};

struct PDFHighlightStyle {
    QColor background = QColor(255, 210, 80, 140);
    QColor border = QColor(255, 170, 40);
};

struct PDFWordRect {
    int page_index = 0;
    QString word;
    QRectF rect;
    int text_offset = 0;
};

class PDFSpeechHighlightManager : public QObject
{
    Q_OBJECT

public:
    explicit PDFSpeechHighlightManager(QObject *parent = nullptr);
    void set_words(const QList<PDFWordRect> &words);
    QList<PDFWordRect> words() const { return words_; }

private:
    QList<PDFWordRect> words_;
};

QString extract_word_positions_from_pdf(const QString &pdf_path, int page_index, QList<PDFWordRect> *words = nullptr);

}  // namespace source::plugin::Audiolivro
