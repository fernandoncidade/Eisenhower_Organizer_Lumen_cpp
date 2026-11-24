#pragma once

#include <QDialog>
#include <QHash>
#include <QList>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QObject>
#include <QPointer>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>
#include <QWidget>

#include "source/modules/pomodoro/pomodoro_host_bridge.hpp"
#include "source/utils/LogManager.hpp"

class QAudioOutput;
class QMediaPlayer;

namespace source::plugin::Pomodoro::compat {

enum class MissingLinkedFileChoice {
    None,
    RemoveLink,
    Search,
};

class GerenciadorBotoesUI {
public:
    explicit GerenciadorBotoesUI(QWidget *owner = nullptr);

    QPushButton *create_button(const QString &text = QString()) const;
    void set_button_text(QPushButton *button, const QString &text) const;

private:
    QPointer<QWidget> owner_;
};

class SoundManager : public QObject {
    Q_OBJECT

public:
    static SoundManager *instance();

    QStringList get_available_sounds() const;
    QString get_sound_display_name(const QString &path) const;
    QString get_current_sound() const;
    void set_sound(const QString &path);
    int get_alarm_interval() const;
    void set_alarm_interval(int seconds);
    bool get_automatic_mode_enabled() const;
    void set_automatic_mode_enabled(bool enabled);
    int get_automatic_alarm_repetitions() const;
    void set_automatic_alarm_repetitions(int repetitions);
    qint64 get_duration_for_path(const QString &path) const;
    void preview(const QString &path);
    void play_looping(int maximum_repetitions = 0);
    void stop();

signals:
    void maximum_repetitions_reached();

private:
    explicit SoundManager(QObject *parent = nullptr);
    QString config_path() const;
    QString sounds_directory() const;
    QString default_sound_path() const;
    QString resolve_sound_path(const QString &path) const;
    void load_config();
    void save_config() const;
    void start_alarm_cycle();
    void play_fallback_beep_cycle();
    qint64 probe_wav_duration_ms(const QString &path) const;

    QTimer beep_timer_;
    QTimer repeat_timer_;
    QAudioOutput *audio_output_ = nullptr;
    QMediaPlayer *player_ = nullptr;
    mutable QHash<QString, qint64> duration_cache_;
    QString current_sound_;
    QString active_sound_;
    int alarm_interval_seconds_ = 0;
    int automatic_alarm_repetitions_ = 3;
    int active_maximum_repetitions_ = 0;
    int completed_alarm_repetitions_ = 0;
    bool automatic_mode_enabled_ = false;
    bool alarm_active_ = false;
    bool preview_active_ = false;
};

class MessageBox : public QMessageBox {
    Q_OBJECT

public:
    using QMessageBox::QMessageBox;

    static MessageBox *create(QWidget *parent = nullptr);

    static void information(QWidget *parent, const QString &title, const QString &text);
    static void warning(QWidget *parent, const QString &title, const QString &text);
    static void warning_file_not_found(QWidget *parent);

    static bool tempo_confirm_attach_file(QWidget *parent, const QString &file_name);
    static bool tempo_confirm_delete_task(QWidget *parent, const QString &task_name);
    static bool tempo_confirm_delete_tasks(QWidget *parent, int total);

    static void tempo_delete_task_success(QWidget *parent);
    static void tempo_delete_task_failure(QWidget *parent);
    static void tempo_delete_task_error(QWidget *parent);

    static void tempo_delete_tasks_success(QWidget *parent, int total);
    static void tempo_delete_tasks_failure(QWidget *parent);
    static void tempo_delete_tasks_error(QWidget *parent);

    static void tempo_edit_task_success(QWidget *parent);
    static void tempo_share_eisenhower_success(QWidget *parent, int total);
    static void tempo_share_feynman_success(QWidget *parent, int total);
    static void tempo_no_tasks_in_column(QWidget *parent);
    static void tempo_no_tasks_anywhere(QWidget *parent);
};

class EventBus : public QObject {
    Q_OBJECT

public:
    static EventBus *instance();

    void send_conceito(const QVariantMap &dados);
    void drain_pending_conceitos();
    void send_tarefa(const QVariantMap &dados);
    void receive_tarefa_from_host(const QVariantMap &dados);
    void drain_pending_tarefas();
    void clear_pending();

signals:
    void conceito_atualizado(const QVariantMap &dados);
    void tarefa_integracao(const QVariantMap &dados);

private:
    explicit EventBus(QObject *parent = nullptr) : QObject(parent) {}

