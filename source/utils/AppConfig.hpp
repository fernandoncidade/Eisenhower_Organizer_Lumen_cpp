#pragma once

#include <QByteArray>
#include <QCoreApplication>
#include <QFileInfo>
#include <QString>
#include <QtGlobal>

#if defined(APP_LUMEN) && defined(APP_EISENHOWER)
#error "Defina apenas um alvo de aplicacao: APP_LUMEN ou APP_EISENHOWER."
#endif

#if !defined(APP_LUMEN) && !defined(APP_EISENHOWER)
#define APP_EISENHOWER 1
#endif

namespace source::utils::AppConfig {

inline constexpr const char *runtime_target_env_var()
{
    return "EISENHOWER_ORGANIZER_RUNTIME_TARGET";
}

inline constexpr const char *runtime_log_file_env_var()
{
    return "EISENHOWER_ORGANIZER_LOG_FILE";
}

inline QString runtime_log_file_path()
{
    return qEnvironmentVariable(runtime_log_file_env_var()).trimmed();
}

inline void set_runtime_log_file_path(const QString &log_file_path)
{
    const QString trimmed_path = log_file_path.trimmed();
    if (trimmed_path.isEmpty()) {
        qunsetenv(runtime_log_file_env_var());
        return;
    }

    qputenv(runtime_log_file_env_var(), QFileInfo(trimmed_path).absoluteFilePath().toUtf8());
}

inline constexpr bool compiled_is_lumen()
{
#if defined(APP_LUMEN)
    return true;
#else
    return false;
#endif
}

inline QString compiled_build_target_name()
{
    return compiled_is_lumen() ? QStringLiteral("LUMEN") : QStringLiteral("EISENHOWER");
}

inline QString normalized_build_target_name(QString target)
{
    target = target.trimmed().toUpper();
    target.replace(QStringLiteral("Ú"), QStringLiteral("U"));
    target.replace(QStringLiteral("APP_"), QString());

    if (target == QStringLiteral("LUMEN")) {
        return QStringLiteral("LUMEN");
    }
    if (target == QStringLiteral("EISENHOWER")) {
        return QStringLiteral("EISENHOWER");
    }
    return {};
}

inline QString executable_build_target_name()
{
    const QString executable_name = QFileInfo(QCoreApplication::applicationFilePath()).completeBaseName();
    return normalized_build_target_name(executable_name);
}

inline QString runtime_build_target_name()
{
    const QString environment_target = normalized_build_target_name(qEnvironmentVariable(runtime_target_env_var()));
    if (!environment_target.isEmpty()) {
        return environment_target;
    }

    const QString executable_target = executable_build_target_name();
    if (!executable_target.isEmpty()) {
        return executable_target;
    }

    return compiled_build_target_name();
}

inline void apply_runtime_build_target_to_environment()
{
    qputenv(runtime_target_env_var(), compiled_build_target_name().toLatin1());
}

inline bool is_lumen()
{
    return runtime_build_target_name() == QStringLiteral("LUMEN");
}

inline QString build_target_name()
{
    return runtime_build_target_name();
}

inline QString assets_subdir_name()
{
    return is_lumen() ? QStringLiteral("lumen") : QStringLiteral("eisenhower");
}

inline QString local_app_data_dir_name()
{
    return is_lumen() ? QStringLiteral("Lumen") : QStringLiteral("EisenhowerOrganizer");
}

inline QString app_display_name()
{
    return is_lumen() ? QStringLiteral("Lúmen") : QStringLiteral("Eisenhower Organizer");
}

inline QString app_brand_name()
{
    return is_lumen() ? QStringLiteral("LÚMEN") : QStringLiteral("EISENHOWER");
}

inline QString app_manual_name()
{
    return is_lumen() ? QStringLiteral("LÚMEN") : QStringLiteral("EISENHOWER ORGANIZER");
}

inline QString main_window_title()
{
    return is_lumen() ? QStringLiteral("LÚMEN — Plataforma de Estudos")
                      : QStringLiteral("Matriz de Eisenhower - Organizador de Tarefas");
}

inline QString about_window_title()
{
    return QStringLiteral("Sobre — ") + app_manual_name();
}

inline QString manual_window_title()
{
    return QStringLiteral("Manual de Utilização — ") + app_manual_name();
}

inline QString translation_compiler_window_title()
{
    return QStringLiteral("Compilador de Traduções — ") + app_manual_name();
}

inline QString default_window_icon_name()
{
    return is_lumen() ? QStringLiteral("autismo.ico") : QStringLiteral("organizador.ico");
}

inline QString default_window_icon_image_name()
{
    return is_lumen() ? QStringLiteral("autismo.png") : QStringLiteral("organizador.png");
}

inline QString unified_tasks_schema_name()
{
    return is_lumen() ? QStringLiteral("lumen_unified_tasks")
                      : QStringLiteral("eisenhower_unified_tasks");
}

inline QString legacy_unified_tasks_schema_name()
{
    return QStringLiteral("eisenhower_unified_tasks");
}

inline QString logger_name()
{
    return is_lumen() ? QStringLiteral("FileLumen") : QStringLiteral("FileEisenhower");
}

inline QString log_file_prefix()
{
    return is_lumen() ? QStringLiteral("file_lumen") : QStringLiteral("file_eisenhower");
}

inline QString startup_log_prefix()
{
    return is_lumen() ? QStringLiteral("Lumen_startup") : QStringLiteral("Eisenhower_startup");
}

inline QString startup_log_title()
{
    return app_display_name() + QStringLiteral(" - startup log");
}

inline QString fatal_startup_title()
{
    return app_display_name() + QStringLiteral(" - falha ao iniciar");
}

inline QString fatal_startup_message(const QString &log_file)
{
    return QStringLiteral("O %1 não conseguiu inicializar.\n\nUm log foi salvo em:\n%2")
        .arg(app_display_name(), log_file);
}

}  // namespace source::utils::AppConfig
