#include "source/modules/gantt/gantt_plugin.hpp"

#include <QCoreApplication>
#include <QMessageBox>
#include <QThread>
#include <QWidget>

#include "source/modules/gantt/Gantt/application/services/GanttTaskIntegrationBridge.hpp"
#include "source/modules/gantt/Gantt/infrastructure/persistence/GanttDatabase.hpp"
#include "source/modules/gantt/Gantt/language/GanttTranslationWorker.hpp"
#include "source/modules/gantt/Gantt/language/tr_01_gerenciadorTraducao.hpp"
#include "source/modules/gantt/Gantt/ui/GanttMainWindow.hpp"
#include "source/modules/gantt/Gantt/ui/TaskLinkDialog.hpp"

namespace source::plugin {

GanttPlugin::GanttPlugin(QObject *parent)
    : QObject(parent)
{
    translation_manager_ = new source::gantt::language::GerenciadorTraducao(this);

    translation_thread_ = new QThread(this);
    translation_worker_ = new source::gantt::language::GanttTranslationWorker();
    translation_worker_->moveToThread(translation_thread_);

    connect(translation_thread_, &QThread::finished, translation_worker_, &QObject::deleteLater);
    connect(translation_worker_, &source::gantt::language::GanttTranslationWorker::translation_prepared,
            this, &GanttPlugin::on_translation_prepared, Qt::QueuedConnection);

    translation_thread_->start();
}

GanttPlugin::~GanttPlugin()
{
    if (translation_thread_) {
        translation_thread_->quit();
        translation_thread_->wait();
    }
    if (window_) {
        delete window_;
        window_ = nullptr;
    }
}

QString GanttPlugin::plugin_id() const
{
    return QStringLiteral("gantt");
}

QString GanttPlugin::display_name() const
{
    return QCoreApplication::translate("GanttPlugin", "Cronograma de Gantt & Engenharia 4D BIM");
}

QString GanttPlugin::open_menu_text() const
{
    return QCoreApplication::translate("GanttPlugin", "Cronograma de Gantt BIM 4D");
}

void GanttPlugin::initialize(GanttHostBridge *host_bridge)
{
    if (host_bridge_ != nullptr && host_bridge_->eisenhower_host() != nullptr) {
        source::gantt::GanttTaskIntegrationBridge::instance().unregister_host(host_bridge_->eisenhower_host());
    }

    host_bridge_ = host_bridge;

    if (host_bridge_ != nullptr && host_bridge_->eisenhower_host() != nullptr) {
        source::gantt::GanttTaskIntegrationBridge::instance().register_host(host_bridge_->eisenhower_host());
    }

    apply_host_language();
}

void GanttPlugin::open_window(QWidget *parent)
{
    source::gantt::GanttMainWindow *win = ensure_window(parent);
    if (!win) return;

    win->show();
    win->raise();
    win->activateWindow();

    if (needs_eisenhower_sync_) {
        needs_eisenhower_sync_ = false;
        QMetaObject::invokeMethod(win, [win]() {
            win->sync_all_eisenhower_tasks(true);
        }, Qt::QueuedConnection);
    }
}

void GanttPlugin::open_gantt_and_highlight_activity(const QString &schedule_item_id, QWidget *parent)
{
    source::gantt::GanttMainWindow *win = ensure_window(parent);
    if (!win) return;

    win->show();
    win->raise();
    win->activateWindow();

    if (!schedule_item_id.isEmpty()) {
        win->on_activity_selected(schedule_item_id);
    }

    if (needs_eisenhower_sync_) {
        needs_eisenhower_sync_ = false;
        QMetaObject::invokeMethod(win, [win]() {
            win->sync_all_eisenhower_tasks(true);
        }, Qt::QueuedConnection);
    }
}

bool GanttPlugin::is_task_linked(const QString &task_id) const
{
    return source::gantt::GanttTaskIntegrationBridge::instance().is_task_linked(task_id);
}

bool GanttPlugin::get_task_gantt_summary(const QString &task_id,
                                        QString *out_wbs_and_name,
                                        QString *out_sched_name,
                                        double *out_progress,
                                        QString *out_status) const
{
    QString sched_id, sched_name, item_id, act_name, wbs_code, status;
    double progress = 0.0;
    const bool linked = source::gantt::GanttTaskIntegrationBridge::instance().is_task_linked(
        task_id, &sched_id, &sched_name, &item_id, &act_name, &wbs_code, &progress, &status);
    if (!linked) {
        return false;
    }
    if (out_wbs_and_name) {
        *out_wbs_and_name = wbs_code.isEmpty() ? act_name : QStringLiteral("[%1] %2").arg(wbs_code, act_name);
    }
    if (out_sched_name) *out_sched_name = sched_name;
    if (out_progress) *out_progress = progress;
    if (out_status) *out_status = status;
    return true;
}

bool GanttPlugin::convert_eisenhower_task_to_gantt_activity(const QVariantMap &task,
                                                           QString *out_item_id,
                                                           QString *error_message)
{
    return source::gantt::GanttTaskIntegrationBridge::instance().convert_eisenhower_task_to_gantt_activity(
        task, out_item_id, error_message);
}

bool GanttPlugin::link_task_to_activity(const QString &task_id,
                                       const QString &activity_id,
                                       const QString &schedule_id,
                                       QString *error_message)
{
    return source::gantt::GanttTaskIntegrationBridge::instance().link_task_to_activity(
        task_id, activity_id, schedule_id, error_message);
}

bool GanttPlugin::unlink_task(const QString &task_id, QString *error_message)
{
    return source::gantt::GanttTaskIntegrationBridge::instance().unlink_task(task_id, error_message);
}

bool GanttPlugin::open_task_link_dialog(const QString &task_id,
                                       const QString &task_title,
                                       QWidget *parent)
{
    auto &db = source::gantt::GanttDatabase::instance();
    source::gantt::Schedule schedule = db.load_default_schedule();
    if (schedule.id.isEmpty()) {
        const QList<source::gantt::Schedule> scheds = db.list_schedules();
        if (!scheds.isEmpty()) {
            schedule = scheds.first();
        }
    }
    if (schedule.id.isEmpty()) {
        QMessageBox::information(
            parent,
            display_name(),
            QObject::tr("Nenhum cronograma de obra encontrado. Crie ou abra um cronograma no menu 'Cronograma de Gantt & BIM 4D' primeiro."));
        return false;
    }

    source::gantt::TaskLinkDialog dlg(task_id, task_title, schedule, parent);
    if (dlg.exec() == QDialog::Accepted) {
        QString err;
        return link_task_to_activity(task_id, dlg.selected_activity_id(), schedule.id, &err);
    }
    return false;
}

bool GanttPlugin::record_field_progress_for_task(const QString &task_id,
                                                double percent,
                                                const QString &notes,
                                                const QString &reporter,
                                                QString *error_message)
{
    return source::gantt::GanttTaskIntegrationBridge::instance().record_field_progress_for_task(
        task_id, percent, notes, reporter, error_message);
}

void GanttPlugin::on_eisenhower_task_completed(const QString &task_id, bool is_completed)
{
    QMetaObject::invokeMethod(&source::gantt::GanttTaskIntegrationBridge::instance(), [task_id, is_completed]() {
        source::gantt::GanttTaskIntegrationBridge::instance().on_eisenhower_task_completed(task_id, is_completed);
    }, Qt::QueuedConnection);
}

void GanttPlugin::on_eisenhower_task_status_changed(const QString &task_id, const QString &status)
{
    QMetaObject::invokeMethod(&source::gantt::GanttTaskIntegrationBridge::instance(), [task_id, status]() {
        const QString s = status.trimmed().toLower();
        if (s == QStringLiteral("in_progress") ||
            s == QStringLiteral("in progress") ||
            s == QStringLiteral("doing") ||
            s == QStringLiteral("em_progresso") ||
            s == QStringLiteral("em progresso") ||
            s == QStringLiteral("em_andamento") ||
            s == QStringLiteral("em andamento") ||
            s.contains(QStringLiteral("progress")) ||
            s.contains(QStringLiteral("andamento"))) {
            source::gantt::GanttTaskIntegrationBridge::instance().on_eisenhower_task_in_progress(task_id);
        } else if (s == QStringLiteral("done") ||
                   s == QStringLiteral("completed") ||
                   s == QStringLiteral("concluida") ||
                   s == QStringLiteral("concluído") ||
                   s == QStringLiteral("concluido")) {
            source::gantt::GanttTaskIntegrationBridge::instance().on_eisenhower_task_completed(task_id, true);
        } else {
            source::gantt::GanttTaskIntegrationBridge::instance().on_eisenhower_task_completed(task_id, false);
        }
    }, Qt::QueuedConnection);
}

void GanttPlugin::on_eisenhower_task_removed(const QString &task_id)
{
    QMetaObject::invokeMethod(&source::gantt::GanttTaskIntegrationBridge::instance(), [task_id]() {
        source::gantt::GanttTaskIntegrationBridge::instance().on_eisenhower_task_removed(task_id);
    }, Qt::QueuedConnection);
}

void GanttPlugin::on_eisenhower_task_duration_changed(const QString &task_id, double duration_days)
{
    QMetaObject::invokeMethod(&source::gantt::GanttTaskIntegrationBridge::instance(), [task_id, duration_days]() {
        source::gantt::GanttTaskIntegrationBridge::instance().on_eisenhower_task_duration_changed(task_id, duration_days);
    }, Qt::QueuedConnection);
}

void GanttPlugin::on_eisenhower_task_modified(const QString &task_id)
{
    QMetaObject::invokeMethod(&source::gantt::GanttTaskIntegrationBridge::instance(), [task_id]() {
        source::gantt::GanttTaskIntegrationBridge::instance().on_eisenhower_task_modified(task_id);
    }, Qt::QueuedConnection);
    if (window_ != nullptr && window_->isVisible()) {
        sync_all_tasks();
    }
}

void GanttPlugin::sync_all_tasks()
{
    if (window_ != nullptr && window_->isVisible()) {
        needs_eisenhower_sync_ = false;
        source::gantt::GanttMainWindow *win = window_.data();
        QMetaObject::invokeMethod(win, [win]() {
            if (win != nullptr && win->isVisible()) {
                win->sync_all_eisenhower_tasks(false);
            }
        }, Qt::QueuedConnection);
    } else {
        needs_eisenhower_sync_ = true;
    }
}

void GanttPlugin::retranslate_ui()
{
    const QString code = host_bridge_ ? host_bridge_->language_code() : QStringLiteral("pt_BR");

    // Fast path em memória (0ms): se o tradutor já estiver no cache, instala sem I/O
    if (translation_manager_ != nullptr) {
        translation_manager_->aplicar_traducao(code);
    }

    // Thread dedicada: resolve arquivos .qm e diretórios em worker thread fora da GUI
    if (translation_worker_ != nullptr) {
        QMetaObject::invokeMethod(translation_worker_, "prepare_translation",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, code));
    }

