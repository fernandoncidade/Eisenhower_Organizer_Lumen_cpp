#include "source/utils/TrialManager.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSaveFile>
#include <cstdlib>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winreg.h>
#endif

#include "source/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/IconUtils.hpp"
#include "source/utils/LogManager.hpp"

namespace source::utils {
namespace {

LogManager *logger()
{
    return LogManager::get_logger();
}

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

#ifdef Q_OS_WIN
std::optional<quint64> read_registry_value(REGSAM access)
{
    HKEY key = nullptr;
    const LONG open_result = RegOpenKeyExW(HKEY_CURRENT_USER,
                                           L"SOFTWARE\\EisenhowerOrganizer",
                                           0,
                                           access,
                                           &key);
    if (open_result != ERROR_SUCCESS) {
        return std::nullopt;
    }

    DWORD type = 0;
    DWORD dword_value = 0;
    DWORD size = sizeof(dword_value);
    LONG query_result = RegQueryValueExW(key, L"FirstRunTimestamp", nullptr, &type,
                                         reinterpret_cast<LPBYTE>(&dword_value), &size);
    if (query_result == ERROR_SUCCESS && (type == REG_DWORD || type == REG_QWORD)) {
        RegCloseKey(key);
        return static_cast<quint64>(dword_value);
    }

    wchar_t buffer[128] = {};
    size = sizeof(buffer);
    query_result = RegQueryValueExW(key, L"FirstRunTimestamp", nullptr, &type,
                                    reinterpret_cast<LPBYTE>(buffer), &size);
    RegCloseKey(key);
    if (query_result == ERROR_SUCCESS) {
        bool ok = false;
        const quint64 value = QString::fromWCharArray(buffer).toULongLong(&ok);
        if (ok) {
            return value;
        }
    }

    return std::nullopt;
}

bool write_registry_value(REGSAM access, quint64 timestamp)
{
    HKEY key = nullptr;
    DWORD disposition = 0;
    const LONG create_result = RegCreateKeyExW(HKEY_CURRENT_USER,
                                               L"SOFTWARE\\EisenhowerOrganizer",
                                               0,
                                               nullptr,
                                               0,
                                               access,
                                               nullptr,
                                               &key,
                                               &disposition);
    Q_UNUSED(disposition);
    if (create_result != ERROR_SUCCESS) {
        return false;
    }

    const DWORD dword_value = static_cast<DWORD>(timestamp);
    LONG set_result = RegSetValueExW(key,
                                     L"FirstRunTimestamp",
                                     0,
                                     REG_DWORD,
                                     reinterpret_cast<const BYTE *>(&dword_value),
                                     sizeof(dword_value));
    if (set_result != ERROR_SUCCESS) {
        const QString text_value = QString::number(timestamp);
        set_result = RegSetValueExW(key,
                                    L"FirstRunTimestamp",
                                    0,
                                    REG_SZ,
                                    reinterpret_cast<const BYTE *>(text_value.utf16()),
                                    static_cast<DWORD>((text_value.size() + 1) * sizeof(char16_t)));
    }

    RegCloseKey(key);
    return set_result == ERROR_SUCCESS;
}

void delete_registry_value(REGSAM access, bool &removed)
{
    HKEY key = nullptr;
    const LONG open_result = RegOpenKeyExW(HKEY_CURRENT_USER,
                                           L"SOFTWARE\\EisenhowerOrganizer",
                                           0,
                                           access,
                                           &key);
    if (open_result != ERROR_SUCCESS) {
        return;
    }

    const LONG delete_result = RegDeleteValueW(key, L"FirstRunTimestamp");
    if (delete_result == ERROR_SUCCESS) {
        removed = true;
    }
    RegCloseKey(key);
}
#endif

}  // namespace

QString TrialManager::get_config_path()
{
    const QString config_dir = obter_caminho_persistente();
    return QDir(config_dir).filePath(QString::fromLatin1(CONFIG_FILE));
}

std::optional<quint64> TrialManager::get_first_run_timestamp()
{
#ifdef Q_OS_WIN
    const REGSAM views[] = {
        KEY_READ | KEY_WOW64_64KEY,
        KEY_READ | KEY_WOW64_32KEY,
        KEY_READ,
    };

    for (const REGSAM access : views) {
        if (const auto value = read_registry_value(access); value.has_value()) {
            return value;
        }
    }
#endif

    return std::nullopt;
}

void TrialManager::set_first_run_timestamp(quint64 timestamp)
{
#ifdef Q_OS_WIN
    const REGSAM views[] = {
        KEY_WRITE | KEY_WOW64_64KEY,
        KEY_WRITE | KEY_WOW64_32KEY,
        KEY_WRITE,
    };

    for (const REGSAM access : views) {
        if (write_registry_value(access, timestamp)) {
            return;
        }
    }
#endif

    logger()->error(QStringLiteral("Não foi possível gravar timestamp no registro com nenhuma visão disponível."));
}

void TrialManager::delete_first_run_timestamp()
{
    bool removed = false;

#ifdef Q_OS_WIN
    const REGSAM views[] = {
        KEY_ALL_ACCESS | KEY_WOW64_64KEY,
        KEY_ALL_ACCESS | KEY_WOW64_32KEY,
        KEY_ALL_ACCESS,
    };

    for (const REGSAM access : views) {
        delete_registry_value(access, removed);
        if (removed) {
            break;
        }
    }
#endif

    const QString path = get_config_path();
    if (QFileInfo::exists(path)) {
        QFile::remove(path);
    }
}

QJsonObject TrialManager::get_trial_info()
{
    const QString path = get_config_path();
    const std::optional<quint64> first_run = get_first_run_timestamp();

    if (QFileInfo::exists(path)) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonObject info = doc.object();
            if (first_run.has_value()) {
                info.insert(QStringLiteral("first_run"), static_cast<qint64>(first_run.value()));
            } else if (info.contains(QStringLiteral("first_run"))) {
                set_first_run_timestamp(info.value(QStringLiteral("first_run")).toInteger());
            }
            return info;
        }
    }

    const quint64 timestamp = first_run.value_or(static_cast<quint64>(QDateTime::currentSecsSinceEpoch()));
    QJsonObject info;
    info.insert(QStringLiteral("first_run"), static_cast<qint64>(timestamp));

    QSaveFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        const QByteArray payload = QJsonDocument(info).toJson(QJsonDocument::Indented);
        if (file.write(payload) == payload.size()) {
            file.commit();
        }
    }

    if (!first_run.has_value()) {
        set_first_run_timestamp(timestamp);
    }

    return info;
}

