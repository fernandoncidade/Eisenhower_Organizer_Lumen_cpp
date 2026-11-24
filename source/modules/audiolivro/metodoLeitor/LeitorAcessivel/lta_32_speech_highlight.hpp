#pragma once

#include <QColor>
#include <QObject>
#include <QString>
#include <QTextCursor>

namespace source::plugin::Audiolivro {

enum class HighlightMode {
    Word,
    Sentence,
    Paragraph
};

struct HighlightStyle {
    QColor background = QColor(255, 225, 90, 160);
    QColor foreground;
    bool bold = false;
    bool underline = false;
};

class SpeechHighlightManager : public QObject
{
    Q_OBJECT

public:
    explicit SpeechHighlightManager(QObject *parent = nullptr);
    void set_text(const QString &text);
    QString text() const { return text_; }

private:
    QString text_;
};

struct SpeechHighlightSettings {
    HighlightMode mode = HighlightMode::Word;
    HighlightStyle style;
    bool auto_scroll = true;
};

}  // namespace source::plugin::Audiolivro
