#include "source/app/app_01_eisenhower_matrix_app.hpp"
#include "source/app/app_02_get_text.hpp"

namespace source {
using namespace app;

EisenhowerMatrixApp::EisenhowerMatrixApp(source::language::GerenciadorTraducao *translation_manager, QWidget *parent)
    : QMainWindow(parent), gerenciador_traducao(translation_manager)
{
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: iniciando construtor."));

    connect(this->gerenciador_traducao,
            &source::language::GerenciadorTraducao::idioma_alterado,
            this,
            [this](const QString &) { atualizar_textos(); });

    pomodoro_host_bridge_ = new source::plugin::PomodoroHostBridge();
    pomodoro_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
    pomodoro_host_bridge_->set_standard_buttons_handler([this](QObject *dialog) {
        if (gerenciador_traducao != nullptr) {
            gerenciador_traducao->traduzir_botoes_padrao(dialog);
        }
    });
    pomodoro_host_bridge_->set_task_to_eisenhower_handler(
        [this](const QVariantMap &task) { _upsert_task_from_external(task); });
    pomodoro_host_bridge_->set_task_to_feynman_handler(
        [this](const QVariantMap &task) { return _send_task_payload_to_feynman(task); });
    pomodoro_host_bridge_->set_feynman_available_checker([this]() { return has_feynman_plugin(); });
    pomodoro_host_bridge_->set_shared_state_changed_handler(
        [this](const QString &origin) { _queue_runtime_sync(origin); });
    pomodoro_host_bridge_->set_sync_operation_info_handler([this](const QString &operation, const QString &origin) {
        if (operation.compare(QStringLiteral("delete"), Qt::CaseInsensitive) == 0) {
            show_task_sync_delete_info(origin);
        } else {
            show_task_sync_add_info(origin);
        }
    });
    pomodoro_host_bridge_->set_task_deleted_handler([this](const QVariantMap &task, const QString &origin) {
        remove_task_from_synced_modules(task, origin);
    });
    pomodoro_host_bridge_->set_task_sort_state_checker([this]() { return has_active_task_sort(); });
    pomodoro_host_bridge_->set_task_sort_less_handler([this](const QVariantMap &left, const QVariantMap &right) {
        return task_sort_less_for_payloads(left, right);
    });
    pomodoro_host_bridge_->set_reserved_space_checker([this]() { return reserved_space_pomodoro_enabled(); });
    pomodoro_host_bridge_->set_watermark_checker([this]() { return reserved_watermark_pomodoro_enabled(); });
    pomodoro_host_bridge_->set_task_sync_enabled_checker([this]() { return sync_pomodoro_enabled(); });
    pomodoro_host_bridge_->set_gantt_available_checker([this]() { return has_gantt_plugin(); });
    pomodoro_host_bridge_->set_gantt_link_handler([this](const QString &task_id, const QString &task_title, QWidget *parent) {
        if (!has_gantt_plugin() || gantt_plugin_ == nullptr) return false;
        return gantt_plugin_->open_task_link_dialog(task_id, task_title, parent != nullptr ? parent : this);
    });
    pomodoro_host_bridge_->set_task_duration_sync_handler([this](const QString &task_id, double duration_days) {
        sync_task_duration(task_id, duration_days);
    });
    pomodoro_host_bridge_->set_gantt_summary_handler(
        [this](const QString &task_id, QString *wbs, QString *sched, double *prog, QString *status) {
            if (!has_gantt_plugin() || gantt_plugin_ == nullptr) return false;
            return gantt_plugin_->get_task_gantt_summary(task_id, wbs, sched, prog, status);
        });
    pomodoro_host_bridge_->set_gantt_highlight_handler([this](const QString &task_id, QWidget *parent) {
        if (!has_gantt_plugin() || gantt_plugin_ == nullptr) return;
        gantt_plugin_->open_gantt_and_highlight_activity(task_id, parent != nullptr ? parent : this);
    });
    pomodoro_host_bridge_->set_gantt_record_progress_handler(
        [this](const QString &task_id, double percent, const QString &notes, const QString &author, QString *error_msg) {
            if (!has_gantt_plugin() || gantt_plugin_ == nullptr) return false;
            return gantt_plugin_->record_field_progress_for_task(task_id, percent, notes, author, error_msg);
        });
    pomodoro_host_bridge_->set_gantt_unlink_handler([this](const QString &task_id) {
        if (!has_gantt_plugin() || gantt_plugin_ == nullptr) return false;
        return gantt_plugin_->unlink_task(task_id);
    });
    pomodoro_host_bridge_->set_default_cycle_count_changed_handler([this](int new_count) {
        on_default_pomodoro_cycle_count_changed(new_count);
    });
    pomodoro_host_bridge_->set_reset_in_progress_cycles_handler([this](int new_cycles, const QVariantList &affected_tasks) {
        reset_in_progress_task_cycles_across_modules(new_cycles, affected_tasks);
    });

    feynman_host_bridge_ = new source::plugin::FeynmanHostBridge();
    feynman_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
    feynman_host_bridge_->set_standard_buttons_handler([this](QObject *dialog) {
        if (gerenciador_traducao != nullptr) {
            gerenciador_traducao->traduzir_botoes_padrao(dialog);
        }
    });
    feynman_host_bridge_->set_task_to_eisenhower_handler(
        [this](const QVariantMap &task) { return _upsert_task_from_external(task); });
    feynman_host_bridge_->set_eisenhower_available_checker([]() { return true; });
    feynman_host_bridge_->set_task_to_pomodoro_handler([this](const QVariantMap &task) {
        if (!has_pomodoro_plugin() || pomodoro_plugin_ == nullptr) {
            return false;
        }

        QVariantMap payload = task;
        if (payload.value(QStringLiteral("origin")).toString().trimmed().isEmpty()) {
            payload.insert(QStringLiteral("origin"), QStringLiteral("feynman"));
        }

        pomodoro_plugin_->receive_eisenhower_task(payload);
        return true;
    });
    feynman_host_bridge_->set_pomodoro_available_checker([this]() { return has_pomodoro_plugin(); });
    feynman_host_bridge_->set_shared_state_changed_handler(
        [this](const QString &origin) { _queue_runtime_sync(origin); });
    feynman_host_bridge_->set_sync_operation_info_handler([this](const QString &operation, const QString &origin) {
        if (operation.compare(QStringLiteral("delete"), Qt::CaseInsensitive) == 0) {
            show_task_sync_delete_info(origin);
        } else {
            show_task_sync_add_info(origin);
        }
    });
    feynman_host_bridge_->set_task_deleted_handler([this](const QVariantMap &task, const QString &origin) {
        remove_task_from_synced_modules(task, origin);
    });
    feynman_host_bridge_->set_task_sort_state_checker([this]() { return has_active_task_sort(); });
    feynman_host_bridge_->set_task_sort_less_handler([this](const QVariantMap &left, const QVariantMap &right) {
        return task_sort_less_for_payloads(left, right);
    });
    feynman_host_bridge_->set_reserved_space_checker([this]() { return reserved_space_feynman_enabled(); });
    feynman_host_bridge_->set_watermark_checker([this]() { return reserved_watermark_feynman_enabled(); });
    feynman_host_bridge_->set_task_sync_enabled_checker([this]() { return sync_feynman_enabled(); });
    feynman_host_bridge_->set_gantt_available_checker([this]() { return has_gantt_plugin(); });
    feynman_host_bridge_->set_gantt_link_handler([this](const QString &task_id, const QString &task_title, QWidget *parent) {
        if (!has_gantt_plugin() || gantt_plugin_ == nullptr) return false;
        return gantt_plugin_->open_task_link_dialog(task_id, task_title, parent != nullptr ? parent : this);
    });
    feynman_host_bridge_->set_task_duration_sync_handler([this](const QString &task_id, double duration_days) {
        sync_task_duration(task_id, duration_days);
    });
    feynman_host_bridge_->set_gantt_summary_handler(
        [this](const QString &task_id, QString *wbs, QString *sched, double *prog, QString *status) {
            if (!has_gantt_plugin() || gantt_plugin_ == nullptr) return false;
            return gantt_plugin_->get_task_gantt_summary(task_id, wbs, sched, prog, status);
        });
    feynman_host_bridge_->set_gantt_highlight_handler([this](const QString &task_id, QWidget *parent) {
        if (!has_gantt_plugin() || gantt_plugin_ == nullptr) return;
        gantt_plugin_->open_gantt_and_highlight_activity(task_id, parent != nullptr ? parent : this);
    });
    feynman_host_bridge_->set_gantt_record_progress_handler(
        [this](const QString &task_id, double percent, const QString &notes, const QString &author, QString *error_msg) {
            if (!has_gantt_plugin() || gantt_plugin_ == nullptr) return false;
            return gantt_plugin_->record_field_progress_for_task(task_id, percent, notes, author, error_msg);
        });
    feynman_host_bridge_->set_gantt_unlink_handler([this](const QString &task_id) {
        if (!has_gantt_plugin() || gantt_plugin_ == nullptr) return false;
        return gantt_plugin_->unlink_task(task_id);
    });
    feynman_host_bridge_->set_pomodoro_default_cycle_count_handler([this]() {
        return default_pomodoro_cycle_count();
    });

    mapamental_host_bridge_ = new source::plugin::MapaMentalHostBridge();
    mapamental_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
    mapamental_host_bridge_->set_standard_buttons_handler([this](QObject *dialog) {
        if (gerenciador_traducao != nullptr) {
            gerenciador_traducao->traduzir_botoes_padrao(dialog);
        }
    });
    mapamental_host_bridge_->set_model_availability_checker([this]() {
        if (modelo_plugin_ == nullptr) {
            return false;
        }
        const QString method = mapamental_summary_method_key();
        if (method.compare(QStringLiteral("heuristic"), Qt::CaseInsensitive) == 0) {
            return false;
        }
        modelo_plugin_->set_active_model(method);
        return modelo_plugin_->is_available();
    });
    mapamental_host_bridge_->set_model_diagnostic_handler([this]() {
        if (modelo_plugin_ == nullptr) {
            return QVariantMap();
        }
        modelo_plugin_->set_active_model(mapamental_summary_method_key());
        return modelo_plugin_->diagnostic();
    });
    mapamental_host_bridge_->set_model_analysis_handler(
        [this](const QString &text,
               const QString &language,
               const QVariantList &concepts,
               const QVariantList &relations) {
            if (modelo_plugin_ == nullptr) {
                return QVariantMap();
            }
            modelo_plugin_->set_active_model(mapamental_summary_method_key());
            return modelo_plugin_->analyze_document(text, language, concepts, relations);
        });

    audiolivro_host_bridge_ = new source::plugin::AudiolivroHostBridge();
    audiolivro_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
    audiolivro_host_bridge_->set_standard_buttons_handler([this](QObject *dialog) {
        if (gerenciador_traducao != nullptr) {
            gerenciador_traducao->traduzir_botoes_padrao(dialog);
        }
    });

    agenda_host_bridge_ = new source::plugin::AgendaHostBridge();
    agenda_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
    agenda_host_bridge_->set_persistent_dir(source::utils::obter_caminho_persistente());
    agenda_host_bridge_->set_standard_buttons_handler([this](QObject *dialog) {
        if (gerenciador_traducao != nullptr) {
            gerenciador_traducao->traduzir_botoes_padrao(dialog);
        }
    });
    agenda_host_bridge_->set_task_to_eisenhower_handler([this](const QVariantMap &task) {
        const bool inserted = _upsert_task_from_external(task);
        if (inserted) {
            show_task_sync_add_info(QStringLiteral("agenda"));
        }
        return inserted;
    });
    agenda_host_bridge_->set_color_mode_changed_handler(
        [this](const QString &mode) { _apply_agenda_color_mode(mode); });

    gantt_host_bridge_ = new source::plugin::GanttHostBridge();
    gantt_host_bridge_->set_eisenhower_host(this);
    gantt_host_bridge_->set_language_code(gerenciador_traducao->obter_idioma_atual());
    gantt_host_bridge_->set_persistent_dir(source::utils::obter_caminho_persistente());

    setWindowTitle(source::utils::titulo_janela_principal());
    setMinimumSize(kInitialWindowMinWidth, kInitialWindowMinHeight);
    resize(1380, 840);
    move(50, 50);

    const QString icon_path = source::utils::get_default_window_icon_path();
    if (!icon_path.isEmpty()) {
        setWindowIcon(QIcon(icon_path));
    }

    const QString persistent_dir = source::utils::obter_caminho_persistente();
    tasks_path = source::utils::UnifiedTaskStore::shared_tasks_path();
    layout_path = QDir(persistent_dir).filePath(QStringLiteral("layout.json"));
    const QString documents_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    export_path = QDir(documents_dir.isEmpty() ? QDir::homePath() : documents_dir)
                      .filePath(QStringLiteral("tarefas.json"));
    const QString legacy_unified_tasks_path =
        QDir(persistent_dir).filePath(QStringLiteral("eisenhower_tarefas.json"));
    const QString legacy_tasks_path = QDir(persistent_dir).filePath(QStringLiteral("tasks.json"));

    if (!QFileInfo::exists(tasks_path)) {
        if (QFileInfo::exists(legacy_unified_tasks_path)) {
            QFile::copy(legacy_unified_tasks_path, tasks_path);
        } else if (QFileInfo::exists(legacy_tasks_path)) {
            QFile::copy(legacy_tasks_path, tasks_path);
        }
    }
    if (QFileInfo::exists(tasks_path)) {
        remove_legacy_task_datastores();
    }

    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregando preferências e UI principal."));
    _load_task_layout_preference();
    arquivo = new source::InterfaceCore::Arquivo(this);
    initUI();
    _apply_startup_geometry_constraints();
    _apply_theme_palette();
    load_tasks();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregando plugin Feynman."));
    _load_feynman_plugin();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregamento do plugin Feynman finalizado."));
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregando plugin Modelo."));
    _load_modelo_plugin();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregamento do plugin Modelo finalizado."));
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregando plugin Mapa Mental."));
    _load_mapamental_plugin();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregamento do plugin Mapa Mental finalizado."));
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregando plugin Audiolivro."));
    _load_audiolivro_plugin();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregamento do plugin Audiolivro finalizado."));
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregando plugin Agenda."));
    _load_agenda_plugin();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregamento do plugin Agenda finalizado."));
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregando plugin Gantt."));
    _load_gantt_plugin();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregamento do plugin Gantt finalizado."));
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregando plugin Pomodoro."));
    _load_pomodoro_plugin();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: carregamento do plugin Pomodoro finalizado."));
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: aplicando restrições finais de geometria."));
    _apply_startup_geometry_constraints();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: restrições finais de geometria aplicadas."));
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: criando menu de configurações."));
    criar_menu_configuracoes();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: menu de configurações criado."));
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: agendando estabilização de geometria."));
    _queue_window_geometry_stabilization();
    logger()->debug(QStringLiteral("EisenhowerMatrixApp: construtor finalizado."));
}

EisenhowerMatrixApp::~EisenhowerMatrixApp()
{
    // 1. Desinicializa os plugins para que eles limpem seus recursos internos e desconectem sinais.
    if (feynman_plugin_) feynman_plugin_->initialize(nullptr);
    if (pomodoro_plugin_) pomodoro_plugin_->initialize(nullptr);
    if (mapamental_plugin_) mapamental_plugin_->initialize(nullptr);
    if (audiolivro_plugin_) audiolivro_plugin_->initialize(nullptr);
    if (agenda_plugin_) agenda_plugin_->initialize(nullptr);
    if (gantt_plugin_) {
        gantt_plugin_->close_window();
        gantt_plugin_->initialize(nullptr);
    }

    // 2. Deleta explicitamente os widgets de UI criados pelos plugins.
    if (feynman_module_page) delete feynman_module_page;
    if (pomodoro_module_page) delete pomodoro_module_page;
    if (mapamental_module_page) delete mapamental_module_page;
    if (audiolivro_module_page) delete audiolivro_module_page;

    // 3. Não descarregamos manualmente as DLLs dos plugins.
    //    O ExitProcess() chamado em main.cpp encerra o processo de forma limpa,
    //    sem executar os destrutores globais do CRT que causam Access Violation
    //    no motor pdfium (Qt6Pdf.dll) quando um PDF foi carregado.

    QCoreApplication::removePostedEvents(this);
    blockSignals(true);

    const QList<QObject *> child_objects = findChildren<QObject *>();
    for (QObject *child : child_objects) {
        if (child == nullptr) {
            continue;
        }

        child->blockSignals(true);
        QObject::disconnect(child, nullptr, this, nullptr);
        QObject::disconnect(this, nullptr, child, nullptr);
    }

    feynman_plugin_ = nullptr;
    pomodoro_plugin_ = nullptr;
    mapamental_plugin_ = nullptr;
    modelo_plugin_ = nullptr;
    audiolivro_plugin_ = nullptr;
    agenda_plugin_ = nullptr;
    gantt_plugin_ = nullptr;
    feynman_module_page = nullptr;
    pomodoro_module_page = nullptr;
    mapamental_module_page = nullptr;
    audiolivro_module_page = nullptr;

    delete feynman_host_bridge_;
    feynman_host_bridge_ = nullptr;

    delete pomodoro_host_bridge_;
    pomodoro_host_bridge_ = nullptr;

    delete mapamental_host_bridge_;
    mapamental_host_bridge_ = nullptr;

    delete audiolivro_host_bridge_;
    audiolivro_host_bridge_ = nullptr;

    delete agenda_host_bridge_;
    agenda_host_bridge_ = nullptr;

    delete gantt_host_bridge_;
    gantt_host_bridge_ = nullptr;
}

void EisenhowerMatrixApp::closeEvent(QCloseEvent *event)
{
    save_tasks_now();
    _flush_runtime_sync();

    for (QPointer<QDialog> dialog : {_sobre_dialog, _manual_dialog, _plugin_manager_dialog, _log_manager_dialog}) {
        if (dialog != nullptr) {
            dialog->close();
        }
    }

    QMainWindow::closeEvent(event);
}

void EisenhowerMatrixApp::changeEvent(QEvent *event)
{
    if (event != nullptr && (event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::PaletteChange
                             || event->type() == QEvent::StyleChange || event->type() == QEvent::ThemeChange)) {
        QTimer::singleShot(0, this, [this]() { _apply_theme_palette(); });
    } else if (event != nullptr
               && (event->type() == QEvent::WindowStateChange || event->type() == QEvent::ActivationChange
                   || event->type() == QEvent::ScreenChangeInternal)) {
        _queue_window_geometry_stabilization();
    }

    QMainWindow::changeEvent(event);
}

void EisenhowerMatrixApp::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    _queue_window_geometry_stabilization(80);
}

}  // namespace source
