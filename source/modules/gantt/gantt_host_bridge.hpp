#pragma once

#include <QString>
#include <QLocale>

namespace source::gantt {
class IEisenhowerHost;
}

namespace source::plugin {

class GanttHostBridge {
public:
    void set_eisenhower_host(source::gantt::IEisenhowerHost *host)
    {
        eisenhower_host_ = host;
    }

    source::gantt::IEisenhowerHost *eisenhower_host() const
    {
        return eisenhower_host_;
    }

    void set_language_code(const QString &language_code)
    {
        language_code_ = language_code.trimmed().isEmpty() ? QLocale::system().name() : language_code.trimmed();
    }

    QString language_code() const
    {
        return language_code_.trimmed().isEmpty() ? QLocale::system().name() : language_code_;
    }

    void set_persistent_dir(const QString &dir)
    {
        persistent_dir_ = dir;
    }

    QString persistent_dir() const
    {
        return persistent_dir_;
    }

private:
    source::gantt::IEisenhowerHost *eisenhower_host_ = nullptr;
    QString language_code_ = QStringLiteral("pt_BR");
    QString persistent_dir_;
};

} // namespace source::plugin
