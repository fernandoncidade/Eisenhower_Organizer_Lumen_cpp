#pragma once

#include <QImage>
#include <QList>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPdfDocument>
#include <QPdfSearchModel>
#include <QPdfView>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QResizeEvent>
#include <QString>
#include <QVector>

#include <memory>

namespace source::plugin::Audiolivro {

struct PageHit {
    int page_index = -1;
    QRectF rect;
    QString text;
    QList<QPolygonF> bounds;
};

class PDFView : public QPdfView
{
    Q_OBJECT

public:
    struct SpeechHighlight {
        int page_index = -1;
        QList<QPolygonF> bounds;
    };
    struct PageTextRange {
        int page_index = -1;
        int start = 0;
        int length = 0;
    };

    explicit PDFView(QWidget *parent = nullptr);
    ~PDFView() override;

    bool load_pdf(const QString &path);
    void clear_pdf();
    QString pdf_path() const { return pdf_path_; }
    int page_count() const;
    QSizeF page_point_size(int page_index) const;
    QString meta_data(QPdfDocument::MetaDataField field) const;
    int current_page() const;
    double zoom_factor() const;
    double effective_zoom_factor() const;
    void set_zoom_factor(double zoom);
    void zoom_in();
    void zoom_out();
    void fit_width();
    void fit_page();
    void goto_page(int page_index);
    QString all_text(int page_index = -1) const;
    QVector<PageHit> find_text(const QString &query, bool match_case = false, bool whole_words = false);
    void set_current_hit(int hit_index);
    void set_search_highlight_all(bool enabled);
    void set_speech_highlight_text(const QString &text);
    void set_speech_highlight_range(int global_start, int length);
    void set_speech_highlight_anchor(int range_start, int length, int anchor_start, const QString &anchor_text);
    void set_speech_highlight_page_ranges(const QVector<PageTextRange> &ranges);
    void clear_speech_highlight();
    void set_text_selection_enabled(bool enabled);
    bool text_selection_enabled() const { return text_selection_enabled_; }
    QString selected_text() const { return selected_text_; }
    bool has_text_selection() const { return !selected_text_.isEmpty(); }
    void clear_text_selection();
    int current_hit() const { return current_hit_; }
    int hit_count() const { return hits_.size(); }
    QSize page_pixel_size(int page_index, double zoom) const;
    QImage render_page(int page_index, double zoom) const;

signals:
    void current_page_changed(int page_index, int total);
    void effective_zoom_changed();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void attach_fresh_document();
    void release_document();
    void invalidate_text_gps();
    void rebuild_text_gps() const;
    QVector<PageTextRange> page_ranges_for_global_range(int global_start, int length) const;
    int corrected_range_start_from_anchor(int range_start, int anchor_start, const QString &anchor_text) const;
    void reset_search();
    void rebuild_search_highlights();

    struct TextGpsToken {
        int page_index = -1;
        int local_start = 0;
        int length = 0;
        int global_start = 0;
        QString text;
    };

    struct TextGpsPage {
        int page_index = -1;
        int global_start = 0;
        int global_end = 0;
        QString text;
        QVector<TextGpsToken> tokens;
    };

    std::unique_ptr<QPdfDocument> document_;
    std::unique_ptr<QPdfSearchModel> search_model_;
    QString pdf_path_;
    QVector<PageHit> hits_;
    QVector<SpeechHighlight> search_highlights_;
    QVector<SpeechHighlight> current_search_highlights_;
    QVector<SpeechHighlight> speech_highlights_;
    QVector<SpeechHighlight> text_selection_highlights_;
    QString selected_text_;
    QPointF text_selection_start_;
    mutable QVector<TextGpsPage> text_gps_pages_;
    mutable QString text_gps_text_;
    int text_selection_page_ = -1;
    int current_hit_ = -1;
    mutable bool text_gps_dirty_ = true;
    bool search_highlight_all_ = true;
    bool speech_search_active_ = false;
    bool text_selection_enabled_ = false;
    bool text_selecting_ = false;
};

}  // namespace source::plugin::Audiolivro
