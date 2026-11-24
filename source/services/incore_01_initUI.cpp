#include "source/services/incore_01_initUI.hpp"

#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDate>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QRegion>
#include <QLocale>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QStackedLayout>
#include <QStyle>
#include <QStyleOption>
#include <QStyleOptionButton>
#include <QStyleOptionFrame>
#include <QStyleOptionGroupBox>
#include <QStyleOptionTab>
#include <QTabWidget>
#include <QTime>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "source/services/incore_10_task_list_widget.hpp"
#include "source/services/incore_13_prioridade_display.hpp"
#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/utils/IconUtils.hpp"

namespace source::InterfaceCore::incore_01_initUI
{
    namespace
    {

        constexpr int kQuadrantColumnHorizontalMargin = 0;
        constexpr int kQuadrantColumnTopMargin = 6;
        constexpr int kQuadrantColumnBottomMargin = 0;
        constexpr int kQuadrantColumnSpacing = 6;
        constexpr int kQuadrantPanelHorizontalInset = 2;
        constexpr int kQuadrantPanelTopInset = 12;
        constexpr int kQuadrantPanelBottomInset = 4;
        constexpr int kQuadrantColumnsGap = 2;

        QList<QColor> default_quadrant_tab_colors()
        {
            return {
                QColor(200, 50, 50),
                QColor(230, 140, 20),
                QColor(230, 210, 30),
                QColor(50, 170, 60),
            };
        }

        QColor completed_tab_color()
        {
            return QColor(45, 115, 215);
        }

        class BorderlessTabPaneStyle : public QProxyStyle
        {
        public:
            using QProxyStyle::QProxyStyle;

            void drawPrimitive(PrimitiveElement element,
                               const QStyleOption *option,
                               QPainter *painter,
                               const QWidget *widget = nullptr) const override
            {
                if (element == PE_FrameTabWidget)
                {
                    return;
                }

                QProxyStyle::drawPrimitive(element, option, painter, widget);
            }

            QRect subElementRect(SubElement element,
                                 const QStyleOption *option,
                                 const QWidget *widget = nullptr) const override
            {
                QRect rect = QProxyStyle::subElementRect(element, option, widget);
                if ((element == SE_TabWidgetTabPane || element == SE_TabWidgetTabContents) && option != nullptr && qobject_cast<const QTabWidget *>(widget) != nullptr)
                {
                    rect.setBottom(option->rect.bottom());
                }
                return rect;
            }
        };

        QString normalize_action_text(QString text)
        {
            text.replace(QStringLiteral("&"), QString());
            text = text.trimmed();
            if (text.endsWith(QStringLiteral("...")))
            {
                text.chop(3);
            }
            return text.trimmed().toLower();
        }

        void apply_section_group_state(QGroupBox *group, QWidget *content, bool expanded)
        {
            if (content != nullptr)
            {
                content->setVisible(expanded);
            }

            if (group == nullptr)
            {
                return;
            }

            bool is_horizontal = false;
            if (auto *parent = group->parentWidget())
            {
                if (auto *box = dynamic_cast<QBoxLayout *>(parent->layout()))
                {
                    is_horizontal = (box->direction() == QBoxLayout::LeftToRight || box->direction() == QBoxLayout::RightToLeft);
                }
            }

            QSizePolicy policy = group->sizePolicy();
            if (is_horizontal)
            {
                policy.setHorizontalPolicy(expanded ? QSizePolicy::Expanding : QSizePolicy::Fixed);
                policy.setVerticalPolicy(QSizePolicy::Expanding);
                if (!expanded)
                {
                    const int collapsed_w = group->minimumSizeHint().width();
                    group->setMinimumWidth(collapsed_w);
                    group->setMaximumWidth(collapsed_w);
                    group->setCursor(Qt::PointingHandCursor);
                }
                else
                {
                    group->setMinimumWidth(0);
                    group->setMaximumWidth(QWIDGETSIZE_MAX);
                    group->unsetCursor();
                }
            }
            else
            {
                policy.setHorizontalPolicy(QSizePolicy::Expanding);
                policy.setVerticalPolicy(expanded ? QSizePolicy::Expanding : QSizePolicy::Fixed);
                group->setMinimumWidth(0);
                group->setMaximumWidth(QWIDGETSIZE_MAX);
                group->unsetCursor();
            }
            group->setSizePolicy(policy);

            if (group->layout() != nullptr)
            {
                group->layout()->invalidate();
            }
            group->updateGeometry();
            group->update();
        }

        QLocale locale_for_date_input(source::EisenhowerMatrixApp *app)
        {
            if (app != nullptr && app->gerenciador_traducao != nullptr)
            {
                const QString idioma = app->gerenciador_traducao->obter_idioma_atual();
                if (idioma.startsWith(QStringLiteral("pt")))
                {
                    return QLocale(QLocale::Portuguese, QLocale::Brazil);
                }
                if (idioma.startsWith(QStringLiteral("en")))
                {
                    return QLocale(QLocale::English, QLocale::UnitedStates);
                }
            }

            return QLocale(QLocale::Portuguese, QLocale::Brazil);
        }

        QString date_display_format_for_app(source::EisenhowerMatrixApp *app)
        {
            if (app != nullptr && app->gerenciador_traducao != nullptr)
            {
                const QString idioma = app->gerenciador_traducao->obter_idioma_atual();
                if (idioma.startsWith(QStringLiteral("en")))
                {
                    return QStringLiteral("MM/dd/yyyy");
                }
            }

            return QStringLiteral("dd/MM/yyyy");
        }

        void apply_locale_to_date_input(source::EisenhowerMatrixApp *app)
        {
            if (app == nullptr || app->date_input == nullptr)
            {
                return;
            }

            const QLocale locale = locale_for_date_input(app);
            app->date_input->setLocale(locale);
            app->date_input->setDisplayFormat(date_display_format_for_app(app));
        }

        void localize_menu(QMenu *menu, source::EisenhowerMatrixApp *app)
        {
            if (menu == nullptr)
            {
                return;
            }

            const bool use_pt = (app == nullptr || app->gerenciador_traducao == nullptr || app->gerenciador_traducao->obter_idioma_atual().startsWith(QStringLiteral("pt")));

            const QMap<QString, QString> translations = {
                {QStringLiteral("undo"), QStringLiteral("Desfazer")},
                {QStringLiteral("redo"), QStringLiteral("Refazer")},
                {QStringLiteral("cut"), QStringLiteral("Recortar")},
                {QStringLiteral("copy"), QStringLiteral("Copiar")},
                {QStringLiteral("paste"), QStringLiteral("Colar")},
                {QStringLiteral("delete"), QStringLiteral("Excluir")},
                {QStringLiteral("select all"), QStringLiteral("Selecionar Tudo")},
                {QStringLiteral("clear"), QStringLiteral("Limpar")},
                {QStringLiteral("insert"), QStringLiteral("Inserir")},
                {QStringLiteral("today"), QStringLiteral("Hoje")},
                {QStringLiteral("none"), QStringLiteral("Nenhum")},
            };

            for (QAction *action : menu->actions())
            {
                if (action == nullptr)
                {
                    continue;
                }
                if (action->menu() != nullptr)
                {
                    localize_menu(action->menu(), app);
                    continue;
                }
                const QString key = normalize_action_text(action->text());
                if (use_pt && translations.contains(key))
                {
                    action->setText(translations.value(key));
                }
            }
        }