bool TrialManager::is_trial_expired()
{
    const QJsonObject info = get_trial_info();
    const quint64 first_run = info.value(QStringLiteral("first_run")).toVariant().toULongLong();
    const quint64 now = static_cast<quint64>(QDateTime::currentSecsSinceEpoch());

    quint64 trial_seconds = 0;
    if (QString::fromLatin1(DEFAULT_TRIAL) == QStringLiteral("minutes")) {
        trial_seconds = static_cast<quint64>(DEFAULT_TRIAL_VALUE) * 60ULL;
    } else if (QString::fromLatin1(DEFAULT_TRIAL) == QStringLiteral("days")) {
        trial_seconds = static_cast<quint64>(DEFAULT_TRIAL_VALUE) * 24ULL * 3600ULL;
    }

    return (now - first_run) > trial_seconds;
}

void TrialManager::enforce_trial(QWidget *parent)
{
    if (LIBERAR_USO_DEFINITIVO || !is_trial_expired()) {
        return;
    }

    QMessageBox msg(parent);
    const QString icon_file = get_default_window_icon_path();
    if (!icon_file.isEmpty()) {
        msg.setWindowIcon(QIcon(icon_file));
    }

    msg.setIcon(QMessageBox::Critical);
    msg.setWindowTitle(get_text(QStringLiteral("trial_expired_title")));
    msg.setTextFormat(Qt::RichText);
    msg.setText(
        QStringLiteral("%1<br>%2<br><br>%3<br><br><a href=\"%4\">%5</a>")
            .arg(get_text(QStringLiteral("trial_expired_message")),
                 get_text(QStringLiteral("trial_buy_message")),
                 get_text(QStringLiteral("trial_uninstall_message")),
                 QString::fromLatin1(PAID_VERSION_URL),
                 get_text(QStringLiteral("trial_paid_link"))));
    msg.setStandardButtons(QMessageBox::Ok);
    msg.exec();
    std::exit(0);
}

}  // namespace source::utils
