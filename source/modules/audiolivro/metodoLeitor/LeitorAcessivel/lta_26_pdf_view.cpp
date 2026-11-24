#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_26_pdf_view.hpp"

#include <QFileInfo>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPdfPageNavigator>
#include <QPdfSelection>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>
#include <QtMath>

#include <utility>

namespace source::plugin::Audiolivro {
class PDFView;

namespace {

struct PageGpsTransform {
    QRect geometry;
    qreal page_scale = 1.0;
    qreal point_to_view = 1.0;
    bool valid = false;
};

QString normalized_gps_token(QString text)
{
    text = text.trimmed();
    while (!text.isEmpty() && !text.front().isLetterOrNumber()) {
        text.remove(0, 1);
    }
    while (!text.isEmpty() && !text.back().isLetterOrNumber()) {
        text.chop(1);
    }
    return text.toCaseFolded();
}

qreal screen_resolution(const QWidget *widget)
{
    Q_UNUSED(widget);
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen == nullptr) {
        return 1.0;
    }

    const qreal dpi = screen->logicalDotsPerInch();
    if (dpi <= 0.0) {
        return 1.0;
    }

    return dpi / 72.0;
}

QSize page_screen_size(const PDFView *view, int page, qreal resolution)
{
    return QSizeF(view->page_point_size(page) * resolution).toSize();
}

PageGpsTransform page_gps_transform(const PDFView *view, int target_page)
{
    if (view == nullptr || target_page < 0 || target_page >= view->page_count()) {
        return {};
    }

    const QMargins margins = view->documentMargins();
    const int start_page = view->pageMode() == QPdfView::PageMode::SinglePage ? view->current_page() : 0;
    const int end_page = view->pageMode() == QPdfView::PageMode::SinglePage ? view->current_page() + 1 : view->page_count();
    if (target_page < start_page || target_page >= end_page) {
        return {};
    }

    struct PageSizeAndScale {
        QSize size;
        qreal scale = 1.0;
    };

    QVector<PageSizeAndScale> pages(view->page_count());
    int total_width = 0;
    const qreal resolution = screen_resolution(view);
    const QSize viewport_size(view->viewport()->width(), view->viewport()->height());
    for (int page = start_page; page < end_page; ++page) {
        QSize page_size;
        qreal page_scale = view->zoomFactor();
        const QSize point_size = page_screen_size(view, page, resolution);
        if (view->zoomMode() == QPdfView::ZoomMode::Custom) {
            page_size = point_size;
            page_size *= view->zoomFactor();
        } else if (view->zoomMode() == QPdfView::ZoomMode::FitToWidth) {
            const int available_width = qMax(1, view->viewport()->width() - margins.left() - margins.right());
            page_scale = point_size.width() > 0 ? qreal(available_width) / qreal(point_size.width()) : 1.0;
            page_size = point_size;
            page_size *= page_scale;
        } else {
            const QSize available_size = viewport_size + QSize(-margins.left() - margins.right(), -view->pageSpacing());
            page_size = point_size.scaled(available_size.expandedTo(QSize(1, 1)), Qt::KeepAspectRatio);
            page_scale = point_size.width() > 0 ? qreal(page_size.width()) / qreal(point_size.width()) : 1.0;
        }

        total_width = qMax(total_width, page_size.width());
        pages[page] = PageSizeAndScale{page_size, page_scale};
    }

    total_width += margins.left() + margins.right();

    int page_y = margins.top();
    for (int page = start_page; page < end_page; ++page) {
        const QSize page_size = pages.at(page).size;
        const int page_x = (qMax(total_width, view->viewport()->width()) - page_size.width()) / 2;
        if (page == target_page) {
            return PageGpsTransform{QRect(QPoint(page_x, page_y), page_size),
                                    pages.at(page).scale,
                                    resolution * pages.at(page).scale,
                                    true};
        }
        page_y += page_size.height() + view->pageSpacing();
    }

    return {};
}

bool page_point_at_viewport_position(const PDFView *view, const QPointF &viewport_pos, int *page_index, QPointF *page_point)
{
    if (view == nullptr) {
        return false;
    }

    const QPointF document_pos = viewport_pos + QPointF(view->horizontalScrollBar()->value(), view->verticalScrollBar()->value());
    for (int page = 0; page < view->page_count(); ++page) {
        const PageGpsTransform geometry = page_gps_transform(view, page);
        if (!geometry.valid || !geometry.geometry.contains(document_pos.toPoint())) {
            continue;
        }

        const QPointF local = document_pos - QPointF(geometry.geometry.topLeft());
        if (geometry.point_to_view <= 0.0) {
            return false;
        }

        if (page_index != nullptr) {
            *page_index = page;
        }
        if (page_point != nullptr) {
            *page_point = local / geometry.point_to_view;
        }
        return true;
    }

    return false;
}

void paint_highlights(QPainter *painter,
                      const PDFView *view,
                      const QVector<PDFView::SpeechHighlight> &highlights,
                      const QColor &fill,
                      const QColor &border)
{
    if (painter == nullptr || view == nullptr || highlights.isEmpty()) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(border, 1.0));
    painter->setBrush(fill);

