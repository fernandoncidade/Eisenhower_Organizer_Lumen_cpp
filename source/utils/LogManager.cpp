#include "source/utils/LogManager.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMessageLogContext>
#include <QMutexLocker>
#include <QStringList>
#include <QTextStream>
#include <QtGlobal>
#include <cstdlib>
#include <exception>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "source/utils/AppConfig.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"

namespace source::utils {
namespace {

std::terminate_handler g_previous_terminate_handler = nullptr;
QtMessageHandler g_previous_qt_message_handler = nullptr;
bool g_qt_message_handler_installed = false;

QString logger_name()
{
    return AppConfig::logger_name();
}

QString qt_message_level(QtMsgType type)
{
    switch (type) {
        case QtDebugMsg:
            return QStringLiteral("DEBUG");
        case QtInfoMsg:
            return QStringLiteral("INFO");
        case QtWarningMsg:
            return QStringLiteral("WARNING");
        case QtCriticalMsg:
            return QStringLiteral("ERROR");
        case QtFatalMsg:
            return QStringLiteral("CRITICAL");
    }
    return QStringLiteral("INFO");
}

QString qt_message_context(const QMessageLogContext &context)
{
    QStringList parts;
    if (context.file != nullptr && context.file[0] != '\0') {
        QString file = QString::fromUtf8(context.file);
        file.replace(QLatin1Char('\\'), QLatin1Char('/'));
        parts << QStringLiteral("%1:%2").arg(file).arg(context.line);
    }
    if (context.function != nullptr && context.function[0] != '\0') {
        parts << QString::fromUtf8(context.function);
    }
    return parts.join(QStringLiteral(" | "));
}

void qt_message_handler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    static thread_local bool handling = false;
    if (handling) {
        if (g_previous_qt_message_handler != nullptr) {
            g_previous_qt_message_handler(type, context, message);
        }
        return;
    }

    handling = true;
    QString text = message;
    const QString context_text = qt_message_context(context);
    if (!context_text.isEmpty()) {
        text = QStringLiteral("Qt[%1] %2").arg(context_text, message);
    } else {
        text = QStringLiteral("Qt: %1").arg(message);
    }
    auto *logger = LogManager::get_logger();
    const QString level = qt_message_level(type);
    if (level == QStringLiteral("DEBUG")) {
        logger->debug(text);
    } else if (level == QStringLiteral("INFO")) {
        logger->info(text);
    } else if (level == QStringLiteral("WARNING")) {
        logger->warning(text);
    } else if (level == QStringLiteral("CRITICAL")) {
        logger->critical(text);
    } else {
        logger->error(text);
    }
    handling = false;

    if (type == QtFatalMsg) {
        std::abort();
    }
}

}  // namespace

LogManager::LogManager()
{
    configure_logging();
}

LogManager::~LogManager()
{
    QMutexLocker locker(&mutex_);
    if (active_log_file_.isOpen()) {
        active_log_file_.flush();
        active_log_file_.close();
    }
    if (startup_file_.isOpen()) {
        startup_file_.flush();
        startup_file_.close();
    }
}

LogManager *LogManager::get_logger()
{
    static LogManager instance;
    return &instance;
}

QString LogManager::get_log_file()
{
    return get_logger()->log_file_;
}

void LogManager::enable_startup_diagnostics()
{
    auto *logger = get_logger();
    if (!logger->startup_log_file_.isEmpty()) {
        return;
    }

    logger->startup_log_file_ = logger->startup_log_path();
    logger->startup_file_.setFileName(logger->startup_log_file_);
    if (!logger->startup_file_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        logger->startup_log_file_.clear();
        return;
    }

    logger->write_startup_log(AppConfig::startup_log_title());
    logger->write_startup_log(QStringLiteral("executable=%1").arg(QCoreApplication::applicationFilePath()));
    logger->write_startup_log(QStringLiteral("cwd=%1").arg(QDir::currentPath()));
    logger->write_startup_log(QStringLiteral("arguments=%1").arg(QCoreApplication::arguments().join(QStringLiteral(" "))));
}

void LogManager::install_exception_hook()
{
    if (g_previous_terminate_handler != nullptr) {
        return;
    }

    g_previous_terminate_handler = std::set_terminate([]() {
        auto *logger = LogManager::get_logger();
        logger->write_startup_log(QStringLiteral("=== FATAL ERROR ==="));
        logger->critical(QStringLiteral("Unhandled exception during startup"));
        LogManager::show_fatal_message(
            AppConfig::fatal_startup_title(),
            AppConfig::fatal_startup_message(LogManager::get_log_file()));

        if (g_previous_terminate_handler != nullptr) {
            g_previous_terminate_handler();
        } else {
            std::abort();
        }
    });
}

void LogManager::install_qt_message_handler()
{
    if (g_qt_message_handler_installed) {
        return;
    }
    g_previous_qt_message_handler = qInstallMessageHandler(qt_message_handler);
    g_qt_message_handler_installed = true;
}

void LogManager::uninstall_qt_message_handler()
{
    if (!g_qt_message_handler_installed) {
        return;
    }
    qInstallMessageHandler(g_previous_qt_message_handler);
    g_previous_qt_message_handler = nullptr;
    g_qt_message_handler_installed = false;
}