        QWidget *build_quadrant_tab(source::EisenhowerMatrixApp *app,
                                    QWidget *parent,
                                    QLabel *&active_label,
                                    QGroupBox *&active_group,
                                    QListWidget *&active_list,
                                    QLabel *&in_progress_label,
                                    QGroupBox *&in_progress_group,
                                    QListWidget *&in_progress_list,
                                    QLabel *&completed_label,
                                    QGroupBox *&completed_group,
                                    QListWidget *&completed_list)
        {
            auto *tab = new QWidget(parent);
            auto *layout = new QVBoxLayout(tab);
            layout->setContentsMargins(kQuadrantColumnHorizontalMargin,
                                       kQuadrantColumnTopMargin,
                                       kQuadrantColumnHorizontalMargin,
                                       kQuadrantColumnBottomMargin);
            layout->setSpacing(kQuadrantColumnSpacing);

            active_label = new QLabel(tab);
            active_label->hide();
            in_progress_label = new QLabel(tab);
            in_progress_label->hide();
            completed_label = new QLabel(tab);
            completed_label->hide();

            active_group = new RotatableQuadrantGroupBox(tab);
            active_group->setCheckable(true);
            active_group->setChecked(true);
            active_group->setFlat(true);
            active_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            auto *active_group_layout = new QVBoxLayout(active_group);
            active_group_layout->setContentsMargins(kQuadrantPanelHorizontalInset,
                                                    kQuadrantPanelTopInset,
                                                    kQuadrantPanelHorizontalInset,
                                                    kQuadrantPanelBottomInset);
            active_group_layout->setSpacing(6);
            active_list = new incore_10_task_list_widget::TaskListWidget(app, false, active_group);
            active_group_layout->addWidget(active_list, 1);

            in_progress_group = new RotatableQuadrantGroupBox(tab);
            in_progress_group->setCheckable(true);
            in_progress_group->setChecked(true);
            in_progress_group->setFlat(true);
            in_progress_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            auto *in_progress_group_layout = new QVBoxLayout(in_progress_group);
            in_progress_group_layout->setContentsMargins(kQuadrantPanelHorizontalInset,
                                                         kQuadrantPanelTopInset,
                                                         kQuadrantPanelHorizontalInset,
                                                         kQuadrantPanelBottomInset);
            in_progress_group_layout->setSpacing(6);
            in_progress_list = new incore_10_task_list_widget::TaskListWidget(app, false, in_progress_group);
            in_progress_group_layout->addWidget(in_progress_list, 1);

            completed_group = new RotatableQuadrantGroupBox(tab);
            completed_group->setCheckable(true);
            completed_group->setChecked(true);
            completed_group->setFlat(true);
            completed_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            auto *completed_group_layout = new QVBoxLayout(completed_group);
            completed_group_layout->setContentsMargins(kQuadrantPanelHorizontalInset,
                                                       kQuadrantPanelTopInset,
                                                       kQuadrantPanelHorizontalInset,
                                                       kQuadrantPanelBottomInset);
            completed_group_layout->setSpacing(6);
            completed_list = new incore_10_task_list_widget::TaskListWidget(app, true, completed_group);
            completed_group_layout->addWidget(completed_list, 1);

            auto refresh_collapsed_alignment = [layout, active_group, in_progress_group, completed_group]()
            {
                const bool active_visible = active_group != nullptr && active_group->isChecked();
                const bool in_progress_visible = in_progress_group != nullptr && in_progress_group->isChecked();
                const bool completed_visible = completed_group != nullptr && completed_group->isChecked();
                const bool any_visible = active_visible || in_progress_visible || completed_visible;

                layout->setStretch(0, active_visible ? 1 : 0);
                layout->setStretch(1, in_progress_visible ? 1 : 0);
                layout->setStretch(2, completed_visible ? 1 : 0);
                layout->setStretch(3, any_visible ? 0 : 1);
                layout->invalidate();
                if (active_group != nullptr)
                {
                    active_group->updateGeometry();
                    active_group->update();
                }
                if (in_progress_group != nullptr)
                {
                    in_progress_group->updateGeometry();
                    in_progress_group->update();
                }
                if (completed_group != nullptr)
                {
                    completed_group->updateGeometry();
                    completed_group->update();
                }
            };

            QObject::connect(active_group, &QGroupBox::toggled, tab, [active_group, active_list, refresh_collapsed_alignment](bool expanded)
                             {
        apply_section_group_state(active_group, active_list, expanded);
        refresh_collapsed_alignment(); });
            QObject::connect(in_progress_group, &QGroupBox::toggled, tab, [in_progress_group, in_progress_list, refresh_collapsed_alignment](bool expanded)
                             {
        apply_section_group_state(in_progress_group, in_progress_list, expanded);
        refresh_collapsed_alignment(); });
            QObject::connect(completed_group,
                             &QGroupBox::toggled,
                             tab,
                             [completed_group, completed_list, refresh_collapsed_alignment](bool expanded)
                             {
                                 apply_section_group_state(completed_group, completed_list, expanded);
                                 refresh_collapsed_alignment();
                             });

            layout->addWidget(active_group, 1);
            layout->addWidget(in_progress_group, 1);
            layout->addWidget(completed_group, 1);
            layout->addStretch(1);
            apply_section_group_state(active_group, active_list, active_group->isChecked());
            apply_section_group_state(in_progress_group, in_progress_list, in_progress_group->isChecked());
            apply_section_group_state(completed_group, completed_list, completed_group->isChecked());
            refresh_collapsed_alignment();
            return tab;
        }

        void open_linked_file(source::EisenhowerMatrixApp *app, QListWidgetItem *item)
        {
            if (app == nullptr || item == nullptr)
            {
                return;
            }
            app->handle_linked_file_open_request(item, app);
        }

    } // namespace

    QString get_text(const QString &text)
    {
        return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
    }

    RotatableQuadrantGroupBox::RotatableQuadrantGroupBox(QWidget *parent)
        : QGroupBox(parent)
    {
    }

    bool RotatableQuadrantGroupBox::is_horizontal_collapsed() const
    {
        if (isChecked())
        {
            return false;
        }
        if (auto *parent = parentWidget())
        {
            if (auto *box = dynamic_cast<QBoxLayout *>(parent->layout()))
            {
                return (box->direction() == QBoxLayout::LeftToRight || box->direction() == QBoxLayout::RightToLeft);
            }
        }
        return false;
    }

    QSize RotatableQuadrantGroupBox::minimumSizeHint() const
    {
        if (is_horizontal_collapsed())
        {
            const QFontMetrics fm = fontMetrics();
            const int w = qMax(30, fm.height() + 14);
            return QSize(w, 120);
        }
        return QGroupBox::minimumSizeHint();
    }

    QSize RotatableQuadrantGroupBox::sizeHint() const
    {
        if (is_horizontal_collapsed())
        {
            return minimumSizeHint();
        }
        return QGroupBox::sizeHint();
    }