    const QPointF scroll_offset(view->horizontalScrollBar()->value(), view->verticalScrollBar()->value());
    for (const PDFView::SpeechHighlight &highlight : highlights) {
        const PageGpsTransform geometry = page_gps_transform(view, highlight.page_index);
        if (!geometry.valid) {
            continue;
        }

        const QTransform transform = QTransform::fromScale(geometry.point_to_view, geometry.point_to_view);
        const QPointF page_top_left = QPointF(geometry.geometry.topLeft()) - scroll_offset;
        for (const QPolygonF &bounds : highlight.bounds) {
            if (bounds.isEmpty()) {
                continue;
            }

            QPolygonF mapped;
            mapped.reserve(bounds.size());
            for (const QPointF &point : bounds) {
                mapped.append(transform.map(point) + page_top_left);
            }

            if (mapped.size() >= 3) {
                painter->drawPolygon(mapped);
            } else {
                painter->drawRoundedRect(mapped.boundingRect().adjusted(-1.0, -1.0, 1.0, 1.0), 2.0, 2.0);
            }
        }
    }
}

}  // namespace

PDFView::PDFView(QWidget *parent)
    : QPdfView(parent)
{
    attach_fresh_document();
    setPageMode(QPdfView::PageMode::MultiPage);
    setZoomMode(QPdfView::ZoomMode::Custom);
    setPageSpacing(14);
    setDocumentMargins(QMargins(12, 12, 12, 12));

    if (pageNavigator() != nullptr) {
        connect(pageNavigator(), &QPdfPageNavigator::currentPageChanged, this, [this](int page) {
            emit current_page_changed(page, page_count());
        });
    }

}

PDFView::~PDFView()
{
    release_document();
}

void PDFView::attach_fresh_document()
{
    invalidate_text_gps();
    document_ = std::make_unique<QPdfDocument>();
    search_model_ = std::make_unique<QPdfSearchModel>();

    setDocument(document_.get());
    search_model_->setDocument(document_.get());
    setSearchModel(search_model_.get());

    connect(document_.get(), &QPdfDocument::statusChanged, this, [this](QPdfDocument::Status status) {
        invalidate_text_gps();
        if (status == QPdfDocument::Status::Ready) {
            fit_width();
            emit current_page_changed(current_page(), page_count());
        } else if (status == QPdfDocument::Status::Error) {
            pdf_path_.clear();
            emit current_page_changed(0, 0);
        }
    });

    connect(document_.get(), &QPdfDocument::pageCountChanged, this, [this](int) {
        emit current_page_changed(current_page(), page_count());
    });
}

void PDFView::release_document()
{
    invalidate_text_gps();

    setSearchModel(nullptr);
    setDocument(nullptr);

    if (search_model_ != nullptr) {
        search_model_->setSearchString(QString());
        search_model_->setDocument(nullptr);
    }
    
    if (document_ != nullptr) {
        document_->close();
    }

    search_model_.reset();
    document_.reset();
}

void PDFView::invalidate_text_gps()
{
    text_gps_dirty_ = true;
    text_gps_pages_.clear();
    text_gps_text_.clear();
}

