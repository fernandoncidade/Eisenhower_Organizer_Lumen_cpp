#pragma once

#include <functional>
#include <utility>

#include <QLocale>
#include <QString>
#include <QVariantMap>
#include <Qt>

class QObject;

namespace source::plugin {

class AgendaHostBridge {
public:
    using TaskHandler = std::function<bool(const QVariantMap &)>;
    using StandardButtonsHandler = std::function<void(QObject *)>;
    using ColorModeHandler = std::function<void(const QString &)>;
    using ColorResolverHandler = std::function<QString(const QString &)>;

    void set_language_code(const QString &language_code)
    {
        language_code_ = language_code.trimmed().isEmpty() ? QLocale::system().name() : language_code.trimmed();
    }

    QString language_code() const
    {
        return language_code_;
    }

    bool is_english() const
    {
        return language_code_.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);
    }

    void set_persistent_dir(const QString &persistent_dir)
    {
        persistent_dir_ = persistent_dir.trimmed();
    }

    QString persistent_dir() const
    {
        return persistent_dir_;
    }

    void set_task_to_eisenhower_handler(TaskHandler handler)
    {
        task_to_eisenhower_handler_ = std::move(handler);
    }

    bool publish_task_to_eisenhower(const QVariantMap &task) const
    {
        return task_to_eisenhower_handler_ ? task_to_eisenhower_handler_(task) : false;
    }

    void set_standard_buttons_handler(StandardButtonsHandler handler)
    {
        standard_buttons_handler_ = std::move(handler);
    }

    void translate_standard_buttons(QObject *dialog) const
    {
        if (standard_buttons_handler_) {
            standard_buttons_handler_(dialog);
        }
    }

    void set_color_mode_changed_handler(ColorModeHandler handler)
    {
        color_mode_changed_handler_ = std::move(handler);
    }

    void notify_color_mode_changed(const QString &mode) const
    {
        if (color_mode_changed_handler_) {
            color_mode_changed_handler_(mode);
        }
    }

    void set_color_resolver_handler(ColorResolverHandler handler)
    {
        color_resolver_handler_ = std::move(handler);
    }

    QString resolve_color_for_title(const QString &title) const
    {
        return color_resolver_handler_ ? color_resolver_handler_(title) : QString();
    }

private:
    QString language_code_ = QLocale::system().name();
    QString persistent_dir_;
    TaskHandler task_to_eisenhower_handler_;
    StandardButtonsHandler standard_buttons_handler_;
    ColorModeHandler color_mode_changed_handler_;
    ColorResolverHandler color_resolver_handler_;
};

}  // namespace source::plugin