    void RotatableQuadrantGroupBox::paintEvent(QPaintEvent *event)
    {
        if (!is_horizontal_collapsed())
        {
            QGroupBox::paintEvent(event);
            return;
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // 1. Query native frame geometry using QStyleOptionGroupBox (for subControlRect only)
        QStyleOptionGroupBox grp_opt;
        grp_opt.initFrom(this);
        grp_opt.text = QString();
        grp_opt.subControls = QStyle::SC_GroupBoxFrame | QStyle::SC_GroupBoxCheckBox;
        grp_opt.lineWidth = 1;

        QRect frame_rect = style()->subControlRect(QStyle::CC_GroupBox, &grp_opt, QStyle::SC_GroupBoxFrame, this);
        if (!frame_rect.isValid() || frame_rect.isEmpty())
        {
            const int top_offset = qMax(8, fontMetrics().height() / 2);
            frame_rect = rect().adjusted(0, top_offset, 0, 0);
        }

        // 2. Indicator dimensions and positioning: centered horizontally, vertically centered on frame top
        const int ind_w = qMax(14, style()->pixelMetric(QStyle::PM_IndicatorWidth, &grp_opt, this));
        const int ind_h = qMax(14, style()->pixelMetric(QStyle::PM_IndicatorHeight, &grp_opt, this));
        const int ind_x = qMax(1, (width() - ind_w) / 2);
        const int ind_y = frame_rect.top() - ind_h / 2;
        const QRect indicator_rect(ind_x, ind_y, ind_w, ind_h);

        // 3. Draw native frame using QStyleOptionFrame (PE_FrameGroupBox expects QStyleOptionFrame)
        const QRect gap_rect(ind_x - 3, frame_rect.top() - 4, ind_w + 6, ind_h + 8);
        QRegion clip_region(rect());
        clip_region = clip_region.subtracted(QRegion(gap_rect));

        QStyleOptionFrame frame_opt;
        frame_opt.initFrom(this);
        frame_opt.rect = frame_rect;
        frame_opt.lineWidth = 1;
        frame_opt.midLineWidth = 0;
        frame_opt.features = QStyleOptionFrame::None;

        painter.save();
        painter.setClipRegion(clip_region);
        style()->drawPrimitive(QStyle::PE_FrameGroupBox, &frame_opt, &painter, this);
        painter.restore();

        // 4. Draw Checkbox Indicator in the top gap
        QStyleOptionButton check_opt;
        check_opt.initFrom(this);
        check_opt.rect = indicator_rect;
        check_opt.state = QStyle::State_Enabled | QStyle::State_Off;
        if (underMouse())
        {
            check_opt.state |= QStyle::State_MouseOver;
        }
        style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &check_opt, &painter, this);

        // 5. Draw Rotated Text at 90° clockwise inside the column
        painter.save();
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        painter.setPen(palette().color(QPalette::WindowText));
        painter.setFont(font());

        const int text_start_y = indicator_rect.bottom() + 8;
        const int avail_h = qMax(10, frame_rect.bottom() - text_start_y - 4);

        painter.translate(width(), text_start_y);
        painter.rotate(90.0);

        painter.drawText(QRect(0, 0, avail_h, width()), Qt::AlignLeft | Qt::AlignVCenter, title());
        painter.restore();
    }

    void RotatableQuadrantGroupBox::mousePressEvent(QMouseEvent *event)
    {
        if (is_horizontal_collapsed())
        {
            if (event->button() == Qt::LeftButton)
            {
                event->accept();
                setChecked(true);
                return;
            }
        }
        QGroupBox::mousePressEvent(event);
    }

    void RotatableQuadrantGroupBox::changeEvent(QEvent *event)
    {
        QGroupBox::changeEvent(event);
        if (event != nullptr && (event->type() == QEvent::LanguageChange || event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::ThemeChange))
        {
            updateGeometry();
            update();
        }
    }

    CustomTimeEdit::CustomTimeEdit(QWidget *parent) : QTimeEdit(parent)
    {
        setWrapping(true);
    }

    void CustomTimeEdit::stepBy(int steps)
    {
        if (currentSection() == QDateTimeEdit::MinuteSection)
        {
            int total_minutes = time().hour() * 60 + time().minute() + steps;
            if (total_minutes < 0)
            {
                total_minutes = 1439;
            }
            else if (total_minutes >= 1440)
            {
                total_minutes = 0;
            }
            setTime(QTime(total_minutes / 60, total_minutes % 60, time().second()));
            return;
        }
        QTimeEdit::stepBy(steps);
    }

    FileDropLineEdit::FileDropLineEdit(source::EisenhowerMatrixApp *app, QWidget *parent)
        : QLineEdit(parent), app_(app)
    {
        setAcceptDrops(true);
    }

    void FileDropLineEdit::dragEnterEvent(QDragEnterEvent *event)
    {
        if (event->mimeData()->hasUrls())
        {
            event->acceptProposedAction();
            return;
        }
        QLineEdit::dragEnterEvent(event);
    }

    void FileDropLineEdit::dragMoveEvent(QDragMoveEvent *event)
    {
        if (event->mimeData()->hasUrls())
        {
            event->acceptProposedAction();
            return;
        }
        QLineEdit::dragMoveEvent(event);
    }