void PDFView::rebuild_text_gps() const
{
    if (!text_gps_dirty_) {
        return;
    }

    text_gps_dirty_ = false;
    text_gps_pages_.clear();
    text_gps_text_.clear();

    if (document_ == nullptr || document_->status() != QPdfDocument::Status::Ready || page_count() <= 0) {
        return;
    }

    bool has_previous_text_page = false;
    for (int page = 0; page < page_count(); ++page) {
        const QString page_text = document_->getAllText(page).text();
        if (page_text.isEmpty()) {
            continue;
        }

        if (has_previous_text_page) {
            text_gps_text_ += QStringLiteral("\n\n");
        }

        TextGpsPage gps_page;
        gps_page.page_index = page;
        gps_page.global_start = text_gps_text_.size();
        gps_page.text = page_text;

        int local = 0;
        while (local < page_text.size()) {
            while (local < page_text.size() && page_text.at(local).isSpace()) {
                ++local;
            }
            const int token_start = local;
            while (local < page_text.size() && !page_text.at(local).isSpace()) {
                ++local;
            }
            const int token_length = local - token_start;
            if (token_length > 0) {
                gps_page.tokens.append(TextGpsToken{page,
                                                    token_start,
                                                    token_length,
                                                    gps_page.global_start + token_start,
                                                    page_text.mid(token_start, token_length)});
            }
        }

        text_gps_text_ += page_text;
        gps_page.global_end = text_gps_text_.size();
        text_gps_pages_.append(std::move(gps_page));
        has_previous_text_page = true;
    }
}

QVector<PDFView::PageTextRange> PDFView::page_ranges_for_global_range(int global_start, int length) const
{
    QVector<PageTextRange> ranges;
    if (length <= 0) {
        return ranges;
    }

    rebuild_text_gps();
    if (text_gps_text_.isEmpty()) {
        return ranges;
    }

    const int range_start = qBound(0, global_start, text_gps_text_.size());
    const int range_end = qBound(range_start, range_start + qMax(1, length), text_gps_text_.size());
    for (const TextGpsPage &page : std::as_const(text_gps_pages_)) {
        if (page.global_end <= range_start) {
            continue;
        }
        if (page.global_start >= range_end) {
            break;
        }

        const int selection_start = qMax(range_start, page.global_start);
        const int selection_end = qMin(range_end, page.global_end);
        if (selection_start < selection_end) {
            ranges.append(PageTextRange{page.page_index, selection_start - page.global_start, selection_end - selection_start});
        }
    }

    return ranges;
}

int PDFView::corrected_range_start_from_anchor(int range_start, int anchor_start, const QString &anchor_text) const
{
    rebuild_text_gps();
    const QString anchor = normalized_gps_token(anchor_text);
    if (anchor.isEmpty() || text_gps_pages_.isEmpty()) {
        return range_start;
    }

    const int expected = qBound(0, anchor_start, text_gps_text_.size());
    if (expected + anchor_text.size() <= text_gps_text_.size()
        && normalized_gps_token(text_gps_text_.mid(expected, anchor_text.size())) == anchor) {
        return range_start;
    }

    const int max_anchor_jump = qMax(360, anchor.size() * 90);
    const int window_start = qMax(0, expected - max_anchor_jump);
    const int window_end = qMin(text_gps_text_.size(), expected + max_anchor_jump);
    int best_start = -1;
    int best_distance = text_gps_text_.size() + 1;

    for (const TextGpsPage &page : std::as_const(text_gps_pages_)) {
        if (page.global_end < window_start) {
            continue;
        }
        if (page.global_start > window_end) {
            break;
        }

        for (const TextGpsToken &token : page.tokens) {
            if (token.global_start < window_start) {
                continue;
            }
            if (token.global_start > window_end) {
                break;
            }

            const QString token_text = normalized_gps_token(token.text);
            if (token_text != anchor) {
                continue;
            }

            const int distance = qAbs(token.global_start - expected);
            if (distance < best_distance) {
                best_distance = distance;
                best_start = token.global_start;
            }
        }
    }

    if (best_start < 0) {
        return range_start;
    }

    if (best_distance > max_anchor_jump) {
        return range_start;
    }

    return qBound(0, range_start + (best_start - expected), text_gps_text_.size());
}

bool PDFView::load_pdf(const QString &path)
{
    if (!QFileInfo::exists(path)) {
        return false;
    }

    clear_speech_highlight();
    clear_text_selection();
    reset_search();
    pdf_path_.clear();
    release_document();
    attach_fresh_document();

    if (document_->load(path) != QPdfDocument::Error::None) {
        viewport()->update();
        emit current_page_changed(0, 0);
        return false;
    }

    pdf_path_ = path;
    if (pageNavigator() != nullptr) {
        pageNavigator()->jump(0, QPointF(), 0);
    }
    return true;
}

void PDFView::clear_pdf()
{
    clear_speech_highlight();
    clear_text_selection();
    reset_search();
    release_document();
    attach_fresh_document();
    pdf_path_.clear();
    viewport()->update();
    emit current_page_changed(0, 0);
}

