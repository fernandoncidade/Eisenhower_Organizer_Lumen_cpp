#include "source/ui/ui_09_Calendar.hpp"

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QSplitter>
#include <QSplitterHandle>
#include <QTimer>
#include <QVBoxLayout>

#include "source/ui/ui_13_RotatedTabButton.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/utils/LogManager.hpp"

namespace source::GerenciamentoUI
{
    namespace
    {

        constexpr int CALENDAR_PANEL_MIN_WIDTH = 400;

        QString get_text(const QString &text)
        {
            return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
        }

        source::utils::LogManager *logger()
        {
            return source::utils::LogManager::get_logger();
        }

    } // namespace

    Calendar::Calendar(source::EisenhowerMatrixApp *app, QWidget *parent)
        : QWidget(parent), app_(app), _layout_margins(0, 0, 0, 0)
    {
        try
        {
            _expanded = (app_ != nullptr) ? app_->calendar_start_expanded : false;
            const int base_panel_width =
                qMax(220, app_ != nullptr ? app_->calendar_min_width : CALENDAR_PANEL_MIN_WIDTH);
            _panel_width = base_panel_width;

            auto *layout = new QHBoxLayout(this);
            layout->setContentsMargins(_layout_margins);
            layout->setSpacing(0);

            setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

            auto *side_tabs_layout = new QVBoxLayout();
            side_tabs_layout->setContentsMargins(0, 6, 0, 0);
            side_tabs_layout->setSpacing(0);

            toggle_button = new RotatedTabButton(get_text(QStringLiteral("Mostrar Calendário")), this);
            toggle_button->setMinimumWidth(25);
            toggle_button->setClickHandler([this]()
                                           { toggle_panel(); });
            side_tabs_layout->addWidget(toggle_button, 0, Qt::AlignLeft | Qt::AlignTop);

            calendar_mode_button =
                new RotatedTabButton(get_text(QStringLiteral("Calendário Anual/Calendário Mensal")), this);
            calendar_mode_button->setMinimumWidth(25);
            calendar_mode_button->setClickHandler([this]()
                                                  { toggle_month_filter_calendar_mode(); });
            calendar_mode_button->setVisible(false);
            calendar_mode_button->setEnabled(false);
            side_tabs_layout->addWidget(calendar_mode_button, 0, Qt::AlignLeft | Qt::AlignTop);
            side_tabs_layout->addStretch(1);

            layout->addLayout(side_tabs_layout, 0);

            calendar_panel = new CalendarPanel(app_, this);
            calendar_panel->set_calendar_content_width(base_panel_width);
            calendar_panel->setMinimumWidth(qMax(calendar_panel->minimumSizeHint().width(), base_panel_width));
            calendar_panel->setMaximumWidth(QWIDGETSIZE_MAX);
            calendar_panel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            layout->addWidget(calendar_panel, 1);

            connect(calendar_panel,
                    &CalendarPanel::month_view_toggle_visibility_changed,
                    this,
                    [this](bool visible)
                    { _set_month_toggle_visibility(visible); });
            connect(calendar_panel,
                    &CalendarPanel::month_view_mode_changed,
                    this,
                    [this](const QString &mode)
                    { _on_month_view_mode_changed(mode); });

            _set_month_toggle_visibility(calendar_panel->is_month_filter_active());
            _on_month_view_mode_changed(calendar_panel->calendar_view_mode());

            calendar_panel->setVisible(_expanded);
            toggle_button->setText(
                _expanded ? get_text(QStringLiteral("Recolher Calendário"))
                          : get_text(QStringLiteral("Mostrar Calendário")));
            setMinimumWidth(effective_min_width());
            _update_splitter_interaction();

            if (app_ != nullptr && app_->gerenciador_traducao != nullptr)
            {
                connect(app_->gerenciador_traducao,
                        &source::language::GerenciadorTraducao::idioma_alterado,
                        this,
                        [this](const QString &)
                        { on_language_changed(); });
            }

            if (_expanded)
            {
                QTimer::singleShot(0, this, [this]()
                                   { _initial_sync_after_show(); });
            }
        }
        catch (...)
        {
            logger()->error(QStringLiteral("Erro ao inicializar Calendar."));
        }
    }

    int Calendar::effective_min_width() const
    {
        try
        {
            int btn_w = qMax(toggle_button->minimumWidth(), toggle_button->sizeHint().width());
            if (calendar_mode_button->isVisible())
            {
                btn_w = qMax(btn_w, qMax(calendar_mode_button->minimumWidth(), calendar_mode_button->sizeHint().width()));
            }

            int panel_w = 0;
            if (_expanded && calendar_panel != nullptr)
            {
                panel_w = qMax(calendar_panel->minimumWidth(), calendar_panel->minimumSizeHint().width());
            }

            return _layout_margins.left() + btn_w + panel_w + _layout_margins.right();
        }
        catch (...)
        {
            logger()->error(QStringLiteral("Erro ao calcular largura minima efetiva."));
            return 25;
        }
    }

    bool Calendar::is_panel_expanded() const
    {
        return _expanded;
    }

