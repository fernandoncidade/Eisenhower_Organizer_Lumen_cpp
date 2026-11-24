#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_30_text_find_actions.hpp"

#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_29_text_findbar.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"

#include <QFont>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextDocument>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::text_find_show()
{
    if (text_find_bar_ != nullptr) {
        text_find_bar_->show_bar();
    }
}

void LeitorAcessivel::text_find_toggle()
{
    if (text_find_bar_ == nullptr) {
        return;
    }
    text_find_bar_->isVisible() ? text_find_bar_->hide_bar() : text_find_bar_->show_bar();
}

void LeitorAcessivel::text_find_clear()
{
    text_hits_.clear();
    current_text_hit_ = -1;
    text_find_apply_highlights();
    if (text_find_bar_ != nullptr) {
        text_find_bar_->set_result_text(QString());
    }
}

void LeitorAcessivel::text_find_set_highlight_all(bool enable)
{
    highlight_all_text_hits_ = enable;
    text_find_apply_highlights();
}

void LeitorAcessivel::text_find_apply(const QString &query, bool match_case, bool whole_words)
{
    text_hits_.clear();
    current_text_hit_ = -1;
    if (texto_area_ == nullptr || query.trimmed().isEmpty()) {
        text_find_apply_highlights();
        return;
    }

    const QString text = texto_area_->toPlainText();
    const Qt::CaseSensitivity cs = match_case ? Qt::CaseSensitive : Qt::CaseInsensitive;
    int pos = 0;
    while ((pos = text.indexOf(query, pos, cs)) >= 0) {
        if (whole_words) {
            const bool left_ok = pos == 0 || !text.at(pos - 1).isLetterOrNumber();
            const int right = pos + query.size();
            const bool right_ok = right >= text.size() || !text.at(right).isLetterOrNumber();
            if (!left_ok || !right_ok) {
                pos += qMax(1, query.size());
                continue;
            }
        }

        QTextCursor cursor(texto_area_->document());
        cursor.setPosition(pos);
        cursor.setPosition(pos + query.size(), QTextCursor::KeepAnchor);
        text_hits_.append(cursor);
        pos += qMax(1, query.size());
    }

    if (!text_hits_.isEmpty()) {
        current_text_hit_ = 0;
        text_find_goto_hit(0);
    }
    text_find_apply_highlights();

    if (text_find_bar_ != nullptr) {
        text_find_bar_->set_result_text(text_hits_.isEmpty() ? tr("0 resultados") : tr("%1 resultados").arg(text_hits_.size()));
    }
}

void LeitorAcessivel::text_find_next()
{
    if (text_hits_.isEmpty()) {
        return;
    }
    text_find_goto_hit(current_text_hit_ + 1);
}

void LeitorAcessivel::text_find_prev()
{
    if (text_hits_.isEmpty()) {
        return;
    }
    text_find_goto_hit(current_text_hit_ - 1);
}

void LeitorAcessivel::text_find_goto_hit(int index)
{
    if (texto_area_ == nullptr || text_hits_.isEmpty()) {
        return;
    }
    current_text_hit_ = (index % text_hits_.size() + text_hits_.size()) % text_hits_.size();
    texto_area_->setTextCursor(text_hits_.at(current_text_hit_));
    texto_area_->ensureCursorVisible();
    text_find_apply_highlights();
}

void LeitorAcessivel::text_find_apply_highlights()
{
    apply_text_extra_selections();
}

void LeitorAcessivel::apply_text_extra_selections()
{
    if (texto_area_ == nullptr) {
        return;
    }

    QList<QTextEdit::ExtraSelection> selections;
    QTextCharFormat all_format;
    all_format.setBackground(QColor(255, 240, 120, 120));
    QTextCharFormat current_format;
    current_format.setBackground(QColor(255, 175, 80, 180));

    for (int i = 0; i < text_hits_.size(); ++i) {
        if (!highlight_all_text_hits_ && i != current_text_hit_) {
            continue;
        }
        QTextEdit::ExtraSelection selection;
        selection.cursor = text_hits_.at(i);
        selection.format = i == current_text_hit_ ? current_format : all_format;
        selections.append(selection);
    }

    if (current_speech_highlight_start_ >= 0 && current_speech_highlight_length_ > 0) {
        const int text_size = texto_area_->toPlainText().size();
        const int start = qBound(0, current_speech_highlight_start_, text_size);
        const int end = qBound(start, current_speech_highlight_start_ + current_speech_highlight_length_, text_size);
        if (end > start) {
            QTextCursor cursor(texto_area_->document());
            cursor.setPosition(start);
            cursor.setPosition(end, QTextCursor::KeepAnchor);

            QTextEdit::ExtraSelection selection;
            selection.cursor = cursor;
            QTextCharFormat speech_format;
            speech_format.setBackground(QColor(255, 215, 90, 170));
            speech_format.setFontWeight(QFont::Bold);
            selection.format = speech_format;
            selections.append(selection);
        }
    }

    texto_area_->setExtraSelections(selections);
}

}  // namespace source::plugin::Audiolivro