int PDFView::page_count() const
{
    if (document_ == nullptr || document_->status() != QPdfDocument::Status::Ready) {
        return 0;
    }
    return document_->pageCount();
}

QSizeF PDFView::page_point_size(int page_index) const
{
    if (document_ == nullptr || document_->status() != QPdfDocument::Status::Ready) {
        return QSizeF();
    }
    if (page_index < 0 || page_index >= page_count()) {
        return QSizeF();
    }
    return document_->pagePointSize(page_index);
}

QString PDFView::meta_data(QPdfDocument::MetaDataField field) const
{
    if (document_ == nullptr || document_->status() != QPdfDocument::Status::Ready) {
        return QString();
    }
    return document_->metaData(field).toString();
}

int PDFView::current_page() const
{
    return pageNavigator() != nullptr ? pageNavigator()->currentPage() : 0;
}

double PDFView::zoom_factor() const
{
    return zoomFactor();
}

double PDFView::effective_zoom_factor() const
{
    const PageGpsTransform geometry = page_gps_transform(this, current_page());
    return geometry.valid ? geometry.page_scale : zoomFactor();
}

void PDFView::set_zoom_factor(double zoom)
{
    setZoomMode(QPdfView::ZoomMode::Custom);
    setZoomFactor(qBound(0.3, zoom, 4.0));
    emit effective_zoom_changed();
}

void PDFView::zoom_in()
{
    set_zoom_factor(effective_zoom_factor() + 0.1);
}

void PDFView::zoom_out()
{
    set_zoom_factor(effective_zoom_factor() - 0.1);
}

void PDFView::fit_width()
{
    setZoomMode(QPdfView::ZoomMode::FitToWidth);
    emit effective_zoom_changed();
    viewport()->update();
}

void PDFView::fit_page()
{
    setZoomMode(QPdfView::ZoomMode::FitInView);
    emit effective_zoom_changed();
    viewport()->update();
}

void PDFView::goto_page(int page_index)
{
    if (pageNavigator() != nullptr && page_count() > 0) {
        pageNavigator()->jump(qBound(0, page_index, page_count() - 1), QPointF(), 0);
    }
}

QString PDFView::all_text(int page_index) const
{
    if (document_ == nullptr || document_->status() != QPdfDocument::Status::Ready) {
        return QString();
    }
    if (page_count() <= 0) {
        return QString();
    }

    rebuild_text_gps();
    if (page_index < 0) {
        return text_gps_text_;
    }

    const int page = qBound(0, page_index, page_count() - 1);
    for (const TextGpsPage &gps_page : std::as_const(text_gps_pages_)) {
        if (gps_page.page_index == page) {
            return gps_page.text;
        }
    }
    return QString();
}

QVector<PageHit> PDFView::find_text(const QString &query, bool match_case, bool whole_words)
{
    hits_.clear();
    current_hit_ = -1;
    search_highlights_.clear();
    current_search_highlights_.clear();
    if (search_model_ != nullptr) {
        search_model_->setSearchString(QString());
    }

    const QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        viewport()->update();
        return hits_;
    }

    const Qt::CaseSensitivity cs = match_case ? Qt::CaseSensitive : Qt::CaseInsensitive;
    for (int page = 0; page < page_count(); ++page) {
        const QString text = all_text(page);
        int pos = 0;
        while ((pos = text.indexOf(trimmed, pos, cs)) >= 0) {
            if (whole_words) {
                const bool left_ok = pos == 0 || !text.at(pos - 1).isLetterOrNumber();
                const int right = pos + trimmed.size();
                const bool right_ok = right >= text.size() || !text.at(right).isLetterOrNumber();
                if (!left_ok || !right_ok) {
                    pos += qMax(1, trimmed.size());
                    continue;
                }
            }
            const QPdfSelection selection = document_->getSelectionAtIndex(page, pos, trimmed.size());
            QList<QPolygonF> bounds;
            QRectF rect;
            if (selection.isValid()) {
                bounds = selection.bounds();
                for (const QPolygonF &bound : std::as_const(bounds)) {
                    rect = rect.united(bound.boundingRect());
                }
            }
            hits_.append(PageHit{page, rect, text.mid(pos, trimmed.size()), bounds});
            pos += qMax(1, trimmed.size());
        }
    }

    if (!hits_.isEmpty()) {
        current_hit_ = 0;
        rebuild_search_highlights();
        goto_page(hits_.first().page_index);
    } else {
        viewport()->update();
    }
    return hits_;
}