    void FileDropLineEdit::dropEvent(QDropEvent *event)
    {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty())
        {
            const QString path = urls.first().toLocalFile();
            if (!path.isEmpty())
            {
                setText(QFileInfo(path).fileName());
                setToolTip(path);
                if (app_ != nullptr)
                {
                    app_->setProperty("_dragged_file_path", path);
                }
            }
            event->acceptProposedAction();
            return;
        }
        QLineEdit::dropEvent(event);
    }

    LocalizedDateEdit::LocalizedDateEdit(QWidget *parent) : QDateEdit(parent) {}

    void LocalizedDateEdit::contextMenuEvent(QContextMenuEvent *event)
    {
        if (lineEdit() != nullptr)
        {
            QMenu *menu = lineEdit()->createStandardContextMenu();
            localize_menu(menu, qobject_cast<source::EisenhowerMatrixApp *>(window()));
            menu->exec(event->globalPos());
            delete menu;
            return;
        }
        QDateEdit::contextMenuEvent(event);
    }

    LocalizedTimeEdit::LocalizedTimeEdit(QWidget *parent) : CustomTimeEdit(parent) {}

    void LocalizedTimeEdit::contextMenuEvent(QContextMenuEvent *event)
    {
        if (lineEdit() != nullptr)
        {
            QMenu *menu = lineEdit()->createStandardContextMenu();
            localize_menu(menu, qobject_cast<source::EisenhowerMatrixApp *>(window()));
            menu->exec(event->globalPos());
            delete menu;
            return;
        }
        QTimeEdit::contextMenuEvent(event);
    }

    LightActiveTabBar::LightActiveTabBar(QWidget *parent) : QTabBar(parent)
    {
        setMouseTracking(true);
        setAttribute(Qt::WA_Hover, true);
    }

    void LightActiveTabBar::set_quadrant_style(QProxyStyle *style)
    {
        quadrant_style_ = style;
        if (style != nullptr)
        {
            setStyle(style);
        }
    }

    void LightActiveTabBar::paintEvent(QPaintEvent *event)
    {
        QTabBar::paintEvent(event);
    }

    QSize LightActiveTabBar::tabSizeHint(int index) const
    {
        QSize size = QTabBar::tabSizeHint(index);
        const int minimum_height = fontMetrics().height() + 10;
        size.setHeight(qMax(minimum_height, size.height() - 4));
        return size;
    }

    void QuadrantTabStyle::set_tab_accent_colors(const QList<QColor> &colors)
    {
        tab_accent_colors_ = colors;
    }

    void QuadrantTabStyle::drawPrimitive(PrimitiveElement element,
                                         const QStyleOption *option,
                                         QPainter *painter,
                                         const QWidget *widget) const
    {
        if (element == PE_FrameTabBarBase)
        {
            return;
        }

        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }

    bool QuadrantTabStyle::is_light(const QColor &color) const
    {
        return color.lightnessF() >= 0.6;
    }

    QColor QuadrantTabStyle::hover_color(const QPalette &palette) const
    {
        QColor hover = palette.color(QPalette::Button);
        hover = is_light(hover) ? hover.darker(110) : hover.lighter(115);
        hover.setAlpha(90);
        return hover;
    }

    QColor QuadrantTabStyle::text_color(const QColor &background) const
    {
        return is_light(background) ? QColor(Qt::black) : QColor(Qt::white);
    }

    QColor QuadrantTabStyle::quadrant_accent_color(const QStyleOption *option, const QWidget *widget) const
    {
        if (option == nullptr)
        {
            return {};
        }

        QColor color;
        if (const auto *tab_bar = qobject_cast<const QTabBar *>(widget))
        {
            const int idx = tab_bar->tabAt(option->rect.center());
            const QList<QColor> colors = tab_accent_colors_.isEmpty() ? default_quadrant_tab_colors() : tab_accent_colors_;
            if (idx >= 0 && idx < colors.size())
            {
                color = colors.at(idx);
            }
        }

        return color;
    }

    QColor QuadrantTabStyle::inactive_tab_color(const QStyleOption *option, const QWidget *widget) const
    {
        if (option == nullptr)
        {
            return {};
        }

        const auto blend_colors = [](const QColor &base, const QColor &accent, qreal accent_ratio)
        {
            const qreal clamped_ratio = qBound(0.0, accent_ratio, 1.0);
            const qreal base_ratio = 1.0 - clamped_ratio;
            return QColor::fromRgbF(base.redF() * base_ratio + accent.redF() * clamped_ratio,
                                    base.greenF() * base_ratio + accent.greenF() * clamped_ratio,
                                    base.blueF() * base_ratio + accent.blueF() * clamped_ratio,
                                    1.0);
        };

        const QColor neutral_button = option->palette.color(QPalette::Button);
        const bool light_theme = is_light(option->palette.color(QPalette::Window));
        QColor color = neutral_button;
        const QColor accent = quadrant_accent_color(option, widget);
        if (accent.isValid())
        {
            color = blend_colors(neutral_button, accent, light_theme ? 0.08 : 0.10);
        }

        if (light_theme)
        {
            color = color.darker(98);
            color.setAlpha(205);
        }
        else
        {
            color = is_light(color) ? color.darker(145) : color.darker(125);
            color.setAlpha(225);
        }
        return color;
    }

    QColor QuadrantTabStyle::hovered_tab_color(const QStyleOption *option, const QWidget *widget) const
    {
        if (option == nullptr)
        {
            return {};
        }

        const auto blend_colors = [](const QColor &base, const QColor &accent, qreal accent_ratio)
        {
            const qreal clamped_ratio = qBound(0.0, accent_ratio, 1.0);
            const qreal base_ratio = 1.0 - clamped_ratio;
            return QColor::fromRgbF(base.redF() * base_ratio + accent.redF() * clamped_ratio,
                                    base.greenF() * base_ratio + accent.greenF() * clamped_ratio,
                                    base.blueF() * base_ratio + accent.blueF() * clamped_ratio,
                                    1.0);
        };

        const QColor neutral_button = option->palette.color(QPalette::Button);
        const QColor accent = quadrant_accent_color(option, widget);
        QColor color = accent.isValid() ? blend_colors(neutral_button, accent, 0.26) : hover_color(option->palette);

        color = is_light(color) ? color.darker(112) : color.lighter(108);
        color.setAlpha(180);
        return color;
    }

    QColor QuadrantTabStyle::selected_quadrant_color(const QStyleOption *option, const QWidget *widget) const
    {
        if (option == nullptr)
        {
            return {};
        }

        const auto blend_colors = [](const QColor &base, const QColor &accent, qreal accent_ratio)
        {
            const qreal clamped_ratio = qBound(0.0, accent_ratio, 1.0);
            const qreal base_ratio = 1.0 - clamped_ratio;
            return QColor::fromRgbF(base.redF() * base_ratio + accent.redF() * clamped_ratio,
                                    base.greenF() * base_ratio + accent.greenF() * clamped_ratio,
                                    base.blueF() * base_ratio + accent.blueF() * clamped_ratio,
                                    1.0);
        };

        QColor color = quadrant_accent_color(option, widget);
        if (!color.isValid())
        {
            color = hover_color(option->palette);
        }
        else
        {
            const QColor neutral_button = option->palette.color(QPalette::Button);
            color = blend_colors(neutral_button, color, 0.38);
            color = is_light(color) ? color.darker(108) : color.darker(116);
            color.setAlpha(92);
        }

        return color;
    }

    QPalette QuadrantTabStyle::selected_tab_palette(const QPalette &base_palette, const QColor &fill_color) const
    {
        QPalette palette(base_palette);
        const QColor foreground = text_color(fill_color);
        auto with_alpha = [fill_color](QColor color)
        {
            color.setAlpha(fill_color.alpha());
            return color;
        };

        for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled})
        {
            palette.setColor(group, QPalette::Button, fill_color);
            palette.setColor(group, QPalette::Window, fill_color);
            palette.setColor(group, QPalette::Light, with_alpha(fill_color.lighter(125)));
            palette.setColor(group, QPalette::Midlight, with_alpha(fill_color.lighter(110)));
            palette.setColor(group, QPalette::Mid, with_alpha(fill_color.darker(115)));
            palette.setColor(group, QPalette::Dark, with_alpha(fill_color.darker(150)));
            palette.setColor(group, QPalette::Shadow, with_alpha(fill_color.darker(190)));
            palette.setColor(group, QPalette::ButtonText, foreground);
            palette.setColor(group, QPalette::Text, foreground);
            palette.setColor(group, QPalette::WindowText, foreground);
            palette.setColor(group, QPalette::HighlightedText, foreground);
        }

        return palette;
    }

    void QuadrantTabStyle::drawControl(ControlElement element,
                                       const QStyleOption *option,
                                       QPainter *painter,
                                       const QWidget *widget) const
    {
        if ((element == CE_TabBarTabShape || element == CE_TabBarTabLabel) && option != nullptr && painter != nullptr)
        {
            if (const auto *tab_option = qstyleoption_cast<const QStyleOptionTab *>(option))
            {
                QStyleOptionTab styled_option(*tab_option);
                const QColor fill_color = (option->state & State_Selected)
                                              ? selected_quadrant_color(option, widget)
                                          : (option->state & State_MouseOver)
                                              ? hovered_tab_color(option, widget)
                                              : inactive_tab_color(option, widget);
                styled_option.palette = selected_tab_palette(styled_option.palette, fill_color);
                QProxyStyle::drawControl(element, &styled_option, painter, widget);
                return;
            }
        }

        QProxyStyle::drawControl(element, option, painter, widget);
    }

    int QuadrantTabStyle::pixelMetric(PixelMetric metric,
                                      const QStyleOption *option,
                                      const QWidget *widget) const
    {
        const int base_value = QProxyStyle::pixelMetric(metric, option, widget);
        if (qobject_cast<const QTabBar *>(widget) == nullptr)
        {
            return base_value;
        }

        switch (metric)
        {
        case PM_TabBarTabHSpace:
            return qMax(14, base_value - 12);
        case PM_TabBarTabVSpace:
            return qMax(8, base_value - 2);
        default:
            return base_value;
        }
    }

    QRect QuadrantTabStyle::subElementRect(SubElement element,
                                           const QStyleOption *option,
                                           const QWidget *widget) const
    {
        QRect rect = QProxyStyle::subElementRect(element, option, widget);
        if (element == SE_TabBarTabText && qobject_cast<const QTabBar *>(widget) != nullptr)
        {
            rect.adjust(0, -1, 0, 1);
        }
        return rect;
    }

    int QuadrantTabStyle::styleHint(StyleHint hint,
                                    const QStyleOption *option,
                                    const QWidget *widget,
                                    QStyleHintReturn *returnData) const
    {
        if (hint == SH_TabBar_Alignment && qobject_cast<const QTabBar *>(widget) != nullptr)
        {
            return Qt::AlignCenter;
        }

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }

    AutoSwitchTabWidget::AutoSwitchTabWidget(QWidget *parent) : QTabWidget(parent)
    {
        pane_style_ = new BorderlessTabPaneStyle();
        pane_style_->setParent(this);
        setStyle(pane_style_);

        tab_bar_ = new LightActiveTabBar(this);
        setTabBar(tab_bar_);
        tab_bar_->setExpanding(false);
        tab_bar_->setDrawBase(false);
        tab_bar_->setAcceptDrops(true);
        tab_bar_->installEventFilter(this);
        quadrant_style_ = new QuadrantTabStyle(QStringLiteral("Fusion"));
        quadrant_style_->setParent(tab_bar_);
        tab_bar_->set_quadrant_style(quadrant_style_);
    }

    void AutoSwitchTabWidget::set_app(source::EisenhowerMatrixApp *app)
    {
        app_ = app;
    }

    void AutoSwitchTabWidget::set_drop_targets(const QList<QListWidget *> &targets)
    {
        drop_targets_ = targets;
    }

    void AutoSwitchTabWidget::set_tab_accent_colors(const QList<QColor> &colors)
    {
        tab_accent_colors_ = colors;
        if (quadrant_style_ != nullptr)
        {
            quadrant_style_->set_tab_accent_colors(tab_accent_colors_);
        }
        if (tab_bar_ != nullptr)
        {
            tab_bar_->update();
        }
    }

    void AutoSwitchTabWidget::refresh_quadrant_style()
    {
        QProxyStyle *previous_style = quadrant_style_;
        quadrant_style_ = new QuadrantTabStyle(QStringLiteral("Fusion"));
        quadrant_style_->set_tab_accent_colors(tab_accent_colors_);
        quadrant_style_->setParent(tab_bar_);
        if (tab_bar_ != nullptr)
        {
            tab_bar_->set_quadrant_style(quadrant_style_);
            tab_bar_->update();
        }
        if (previous_style != nullptr)
        {
            previous_style->deleteLater();
        }
        update();
    }

    QPoint AutoSwitchTabWidget::event_pos(QEvent *event) const
    {
        if (auto *drop = dynamic_cast<QDropEvent *>(event))
        {
            return drop->position().toPoint();
        }
        if (auto *drag = dynamic_cast<QDragMoveEvent *>(event))
        {
            return drag->position().toPoint();
        }
        return QPoint();
    }

    bool AutoSwitchTabWidget::is_completed_list(QListWidget *list) const
    {
        return app_ != nullptr && app_->is_completed_list(list);
    }

    QListWidget *AutoSwitchTabWidget::target_list_for_index(int index, QListWidget *source_list) const
    {
        if (app_ == nullptr || index < 0)
        {
            return nullptr;
        }

        Q_UNUSED(source_list);

        if (index < drop_targets_.size())
        {
            return drop_targets_.at(index);
        }

        if (index > 3)
        {
            return nullptr;
        }

        const QList<QListWidget *> pending = {app_->quadrant1_list, app_->quadrant2_list, app_->quadrant3_list, app_->quadrant4_list};
        return pending.at(index);
    }

    void AutoSwitchTabWidget::handle_tab_drop(QDropEvent *event, int index)
    {
        if (app_ == nullptr || event == nullptr)
        {
            return;
        }

        auto *source = qobject_cast<QListWidget *>(event->source());
        if (source == nullptr)
        {
            if (auto *source_widget = qobject_cast<QWidget *>(event->source()))
            {
                source = qobject_cast<QListWidget *>(source_widget->parentWidget());
            }
        }

        QListWidget *target = target_list_for_index(index, source);
        if (source == nullptr || target == nullptr || source == target)
        {
            return;
        }

        QList<QListWidgetItem *> items;
        for (QListWidgetItem *item : source->selectedItems())
        {
            if (item != nullptr && (item->flags() & Qt::ItemIsSelectable))
            {
                items << item;
            }
        }
        if (items.isEmpty())
        {
            return;
        }

        const Qt::CheckState new_state = is_completed_list(target) ? Qt::Checked : Qt::Unchecked;
        source->blockSignals(true);
        target->blockSignals(true);
        for (QListWidgetItem *item : items)
        {
            app_->move_item_between_lists(item, source, target, new_state);
        }
        source->blockSignals(false);
        target->blockSignals(false);
        app_->save_tasks();
        if (app_->calendar_pane != nullptr && app_->calendar_pane->calendar_panel != nullptr)
        {
            app_->calendar_pane->calendar_panel->update_task_list();
        }
        event->acceptProposedAction();
    }

    bool AutoSwitchTabWidget::eventFilter(QObject *obj, QEvent *event)
    {
        if (obj == tab_bar_ && event != nullptr && (event->type() == QEvent::DragEnter || event->type() == QEvent::DragMove))
        {
            if (auto *drag_enter = dynamic_cast<QDragEnterEvent *>(event))
            {
                drag_enter->acceptProposedAction();
            }
            if (auto *drag_move = dynamic_cast<QDragMoveEvent *>(event))
            {
                drag_move->acceptProposedAction();
            }

            const int idx = tab_bar_->tabAt(event_pos(event));
            if (idx >= 0 && idx != currentIndex())
            {
                setCurrentIndex(idx);
            }
            return true;
        }

        if (obj == tab_bar_ && event != nullptr && event->type() == QEvent::Drop)
        {
            int idx = tab_bar_->tabAt(event_pos(event));
            if (idx < 0)
            {
                idx = currentIndex();
            }
            if (idx >= 0)
            {
                setCurrentIndex(idx);
                handle_tab_drop(static_cast<QDropEvent *>(event), idx);
                return true;
            }
        }

        return QTabWidget::eventFilter(obj, event);
    }

    class AutoHeightScrollArea : public QScrollArea
    {
    public:
        explicit AutoHeightScrollArea(QWidget *parent = nullptr) : QScrollArea(parent)
        {
            setWidgetResizable(true);
            setFrameShape(QFrame::NoFrame);
            setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

            if (QScrollBar *hbar = horizontalScrollBar())
            {
                QObject::connect(hbar, &QScrollBar::rangeChanged, this, [this](int, int)
                                 { _notify_geometry_change(); });
            }
        }

        QSize sizeHint() const override
        {
            QSize s = QScrollArea::sizeHint();
            if (widget() != nullptr)
            {
                int h = widget()->sizeHint().height();
                if (horizontalScrollBar() != nullptr && horizontalScrollBar()->isVisible())
                {
                    h += horizontalScrollBar()->sizeHint().height();
                }
                s.setHeight(h + frameWidth() * 2);
                s.setWidth(widget()->sizeHint().width() + frameWidth() * 2);
            }
            return s;
        }

        QSize minimumSizeHint() const override
        {
            QSize s = QScrollArea::minimumSizeHint();
            if (widget() != nullptr)
            {
                int h = widget()->minimumSizeHint().height();
                if (horizontalScrollBar() != nullptr && horizontalScrollBar()->isVisible())
                {
                    h += horizontalScrollBar()->sizeHint().height();
                }
                s.setHeight(h + frameWidth() * 2);
                s.setWidth(widget()->minimumSizeHint().width() + frameWidth() * 2);
            }
            return s;
        }

    protected:
        void resizeEvent(QResizeEvent *event) override
        {
            QScrollArea::resizeEvent(event);
            _notify_geometry_change();
        }

    private:
        void _notify_geometry_change()
        {
            updateGeometry();
            if (parentWidget() != nullptr)
            {
                parentWidget()->updateGeometry();
                if (parentWidget()->layout() != nullptr)
                {
                    parentWidget()->layout()->invalidate();
                    parentWidget()->layout()->activate();
                }
                if (parentWidget()->parentWidget() != nullptr)
                {
                    parentWidget()->parentWidget()->updateGeometry();
                    if (parentWidget()->parentWidget()->layout() != nullptr)
                    {
                        parentWidget()->parentWidget()->layout()->invalidate();
                        parentWidget()->parentWidget()->layout()->activate();
                    }
                }
            }
        }
    };

    void init_ui(source::EisenhowerMatrixApp *app)
    {
        if (app == nullptr)
        {
            return;
        }

        auto *central = new QWidget(app);
        auto *main_layout = new QVBoxLayout(central);
        main_layout->setContentsMargins(2, 6, 6, 6);
        main_layout->setSpacing(6);

        app->new_task_group = new QGroupBox(get_text(QStringLiteral("Nova Tarefa")), central);
        app->new_task_group->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        auto *new_task_group_layout = new QVBoxLayout(app->new_task_group);
        new_task_group_layout->setContentsMargins(6, 6, 6, 6);
        new_task_group_layout->setSpacing(0);

        app->new_task_scroll = new AutoHeightScrollArea(app->new_task_group);

        app->new_task_content = new QWidget(app->new_task_scroll);
        app->new_task_content->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        auto *new_task_layout = new QVBoxLayout(app->new_task_content);
        new_task_layout->setContentsMargins(2, 2, 2, 2);
        new_task_layout->setSpacing(6);
        new_task_layout->setSizeConstraint(QLayout::SetMinimumSize);

        auto *input_row = new QHBoxLayout();
        app->task_input = new FileDropLineEdit(app, app->new_task_content);
        app->task_input->setPlaceholderText(get_text(QStringLiteral("Adicione uma tarefa ou arraste um arquivo...")));
        app->task_input->setMinimumWidth(180);
        input_row->addWidget(app->task_input, 1);

        app->add_button = new QPushButton(get_text(QStringLiteral("Adicionar Tarefa")), app->new_task_content);
        app->add_task_button = app->add_button;
        const QString add_icon_path = source::utils::get_icon_path(QStringLiteral("organizador.png"));
        if (!add_icon_path.isEmpty())
        {
            app->add_button->setIcon(QIcon(add_icon_path));
        }
        QObject::connect(app->add_button, &QPushButton::clicked, app, [app]()
                         { app->add_task(); });
        input_row->addWidget(app->add_button);

        app->calendar_button = new QPushButton(get_text(QStringLiteral("Calendário")), app->new_task_content);
        const QString calendar_icon_path = source::utils::get_icon_path(QStringLiteral("calendar.png"));
        if (!calendar_icon_path.isEmpty())
        {
            app->calendar_button->setIcon(QIcon(calendar_icon_path));
        }
        QObject::connect(app->calendar_button, &QPushButton::clicked, app, [app]()
                         { app->open_calendar(); });
        input_row->addWidget(app->calendar_button);

        new_task_layout->addLayout(input_row);

        auto *second_row = new QHBoxLayout();
        second_row->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        second_row->setSpacing(8);
        second_row->setSizeConstraint(QLayout::SetMinimumSize);

        app->date_checkbox = new QCheckBox(get_text(QStringLiteral("Vincular data")), app->new_task_content);
        app->date_checkbox->setChecked(true);
        app->date_checkbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        second_row->addWidget(app->date_checkbox);

        app->date_input = new LocalizedDateEdit(app->new_task_content);
        app->date_input->setCalendarPopup(true);
        app->date_input->setDate(QDate::currentDate());
        apply_locale_to_date_input(app);
        app->date_input->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        second_row->addWidget(app->date_input);

        if (app->gerenciador_traducao != nullptr)
        {
            QObject::connect(app->gerenciador_traducao,
                             &source::language::GerenciadorTraducao::idioma_alterado,
                             app,
                             [app](const QString &)
                             { apply_locale_to_date_input(app); });
        }

        app->time_checkbox = new QCheckBox(get_text(QStringLiteral("Vincular horário")), app->new_task_content);
        app->time_checkbox->setChecked(true);
        app->time_checkbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        second_row->addWidget(app->time_checkbox);

        app->time_input = new LocalizedTimeEdit(app->new_task_content);
        app->time_input->setDisplayFormat(QStringLiteral("HH:mm"));
        app->time_input->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        second_row->addWidget(app->time_input);

        QObject::connect(app->time_checkbox, &QCheckBox::toggled, app, [app](bool checked)
                         {
        if (app->time_input != nullptr && app->date_checkbox != nullptr) {
            app->time_input->setEnabled(checked && app->date_checkbox->isChecked());
        } });
        QObject::connect(app->date_checkbox, &QCheckBox::toggled, app, [app](bool checked)
                         {
        if (app->time_checkbox != nullptr) {
            app->time_checkbox->setEnabled(checked);
            if (!checked) {
                app->time_checkbox->setChecked(false);
            }
        }
        if (app->time_input != nullptr && app->time_checkbox != nullptr) {
            app->time_input->setEnabled(checked && app->time_checkbox->isChecked());
        } });

        app->duration_label = new QLabel(get_text(QStringLiteral("Duração:")), app->new_task_content);
        app->duration_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        second_row->addWidget(app->duration_label);

        app->duration_spin = new QDoubleSpinBox(app->new_task_content);
        app->duration_spin->setRange(0.0, 1000.0);
        app->duration_spin->setValue(1.0);
        app->duration_spin->setSingleStep(0.5);
        app->duration_spin->setSuffix(get_text(QStringLiteral(" dias")));
        app->duration_spin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        second_row->addWidget(app->duration_spin);

        app->priority_label = new QLabel(get_text(QStringLiteral("Prioridade:")), app->new_task_content);
        app->priority_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        second_row->addWidget(app->priority_label);

        app->quadrant_selector = new QComboBox(app->new_task_content);
        app->quadrant_selector->addItems({
            get_text(QStringLiteral("🔴 Importante e Urgente")),
            get_text(QStringLiteral("🟠 Importante, mas Não Urgente")),
            get_text(QStringLiteral("🟡 Não Importante, mas Urgente")),
            get_text(QStringLiteral("🟢 Não Importante e Não Urgente")),
        });
        app->quadrant_selector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        app->quadrant_selector->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        second_row->addWidget(app->quadrant_selector);

        app->domain_label = new QLabel(get_text(QStringLiteral("Nível de Domínio:")), app->new_task_content);
        app->domain_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        second_row->addWidget(app->domain_label);

        app->domain_selector = new QComboBox(app->new_task_content);
        app->domain_selector->addItems({
            source::InterfaceCore::dominio_para_texto(0, false),
            source::InterfaceCore::dominio_para_texto(1, false),
            source::InterfaceCore::dominio_para_texto(2, false),
        });
        app->domain_selector->setCurrentIndex(0);
        app->domain_selector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        app->domain_selector->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        second_row->addWidget(app->domain_selector);
        second_row->addStretch(1);

        new_task_layout->addLayout(second_row);
        app->new_task_content->setMinimumWidth(1080);
        app->new_task_scroll->setWidget(app->new_task_content);
        new_task_group_layout->addWidget(app->new_task_scroll);
        main_layout->addWidget(app->new_task_group);

        app->task_layout_host = new QWidget(central);
        app->task_layout_stack = new QStackedLayout(app->task_layout_host);
        app->task_layout_stack->setContentsMargins(0, 0, 0, 0);
        app->task_layout_stack->setSpacing(0);

        app->quadrant_tabs_page = new QWidget(app->task_layout_host);
        auto *tabs_page_layout = new QVBoxLayout(app->quadrant_tabs_page);
        tabs_page_layout->setContentsMargins(0, 0, 0, 0);
        tabs_page_layout->setSpacing(0);

        auto *tabs = new AutoSwitchTabWidget(app->quadrant_tabs_page);
        tabs->set_app(app);
        app->quadrant_tabs = tabs;
        tabs_page_layout->addWidget(tabs, 1);

        app->quadrants_page = new QWidget(app->task_layout_host);
        app->quadrants_layout = new QHBoxLayout(app->quadrants_page);
        app->quadrants_layout->setContentsMargins(0, 0, 0, 0);
        app->quadrants_layout->setSpacing(kQuadrantColumnsGap);

        app->matrix2x2_page = new QWidget(app->task_layout_host);
        app->matrix2x2_layout = new QGridLayout(app->matrix2x2_page);
        app->matrix2x2_layout->setContentsMargins(0, 0, 0, 0);
        app->matrix2x2_layout->setHorizontalSpacing(kQuadrantColumnsGap);
        app->matrix2x2_layout->setVerticalSpacing(kQuadrantColumnsGap);
        app->matrix2x2_layout->setRowStretch(0, 1);
        app->matrix2x2_layout->setRowStretch(1, 1);
        app->matrix2x2_layout->setColumnStretch(0, 1);
        app->matrix2x2_layout->setColumnStretch(1, 1);

        app->quadrant1_tab = build_quadrant_tab(app,
                                                tabs,
                                                app->quadrant1_label,
                                                app->quadrant1_group,
                                                app->quadrant1_list,
                                                app->quadrant1_in_progress_label,
                                                app->quadrant1_in_progress_group,
                                                app->quadrant1_in_progress_list,
                                                app->quadrant1_completed_label,
                                                app->quadrant1_completed_group,
                                                app->quadrant1_completed_list);
        app->quadrant2_tab = build_quadrant_tab(app,
                                                tabs,
                                                app->quadrant2_label,
                                                app->quadrant2_group,
                                                app->quadrant2_list,
                                                app->quadrant2_in_progress_label,
                                                app->quadrant2_in_progress_group,
                                                app->quadrant2_in_progress_list,
                                                app->quadrant2_completed_label,
                                                app->quadrant2_completed_group,
                                                app->quadrant2_completed_list);
        app->quadrant3_tab = build_quadrant_tab(app,
                                                tabs,
                                                app->quadrant3_label,
                                                app->quadrant3_group,
                                                app->quadrant3_list,
                                                app->quadrant3_in_progress_label,
                                                app->quadrant3_in_progress_group,
                                                app->quadrant3_in_progress_list,
                                                app->quadrant3_completed_label,
                                                app->quadrant3_completed_group,
                                                app->quadrant3_completed_list);
        app->quadrant4_tab = build_quadrant_tab(app,
                                                tabs,
                                                app->quadrant4_label,
                                                app->quadrant4_group,
                                                app->quadrant4_list,
                                                app->quadrant4_in_progress_label,
                                                app->quadrant4_in_progress_group,
                                                app->quadrant4_in_progress_list,
                                                app->quadrant4_completed_label,
                                                app->quadrant4_completed_group,
                                                app->quadrant4_completed_list);

        app->quadrant1_label->setText(get_text(QStringLiteral("🔴 Importante e Urgente")));
        app->quadrant2_label->setText(get_text(QStringLiteral("🟠 Importante, mas Não Urgente")));
        app->quadrant3_label->setText(get_text(QStringLiteral("🟡 Não Importante, mas Urgente")));
        app->quadrant4_label->setText(get_text(QStringLiteral("🟢 Não Importante e Não Urgente")));
        app->quadrant1_in_progress_label->setText(get_text(QStringLiteral("🔴 Em Progresso")));
        app->quadrant2_in_progress_label->setText(get_text(QStringLiteral("🟠 Em Progresso")));
        app->quadrant3_in_progress_label->setText(get_text(QStringLiteral("🟡 Em Progresso")));
        app->quadrant4_in_progress_label->setText(get_text(QStringLiteral("🟢 Em Progresso")));
        app->quadrant1_completed_label->setText(get_text(QStringLiteral("🔴 Concluídas")));
        app->quadrant2_completed_label->setText(get_text(QStringLiteral("🟠 Concluídas")));
        app->quadrant3_completed_label->setText(get_text(QStringLiteral("🟡 Concluídas")));
        app->quadrant4_completed_label->setText(get_text(QStringLiteral("🟢 Concluídas")));

        const auto make_status_tabs = [app](QWidget *parent, int quadrant_index)
        {
            auto *status_tabs = new AutoSwitchTabWidget(parent);
            status_tabs->set_app(app);
            const QList<QColor> quadrant_colors = default_quadrant_tab_colors();
            status_tabs->set_tab_accent_colors({quadrant_colors.value(quadrant_index), completed_tab_color()});
            status_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            return status_tabs;
        };

        app->quadrant1_status_tabs = make_status_tabs(app->matrix2x2_page, 0);
        app->quadrant2_status_tabs = make_status_tabs(app->matrix2x2_page, 1);
        app->quadrant3_status_tabs = make_status_tabs(app->matrix2x2_page, 2);
        app->quadrant4_status_tabs = make_status_tabs(app->matrix2x2_page, 3);

        if (app->quadrant1_group != nullptr)
        {
            app->quadrant1_group->setTitle(app->quadrant1_label->text());
        }
        if (app->quadrant2_group != nullptr)
        {
            app->quadrant2_group->setTitle(app->quadrant2_label->text());
        }
        if (app->quadrant3_group != nullptr)
        {
            app->quadrant3_group->setTitle(app->quadrant3_label->text());
        }
        if (app->quadrant4_group != nullptr)
        {
            app->quadrant4_group->setTitle(app->quadrant4_label->text());
        }
        if (app->quadrant1_completed_group != nullptr)
        {
            app->quadrant1_completed_group->setTitle(app->quadrant1_completed_label->text());
        }
        if (app->quadrant2_completed_group != nullptr)
        {
            app->quadrant2_completed_group->setTitle(app->quadrant2_completed_label->text());
        }
        if (app->quadrant3_completed_group != nullptr)
        {
            app->quadrant3_completed_group->setTitle(app->quadrant3_completed_label->text());
        }
        if (app->quadrant4_completed_group != nullptr)
        {
            app->quadrant4_completed_group->setTitle(app->quadrant4_completed_label->text());
        }

        tabs->addTab(app->quadrant1_tab, get_text(QStringLiteral("🔴 Importante e Urgente")));
        tabs->addTab(app->quadrant2_tab, get_text(QStringLiteral("🟠 Importante, mas Não Urgente")));
        tabs->addTab(app->quadrant3_tab, get_text(QStringLiteral("🟡 Não Importante, mas Urgente")));
        tabs->addTab(app->quadrant4_tab, get_text(QStringLiteral("🟢 Não Importante e Não Urgente")));

        app->quadrant_tabs_scroll = new QScrollArea(app->task_layout_host);
        app->quadrant_tabs_scroll->setWidgetResizable(true);
        app->quadrant_tabs_scroll->setFrameShape(QFrame::NoFrame);
        app->quadrant_tabs_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        app->quadrant_tabs_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        app->quadrant_tabs_scroll->setWidget(app->quadrant_tabs_page);

        app->quadrants_scroll = new QScrollArea(app->task_layout_host);
        app->quadrants_scroll->setWidgetResizable(true);
        app->quadrants_scroll->setFrameShape(QFrame::NoFrame);
        app->quadrants_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        app->quadrants_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        app->quadrants_scroll->setWidget(app->quadrants_page);

        app->matrix2x2_scroll = new QScrollArea(app->task_layout_host);
        app->matrix2x2_scroll->setWidgetResizable(true);
        app->matrix2x2_scroll->setFrameShape(QFrame::NoFrame);
        app->matrix2x2_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        app->matrix2x2_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        app->matrix2x2_scroll->setWidget(app->matrix2x2_page);

        app->task_layout_stack->addWidget(app->quadrant_tabs_scroll);
        app->task_layout_stack->addWidget(app->quadrants_scroll);
        app->task_layout_stack->addWidget(app->matrix2x2_scroll);
        main_layout->addWidget(app->task_layout_host, 1);

        app->add_placeholder(app->quadrant1_list, get_text(QStringLiteral("1º Quadrante")));
        app->add_placeholder(app->quadrant2_list, get_text(QStringLiteral("2º Quadrante")));
        app->add_placeholder(app->quadrant3_list, get_text(QStringLiteral("3º Quadrante")));
        app->add_placeholder(app->quadrant4_list, get_text(QStringLiteral("4º Quadrante")));
        app->add_placeholder(app->quadrant1_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
        app->add_placeholder(app->quadrant2_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
        app->add_placeholder(app->quadrant3_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
        app->add_placeholder(app->quadrant4_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));

        tabs->set_drop_targets({app->quadrant1_list, app->quadrant2_list, app->quadrant3_list, app->quadrant4_list});
        if (auto *status_tabs = dynamic_cast<AutoSwitchTabWidget *>(app->quadrant1_status_tabs))
        {
            status_tabs->set_drop_targets({app->quadrant1_list, app->quadrant1_in_progress_list, app->quadrant1_completed_list});
        }
        if (auto *status_tabs = dynamic_cast<AutoSwitchTabWidget *>(app->quadrant2_status_tabs))
        {
            status_tabs->set_drop_targets({app->quadrant2_list, app->quadrant2_in_progress_list, app->quadrant2_completed_list});
        }
        if (auto *status_tabs = dynamic_cast<AutoSwitchTabWidget *>(app->quadrant3_status_tabs))
        {
            status_tabs->set_drop_targets({app->quadrant3_list, app->quadrant3_in_progress_list, app->quadrant3_completed_list});
        }
        if (auto *status_tabs = dynamic_cast<AutoSwitchTabWidget *>(app->quadrant4_status_tabs))
        {
            status_tabs->set_drop_targets({app->quadrant4_list, app->quadrant4_in_progress_list, app->quadrant4_completed_list});
        }

        for (QListWidget *list : {app->quadrant1_list,
                                  app->quadrant2_list,
                                  app->quadrant3_list,
                                  app->quadrant4_list,
                                  app->quadrant1_in_progress_list,
                                  app->quadrant2_in_progress_list,
                                  app->quadrant3_in_progress_list,
                                  app->quadrant4_in_progress_list,
                                  app->quadrant1_completed_list,
                                  app->quadrant2_completed_list,
                                  app->quadrant3_completed_list,
                                  app->quadrant4_completed_list})
        {
            list->setContextMenuPolicy(Qt::CustomContextMenu);
            QObject::connect(list,
                             &QListWidget::customContextMenuRequested,
                             app,
                             [app, list](const QPoint &point)
                             { app->show_context_menu(point, list); });
            QObject::connect(list, &QListWidget::itemDoubleClicked, app, [app](QListWidgetItem *item)
                             { open_linked_file(app, item); });
        }

        QObject::connect(app->quadrant1_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant1_list, app->quadrant1_completed_list); });
        QObject::connect(app->quadrant2_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant2_list, app->quadrant2_completed_list); });
        QObject::connect(app->quadrant3_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant3_list, app->quadrant3_completed_list); });
        QObject::connect(app->quadrant4_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant4_list, app->quadrant4_completed_list); });

        QObject::connect(app->quadrant1_in_progress_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant1_in_progress_list, app->quadrant1_completed_list); });
        QObject::connect(app->quadrant2_in_progress_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant2_in_progress_list, app->quadrant2_completed_list); });
        QObject::connect(app->quadrant3_in_progress_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant3_in_progress_list, app->quadrant3_completed_list); });
        QObject::connect(app->quadrant4_in_progress_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant4_in_progress_list, app->quadrant4_completed_list); });
        QObject::connect(app->quadrant1_completed_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant1_completed_list, app->quadrant1_list); });
        QObject::connect(app->quadrant2_completed_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant2_completed_list, app->quadrant2_list); });
        QObject::connect(app->quadrant3_completed_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant3_completed_list, app->quadrant3_list); });
        QObject::connect(app->quadrant4_completed_list,
                         &QListWidget::itemChanged,
                         app,
                         [app](QListWidgetItem *item)
                         { app->handle_item_checked(item, app->quadrant4_completed_list, app->quadrant4_list); });

        app->setCentralWidget(central);
    }

} // namespace source::InterfaceCore::incore_01_initUI