    QList<QVariantMap> pending_conceitos_;
    QList<QVariantMap> pending_tarefas_from_host_;
};

EventBus *get_event_bus();
void set_host_bridge(source::plugin::PomodoroHostBridge *host_bridge);
source::plugin::PomodoroHostBridge *host_bridge();
QString plugin_persistent_dir();
QString effective_language_code(const QWidget *context);
QLocale effective_locale(const QWidget *context);
QString effective_date_format(const QWidget *context);
void translate_standard_buttons(QDialog *dialog, const QWidget *context);
MissingLinkedFileChoice prompt_missing_linked_file(QWidget *parent = nullptr);
QString browse_linked_file_replacement(QWidget *parent, const QString &current_path = QString());
void show_link_removed_success(QWidget *parent = nullptr);
bool open_local_path(const QString &path, QWidget *parent = nullptr);
QString strip_priority_emoji(const QString &text);
QString normalized_task_text(const QString &text);

template <typename TaskLike>
QString task_identity_signature(const TaskLike &task)
{
    const QString id = normalized_task_text(task.id);
    if (!id.isEmpty()) {
        return QStringLiteral("id:%1").arg(id);
    }

    return QStringLiteral("sig:%1|%2|%3|%4|%5")
        .arg(normalized_task_text(task.titulo),
             normalized_task_text(task.prioridade),
             normalized_task_text(task.date),
             normalized_task_text(task.time),
             normalized_task_text(task.descricao));
}

template <typename TaskLike>
bool tasks_match(const TaskLike &task, const QVariantMap &candidate)
{
    const QString current_id = normalized_task_text(task.id);
    const QString candidate_id = normalized_task_text(candidate.value(QStringLiteral("id")).toString());

    if (!current_id.isEmpty() && !candidate_id.isEmpty()) {
        return current_id == candidate_id;
    }

    return normalized_task_text(task.titulo) == normalized_task_text(candidate.value(QStringLiteral("titulo")).toString())
        && normalized_task_text(task.prioridade) == normalized_task_text(candidate.value(QStringLiteral("prioridade")).toString())
        && normalized_task_text(task.date) == normalized_task_text(candidate.value(QStringLiteral("date")).toString())
        && normalized_task_text(task.time) == normalized_task_text(candidate.value(QStringLiteral("time")).toString())
        && normalized_task_text(task.descricao) == normalized_task_text(candidate.value(QStringLiteral("description")).toString());
}

}  // namespace source::plugin::Pomodoro::compat

namespace source::plugin::Pomodoro::layout {

struct UiLayoutParts {
    QString mode;
    QString panel;
};

inline UiLayoutParts normalized_ui_layout_parts(const QString &raw)
{
    QString key = raw.trimmed().toLower();
    QStringList parts;
    parts.reserve(2);
    QString current;

    for (QChar ch : key) {
        if (ch == QLatin1Char(':') || ch == QLatin1Char('/') || ch == QLatin1Char('-') || ch == QLatin1Char('_')
            || ch == QLatin1Char('|')) {
            if (!current.isEmpty()) {
                parts.append(current);
                current.clear();
            }
            continue;
        }
        current.append(ch);
    }

    if (!current.isEmpty()) {
        parts.append(current);
    }

    QString mode = parts.value(0);
    if (mode != QStringLiteral("robust")) {
        mode = QStringLiteral("simple");
    }

    QString panel = parts.value(1);
    if (panel != QStringLiteral("vertical") && panel != QStringLiteral("horizontal")) {
        panel = QStringLiteral("tabs");
    }

    return {mode, panel};
}

inline QString normalized_ui_layout_key(const QString &raw)
{
    const UiLayoutParts parts = normalized_ui_layout_parts(raw);
    return QStringLiteral("%1:%2").arg(parts.mode, parts.panel);
}

inline QString normalized_base_layout_key(const QString &raw)
{
    return normalized_ui_layout_parts(raw).mode;
}

inline QString normalized_panel_layout_key(const QString &raw)
{
    return normalized_ui_layout_parts(raw).panel;
}

inline bool is_robust_ui_layout(const QString &raw)
{
    return normalized_base_layout_key(raw) == QStringLiteral("robust");
}

inline bool is_tabs_panel_layout(const QString &raw)
{
    return normalized_panel_layout_key(raw) == QStringLiteral("tabs");
}

inline bool is_vertical_panel_layout(const QString &raw)
{
    return normalized_panel_layout_key(raw) == QStringLiteral("vertical");
}

inline bool is_horizontal_panel_layout(const QString &raw)
{
    return normalized_panel_layout_key(raw) == QStringLiteral("horizontal");
}

}  // namespace source::plugin::Pomodoro::layout