void PDFView::set_current_hit(int hit_index)
{
    if (hits_.isEmpty()) {
        current_hit_ = -1;
        return;
    }

    current_hit_ = (hit_index % hits_.size() + hits_.size()) % hits_.size();
    goto_page(hits_.at(current_hit_).page_index);
    rebuild_search_highlights();
}

void PDFView::set_search_highlight_all(bool enabled)
{
    if (search_highlight_all_ == enabled) {
        return;
    }

    search_highlight_all_ = enabled;
    rebuild_search_highlights();
}

void PDFView::set_speech_highlight_text(const QString &text)
{
    QString query = text.simplified();
    if (query.size() > 180) {
        query = query.left(180).trimmed();
    }

    if (query.isEmpty()) {
        clear_speech_highlight();
        return;
    }

    const int exact = all_text(-1).indexOf(query, 0, Qt::CaseInsensitive);
    if (exact >= 0) {
        set_speech_highlight_range(exact, query.size());
        return;
    }

    speech_search_active_ = true;
    if (search_model_ != nullptr && search_model_->searchString() != query) {
        search_model_->setSearchString(query);
    }
    setCurrentSearchResultIndex(0);
}

void PDFView::set_speech_highlight_range(int global_start, int length)
{
    if (document_ == nullptr || document_->status() != QPdfDocument::Status::Ready || page_count() <= 0 || length <= 0) {
        speech_highlights_.clear();
        viewport()->update();
        return;
    }

    set_speech_highlight_page_ranges(page_ranges_for_global_range(global_start, length));
}

void PDFView::set_speech_highlight_anchor(int range_start, int length, int anchor_start, const QString &anchor_text)
{
    Q_UNUSED(anchor_start);
    Q_UNUSED(anchor_text);
    set_speech_highlight_range(range_start, length);
}

void PDFView::set_speech_highlight_page_ranges(const QVector<PageTextRange> &ranges)
{
    speech_highlights_.clear();
    if (document_ == nullptr || document_->status() != QPdfDocument::Status::Ready || ranges.isEmpty()) {
        viewport()->update();
        return;
    }

    int first_page = -1;
    for (const PageTextRange &range : ranges) {
        if (range.page_index < 0 || range.page_index >= page_count() || range.length <= 0) {
            continue;
        }

        const QString page_text = document_->getAllText(range.page_index).text();
        const int local_start = qBound(0, range.start, page_text.size());
        const int local_length = qMin(qMax(1, range.length), page_text.size() - local_start);
        if (local_length <= 0) {
            continue;
        }

        const QPdfSelection selection = document_->getSelectionAtIndex(range.page_index, local_start, local_length);
        if (selection.isValid() && !selection.bounds().isEmpty()) {
            speech_highlights_.append(SpeechHighlight{range.page_index, selection.bounds()});
            if (first_page < 0) {
                first_page = range.page_index;
            }
        }
    }

    if (first_page >= 0 && first_page != current_page()) {
        goto_page(first_page);
    }
    viewport()->update();
}

void PDFView::clear_speech_highlight()
{
    const bool had_custom_highlight = !speech_highlights_.isEmpty();
    speech_highlights_.clear();

    if (speech_search_active_) {
        speech_search_active_ = false;
        if (search_model_ != nullptr) {
            search_model_->setSearchString(QString());
        }
        setCurrentSearchResultIndex(-1);
    }

    if (had_custom_highlight) {
        viewport()->update();
    }
}

void PDFView::set_text_selection_enabled(bool enabled)
{
    text_selection_enabled_ = enabled;
    text_selecting_ = false;
    viewport()->setCursor(enabled ? Qt::IBeamCursor : Qt::ArrowCursor);
}

void PDFView::clear_text_selection()
{
    const bool had_selection = !selected_text_.isEmpty() || !text_selection_highlights_.isEmpty();
    selected_text_.clear();
    text_selection_highlights_.clear();
    text_selection_page_ = -1;
    text_selecting_ = false;
    if (had_selection) {
        viewport()->update();
    }
}

void PDFView::paintEvent(QPaintEvent *event)
{
    QPdfView::paintEvent(event);

    QPainter painter(viewport());
    paint_highlights(&painter, this, text_selection_highlights_, QColor(58, 134, 255, 78), QColor(38, 100, 210, 150));
    paint_highlights(&painter, this, search_highlights_, QColor(255, 232, 92, 92), QColor(214, 143, 0, 120));
    paint_highlights(&painter, this, current_search_highlights_, QColor(255, 190, 71, 135), QColor(190, 105, 0, 190));
    paint_highlights(&painter, this, speech_highlights_, QColor(255, 224, 102, 118), QColor(214, 143, 0, 180));
}

