#pragma once

#include <QHeaderView>
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>
#include <algorithm>

namespace source::gantt {

/// Custom header view that supports bi-directional section resizing.
///
/// When dragging the divider between two adjacent visible sections (Left Section and Right Section):
///   • Dragging to the RIGHT:
///       - Left Section increases in width (its right border moves right)
///       - Right Section decreases in width (its left border moves right)
///       - Other columns to the left and right are NOT displaced / pushed!
///   • Dragging to the LEFT:
///       - Left Section decreases in width (its right border moves left)
///       - Right Section increases in width (its left border moves left)
///       - Other columns to the left and right are NOT displaced / pushed!
///
/// When dragging the outer edge of the last visible section:
///   • Dragging right increases its width; dragging left decreases its width.
///
/// Clicking inside a section (away from divider borders) passes through to QHeaderView
/// for normal item/column selection and sorting.
class BiDirectionalResizeHeaderView : public QHeaderView {
public:
    explicit BiDirectionalResizeHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QHeaderView(orientation, parent)
    {
        setMouseTracking(true);
        if (viewport()) {
            viewport()->setMouseTracking(true);
        }
    }

protected:

    void leaveEvent(QEvent *e) override
    {
        if (!resizing_) {
            reset_cursor();
        }
        QHeaderView::leaveEvent(e);
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton) {
            const int pos = axis_pos(e);
            BorderHit hit = find_border_at(pos);
            if (hit.isValid()) {
                begin_resize(hit, pos);
                e->accept();
                return;
            }
        }

        // Not on a border -> let Qt handle section click, selection, sorting, etc.
        QHeaderView::mousePressEvent(e);
    }

    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (resizing_) {
            const int pos = axis_pos(e);
            const int delta = pos - drag_origin_;

            if (right_logical_ >= 0) {
                // Divider between left_logical_ and right_logical_
                const int min_left = qMax(10, minimumSectionSize());
                const int min_right = qMax(10, minimumSectionSize());

                int min_delta = min_left - orig_left_size_;
                int max_delta = orig_right_size_ - min_right;
                if (min_delta > 0) min_delta = 0;
                if (max_delta < 0) max_delta = 0;
                if (min_delta > max_delta) min_delta = max_delta = 0;

                const int clamped_delta = qBound(min_delta, delta, max_delta);

                const int new_left = orig_left_size_ + clamped_delta;
                const int new_right = orig_right_size_ - clamped_delta;

                resizeSection(left_logical_, new_left);
                resizeSection(right_logical_, new_right);
            } else {
                // Outer edge of the last visible section
                const int min_left = qMax(10, minimumSectionSize());
                const int new_left = qMax(min_left, orig_left_size_ + delta);
                resizeSection(left_logical_, new_left);
            }

            e->accept();
            return;
        }

        // Hover handling
        if (e->buttons() == Qt::NoButton) {
            BorderHit hit = find_border_at(axis_pos(e));
            if (hit.isValid()) {
                update_cursor(split_cursor());
                return;
            } else {
                reset_cursor();
            }
        }

        QHeaderView::mouseMoveEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (resizing_) {
            resizing_       = false;
            left_logical_   = -1;
            right_logical_  = -1;
            reset_cursor();
            e->accept();
            return;
        }

        QHeaderView::mouseReleaseEvent(e);
    }

private:
    struct BorderHit {
        int left_logical  = -1;
        int right_logical = -1; // -1 for outer edge of last section
        int border_pos    = -1;
        int distance      = 999999;

        bool isValid() const { return left_logical >= 0; }
    };

    int axis_pos(const QMouseEvent *e) const
    {
        return (orientation() == Qt::Horizontal)
                   ? static_cast<int>(e->position().x())
                   : static_cast<int>(e->position().y());
    }

    Qt::CursorShape split_cursor() const
    {
        return (orientation() == Qt::Horizontal)
                   ? Qt::SplitHCursor
                   : Qt::SplitVCursor;
    }

    void update_cursor(const QCursor &cursor)
    {
        setCursor(cursor);
        if (viewport()) {
            viewport()->setCursor(cursor);
        }
    }

    void reset_cursor()
    {
        unsetCursor();
        if (viewport()) {
            viewport()->unsetCursor();
        }
    }

    int grip_margin() const
    {
        int m = style() ? style()->pixelMetric(QStyle::PM_HeaderGripMargin, nullptr, this) : 4;
        return qMax(4, m);
    }

    QList<int> get_visible_sections() const
    {
        QList<int> visible;
        const int cnt = count();
        visible.reserve(cnt);
        for (int vis = 0; vis < cnt; ++vis) {
            const int log = logicalIndex(vis);
            if (!isSectionHidden(log) && sectionSize(log) > 0) {
                visible.append(log);
            }
        }
        return visible;
    }

    BorderHit find_border_at(int pos) const
    {
        BorderHit best_hit;
        const QList<int> visible = get_visible_sections();
        if (visible.isEmpty()) {
            return best_hit;
        }

        const int margin = grip_margin();

        // 1. Internal dividers between adjacent visible sections
        for (int i = 1; i < visible.size(); ++i) {
            const int left_log  = visible[i - 1];
            const int right_log = visible[i];

            // Ignore borders where either column is Fixed
            if (sectionResizeMode(left_log) == Fixed || sectionResizeMode(right_log) == Fixed) {
                continue;
            }

            const int border = sectionViewportPosition(right_log);
            const int dist   = qAbs(pos - border);
            if (dist <= margin && dist < best_hit.distance) {
                best_hit.left_logical  = left_log;
                best_hit.right_logical = right_log;
                best_hit.border_pos    = border;
                best_hit.distance      = dist;
            }
        }

        // 2. Outer border of the last visible section
        const int last_log = visible.last();
        if (sectionResizeMode(last_log) != Fixed) {
            const int last_border = sectionViewportPosition(last_log) + sectionSize(last_log);
            const int last_dist   = qAbs(pos - last_border);
            if (last_dist <= margin && last_dist < best_hit.distance) {
                best_hit.left_logical  = last_log;
                best_hit.right_logical = -1;
                best_hit.border_pos    = last_border;
                best_hit.distance      = last_dist;
            }
        }

        return best_hit;
    }

    void begin_resize(const BorderHit &hit, int pos)
    {
        resizing_        = true;
        left_logical_    = hit.left_logical;
        right_logical_   = hit.right_logical;
        drag_origin_     = pos;
        orig_left_size_  = sectionSize(left_logical_);
        orig_right_size_ = (right_logical_ >= 0) ? sectionSize(right_logical_) : 0;

        // Ensure resizing sections are in Interactive mode so Qt doesn't override them
        if (sectionResizeMode(left_logical_) == Stretch) {
            setSectionResizeMode(left_logical_, Interactive);
        }
        if (right_logical_ >= 0 && sectionResizeMode(right_logical_) == Stretch) {
            setSectionResizeMode(right_logical_, Interactive);
        }

        update_cursor(split_cursor());
    }

    // ── state ───────────────────────────────────────────────────────
    bool resizing_        = false;
    int  left_logical_    = -1;
    int  right_logical_   = -1;
    int  drag_origin_     = 0;
    int  orig_left_size_  = 0;
    int  orig_right_size_ = 0;
};

} // namespace source::gantt
