#pragma once

#include <functional>
#include <utility>

#include <QLocale>
#include <QString>
#include <Qt>

class QObject;

namespace source::plugin {

class AudiolivroHostBridge {
public:
    using StandardButtonsHandler = std::function<void(QObject *)>;

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

private:
    QString language_code_ = QLocale::system().name();
    StandardButtonsHandler standard_buttons_handler_;
};

}  // namespace source::plugin
