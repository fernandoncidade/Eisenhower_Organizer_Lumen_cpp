#pragma once

#include <QApplication>
#include <QColor>
#include <QPainter>
#include <QPalette>
#include <QProxyStyle>
#include <QStyle>
#include <QStyleHintReturn>
#include <QStyleOptionTab>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
#include <QVariant>

#include "source/utils/AppConfig.hpp"

namespace source::utils {

class AccentTabStyle : public QProxyStyle {
public:
    using QProxyStyle::QProxyStyle;

    static QColor accent_blue() { return AppConfig::is_lumen() ? QColor(56, 132, 255) : QColor(55, 130, 220); }
    static QColor accent_yellow() { return AppConfig::is_lumen() ? QColor(246, 196, 68) : QColor(230, 210, 30); }
    static QColor accent_orange() { return AppConfig::is_lumen() ? QColor(235, 135, 70) : QColor(230, 140, 20); }
    static QColor accent_red() { return AppConfig::is_lumen() ? QColor(218, 73, 97) : QColor(200, 50, 50); }
    static QColor accent_green() { return AppConfig::is_lumen() ? QColor(20, 155, 132) : QColor(50, 170, 60); }

    static void install(QTabWidget *tab_widget)
    {
        if (tab_widget != nullptr) {
            install(tab_widget->tabBar());
        }
    }

    static void install(QTabBar *tab_bar)
    {
        if (tab_bar == nullptr) {
            return;
        }

        tab_bar->setMouseTracking(true);
        tab_bar->setAttribute(Qt::WA_Hover, true);

        QStyle *owned_previous_style = nullptr;
        if (tab_bar->style() != nullptr && tab_bar->style()->parent() == tab_bar) {
            owned_previous_style = tab_bar->style();
        }

        if (dynamic_cast<AccentTabStyle *>(tab_bar->style()) == nullptr) {
            auto *style = new AccentTabStyle(QStringLiteral("Fusion"));
            style->setParent(tab_bar);
            tab_bar->setStyle(style);
        }

        if (owned_previous_style != nullptr && owned_previous_style != tab_bar->style()) {
            owned_previous_style->deleteLater();
        }

        tab_bar->update();
    }

    static void refresh(QTabWidget *tab_widget)
    {
        if (tab_widget != nullptr) {
            refresh(tab_widget->tabBar());
        }
    }

    static void refresh(QTabBar *tab_bar)
    {
        if (tab_bar != nullptr) {
            update_tab_bar(tab_bar);
            QTimer::singleShot(0, tab_bar, [tab_bar]() { AccentTabStyle::update_tab_bar(tab_bar); });
            QTimer::singleShot(80, tab_bar, [tab_bar]() { AccentTabStyle::update_tab_bar(tab_bar); });
        }
    }

    static void set_alignment(QTabWidget *tab_widget, Qt::Alignment alignment)
    {
        if (tab_widget != nullptr) {
            set_alignment(tab_widget->tabBar(), alignment);
        }
    }

    static void set_alignment(QTabBar *tab_bar, Qt::Alignment alignment)
    {
        if (tab_bar == nullptr) {
            return;
        }

        tab_bar->setProperty("accent_tab_alignment", static_cast<int>(alignment));
        tab_bar->update();
    }

    static void set_tab_accent(QTabWidget *tab_widget, int index, const QColor &color)
    {
        if (tab_widget != nullptr) {
            set_tab_accent(tab_widget->tabBar(), index, color);
        }
    }

    static void set_tab_accent(QTabWidget *tab_widget, QWidget *page, const QColor &color)
    {
        if (tab_widget != nullptr && page != nullptr) {
            set_tab_accent(tab_widget, tab_widget->indexOf(page), color);
        }
    }