void LogManager::close_startup_diagnostics()
{
    auto *logger = get_logger();
    if (logger->startup_file_.isOpen()) {
        logger->startup_file_.flush();
        logger->startup_file_.close();
    }
}

QString LogManager::ensure_unicode(const QString &message)
{
    return message;
}

QString LogManager::ensure_unicode(const QByteArray &message)
{
    return QString::fromUtf8(message);
}

void LogManager::debug(const QString &message)
{
    log(QStringLiteral("DEBUG"), ensure_unicode(message));
}

void LogManager::info(const QString &message)
{
    log(QStringLiteral("INFO"), ensure_unicode(message));
}

void LogManager::warning(const QString &message)
{
    log(QStringLiteral("WARNING"), ensure_unicode(message));
}

void LogManager::error(const QString &message, bool)
{
    log(QStringLiteral("ERROR"), ensure_unicode(message));
}

void LogManager::critical(const QString &message, bool)
{
    log(QStringLiteral("CRITICAL"), ensure_unicode(message));
}

QString LogManager::persistent_base_dir() const
{
    return source::utils::obter_caminho_persistente();
}

QString LogManager::log_dir() const
{
    const QString directory = QDir(persistent_base_dir()).filePath(QStringLiteral("logs"));
    QDir().mkpath(directory);
    return directory;
}

QString LogManager::startup_log_path() const
{
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss"));
    return QDir(log_dir()).filePath(QStringLiteral("%1_%2.log").arg(AppConfig::startup_log_prefix(), stamp));
}

void LogManager::configure_logging()
{
    const QString runtime_log_file = AppConfig::runtime_log_file_path();
    if (!runtime_log_file.isEmpty()) {
        const QFileInfo log_file_info(runtime_log_file);
        QDir().mkpath(log_file_info.absoluteDir().absolutePath());
        log_file_ = log_file_info.absoluteFilePath();

        active_log_file_.setFileName(log_file_);
        if (active_log_file_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            active_log_stream_.setDevice(&active_log_file_);
            return;
        }

        log_file_.clear();
    }

    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss"));
    log_file_ = QDir(log_dir()).filePath(QStringLiteral("%1_%2.log").arg(AppConfig::log_file_prefix(), stamp));

    active_log_file_.setFileName(log_file_);
    if (!active_log_file_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        log_file_.clear();
        return;
    }
    active_log_stream_.setDevice(&active_log_file_);

    AppConfig::set_runtime_log_file_path(log_file_);
    cleanup_logs();
}

void LogManager::write_startup_log(const QString &text)
{
    if (startup_log_file_.isEmpty()) {
        startup_log_file_ = startup_log_path();
    }

    QFile file(startup_log_file_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out << text;
    if (!text.endsWith(QLatin1Char('\n'))) {
        out << Qt::endl;
    }
}

void LogManager::cleanup_logs()
{
    QDir dir(log_dir());
    QFileInfoList entries = dir.entryInfoList(
        QStringList() << QStringLiteral("%1_*.log").arg(AppConfig::log_file_prefix())
                      << QStringLiteral("%1_*.log").arg(AppConfig::startup_log_prefix()),
        QDir::Files,
        QDir::Time | QDir::Reversed);

    if (entries.size() < 12) {
        return;
    }

    int removed = 0;
    for (const QFileInfo &entry : entries) {
        if (removed >= 2) {
            break;
        }

        const QString absolute_path = entry.absoluteFilePath();
        if (absolute_path == QFileInfo(log_file_).absoluteFilePath()
            || (!startup_log_file_.isEmpty() && absolute_path == QFileInfo(startup_log_file_).absoluteFilePath())) {
            continue;
        }

        if (QFile::remove(absolute_path)) {
            ++removed;
        }
    }
}

void LogManager::log(const QString &level, const QString &message)
{
    QMutexLocker locker(&mutex_);

    const QString line = QStringLiteral("%1 [%2] %3: %4")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")),
                                  level,
                                  logger_name(),
                                  message);

    const bool flush_now = (level == QStringLiteral("CRITICAL") || level == QStringLiteral("ERROR")
                            || level == QStringLiteral("WARNING"));

    if (active_log_file_.isOpen()) {
        active_log_stream_ << line << Qt::endl;
        if (flush_now) {
            active_log_file_.flush();
        }
    } else if (!log_file_.isEmpty()) {
        active_log_file_.setFileName(log_file_);
        if (active_log_file_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            active_log_stream_.setDevice(&active_log_file_);
            active_log_stream_ << line << Qt::endl;
            if (flush_now) {
                active_log_file_.flush();
            }
        }
    }

    QTextStream err(stderr);
    err << line << Qt::endl;

    if (startup_file_.isOpen()) {
        QTextStream startup_out(&startup_file_);
        startup_out << line << Qt::endl;
        if (flush_now) {
            startup_file_.flush();
        }
    }
}

void LogManager::show_fatal_message(const QString &title, const QString &message)
{
#ifdef Q_OS_WIN
    MessageBoxW(nullptr,
                reinterpret_cast<LPCWSTR>(message.utf16()),
                reinterpret_cast<LPCWSTR>(title.utf16()),
                MB_ICONERROR | MB_OK);
#else
    Q_UNUSED(title);
    Q_UNUSED(message);
#endif
}

}  // namespace source::utils
