#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_33_speech_highlight_controls.hpp"

#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_38_FileDropPDFView.hpp"

#include <QComboBox>
#include <QMediaPlayer>
#include <QPushButton>
#include <QTextCharFormat>

namespace source::plugin::Audiolivro {
namespace {

constexpr qint64 kHighlightLeadMs = 0;
constexpr qint64 kSeekBackToleranceMs = 250;

int first_non_space(const QString &text, int pos, int fallback)
{
    while (pos < text.size() && text.at(pos).isSpace()) {
        ++pos;
    }
    return qBound(0, pos, text.size() > 0 ? text.size() : fallback);
}

int last_non_space_exclusive(const QString &text, int pos, int fallback)
{
    pos = qBound(0, pos, text.size());
    while (pos > 0 && text.at(pos - 1).isSpace()) {
        --pos;
    }
    return pos > 0 ? pos : fallback;
}

bool is_horizontal_space(QChar ch)
{
    return ch == QLatin1Char(' ') || ch == QLatin1Char('\t') || ch == QLatin1Char('\r');
}

bool is_sentence_terminal(QChar ch)
{
    return ch == QLatin1Char('.') || ch == QLatin1Char('!') || ch == QLatin1Char('?')
           || ch == QLatin1Char(';') || ch == QLatin1Char(':') || ch.unicode() == 0x2026;
}

bool is_sentence_closer(QChar ch)
{
    return ch == QLatin1Char('"') || ch == QLatin1Char('\'') || ch == QLatin1Char(')')
           || ch == QLatin1Char(']') || ch == QLatin1Char('}') || ch.unicode() == 0x201D
           || ch.unicode() == 0x2019 || ch.unicode() == 0x00BB;
}

bool previous_non_space_is_sentence_end(const QString &text, int pos)
{
    --pos;
    while (pos >= 0 && is_horizontal_space(text.at(pos))) {
        --pos;
    }
    while (pos >= 0 && is_sentence_closer(text.at(pos))) {
        --pos;
    }
    return pos >= 0 && is_sentence_terminal(text.at(pos));
}

bool next_starts_paragraph(const QString &text, int pos)
{
    while (pos < text.size() && is_horizontal_space(text.at(pos))) {
        ++pos;
    }
    if (pos >= text.size() || text.at(pos) == QLatin1Char('\n')) {
        return false;
    }

    const QChar ch = text.at(pos);
    return ch.isUpper() || ch.isDigit() || ch == QLatin1Char('-') || ch == QLatin1Char('*');
}

bool is_paragraph_separator_at(const QString &text, int newline_index)
{
    if (newline_index < 0 || newline_index >= text.size() || text.at(newline_index) != QLatin1Char('\n')) {
        return false;
    }

    int before = newline_index - 1;
    while (before >= 0 && is_horizontal_space(text.at(before))) {
        --before;
    }

    int after = newline_index + 1;
    while (after < text.size() && is_horizontal_space(text.at(after))) {
        ++after;
    }

    if ((before >= 0 && text.at(before) == QLatin1Char('\n'))
        || (after < text.size() && text.at(after) == QLatin1Char('\n'))) {
        return true;
    }

    return previous_non_space_is_sentence_end(text, newline_index) && next_starts_paragraph(text, newline_index + 1);
}

int paragraph_start_before(const QString &text, int offset)
{
    if (text.isEmpty()) {
        return 0;
    }

    for (int i = qBound(0, offset - 1, text.size() - 1); i >= 0; --i) {
        if (is_paragraph_separator_at(text, i)) {
            int start = i + 1;
            while (start < text.size() && text.at(start).isSpace()) {
                ++start;
            }
            return start;
        }
    }
    return 0;
}

int paragraph_end_after(const QString &text, int offset)
{
    for (int i = qBound(0, offset, text.size()); i < text.size(); ++i) {
        if (is_paragraph_separator_at(text, i)) {
            int end = i;
            while (end > 0 && text.at(end - 1).isSpace()) {
                --end;
            }
            return end;
        }
    }
    return text.size();
}

QPair<int, int> word_range(const QString &text, int offset, int length)
{
    int start = qBound(0, offset, text.size());
    int end = qBound(start, offset + qMax(1, length), text.size());

    while (start > 0 && !text.at(start - 1).isSpace()) {
        --start;
    }
    while (end < text.size() && !text.at(end).isSpace()) {
        ++end;
    }

    return {start, qMax(0, end - start)};
}

QPair<int, int> sentence_range(const QString &text, int offset, int length)
{
    int start = qBound(0, offset, text.size());
    int end = qBound(start, offset + qMax(1, length), text.size());

    while (start > 0) {
        const QChar ch = text.at(start - 1);
        if (is_sentence_terminal(ch)) {
            break;
        }
        if (ch == QLatin1Char('\n') && is_paragraph_separator_at(text, start - 1)) {
            break;
        }
        --start;
    }

    while (end < text.size()) {
        const QChar ch = text.at(end);
        ++end;
        if (is_sentence_terminal(ch)) {
            while (end < text.size() && is_sentence_closer(text.at(end))) {
                ++end;
            }
            break;
        }
        if (ch == QLatin1Char('\n') && is_paragraph_separator_at(text, end - 1)) {
            break;
        }
    }

    start = first_non_space(text, start, offset);
    end = last_non_space_exclusive(text, end, start + qMax(1, length));
    return {start, qMax(0, end - start)};
}

QPair<int, int> paragraph_range(const QString &text, int offset, int length)
{
    int start = qBound(0, offset, text.size());
    int end = qBound(start, offset + qMax(1, length), text.size());

    start = paragraph_start_before(text, start);
    end = paragraph_end_after(text, end);

    start = first_non_space(text, start, offset);
    end = last_non_space_exclusive(text, end, start + qMax(1, length));
    return {start, qMax(0, end - start)};
}

QPair<int, int> range_for_mode(const QString &text, int offset, int length, const QString &mode)
{
    if (mode == QStringLiteral("paragraph")) {
        return paragraph_range(text, offset, length);
    }
    if (mode == QStringLiteral("sentence")) {
        return sentence_range(text, offset, length);
    }
    return word_range(text, offset, length);
}

qint64 timestamp_start_ms(const QVariantList &timestamps, int index)
{
    if (index < 0 || index >= timestamps.size()) {
        return 0;
    }
    return timestamps.at(index).toMap().value(QStringLiteral("offset_ms")).toLongLong();
}

qint64 timestamp_end_ms(const QVariantList &timestamps, int index)
{
    const qint64 start = timestamp_start_ms(timestamps, index);
    if (index + 1 < timestamps.size()) {
        return qMax(start + 1, timestamp_start_ms(timestamps, index + 1));
    }

    const qint64 duration = timestamps.at(index).toMap().value(QStringLiteral("duration_ms")).toLongLong();
    return start + qMax<qint64>(1, duration);
}

int timestamp_index_for_position(const QVariantList &timestamps, qint64 position, int current_index)
{
    if (timestamps.isEmpty()) {
        return -1;
    }

    const qint64 effective_position = qMax<qint64>(0, position + kHighlightLeadMs);
    bool moved_back = false;
    int search_start = 0;

    if (current_index >= 0 && current_index < timestamps.size()) {
        const qint64 current_start = timestamp_start_ms(timestamps, current_index);
        const qint64 current_end = timestamp_end_ms(timestamps, current_index);
        moved_back = position + kSeekBackToleranceMs < current_start;

        if (!moved_back && effective_position >= current_start && effective_position < current_end) {
            return current_index;
        }
        if (!moved_back) {
            search_start = current_index;
        }
    }

    for (int i = search_start; i < timestamps.size(); ++i) {
        const qint64 start = timestamp_start_ms(timestamps, i);
        const qint64 end = timestamp_end_ms(timestamps, i);
        if (effective_position >= start && effective_position < end) {
            if (!moved_back && current_index >= 0 && i < current_index) {
                return current_index;
            }
            return i;
        }
        if (start > effective_position) {
            break;
        }
    }

    return -1;
}

int resolve_word_offset(const QString &full_text, const QString &word, int expected_offset, int previous_word_offset)
{
    if (full_text.isEmpty()) {
        return 0;
    }

    Q_UNUSED(word)
    const int clamped = qBound(0, expected_offset, full_text.size());
    if (previous_word_offset >= 0 && clamped < previous_word_offset) {
        return -1;
    }
    return clamped;
}

}  // namespace

void LeitorAcessivel::init_speech_highlight()
{
    speech_timer_.setInterval(30);
    auto update_speech_highlight = [this] {
        const bool highlight_enabled = current_reading_is_pdf_ ? pdf_speech_enabled_ : speech_enabled_;
        if (!highlight_enabled || texto_area_ == nullptr || player_ == nullptr || current_timestamps_.isEmpty()) {
            return;
        }

        const qint64 position = player_->position();
        const QString mode = current_reading_is_pdf_ ? pdf_speech_mode_ : speech_mode_;
        const bool force_refresh = speech_highlight_force_refresh_ || mode != last_speech_render_mode_;
        const int previous_timestamp_index = current_timestamp_index_;
        const int match_index = timestamp_index_for_position(current_timestamps_, position, current_timestamp_index_);

        if (match_index < 0 || (match_index == current_timestamp_index_ && !force_refresh)) {
            return;
        }

        const QVariantMap item = current_timestamps_.at(match_index).toMap();
        const QString word = item.value(QStringLiteral("text")).toString();
        const QString full_text = current_reading_is_pdf_ && !current_reading_text_.isEmpty()
                                      ? current_reading_text_
                                      : texto_area_->toPlainText();
        const bool same_timestamp_refresh = force_refresh && match_index == previous_timestamp_index;
        const int expected_offset = item.value(QStringLiteral("text_offset")).toInt();
        const int offset = current_reading_is_pdf_
                               ? qBound(0, expected_offset, full_text.size())
                               : resolve_word_offset(full_text,
                                                     word,
                                                     expected_offset,
                                                     same_timestamp_refresh ? -1 : last_speech_word_offset_);
        if (offset < 0) {
            return;
        }
        if (!same_timestamp_refresh && last_speech_word_offset_ >= 0 && offset < last_speech_word_offset_) {
            return;
        }

        const int source_length = current_reading_is_pdf_
                                      ? item.value(QStringLiteral("source_length"), word.size()).toInt()
                                      : word.size();
        const QPair<int, int> range = range_for_mode(full_text, offset, source_length, mode);
        if (!same_timestamp_refresh && mode == QStringLiteral("word") && last_speech_highlight_start_ >= 0
            && range.first < last_speech_highlight_start_) {
            return;
        }

        current_timestamp_index_ = match_index;
        last_speech_word_offset_ = offset;
        last_speech_highlight_start_ = range.first;
        last_speech_render_mode_ = mode;
        speech_highlight_force_refresh_ = false;

        QTextCursor cursor(texto_area_->document());
        if (current_reading_is_pdf_) {
            current_speech_highlight_start_ = -1;
            current_speech_highlight_length_ = 0;
            apply_text_extra_selections();
        } else {
            current_speech_highlight_start_ = range.first;
            current_speech_highlight_length_ = range.second;

            const int text_size = texto_area_->toPlainText().size();
            cursor.setPosition(qBound(0, range.first, text_size));
            cursor.setPosition(qBound(0, range.first + range.second, text_size), QTextCursor::KeepAnchor);
            apply_text_extra_selections();
        }

        if (current_reading_is_pdf_ && pdf_view_ != nullptr) {
            pdf_view_->set_speech_highlight_anchor(range.first, range.second, offset, word);
        } else if (pdf_view_ != nullptr) {
            pdf_view_->clear_speech_highlight();
        }

        if (!current_reading_is_pdf_ && speech_auto_scroll_) {
            texto_area_->setTextCursor(cursor);
            texto_area_->ensureCursorVisible();
        }
    };
    connect(&speech_timer_, &QTimer::timeout, this, update_speech_highlight);
    if (player_ != nullptr) {
        connect(player_, &QMediaPlayer::positionChanged, this, [update_speech_highlight](qint64) {
            update_speech_highlight();
        });
    }
}

void LeitorAcessivel::setup_speech_highlight_for_reading(const QString &text)
{
    current_reading_text_ = text;
    current_timestamp_index_ = -1;
    last_speech_word_offset_ = -1;
    last_speech_highlight_start_ = -1;
    current_speech_highlight_start_ = -1;
    current_speech_highlight_length_ = 0;
    speech_highlight_force_refresh_ = false;
    last_speech_render_mode_.clear();
    current_timestamps_.clear();
    if (texto_area_ != nullptr) {
        apply_text_extra_selections();
    }
}

void LeitorAcessivel::on_timestamps_received(const QVariantList &timestamps)
{
    current_timestamps_ = timestamps;
    current_timestamp_index_ = -1;
    last_speech_word_offset_ = -1;
    last_speech_highlight_start_ = -1;
    current_speech_highlight_start_ = -1;
    current_speech_highlight_length_ = 0;
    speech_highlight_force_refresh_ = false;
    last_speech_render_mode_.clear();
}

void LeitorAcessivel::start_speech_highlight()
{
    if (speech_enabled_) {
        speech_timer_.start();
    }
}

void LeitorAcessivel::pause_speech_highlight()
{
    speech_timer_.stop();
}

void LeitorAcessivel::resume_speech_highlight()
{
    start_speech_highlight();
}

void LeitorAcessivel::stop_speech_highlight()
{
    speech_timer_.stop();
    current_timestamp_index_ = -1;
    last_speech_word_offset_ = -1;
    last_speech_highlight_start_ = -1;
    current_speech_highlight_start_ = -1;
    current_speech_highlight_length_ = 0;
    speech_highlight_force_refresh_ = false;
    last_speech_render_mode_.clear();
    if (texto_area_ != nullptr) {
        apply_text_extra_selections();
    }
    if (pdf_view_ != nullptr) {
        pdf_view_->clear_speech_highlight();
    }
}

void LeitorAcessivel::toggle_speech_highlight()
{
    speech_enabled_ = !speech_enabled_;
    update_leitura_assistida_button();
}

void LeitorAcessivel::set_speech_highlight_mode(const QString &mode)
{
    speech_mode_ = mode;
}

void LeitorAcessivel::set_speech_highlight_style(const QColor &background, const QColor &foreground, bool bold, bool underline)
{
    Q_UNUSED(background)
    Q_UNUSED(foreground)
    Q_UNUSED(bold)
    Q_UNUSED(underline)
}

void LeitorAcessivel::set_speech_highlight_auto_scroll(bool enabled)
{
    speech_auto_scroll_ = enabled;
}

bool LeitorAcessivel::speech_highlight_enabled() const
{
    return speech_enabled_;
}

void LeitorAcessivel::on_toggle_leitura_assistida()
{
    speech_enabled_ = btn_leitura_assistida_ == nullptr || btn_leitura_assistida_->isChecked();
    pdf_speech_enabled_ = speech_enabled_;
    update_leitura_assistida_button();

    if (speech_enabled_) {
        if (player_ != nullptr && player_->playbackState() == QMediaPlayer::PlaybackState::PlayingState) {
            speech_timer_.start();
        }
        return;
    }

    speech_timer_.stop();
    current_timestamp_index_ = -1;
    last_speech_word_offset_ = -1;
    last_speech_highlight_start_ = -1;
    current_speech_highlight_start_ = -1;
    current_speech_highlight_length_ = 0;
    speech_highlight_force_refresh_ = false;
    last_speech_render_mode_.clear();
    if (texto_area_ != nullptr) {
        apply_text_extra_selections();
    }
    if (pdf_view_ != nullptr) {
        pdf_view_->clear_speech_highlight();
    }
}

void LeitorAcessivel::on_modo_leitura_changed(int index)
{
    if (index == 1) {
        set_speech_highlight_mode(QStringLiteral("sentence"));
        set_pdf_speech_highlight_mode(QStringLiteral("sentence"));
    } else if (index == 2) {
        set_speech_highlight_mode(QStringLiteral("paragraph"));
        set_pdf_speech_highlight_mode(QStringLiteral("paragraph"));
    } else {
        set_speech_highlight_mode(QStringLiteral("word"));
        set_pdf_speech_highlight_mode(QStringLiteral("word"));
    }
    speech_highlight_force_refresh_ = true;
}

void LeitorAcessivel::update_leitura_assistida_button()
{
    if (btn_leitura_assistida_ != nullptr) {
        btn_leitura_assistida_->setChecked(speech_enabled_);
        btn_leitura_assistida_->setText(speech_enabled_ ? tr("🔦 Desativar Leitura Assistida")
                                                        : tr("🔦 Ativar Leitura Assistida"));
    }
}

void LeitorAcessivel::init_leitura_assistida_state()
{
    speech_enabled_ = false;
    pdf_speech_enabled_ = false;
    update_leitura_assistida_button();
}

void LeitorAcessivel::register_generated_audio_timestamps(const QString &audio_path, const QVariantList &timestamps)
{
    if (!audio_path.trimmed().isEmpty()) {
        generated_timestamps_.insert(audio_path, timestamps);
    }
}

void LeitorAcessivel::activate_timestamps_for_audio(const QString &audio_path)
{
    current_timestamps_ = generated_timestamps_.take(audio_path);
    current_timestamp_index_ = -1;
}

void LeitorAcessivel::notify_chunk_changed(int chunk_index, int base_offset_ms)
{
    Q_UNUSED(base_offset_ms)
    current_chunk_index_ = chunk_index;
    current_timestamp_index_ = -1;
}

void LeitorAcessivel::notify_pdf_chunk_changed(int chunk_index, int base_offset_ms)
{
    Q_UNUSED(chunk_index)
    Q_UNUSED(base_offset_ms)
}

}  // namespace source::plugin::Audiolivro