    static void set_tab_accent(QTabBar *tab_bar, int index, const QColor &color)
    {
        if (tab_bar == nullptr || index < 0) {
            return;
        }

        tab_bar->setTabData(index, color);
        tab_bar->update(tab_bar->tabRect(index));
    }

protected:
    void drawControl(ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const override
    {
        if ((element == CE_TabBarTabShape || element == CE_TabBarTabLabel) && option != nullptr && painter != nullptr) {
            if (const auto *tab_option = qstyleoption_cast<const QStyleOptionTab *>(option)) {
                QStyleOptionTab styled_option(*tab_option);
                styled_option.palette = current_application_palette(styled_option.palette);
                const QColor fill_color = (styled_option.state & State_Selected)
                                              ? selected_tab_color(&styled_option, widget)
                                          : (styled_option.state & State_MouseOver)
                                              ? hovered_tab_color(&styled_option, widget)
                                              : inactive_tab_color(&styled_option, widget);
                styled_option.palette = selected_tab_palette(styled_option.palette, fill_color);
                QProxyStyle::drawControl(element, &styled_option, painter, widget);
                return;
            }
        }

        QProxyStyle::drawControl(element, option, painter, widget);
    }

    int pixelMetric(PixelMetric metric,
                    const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override
    {
        const int base_value = QProxyStyle::pixelMetric(metric, option, widget);
        if (qobject_cast<const QTabBar *>(widget) == nullptr) {
            return base_value;
        }

        switch (metric) {
        case PM_TabBarTabHSpace:
            return qMax(14, base_value - 12);
        case PM_TabBarTabVSpace:
            return qMax(8, base_value - 2);
        default:
            return base_value;
        }
    }

    QRect subElementRect(SubElement element,
                         const QStyleOption *option,
                         const QWidget *widget = nullptr) const override
    {
        QRect rect = QProxyStyle::subElementRect(element, option, widget);
        if (element == SE_TabBarTabText && qobject_cast<const QTabBar *>(widget) != nullptr) {
            rect.adjust(0, -1, 0, 1);
        }
        return rect;
    }

    int styleHint(StyleHint hint,
                  const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr,
                  QStyleHintReturn *returnData = nullptr) const override
    {
        if (hint == SH_TabBar_Alignment && qobject_cast<const QTabBar *>(widget) != nullptr) {
            const QVariant alignment = widget->property("accent_tab_alignment");
            if (alignment.isValid()) {
                return alignment.toInt();
            }
            return Qt::AlignCenter;
        }

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }

private:
    static QColor blend_colors(const QColor &base, const QColor &accent, qreal accent_ratio)
    {
        const qreal clamped_ratio = qBound(0.0, accent_ratio, 1.0);
        const qreal base_ratio = 1.0 - clamped_ratio;
        return QColor::fromRgbF(base.redF() * base_ratio + accent.redF() * clamped_ratio,
                                base.greenF() * base_ratio + accent.greenF() * clamped_ratio,
                                base.blueF() * base_ratio + accent.blueF() * clamped_ratio,
                                1.0);
    }

    static QPalette current_application_palette(const QPalette &fallback)
    {
        return qApp != nullptr ? qApp->palette() : fallback;
    }

    static void update_tab_bar(QTabBar *tab_bar)
    {
        if (tab_bar == nullptr) {
            return;
        }

        tab_bar->update();
        if (QWidget *parent = tab_bar->parentWidget(); parent != nullptr) {
            parent->update();
        }
    }

    bool is_light(const QColor &color) const
    {
        return color.lightnessF() >= 0.6;
    }

    QColor hover_color(const QPalette &palette) const
    {
        QColor hover = palette.color(QPalette::Button);
        hover = is_light(hover) ? hover.darker(110) : hover.lighter(115);
        hover.setAlpha(90);
        return hover;
    }

    QColor text_color(const QColor &background) const
    {
        return is_light(background) ? QColor(Qt::black) : QColor(Qt::white);
    }

    QColor tab_accent_color(const QStyleOption *option, const QWidget *widget) const
    {
        if (option == nullptr) {
            return {};
        }

        if (const auto *tab_bar = qobject_cast<const QTabBar *>(widget)) {
            const int index = tab_bar->tabAt(option->rect.center());
            if (index >= 0) {
                return tab_bar->tabData(index).value<QColor>();
            }
        }

        return {};
    }

    QColor inactive_tab_color(const QStyleOption *option, const QWidget *widget) const
    {
        if (option == nullptr) {
            return {};
        }

        const QColor neutral_button = option->palette.color(QPalette::Button);
        const bool light_theme = is_light(option->palette.color(QPalette::Window));
        QColor color = neutral_button;
        const QColor accent = tab_accent_color(option, widget);
        if (accent.isValid()) {
            color = blend_colors(neutral_button, accent, light_theme ? 0.08 : 0.10);
        }

        if (light_theme) {
            color = color.darker(98);
            color.setAlpha(205);
        } else {
            color = is_light(color) ? color.darker(145) : color.darker(125);
            color.setAlpha(225);
        }
        return color;
    }

    QColor hovered_tab_color(const QStyleOption *option, const QWidget *widget) const
    {
        if (option == nullptr) {
            return {};
        }

        const QColor neutral_button = option->palette.color(QPalette::Button);
        const QColor accent = tab_accent_color(option, widget);
        QColor color = accent.isValid() ? blend_colors(neutral_button, accent, 0.26) : hover_color(option->palette);

        color = is_light(color) ? color.darker(112) : color.lighter(108);
        color.setAlpha(180);
        return color;
    }

    QColor selected_tab_color(const QStyleOption *option, const QWidget *widget) const
    {
        if (option == nullptr) {
            return {};
        }

        QColor color = tab_accent_color(option, widget);
        if (!color.isValid()) {
            color = hover_color(option->palette);
        } else {
            const QColor neutral_button = option->palette.color(QPalette::Button);
            color = blend_colors(neutral_button, color, 0.38);
            color = is_light(color) ? color.darker(108) : color.darker(116);
            color.setAlpha(92);
        }

        return color;
    }

    QPalette selected_tab_palette(const QPalette &base_palette, const QColor &fill_color) const
    {
        QPalette palette(base_palette);
        const QColor foreground = text_color(fill_color);
        const auto with_alpha = [fill_color](QColor color) {
            color.setAlpha(fill_color.alpha());
            return color;
        };

        for (QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
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
};

}  // namespace source::utils
