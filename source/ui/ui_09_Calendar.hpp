#pragma once

#include <QMargins>
#include <QWidget>

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI {

class CalendarPanel;
class RotatedTabButton;

class Calendar : public QWidget {
    Q_OBJECT

public:
    explicit Calendar(source::EisenhowerMatrixApp *app, QWidget *parent = nullptr);

    CalendarPanel *calendar_panel = nullptr;

    int effective_min_width() const;
    bool is_panel_expanded() const;
    void set_panel_expanded(bool expanded);
    void toggle_panel(bool open_if_hidden = false);
    void toggle_month_filter_calendar_mode();
    void on_language_changed();

private:
    void _initial_sync_after_show();
    void _sync_splitter_sizes_after_toggle(bool preserve_current = true);
    void _update_splitter_interaction();
    void _set_month_toggle_visibility(bool visible);
    void _on_month_view_mode_changed(const QString &mode);

    source::EisenhowerMatrixApp *app_;
    bool _expanded = true;
    int _panel_width = 400;
    QMargins _layout_margins;
    RotatedTabButton *toggle_button = nullptr;
    RotatedTabButton *calendar_mode_button = nullptr;
};

}  // namespace source::GerenciamentoUI