    // Retraduz a janela se ela existir
    if (window_ != nullptr) {
        window_->retranslate_ui();
    }
}

void GanttPlugin::on_translation_prepared(const QString &language_code, const QString &qm_file_path)
{
    if (translation_manager_ != nullptr && !qm_file_path.isEmpty()) {
        translation_manager_->carregar_traducao_direta(language_code, qm_file_path);
    }

    if (window_ != nullptr) {
        window_->retranslate_ui();
    }
}

void GanttPlugin::close_window()
{
    if (window_) {
        window_->close();
    }
}

namespace {
class ParentCloseEventFilter : public QObject {
public:
    explicit ParentCloseEventFilter(QWidget *target, QObject *parent = nullptr)
        : QObject(parent), target_(target) {}
protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event && event->type() == QEvent::Close && target_) {
            target_->close();
        }
        return QObject::eventFilter(obj, event);
    }
private:
    QPointer<QWidget> target_;
};
} // namespace

source::gantt::GanttMainWindow *GanttPlugin::ensure_window(QWidget *parent)
{
    if (!window_) {
        window_ = new source::gantt::GanttMainWindow(nullptr);
        window_->setWindowFlag(Qt::Window, true);
        window_->setAttribute(Qt::WA_DeleteOnClose, false);
        window_->retranslate_ui();

        if (parent != nullptr) {
            QObject::disconnect(host_destroyed_connection_);
            host_destroyed_connection_ = QObject::connect(parent, &QObject::destroyed, this, [this]() {
                if (window_ != nullptr) {
                    delete window_;
                    window_ = nullptr;
                }
                host_destroyed_connection_ = {};
            });
            parent->installEventFilter(new ParentCloseEventFilter(window_, parent));
        }
    }
    return window_;
}

void GanttPlugin::apply_host_language()
{
    if (translation_manager_ == nullptr) {
        return;
    }

    const QString code = host_bridge_ ? host_bridge_->language_code() : QStringLiteral("pt_BR");
    translation_manager_->aplicar_traducao(code);

    if (translation_worker_ != nullptr) {
        QMetaObject::invokeMethod(translation_worker_, "prepare_translation",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, code));
    }
}

} // namespace source::plugin