void PDFView::resizeEvent(QResizeEvent *event)
{
    QPdfView::resizeEvent(event);
    if (zoomMode() != QPdfView::ZoomMode::Custom) {
        emit effective_zoom_changed();
        viewport()->update();
    }
}

void PDFView::mousePressEvent(QMouseEvent *event)
{
    if (event != nullptr && text_selection_enabled_ && event->button() == Qt::LeftButton) {
        int page = -1;
        QPointF page_point;
        if (page_point_at_viewport_position(this, event->position(), &page, &page_point)) {
            clear_text_selection();
            text_selecting_ = true;
            text_selection_page_ = page;
            text_selection_start_ = page_point;
            event->accept();
            return;
        }
    }

    QPdfView::mousePressEvent(event);
}

void PDFView::mouseMoveEvent(QMouseEvent *event)
{
    if (event != nullptr && text_selection_enabled_ && text_selecting_) {
        int page = -1;
        QPointF page_point;
        if (page_point_at_viewport_position(this, event->position(), &page, &page_point) && page == text_selection_page_) {
            const QPdfSelection selection = document_->getSelection(page, text_selection_start_, page_point);
            text_selection_highlights_.clear();
            selected_text_.clear();
            if (selection.isValid()) {
                selected_text_ = selection.text();
                if (!selection.bounds().isEmpty()) {
                    text_selection_highlights_.append(SpeechHighlight{page, selection.bounds()});
                }
            }
            viewport()->update();
            event->accept();
            return;
        }
    }

    if (event != nullptr && text_selection_enabled_) {
        viewport()->setCursor(Qt::IBeamCursor);
        event->accept();
        return;
    }

    QPdfView::mouseMoveEvent(event);
}

void PDFView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event != nullptr && text_selection_enabled_ && text_selecting_ && event->button() == Qt::LeftButton) {
        mouseMoveEvent(event);
        text_selecting_ = false;
        event->accept();
        return;
    }

    QPdfView::mouseReleaseEvent(event);
}

QSize PDFView::page_pixel_size(int page_index, double zoom) const
{
    const QSizeF points = page_point_size(page_index);
    if (!points.isValid() || points.isEmpty()) {
        return QSize();
    }

    const double pixel_w = points.width() * zoom;
    const double pixel_h = points.height() * zoom;
    if (!qIsFinite(pixel_w) || !qIsFinite(pixel_h) || pixel_w <= 0.0 || pixel_h <= 0.0) {
        return QSize();
    }

    return QSize(qMax(32, int(qCeil(pixel_w))), qMax(32, int(qCeil(pixel_h))));
}

QImage PDFView::render_page(int page_index, double zoom) const
{
    const QSize logical = page_pixel_size(page_index, zoom);
    if (logical.isEmpty()) {
        return QImage();
    }

    double dpr = devicePixelRatioF();
    if (dpr <= 0.0) {
        dpr = 1.0;
    }

    const QSize render_size(qMax(1, int(qCeil(logical.width() * dpr))),
                            qMax(1, int(qCeil(logical.height() * dpr))));
    if (document_ == nullptr || document_->status() != QPdfDocument::Status::Ready) {
        return QImage();
    }

    QImage rendered = document_->render(page_index, render_size);
    if (rendered.isNull()) {
        return QImage();
    }
    rendered.setDevicePixelRatio(dpr);
    return rendered;
}

void PDFView::reset_search()
{
    hits_.clear();
    current_hit_ = -1;
    search_highlights_.clear();
    current_search_highlights_.clear();
    if (search_model_ != nullptr) {
        search_model_->setSearchString(QString());
    }
    viewport()->update();
}

void PDFView::rebuild_search_highlights()
{
    search_highlights_.clear();
    current_search_highlights_.clear();

    for (int i = 0; i < hits_.size(); ++i) {
        const PageHit &hit = hits_.at(i);
        if (hit.bounds.isEmpty()) {
            continue;
        }

        const SpeechHighlight highlight{hit.page_index, hit.bounds};
        if (i == current_hit_) {
            current_search_highlights_.append(highlight);
        } else if (search_highlight_all_) {
            search_highlights_.append(highlight);
        }
    }

    viewport()->update();
}

}  // namespace source::plugin::Audiolivro