    void Calendar::set_panel_expanded(bool expanded)
    {
        if (_expanded == expanded)
        {
            return;
        }

        if (expanded)
        {
            toggle_panel(true);
            return;
        }

        toggle_panel(false);
    }

    void Calendar::_initial_sync_after_show()
    {
        setMinimumWidth(effective_min_width());
        updateGeometry();
        _sync_splitter_sizes_after_toggle(false);
        _update_splitter_interaction();

        if (calendar_panel != nullptr && calendar_panel->_is_month_view_active())
        {
            calendar_panel->_update_month_calendars_for_width();
        }
        if (calendar_panel != nullptr && calendar_panel->_is_year_view_active())
        {
            calendar_panel->_update_year_calendars_for_width();
        }
    }

    void Calendar::_sync_splitter_sizes_after_toggle(bool preserve_current)
    {
        if (app_ == nullptr || app_->main_splitter == nullptr)
        {
            return;
        }

        const QList<int> sizes = app_->main_splitter->sizes();
        if (sizes.size() < 2)
        {
            return;
        }

        const int left_min = qMax(minimumWidth(), effective_min_width());
        QWidget *right_widget = app_->main_splitter->widget(1);
        const int right_min = (right_widget != nullptr) ? right_widget->minimumWidth() : 0;
        const int total = qMax(0, sizes.at(0) + sizes.at(1));

        int target_left = left_min;
        if (_expanded && preserve_current)
        {
            target_left = qMax(sizes.at(0), left_min);
        }

        const int max_left = qMax(left_min, total - right_min);
        target_left = qMin(target_left, max_left);
        int target_right = qMax(right_min, total - target_left);

        if (target_left + target_right != total)
        {
            target_right = qMax(right_min, total - target_left);
            target_left = qMax(left_min, total - target_right);
        }

        app_->main_splitter->setSizes({target_left, target_right});
    }

    void Calendar::_update_splitter_interaction()
    {
        if (app_ == nullptr || app_->main_splitter == nullptr)
        {
            return;
        }

        if (_expanded)
        {
            app_->main_splitter->setHandleWidth(app_->calendar_splitter_handle_width);
            if (QSplitterHandle *handle = app_->main_splitter->handle(1))
            {
                handle->setEnabled(true);
            }
        }
        else
        {
            app_->main_splitter->setHandleWidth(0);
            if (QSplitterHandle *handle = app_->main_splitter->handle(1))
            {
                handle->setEnabled(false);
            }
        }
    }

    void Calendar::toggle_panel(bool open_if_hidden)
    {
        if (open_if_hidden && !_expanded)
        {
            _expanded = true;
        }
        else
        {
            _expanded = !_expanded;
        }

        calendar_panel->setVisible(_expanded);
        if (_expanded)
        {
            const int hint_w = qMax(qMax(calendar_panel->sizeHint().width(), calendar_panel->minimumSizeHint().width()),
                                    _panel_width);
            _panel_width = qMax(_panel_width, hint_w);
            toggle_button->setText(get_text(QStringLiteral("Recolher Calendário")));
        }
        else
        {
            toggle_button->setText(get_text(QStringLiteral("Mostrar Calendário")));
        }

        setMinimumWidth(effective_min_width());
        updateGeometry();
        _sync_splitter_sizes_after_toggle();
        _update_splitter_interaction();

        if (_expanded && calendar_panel != nullptr)
        {
            if (calendar_panel->_is_month_view_active())
            {
                QTimer::singleShot(0, calendar_panel, [this]()
                                   { calendar_panel->_update_month_calendars_for_width(); });
            }
            if (calendar_panel->_is_year_view_active())
            {
                QTimer::singleShot(0, calendar_panel, [this]()
                                   { calendar_panel->_update_year_calendars_for_width(); });
            }
        }
    }

    void Calendar::toggle_month_filter_calendar_mode()
    {
        if (calendar_panel != nullptr)
        {
            calendar_panel->toggle_month_filter_view_mode();
        }
    }

    void Calendar::_set_month_toggle_visibility(bool visible)
    {
        calendar_mode_button->setVisible(visible);
        calendar_mode_button->setEnabled(visible);
        setMinimumWidth(effective_min_width());
        updateGeometry();
    }

    void Calendar::_on_month_view_mode_changed(const QString &mode)
    {
        if (mode == QStringLiteral("year"))
        {
            calendar_mode_button->setText(get_text(QStringLiteral("Calendário Mensal")));
            calendar_mode_button->setToolTip(
                get_text(QStringLiteral("Visualização atual: calendário anual. Clique para calendário mensal.")));
        }
        else
        {
            calendar_mode_button->setText(get_text(QStringLiteral("Calendário Anual")));
            calendar_mode_button->setToolTip(
                get_text(QStringLiteral("Visualização atual: calendário mensal. Clique para calendário anual.")));
        }
    }

    void Calendar::on_language_changed()
    {
        toggle_button->setText(
            _expanded ? get_text(QStringLiteral("Recolher Calendário"))
                      : get_text(QStringLiteral("Mostrar Calendário")));
        if (calendar_panel != nullptr)
        {
            _on_month_view_mode_changed(calendar_panel->calendar_view_mode());
            calendar_panel->_on_language_changed();
        }
    }

} // namespace source::GerenciamentoUI
