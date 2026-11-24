#pragma once

#include <QFont>
#include <QJsonObject>
#include <QString>

namespace source::plugin::Pomodoro::utils {

class FontManager {
public:
    static QJsonObject get_config();
    static bool save_config(const QJsonObject &config);
    static QFont get_font();
    static QString get_html_style();
    static bool reset_to_default();

private:
    static QString config_path();
    static QJsonObject default_config();
};

}  // namespace source::plugin::Pomodoro::utils
