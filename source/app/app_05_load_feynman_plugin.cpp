#include "source/app/app_05_load_feynman_plugin.hpp"
#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/UnifiedTaskStore.hpp"
#if !defined(EISENHOWER_MODULAR_PLUGINS)
#include "source/modules/feynman/feynman_plugin.hpp"
#include "source/modules/pomodoro/pomodoro_plugin.hpp"
#include "source/modules/mapamental/mapamental_plugin.hpp"
#include "source/modules/modelo/modelo_plugin.hpp"
#include "source/modules/audiolivro/audiolivro_plugin.hpp"
#include "source/modules/agenda/agenda_plugin.hpp"
#include "source/modules/gantt/gantt_plugin.hpp"
#include "source/modules/modelo/metodoModelo/ollama/ia_02_ollama_local.hpp"
#include "source/modules/modelo/metodoModelo/qwen/ia_01_llama_local.hpp"
#endif
#include "source/ui/ui_17_OllamaInstrucoesDialog.hpp"
#include "source/ui/ui_18_ModelDownloadDialog.hpp"
#include "source/services/incore_03_criar_menu_configuracoes.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPluginLoader>
#include <QSet>

namespace source {
using namespace app;

#if defined(EISENHOWER_MODULAR_PLUGINS)
namespace {

template <typename InterfaceType>
InterfaceType *load_dynamic_plugin(QObject *owner,
                                   const QString &plugin_key,
                                   const QStringList &candidates,
                                   QPluginLoader *&loader_slot,
                                   QString &path_slot,
                                   QMap<QString, QString> &error_map,
                                   source::utils::LogManager *log)
{
    QStringList search_path_entries{QCoreApplication::applicationDirPath()};
    QStringList existing_candidates;
    for (const QString &candidate : candidates) {
        const QFileInfo info(candidate);
        if (!info.exists() || !info.isFile()) {
            continue;
        }
        const QString abs_path = info.absoluteFilePath();
        if (!existing_candidates.contains(abs_path)) {
            existing_candidates.append(abs_path);
        }
        const QString dir_path = info.absolutePath();
        if (!search_path_entries.contains(dir_path)) {
            search_path_entries.append(dir_path);
        }
    }

    prepend_process_path(search_path_entries);

    if (existing_candidates.isEmpty()) {
        const QString err = QStringLiteral("DLL do plugin '%1' não encontrada em nenhum caminho candidato.").arg(plugin_key);
        error_map.insert(plugin_key, err);
        if (log != nullptr) {
            log->warning(err);
        }
        return nullptr;
    }

    QString last_error;
    for (const QString &candidate : std::as_const(existing_candidates)) {
        auto *loader = new QPluginLoader(candidate, owner);
        QObject *instance = loader->instance();
        if (instance == nullptr) {
            last_error = loader->errorString();
            delete loader;
            continue;
        }

        auto *typed = qobject_cast<InterfaceType *>(instance);
        if (typed == nullptr) {
            last_error = QStringLiteral("Objeto raiz de '%1' não implementa a interface esperada (%2).")
                             .arg(candidate, plugin_key);
            loader->unload();
            delete loader;
            continue;
        }

        loader_slot = loader;
        path_slot = candidate;
        error_map.remove(plugin_key);
        return typed;
    }

    if (last_error.isEmpty()) {
        last_error = QStringLiteral("Falha desconhecida ao carregar o plugin '%1'.").arg(plugin_key);
    }
    error_map.insert(plugin_key, last_error);
    if (log != nullptr) {
        log->warning(QStringLiteral("Falha ao carregar plugin dinâmico '%1': %2").arg(plugin_key, last_error));
    }
    return nullptr;
}

}  // namespace
#endif

void EisenhowerMatrixApp::_load_feynman_plugin()
{
    if (feynman_plugin_ != nullptr) {
        return;
    }

#if defined(EISENHOWER_MODULAR_PLUGINS)
    auto *plugin = load_dynamic_plugin<source::plugin::IFeynmanPlugin>(
        this,
        QStringLiteral("feynman"),
        feynman_plugin_candidates(),
        feynman_plugin_loader_,
        feynman_plugin_path_,
        plugin_load_errors_,
        logger());
    if (plugin == nullptr) {
        return;
    }
#else
    auto *plugin = new source::plugin::FeynmanPlugin();
    plugin->setParent(this);
    feynman_plugin_path_ = QStringLiteral("integrated:source/modules/feynman");
    plugin_load_errors_.remove(QStringLiteral("feynman"));
#endif

    feynman_plugin_ = plugin;
    _sync_feynman_host_bridge();
    feynman_plugin_->initialize(feynman_host_bridge_);
    const bool startup_task_panel_visible = feynman_task_panel_start_expanded;
    feynman_module_page = feynman_plugin_->create_widget(module_tabs);
    set_feynman_task_panel_visible(startup_task_panel_visible, false);
    set_feynman_ui_layout_mode_key(feynman_ui_layout_mode_key_, false);
    relax_module_page(feynman_module_page);
    set_feynman_task_panel_visible(startup_task_panel_visible, false);
    QWidget *created_feynman_page = feynman_module_page;
    QTimer::singleShot(0, this, [this, created_feynman_page, startup_task_panel_visible]() {
        if (feynman_module_page == created_feynman_page) {
            set_feynman_task_panel_visible(startup_task_panel_visible, false);
        }
    });
    QTimer::singleShot(80, this, [this, created_feynman_page, startup_task_panel_visible]() {
        if (feynman_module_page == created_feynman_page) {
            set_feynman_task_panel_visible(startup_task_panel_visible, false);
        }
    });
    if (feynman_module_page != nullptr && module_tabs != nullptr && module_tabs->indexOf(feynman_module_page) < 0) {
        module_tabs->addTab(feynman_module_page, feynman_module_tab_text());
    }
    _connect_feynman_task_counter_sources();
    _refresh_module_tabs();
    logger()->info(QStringLiteral("Módulo Feynman inicializado com sucesso (%1).").arg(feynman_plugin_path_));
}

void EisenhowerMatrixApp::_load_pomodoro_plugin()
{
    if (pomodoro_plugin_ != nullptr) {
        return;
    }

#if defined(EISENHOWER_MODULAR_PLUGINS)
    auto *plugin = load_dynamic_plugin<source::plugin::IPomodoroPlugin>(
        this,
        QStringLiteral("pomodoro"),
        pomodoro_plugin_candidates(),
        pomodoro_plugin_loader_,
        pomodoro_plugin_path_,
        plugin_load_errors_,
        logger());
    if (plugin == nullptr) {
        return;
    }
#else
    auto *plugin = new source::plugin::PomodoroPlugin();
    plugin->setParent(this);
    pomodoro_plugin_path_ = QStringLiteral("integrated:source/modules/pomodoro");
    plugin_load_errors_.remove(QStringLiteral("pomodoro"));
#endif

    pomodoro_plugin_ = plugin;
    _sync_pomodoro_host_bridge();
    pomodoro_plugin_->initialize(pomodoro_host_bridge_);
    pomodoro_module_page = pomodoro_plugin_->create_widget(module_tabs);
    set_pomodoro_ui_layout_mode_key(pomodoro_ui_layout_mode_key_, false);
    relax_module_page(pomodoro_module_page);
    set_pomodoro_in_progress_panel_visible(pomodoro_in_progress_panel_start_expanded, false);
    if (pomodoro_module_page != nullptr && module_tabs != nullptr && module_tabs->indexOf(pomodoro_module_page) < 0) {
        module_tabs->addTab(pomodoro_module_page, pomodoro_module_tab_text());
    }
    _connect_pomodoro_task_counter_sources();
    _refresh_module_tabs();
    logger()->info(QStringLiteral("Módulo Pomodoro inicializado com sucesso (%1).").arg(pomodoro_plugin_path_));
}

void EisenhowerMatrixApp::_load_mapamental_plugin()
{
    if (mapamental_plugin_ != nullptr) {
        return;
    }

#if defined(EISENHOWER_MODULAR_PLUGINS)
    auto *plugin = load_dynamic_plugin<source::plugin::IMapaMentalPlugin>(
        this,
        QStringLiteral("mapamental"),
        mapamental_plugin_candidates(),
        mapamental_plugin_loader_,
        mapamental_plugin_path_,
        plugin_load_errors_,
        logger());
    if (plugin == nullptr) {
        return;
    }
#else
    auto *plugin = new source::plugin::MapaMentalPlugin();
    plugin->setParent(this);
    mapamental_plugin_path_ = QStringLiteral("integrated:source/modules/mapamental");
    plugin_load_errors_.remove(QStringLiteral("mapamental"));
#endif

    mapamental_plugin_ = plugin;
    _sync_mapamental_host_bridge();
    mapamental_plugin_->initialize(mapamental_host_bridge_);
    mapamental_module_page = mapamental_plugin_->create_widget(module_tabs);
    if (modelo_plugin_ != nullptr) {
        modelo_plugin_->set_active_model(mapamental_summary_method_key());
    }
    relax_module_page(mapamental_module_page);
    if (mapamental_module_page != nullptr && module_tabs != nullptr && module_tabs->indexOf(mapamental_module_page) < 0) {
        module_tabs->addTab(mapamental_module_page, mapamental_module_tab_text(mapamental_display_name()));
    }
    _connect_mapamental_task_counter_sources();
    _refresh_module_tabs();
    logger()->info(QStringLiteral("Módulo Mapa Mental inicializado com sucesso (%1).").arg(mapamental_plugin_path_));
}

void EisenhowerMatrixApp::_load_modelo_plugin()
{
    if (modelo_plugin_ != nullptr) {
        return;
    }

#if defined(EISENHOWER_MODULAR_PLUGINS)
    auto *plugin = load_dynamic_plugin<source::plugin::IModeloPlugin>(
        this,
        QStringLiteral("modelo"),
        modelo_plugin_candidates(),
        modelo_plugin_loader_,
        modelo_plugin_path_,
        plugin_load_errors_,
        logger());
    if (plugin == nullptr) {
        return;
    }
#else
    auto *plugin = new source::plugin::ModeloPlugin();
    plugin->setParent(this);
    modelo_plugin_path_ = QStringLiteral("integrated:source/modules/modelo");
    plugin_load_errors_.remove(QStringLiteral("modelo"));
#endif

    modelo_plugin_ = plugin;
    logger()->info(QStringLiteral("Módulo Modelo inicializado com sucesso (%1, disponível=%2)")
                       .arg(modelo_plugin_path_, plugin->is_available() ? QStringLiteral("sim") : QStringLiteral("não")));
}

void EisenhowerMatrixApp::_load_audiolivro_plugin()
{
    if (audiolivro_plugin_ != nullptr) {
        return;
    }

#if defined(EISENHOWER_MODULAR_PLUGINS)
    auto *plugin = load_dynamic_plugin<source::plugin::IAudiolivroPlugin>(
        this,
        QStringLiteral("audiolivro"),
        audiolivro_plugin_candidates(),
        audiolivro_plugin_loader_,
        audiolivro_plugin_path_,
        plugin_load_errors_,
        logger());
    if (plugin == nullptr) {
        return;
    }
#else
    auto *plugin = new source::plugin::AudiolivroPlugin();
    plugin->setParent(this);
    audiolivro_plugin_path_ = QStringLiteral("integrated:source/modules/audiolivro");
    plugin_load_errors_.remove(QStringLiteral("audiolivro"));
#endif

    audiolivro_plugin_ = plugin;
    _sync_audiolivro_host_bridge();
    audiolivro_plugin_->initialize(audiolivro_host_bridge_);
    audiolivro_module_page = audiolivro_plugin_->create_widget(module_tabs);
    relax_module_page(audiolivro_module_page);
    if (audiolivro_module_page != nullptr && module_tabs != nullptr && module_tabs->indexOf(audiolivro_module_page) < 0) {
        module_tabs->addTab(audiolivro_module_page, audiolivro_module_tab_text(audiolivro_display_name()));
    }
    _connect_audiolivro_task_counter_sources();
    _refresh_module_tabs();
    logger()->info(QStringLiteral("Módulo Audiolivro inicializado com sucesso (%1).").arg(audiolivro_plugin_path_));
}

void EisenhowerMatrixApp::_load_agenda_plugin()
{
    if (agenda_plugin_ != nullptr) {
        return;
    }

#if defined(EISENHOWER_MODULAR_PLUGINS)
    auto *plugin = load_dynamic_plugin<source::plugin::IAgendaPlugin>(
        this,
        QStringLiteral("agenda"),
        agenda_plugin_candidates(),
        agenda_plugin_loader_,
        agenda_plugin_path_,
        plugin_load_errors_,
        logger());
    if (plugin == nullptr) {
        return;
    }
#else
    auto *plugin = new source::plugin::AgendaPlugin();
    plugin->setParent(this);
    agenda_plugin_path_ = QStringLiteral("integrated:source/modules/agenda");
    plugin_load_errors_.remove(QStringLiteral("agenda"));
#endif

    agenda_plugin_ = plugin;
    _sync_agenda_host_bridge();
    agenda_plugin_->initialize(agenda_host_bridge_);
    logger()->info(QStringLiteral("Módulo Registro de Atividades inicializado com sucesso (%1).").arg(agenda_plugin_path_));
}

void EisenhowerMatrixApp::_load_gantt_plugin()
{
    if (gantt_plugin_ != nullptr) {
        return;
    }

#if defined(EISENHOWER_MODULAR_PLUGINS)
    auto *plugin = load_dynamic_plugin<source::plugin::IGanttPlugin>(
        this,
        QStringLiteral("gantt"),
        gantt_plugin_candidates(),
        gantt_plugin_loader_,
        gantt_plugin_path_,
        plugin_load_errors_,
        logger());
    if (plugin == nullptr) {
        return;
    }
#else
    auto *plugin = new source::plugin::GanttPlugin(this);
    gantt_plugin_path_ = QStringLiteral("integrated:source/modules/gantt");
    plugin_load_errors_.remove(QStringLiteral("gantt"));
#endif

    gantt_plugin_ = plugin;
    _sync_gantt_host_bridge();
    gantt_plugin_->initialize(gantt_host_bridge_);
    logger()->info(QStringLiteral("Módulo Cronograma Gantt inicializado com sucesso (%1).").arg(gantt_plugin_path_));
}

bool EisenhowerMatrixApp::has_feynman_plugin() const
{
    return feynman_plugin_ != nullptr && feynman_module_page != nullptr;
}

QString EisenhowerMatrixApp::feynman_display_name() const
{
    return feynman_plugin_ != nullptr ? feynman_plugin_->display_name() : get_text(QStringLiteral("Feynman"));
}

QString EisenhowerMatrixApp::feynman_open_menu_text() const
{
    return feynman_plugin_ != nullptr ? feynman_plugin_->open_menu_text() : get_text(QStringLiteral("Abrir Feynman"));
}

QString EisenhowerMatrixApp::feynman_ui_layout_mode_key() const
{
    return feynman_ui_layout_mode_key_;
}

bool EisenhowerMatrixApp::set_feynman_ui_layout_mode_key(const QString &mode_key, bool persist)
{
    const QString normalized = normalized_feynman_ui_layout_key(mode_key);
    const QString previous = feynman_ui_layout_mode_key_;

    if (feynman_module_page != nullptr) {
        const bool task_panel_visible = feynman_task_panel_visible();
        bool applied = false;
        const bool invoked = QMetaObject::invokeMethod(feynman_module_page,
                                                       "set_ui_layout_mode_key",
                                                       Qt::DirectConnection,
                                                       Q_RETURN_ARG(bool, applied),
                                                       Q_ARG(QString, normalized));
        if (!invoked || !applied) {
            feynman_ui_layout_mode_key_ = previous;
            return false;
        }

        feynman_module_page->updateGeometry();
        relax_module_page(feynman_module_page);
        set_feynman_task_panel_visible(task_panel_visible, false);
        refresh_module_task_counter();
        _queue_window_geometry_stabilization();
    }

    feynman_ui_layout_mode_key_ = normalized;
    if (persist) {
        _save_task_layout_preference();
    }
    return true;
}

void EisenhowerMatrixApp::open_eisenhower()
{
    if (module_tabs != nullptr && eisenhower_module_page != nullptr) {
        set_module_tab_visible(QStringLiteral("eisenhower"), true);
        module_tabs->setCurrentWidget(eisenhower_module_page);
    }
}

void EisenhowerMatrixApp::open_feynman()
{
    if (module_tabs != nullptr && feynman_module_page != nullptr) {
        set_module_tab_visible(QStringLiteral("feynman"), true);
        module_tabs->setCurrentWidget(feynman_module_page);
    }
}

bool EisenhowerMatrixApp::has_pomodoro_plugin() const
{
    return pomodoro_plugin_ != nullptr && pomodoro_module_page != nullptr;
}

QString EisenhowerMatrixApp::pomodoro_display_name() const
{
    return pomodoro_plugin_ != nullptr ? pomodoro_plugin_->display_name() : get_text(QStringLiteral("Pomodoro"));
}

QString EisenhowerMatrixApp::pomodoro_open_menu_text() const
{
    return pomodoro_plugin_ != nullptr ? pomodoro_plugin_->open_menu_text() : get_text(QStringLiteral("Abrir Pomodoro"));
}

QString EisenhowerMatrixApp::pomodoro_alarm_menu_text() const
{
    return pomodoro_plugin_ != nullptr ? pomodoro_plugin_->alarm_menu_text() : get_text(QStringLiteral("Alarme"));
}

QString EisenhowerMatrixApp::pomodoro_automatic_mode_menu_text() const
{
    return pomodoro_plugin_ != nullptr ? pomodoro_plugin_->automatic_mode_menu_text()
                                      : get_text(QStringLiteral("Modo automático"));
}

QString EisenhowerMatrixApp::pomodoro_ui_layout_mode_key() const
{
    return pomodoro_ui_layout_mode_key_;
}

bool EisenhowerMatrixApp::set_pomodoro_ui_layout_mode_key(const QString &mode_key, bool persist)
{
    const QString normalized = normalized_pomodoro_ui_layout_key(mode_key);
    const QString previous = pomodoro_ui_layout_mode_key_;

    if (pomodoro_module_page != nullptr) {
        bool applied = false;
        const bool invoked = QMetaObject::invokeMethod(pomodoro_module_page,
                                                       "set_ui_layout_mode_key",
                                                       Qt::DirectConnection,
                                                       Q_RETURN_ARG(bool, applied),
                                                       Q_ARG(QString, normalized));
        if (!invoked || !applied) {
            pomodoro_ui_layout_mode_key_ = previous;
            return false;
        }

        pomodoro_module_page->updateGeometry();
        relax_module_page(pomodoro_module_page);
        set_pomodoro_in_progress_panel_visible(pomodoro_in_progress_panel_start_expanded, false);
        refresh_module_task_counter();
        _queue_window_geometry_stabilization();
    }

    pomodoro_ui_layout_mode_key_ = normalized;
    if (persist) {
        _save_task_layout_preference();
    }
    return true;
}

void EisenhowerMatrixApp::open_pomodoro()
{
    if (module_tabs != nullptr && pomodoro_module_page != nullptr) {
        set_module_tab_visible(QStringLiteral("pomodoro"), true);
        module_tabs->setCurrentWidget(pomodoro_module_page);
    }
}

bool EisenhowerMatrixApp::calendar_panel_visible() const
{
    return calendar_pane != nullptr ? calendar_pane->is_panel_expanded() : calendar_start_expanded;
}

void EisenhowerMatrixApp::set_calendar_panel_visible(bool visible, bool persist)
{
    calendar_start_expanded = visible;
    if (calendar_pane != nullptr) {
        calendar_pane->set_panel_expanded(visible);
        _sync_splitter_for_calendar();
    }

    if (persist) {
        _save_task_layout_preference();
    }
}

bool EisenhowerMatrixApp::feynman_task_panel_visible() const
{
    bool visible = feynman_task_panel_start_expanded;
    if (feynman_module_page != nullptr) {
        QMetaObject::invokeMethod(
            feynman_module_page, "is_task_panel_expanded", Qt::DirectConnection, Q_RETURN_ARG(bool, visible));
    }
    return visible;
}

void EisenhowerMatrixApp::set_feynman_task_panel_visible(bool visible, bool persist)
{
    feynman_task_panel_start_expanded = visible;
    if (feynman_module_page != nullptr) {
        QMetaObject::invokeMethod(
            feynman_module_page, "set_task_panel_expanded", Qt::DirectConnection, Q_ARG(bool, visible));
    }

    if (persist) {
        _save_task_layout_preference();
    }
}

bool EisenhowerMatrixApp::pomodoro_in_progress_panel_visible() const
{
    bool visible = pomodoro_in_progress_panel_start_expanded;
    if (pomodoro_module_page != nullptr) {
        QMetaObject::invokeMethod(pomodoro_module_page,
                                  "is_in_progress_panel_expanded",
                                  Qt::DirectConnection,
                                  Q_RETURN_ARG(bool, visible));
    }
    return visible;
}

void EisenhowerMatrixApp::set_pomodoro_in_progress_panel_visible(bool visible, bool persist)
{
    pomodoro_in_progress_panel_start_expanded = visible;
    if (pomodoro_module_page != nullptr) {
        QMetaObject::invokeMethod(pomodoro_module_page,
                                  "set_in_progress_panel_expanded",
                                  Qt::DirectConnection,
                                  Q_ARG(bool, visible));
    }

    if (persist) {
        _save_task_layout_preference();
    }
}

bool EisenhowerMatrixApp::open_pomodoro_alarm_settings(QWidget *parent)
{
    if (pomodoro_plugin_ == nullptr) {
        return false;
    }

    pomodoro_plugin_->open_alarm_settings(parent != nullptr ? parent : this);
    return true;
}

bool EisenhowerMatrixApp::open_pomodoro_automatic_mode_settings(QWidget *parent)
{
    if (pomodoro_plugin_ == nullptr) {
        return false;
    }

    pomodoro_plugin_->open_automatic_mode_settings(parent != nullptr ? parent : this);
    return true;
}

int EisenhowerMatrixApp::default_pomodoro_cycle_count() const
{
    if (pomodoro_plugin_ != nullptr) {
        return pomodoro_plugin_->default_pomodoro_cycle_count();
    }

    const QString config_path = QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("pomodoro_timer_config.json"));
    QFile file(config_path);
    if (file.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            const int c = doc.object().value(QStringLiteral("ciclos_permitidos")).toInt();
            if (c > 0) {
                return c;
            }
        }
    }
    return 4;
}

void EisenhowerMatrixApp::on_default_pomodoro_cycle_count_changed(int new_count)
{
    if (new_count <= 0) {
        return;
    }

    const std::array<QListWidget *, 4> in_progress_lists = {
        quadrant1_in_progress_list,
        quadrant2_in_progress_list,
        quadrant3_in_progress_list,
        quadrant4_in_progress_list
    };

    bool changed = false;
    for (auto *list : in_progress_lists) {
        if (list == nullptr) {
            continue;
        }
        for (int i = 0; i < list->count(); ++i) {
            auto *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
                continue;
            }
            QVariantMap data = item->data(Qt::UserRole).toMap();
            const bool custom = data.value(QStringLiteral("ciclos_customizados"), false).toBool();
            if (!custom) {
                data.insert(QStringLiteral("ciclos_totais"), new_count);
                data.insert(QStringLiteral("ciclos_permitidos"), new_count);
                data.insert(QStringLiteral("total_ciclos"), new_count);
                item->setData(Qt::UserRole, data);
                _apply_task_item_appearance(item);
                changed = true;
            }
        }
    }

    if (changed) {
        save_tasks();
    }

    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }

    if (feynman_plugin_ != nullptr) {
        feynman_plugin_->refresh_task_views();
    }
}

void EisenhowerMatrixApp::reset_in_progress_task_cycles_across_modules(int new_cycles, const QVariantList &affected_tasks)
{
    if (new_cycles <= 0) {
        return;
    }

    const std::array<QListWidget *, 4> in_progress_lists = {
        quadrant1_in_progress_list,
        quadrant2_in_progress_list,
        quadrant3_in_progress_list,
        quadrant4_in_progress_list
    };

    bool changed = false;
    for (auto *list : in_progress_lists) {
        if (list == nullptr) {
            continue;
        }
        for (int i = 0; i < list->count(); ++i) {
            auto *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || _is_group_header(item)) {
                continue;
            }
            QVariantMap data = item->data(Qt::UserRole).toMap();
            const QString iid = data.value(QStringLiteral("id")).toString().trimmed();
            const QString i_integ = data.value(QStringLiteral("integracao_task_id")).toString().trimmed();
            const QString ititle = data.value(QStringLiteral("text"),
                                   data.value(QStringLiteral("titulo"),
                                   data.value(QStringLiteral("title")))).toString().trimmed();

            bool match = false;
            for (const QVariant &v : affected_tasks) {
                const QVariantMap task = v.toMap();
                const QString tid = task.value(QStringLiteral("id")).toString().trimmed();
                const QString tinteg = task.value(QStringLiteral("integracao_task_id")).toString().trimmed();
                const QString ttitle = task.value(QStringLiteral("titulo")).toString().trimmed();

                if (!tid.isEmpty() && (iid == tid || (!i_integ.isEmpty() && i_integ == tid))) {
                    match = true;
                    break;
                }
                if (!tinteg.isEmpty() && (iid == tinteg || (!i_integ.isEmpty() && i_integ == tinteg))) {
                    match = true;
                    break;
                }
                if (!ititle.isEmpty() && !ttitle.isEmpty() && ititle.compare(ttitle, Qt::CaseInsensitive) == 0) {
                    match = true;
                    break;
                }
            }

            if (match) {
                data.insert(QStringLiteral("ciclos_totais"), new_cycles);
                data.insert(QStringLiteral("ciclos_permitidos"), new_cycles);
                data.insert(QStringLiteral("total_ciclos"), new_cycles);
                data.insert(QStringLiteral("ciclos_customizados"), false);
                item->setData(Qt::UserRole, data);
                _apply_task_item_appearance(item);
                changed = true;
            }
        }
    }

    if (changed) {
        save_tasks_now();
    }

    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }

    if (feynman_plugin_ != nullptr) {
        feynman_plugin_->reset_in_progress_task_cycles(new_cycles, affected_tasks);
    }
}

bool EisenhowerMatrixApp::has_mapamental_plugin() const
{
    return mapamental_plugin_ != nullptr && mapamental_module_page != nullptr;
}

bool EisenhowerMatrixApp::has_modelo_plugin() const
{
    return modelo_plugin_ != nullptr;
}

QString EisenhowerMatrixApp::mapamental_display_name() const
{
    return mapamental_plugin_ != nullptr ? mapamental_plugin_->display_name() : get_text(QStringLiteral("Mapa Mental"));
}

QString EisenhowerMatrixApp::mapamental_open_menu_text() const
{
    return mapamental_plugin_ != nullptr ? mapamental_plugin_->open_menu_text()
                                         : get_text(QStringLiteral("Abrir Mapa Mental"));
}

QString EisenhowerMatrixApp::mapamental_layout_mode_key() const
{
    return mapamental_plugin_ != nullptr ? mapamental_plugin_->layout_mode_key() : QStringLiteral("vertical");
}

void EisenhowerMatrixApp::set_mapamental_layout_mode_key(const QString &mode_key)
{
    if (mapamental_plugin_ != nullptr) {
        mapamental_plugin_->set_layout_mode_key(mode_key);
    }
}

QString EisenhowerMatrixApp::mapamental_summary_method_key() const
{
    return mapamental_plugin_ != nullptr ? mapamental_plugin_->summary_method_key() : QStringLiteral("qwen");
}

void EisenhowerMatrixApp::set_mapamental_summary_method_key(const QString &method_key)
{
    if (mapamental_plugin_ != nullptr) {
        mapamental_plugin_->set_summary_method_key(method_key);
    }
    if (modelo_plugin_ != nullptr) {
        modelo_plugin_->set_active_model(method_key);
    }
}

QVariantList EisenhowerMatrixApp::ollama_installed_models() const
{
    if (modelo_plugin_ != nullptr) {
        return modelo_plugin_->available_models();
    }
#if !defined(EISENHOWER_MODULAR_PLUGINS)
    QVariantList list;
    const auto models = source::plugin::Modelo::ProcessadorIAFns::descobrir_modelos_ollama(false);
    for (const auto &info : models) {
        QVariantMap item;
        item.insert(QStringLiteral("key"), QStringLiteral("ollama:%1").arg(info.name));
        item.insert(QStringLiteral("name"), info.name);
        item.insert(QStringLiteral("display"), info.displayName);
        item.insert(QStringLiteral("provider"), QStringLiteral("ollama"));
        item.insert(QStringLiteral("size_bytes"), info.sizeBytes);
        list.append(item);
    }
    return list;
#else
    return {};
#endif
}

void EisenhowerMatrixApp::refresh_ollama_models()
{
    if (modelo_plugin_ != nullptr) {
        modelo_plugin_->refresh_models();
    }
#if !defined(EISENHOWER_MODULAR_PLUGINS)
    else {
        source::plugin::Modelo::ProcessadorIAFns::descobrir_modelos_ollama(true);
    }
#endif
}

QVariantMap EisenhowerMatrixApp::ollama_diagnostic() const
{
    if (modelo_plugin_ != nullptr) {
        return modelo_plugin_->ollama_diagnostic();
    }
#if !defined(EISENHOWER_MODULAR_PLUGINS)
    return source::plugin::Modelo::ProcessadorIAFns::diagnosticar_ollama();
#else
    return {};
#endif
}

void EisenhowerMatrixApp::show_ollama_instructions_dialog()
{
    if (_ollama_instructions_dialog == nullptr) {
        _ollama_instructions_dialog = new source::GerenciamentoUI::OllamaInstrucoesDialog(this, this);
        _ollama_instructions_dialog->setAttribute(Qt::WA_DeleteOnClose);
    }
    _ollama_instructions_dialog->show();
    _ollama_instructions_dialog->raise();
    _ollama_instructions_dialog->activateWindow();
}

void EisenhowerMatrixApp::show_model_download_dialog()
{
    if (_model_download_dialog == nullptr) {
        auto *dialog = new source::GerenciamentoUI::ModelDownloadDialog(this, this);
        connect(dialog, &source::GerenciamentoUI::ModelDownloadDialog::model_download_status_changed,
                this, [this]() {
                    this->criar_menu_configuracoes();
                });
        _model_download_dialog = dialog;
    }
    _model_download_dialog->show();
    _model_download_dialog->raise();
    _model_download_dialog->activateWindow();
}

bool EisenhowerMatrixApp::is_qwen_model_installed() const
{
    if (modelo_plugin_ != nullptr) {
        return modelo_plugin_->is_gguf_model_installed();
    }
#if !defined(EISENHOWER_MODULAR_PLUGINS)
    return source::plugin::Modelo::ProcessadorIAFns::modelo_gguf_instalado();
#else
    QFile file(qwen_model_path());
    return file.open(QIODevice::ReadOnly) && file.read(4) == QByteArrayLiteral("GGUF");
#endif
}

QString EisenhowerMatrixApp::qwen_models_dir() const
{
    if (modelo_plugin_ != nullptr) {
        const QString dir = modelo_plugin_->gguf_models_dir();
        if (!dir.isEmpty()) {
            return dir;
        }
    }
#if !defined(EISENHOWER_MODULAR_PLUGINS)
    return source::plugin::Modelo::ProcessadorIAFns::diretorio_modelos_persistente();
#else
    return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("models"));
#endif
}

QString EisenhowerMatrixApp::qwen_model_path() const
{
    if (modelo_plugin_ != nullptr) {
        const QString path = modelo_plugin_->gguf_model_path();
        if (!path.isEmpty()) {
            return path;
        }
    }
#if !defined(EISENHOWER_MODULAR_PLUGINS)
    return source::plugin::Modelo::ProcessadorIAFns::caminho_destino_modelo_gguf();
#else
    return QDir(qwen_models_dir()).filePath(QStringLiteral("qwen2.5-3b-instruct-q4_k_m.gguf"));
#endif
}

QString EisenhowerMatrixApp::qwen_download_url() const
{
    if (modelo_plugin_ != nullptr) {
        const QString url = modelo_plugin_->gguf_download_url();
        if (!url.isEmpty()) {
            return url;
        }
    }
#if !defined(EISENHOWER_MODULAR_PLUGINS)
    return source::plugin::Modelo::ProcessadorIAFns::url_download_modelo_gguf();
#else
    return QStringLiteral("https://huggingface.co/Qwen/Qwen2.5-3B-Instruct-GGUF/resolve/main/qwen2.5-3b-instruct-q4_k_m.gguf");
#endif
}

qint64 EisenhowerMatrixApp::qwen_model_size_bytes() const
{
    if (modelo_plugin_ != nullptr) {
        return modelo_plugin_->gguf_model_size_bytes();
    }
#if !defined(EISENHOWER_MODULAR_PLUGINS)
    return source::plugin::Modelo::ProcessadorIAFns::tamanho_modelo_gguf_instalado();
#else
    return QFileInfo(qwen_model_path()).size();
#endif
}

bool EisenhowerMatrixApp::remove_qwen_model()
{
    if (modelo_plugin_ != nullptr) {
        return modelo_plugin_->remove_gguf_model();
    }
#if !defined(EISENHOWER_MODULAR_PLUGINS)
    return source::plugin::Modelo::ProcessadorIAFns::remover_modelo_gguf_instalado();
#else
    const QString dest = qwen_model_path();
    bool removed = false;
    if (QFile::exists(dest)) {
        removed = QFile::remove(dest) || removed;
    }
    const QString part = dest + QStringLiteral(".part");
    if (QFile::exists(part)) {
        QFile::remove(part);
    }
    return removed;
#endif
}

void EisenhowerMatrixApp::open_mapamental()
{
    if (module_tabs != nullptr && mapamental_module_page != nullptr) {
        set_module_tab_visible(QStringLiteral("mapamental"), true);
        module_tabs->setCurrentWidget(mapamental_module_page);
    }
}

void EisenhowerMatrixApp::reload_mapamental_from_backup()
{
    if (mapamental_plugin_ != nullptr) {
        mapamental_plugin_->reload_shared_state();
        mapamental_plugin_->refresh_task_views();
    }
}

bool EisenhowerMatrixApp::has_audiolivro_plugin() const
{
    return audiolivro_plugin_ != nullptr && audiolivro_module_page != nullptr;
}

QString EisenhowerMatrixApp::audiolivro_display_name() const
{
    return audiolivro_plugin_ != nullptr ? audiolivro_plugin_->display_name() : get_text(QStringLiteral("Audiolivro"));
}

QString EisenhowerMatrixApp::audiolivro_open_menu_text() const
{
    return audiolivro_plugin_ != nullptr ? audiolivro_plugin_->open_menu_text()
                                         : get_text(QStringLiteral("Abrir Audiolivro"));
}

QVariantList EisenhowerMatrixApp::plugin_status_report() const
{
    const auto resolve_version = [](const QPluginLoader *loader, const QString &key, const QString &relative_path) {
        if (loader != nullptr) {
            const QString meta_ver = loader->metaData()
                                         .value(QStringLiteral("MetaData"))
                                         .toObject()
                                         .value(QStringLiteral("version"))
                                         .toString();
            if (!meta_ver.isEmpty()) {
                return meta_ver;
            }
        }

        const QStringList candidates = {
            QDir(QCoreApplication::applicationDirPath())
                .absoluteFilePath(QStringLiteral("plugins/%1/%1_plugin.json").arg(key)),
            QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(relative_path),
            QDir(QDir::currentPath()).absoluteFilePath(relative_path),
        };
        for (const QString &candidate : candidates) {
            QFile file(candidate);
            if (!file.open(QIODevice::ReadOnly)) {
                continue;
            }
            const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
            if (document.isObject()) {
                const QString ver = document.object().value(QStringLiteral("version")).toString();
                if (!ver.isEmpty()) {
                    return ver;
                }
            }
        }

        const QString app_ver = QCoreApplication::applicationVersion();
        return app_ver.isEmpty() ? QStringLiteral("2026.11.22.0") : app_ver;
    };

    const auto entry = [this, resolve_version](const QString &key,
                                                 const QString &name,
                                                 bool loaded,
                                                 const QString &path,
                                                 const QPluginLoader *loader,
                                                 const QString &manifest) {
        QVariantMap item;
        item.insert(QStringLiteral("key"), key);
        item.insert(QStringLiteral("name"), name);
        item.insert(QStringLiteral("loaded"), loaded);
        item.insert(QStringLiteral("status"),
                    loaded ? get_text(QStringLiteral("Carregado")) : get_text(QStringLiteral("Não carregado")));
        item.insert(QStringLiteral("version"), resolve_version(loader, key, manifest));
        item.insert(QStringLiteral("path"), path);
        item.insert(QStringLiteral("error"), plugin_load_errors_.value(key));
        return item;
    };

    QVariantList report = {
        entry(QStringLiteral("feynman"),
              feynman_display_name(),
              has_feynman_plugin(),
              feynman_plugin_path_,
              feynman_plugin_loader_,
              QStringLiteral("source/modules/feynman/feynman_plugin.json")),
        entry(QStringLiteral("pomodoro"),
              pomodoro_display_name(),
              has_pomodoro_plugin(),
              pomodoro_plugin_path_,
              pomodoro_plugin_loader_,
              QStringLiteral("source/modules/pomodoro/pomodoro_plugin.json")),
        entry(QStringLiteral("mapamental"),
              mapamental_display_name(),
              has_mapamental_plugin(),
              mapamental_plugin_path_,
              mapamental_plugin_loader_,
              QStringLiteral("source/modules/mapamental/mapamental_plugin.json")),
        entry(QStringLiteral("audiolivro"),
              audiolivro_display_name(),
              has_audiolivro_plugin(),
              audiolivro_plugin_path_,
              audiolivro_plugin_loader_,
              QStringLiteral("source/modules/audiolivro/audiolivro_plugin.json")),
    };
    if (modelo_plugin_ != nullptr || plugin_load_errors_.contains(QStringLiteral("modelo"))) {
        report.append(entry(QStringLiteral("modelo"),
                            get_text(QStringLiteral("modelo qwen2.5-3b")),
                            modelo_plugin_ != nullptr,
                            modelo_plugin_path_,
                            modelo_plugin_loader_,
                            QStringLiteral("source/modules/modelo/modelo_plugin.json")));
    }
    report.append(entry(QStringLiteral("agenda"),
              agenda_display_name(),
              has_agenda_plugin(),
              agenda_plugin_path_,
              agenda_plugin_loader_,
              QStringLiteral("source/modules/agenda/agenda_plugin.json")));
    report.append(entry(QStringLiteral("gantt"),
              gantt_display_name(),
              has_gantt_plugin(),
              gantt_plugin_path_,
              gantt_plugin_loader_,
              QStringLiteral("source/modules/gantt/gantt_plugin.json")));
    return report;
}

QString EisenhowerMatrixApp::audiolivro_font_settings_menu_text() const
{
    return audiolivro_plugin_ != nullptr ? audiolivro_plugin_->font_settings_menu_text()
                                         : get_text(QStringLiteral("Configuracao de Fontes"));
}

QVariantList EisenhowerMatrixApp::audiolivro_voice_options() const
{
    return audiolivro_plugin_ != nullptr ? audiolivro_plugin_->voice_options() : QVariantList{};
}

QString EisenhowerMatrixApp::audiolivro_current_voice_id() const
{
    return audiolivro_plugin_ != nullptr ? audiolivro_plugin_->current_voice_id() : QString();
}

void EisenhowerMatrixApp::set_audiolivro_voice_id(const QString &voice_id)
{
    if (audiolivro_plugin_ != nullptr) {
        audiolivro_plugin_->set_voice_id(voice_id);
    }
}

bool EisenhowerMatrixApp::open_audiolivro_font_settings(QWidget *parent)
{
    if (audiolivro_plugin_ == nullptr) {
        return false;
    }

    audiolivro_plugin_->open_font_settings(parent != nullptr ? parent : this);
    return true;
}

void EisenhowerMatrixApp::open_audiolivro()
{
    if (module_tabs != nullptr && audiolivro_module_page != nullptr) {
        set_module_tab_visible(QStringLiteral("audiolivro"), true);
        module_tabs->setCurrentWidget(audiolivro_module_page);
    }
}

bool EisenhowerMatrixApp::has_agenda_plugin() const
{
    return agenda_plugin_ != nullptr;
}

QString EisenhowerMatrixApp::agenda_display_name() const
{
    return agenda_plugin_ != nullptr ? agenda_plugin_->display_name() : get_text(QStringLiteral("Registro de Atividades"));
}

QString EisenhowerMatrixApp::agenda_open_menu_text() const
{
    return agenda_plugin_ != nullptr ? agenda_plugin_->open_menu_text()
                                     : get_text(QStringLiteral("Registro de Atividades"));
}

void EisenhowerMatrixApp::open_agenda()
{
    if (agenda_plugin_ != nullptr) {
        agenda_plugin_->open_window(this);
    }
}

void EisenhowerMatrixApp::populate_agenda_delete_item_menu(QMenu *menu)
{
    if (agenda_plugin_ != nullptr) {
        agenda_plugin_->populate_delete_item_menu(menu);
    }
}

void EisenhowerMatrixApp::populate_agenda_color_menu(QMenu *menu)
{
    if (agenda_plugin_ != nullptr) {
        agenda_plugin_->populate_color_menu(menu);
    }
}

bool EisenhowerMatrixApp::has_gantt_plugin() const
{
    return gantt_plugin_ != nullptr;
}

QString EisenhowerMatrixApp::gantt_display_name() const
{
    return gantt_plugin_ != nullptr ? gantt_plugin_->display_name() : get_text(QStringLiteral("Cronograma de Gantt BIM 4D"));
}

QString EisenhowerMatrixApp::gantt_open_menu_text() const
{
    return gantt_plugin_ != nullptr ? gantt_plugin_->open_menu_text()
                                     : get_text(QStringLiteral("Cronograma de Gantt BIM 4D"));
}

void EisenhowerMatrixApp::open_gantt()
{
    if (gantt_plugin_ != nullptr) {
        gantt_plugin_->open_window(this);
    }
}

void EisenhowerMatrixApp::open_gantt_for_task(const QString &schedule_item_id)
{
    if (gantt_plugin_ != nullptr) {
        gantt_plugin_->open_gantt_and_highlight_activity(schedule_item_id, this);
    }
}

bool EisenhowerMatrixApp::open_gantt_link_dialog(QListWidgetItem *item)
{
    if (gantt_plugin_ == nullptr || item == nullptr) {
        return false;
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString task_id = data.value(QStringLiteral("id")).toString().trimmed();
    QString task_title = data.value(QStringLiteral("text")).toString().trimmed();
    if (task_title.isEmpty()) {
        task_title = base_task_text(item);
    }
    if (task_id.isEmpty()) {
        return false;
    }

    return gantt_plugin_->open_task_link_dialog(task_id, task_title, this);
}

bool EisenhowerMatrixApp::send_task_to_gantt(QListWidgetItem *item, QString *out_item_id, QString *error_message)
{
    if (gantt_plugin_ == nullptr || item == nullptr) {
        if (error_message != nullptr) {
            *error_message = get_text(QStringLiteral("Plugin Gantt não está disponível."));
        }
        return false;
    }

    QVariantMap data = item->data(Qt::UserRole).toMap();
    if (data.value(QStringLiteral("text")).toString().trimmed().isEmpty()) {
        data.insert(QStringLiteral("text"), base_task_text(item));
    }
    return gantt_plugin_->convert_eisenhower_task_to_gantt_activity(data, out_item_id, error_message);
}

QVariantList EisenhowerMatrixApp::get_available_tasks() const
{
    QVariantList tasks;
    QSet<QString> seen_ids;
    const auto lists = all_task_lists(const_cast<EisenhowerMatrixApp *>(this));
    for (QListWidget *list : lists) {
        if (list == nullptr) continue;
        const bool completed = is_completed_list(list);
        const bool in_progress = is_in_progress_list(list);
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || _is_group_header(item)) {
                continue;
            }
            QVariantMap data = item->data(Qt::UserRole).toMap();
            if (data.isEmpty()) {
                continue;
            }
            if (data.value(QStringLiteral("text")).toString().trimmed().isEmpty()) {
                data.insert(QStringLiteral("text"), base_task_text(item));
            }
            const bool is_item_done = completed || item->checkState() == Qt::Checked;
            data.insert(QStringLiteral("completed"), is_item_done);
            data.insert(QStringLiteral("concluida"), is_item_done);
            if (is_item_done) {
                data.insert(QStringLiteral("status"), QStringLiteral("Done"));
                data.insert(QStringLiteral("in_progress"), false);
                data.insert(QStringLiteral("em_progresso"), false);
                data.insert(QStringLiteral("em_andamento"), false);
            } else if (in_progress || data.value(QStringLiteral("in_progress")).toBool() ||
                       data.value(QStringLiteral("em_progresso")).toBool() ||
                       data.value(QStringLiteral("em_andamento")).toBool() ||
                       data.value(QStringLiteral("status")).toString().compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0) {
                data.insert(QStringLiteral("status"), QStringLiteral("Doing"));
                data.insert(QStringLiteral("in_progress"), true);
                data.insert(QStringLiteral("em_progresso"), true);
                data.insert(QStringLiteral("em_andamento"), true);
            } else {
                data.insert(QStringLiteral("status"), QStringLiteral("Todo"));
                data.insert(QStringLiteral("in_progress"), false);
                data.insert(QStringLiteral("em_progresso"), false);
                data.insert(QStringLiteral("em_andamento"), false);
            }
            const QString tid = data.value(QStringLiteral("id")).toString().trimmed();
            if (!tid.isEmpty()) {
                seen_ids.insert(tid);
            }
            const QString tinteg = data.value(QStringLiteral("integracao_task_id")).toString().trimmed();
            if (!tinteg.isEmpty()) {
                seen_ids.insert(tinteg);
            }
            tasks.append(data);
        }
    }

    // Carregar tarefas unificadas (Pomodoro, Feynman, etc.) que não estão diretamente nas listas do Eisenhower
    const QVariantList records = source::utils::UnifiedTaskStore::load_records(tasks_path);
    for (const QVariant &vrec : records) {
        QVariantMap rec = vrec.toMap();
        const QString rid = rec.value(QStringLiteral("id")).toString().trimmed();
        const QString rinteg = rec.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        const QString effective_id = !rid.isEmpty() ? rid : rinteg;
        if (effective_id.isEmpty() || seen_ids.contains(effective_id) || (!rinteg.isEmpty() && seen_ids.contains(rinteg))) {
            continue;
        }

        QString title = rec.value(QStringLiteral("text")).toString().trimmed();
        if (title.isEmpty()) title = rec.value(QStringLiteral("titulo")).toString().trimmed();
        if (title.isEmpty()) title = rec.value(QStringLiteral("title")).toString().trimmed();
        if (title.isEmpty()) continue;

        rec.insert(QStringLiteral("id"), effective_id);
        rec.insert(QStringLiteral("text"), title);
        rec.insert(QStringLiteral("titulo"), title);

        bool is_done = rec.value(QStringLiteral("completed"), rec.value(QStringLiteral("concluida"), false)).toBool();
        const QString st = rec.value(QStringLiteral("status")).toString().trimmed().toLower();
        if (st == QStringLiteral("done") || st == QStringLiteral("completed") || st == QStringLiteral("concluida")) {
            is_done = true;
        }
        rec.insert(QStringLiteral("completed"), is_done);
        rec.insert(QStringLiteral("concluida"), is_done);

        const bool in_prog = !is_done && (st == QStringLiteral("doing") ||
                                          st.contains(QStringLiteral("progress")) ||
                                          st.contains(QStringLiteral("andamento")) ||
                                          rec.value(QStringLiteral("in_progress")).toBool() ||
                                          rec.value(QStringLiteral("em_progresso")).toBool() ||
                                          rec.value(QStringLiteral("em_andamento")).toBool());
        if (is_done) {
            rec.insert(QStringLiteral("status"), QStringLiteral("Done"));
            rec.insert(QStringLiteral("in_progress"), false);
            rec.insert(QStringLiteral("em_progresso"), false);
            rec.insert(QStringLiteral("em_andamento"), false);
        } else if (in_prog) {
            rec.insert(QStringLiteral("status"), QStringLiteral("Doing"));
            rec.insert(QStringLiteral("in_progress"), true);
            rec.insert(QStringLiteral("em_progresso"), true);
            rec.insert(QStringLiteral("em_andamento"), true);
        } else {
            rec.insert(QStringLiteral("status"), QStringLiteral("Todo"));
            rec.insert(QStringLiteral("in_progress"), false);
            rec.insert(QStringLiteral("em_progresso"), false);
            rec.insert(QStringLiteral("em_andamento"), false);
        }

        seen_ids.insert(effective_id);
        if (!rinteg.isEmpty()) {
            seen_ids.insert(rinteg);
        }
        tasks.append(rec);
    }

    return tasks;
}

QVariantMap EisenhowerMatrixApp::get_task_by_id(const QString &task_id) const
{
    const QString target_id = task_id.trimmed();
    if (target_id.isEmpty()) return {};

    const auto lists = all_task_lists(const_cast<EisenhowerMatrixApp *>(this));
    for (QListWidget *list : lists) {
        if (list == nullptr) continue;
        const bool completed = is_completed_list(list);
        const bool in_progress = is_in_progress_list(list);
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || _is_group_header(item)) {
                continue;
            }
            QVariantMap data = item->data(Qt::UserRole).toMap();
            if (data.value(QStringLiteral("id")).toString().trimmed() == target_id) {
                if (data.value(QStringLiteral("text")).toString().trimmed().isEmpty()) {
                    data.insert(QStringLiteral("text"), base_task_text(item));
                }
                const bool is_item_done = completed || item->checkState() == Qt::Checked;
                data.insert(QStringLiteral("completed"), is_item_done);
                data.insert(QStringLiteral("concluida"), is_item_done);
                if (is_item_done) {
                    data.insert(QStringLiteral("status"), QStringLiteral("Done"));
                    data.insert(QStringLiteral("in_progress"), false);
                    data.insert(QStringLiteral("em_progresso"), false);
                    data.insert(QStringLiteral("em_andamento"), false);
                } else if (in_progress || data.value(QStringLiteral("in_progress")).toBool() ||
                           data.value(QStringLiteral("em_progresso")).toBool() ||
                           data.value(QStringLiteral("em_andamento")).toBool() ||
                           data.value(QStringLiteral("status")).toString().compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0) {
                    data.insert(QStringLiteral("status"), QStringLiteral("Doing"));
                    data.insert(QStringLiteral("in_progress"), true);
                    data.insert(QStringLiteral("em_progresso"), true);
                    data.insert(QStringLiteral("em_andamento"), true);
                } else {
                    data.insert(QStringLiteral("status"), QStringLiteral("Todo"));
                    data.insert(QStringLiteral("in_progress"), false);
                    data.insert(QStringLiteral("em_progresso"), false);
                    data.insert(QStringLiteral("em_andamento"), false);
                }
                return data;
            }
        }
    }
    return {};
}

void EisenhowerMatrixApp::sync_task_progress(const QString &task_id, double percent, bool is_done)
{
    const QString target_id = task_id.trimmed();
    if (target_id.isEmpty()) return;

    bool eisenhower_modified = false;
    const auto lists = all_task_lists(this);
    for (QListWidget *list : lists) {
        if (list == nullptr) continue;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || _is_group_header(item)) {
                continue;
            }
            QVariantMap data = item->data(Qt::UserRole).toMap();
            const QString iid = data.value(QStringLiteral("id")).toString().trimmed();
            const QString i_integ = data.value(QStringLiteral("integracao_task_id")).toString().trimmed();
            if (iid == target_id || (!i_integ.isEmpty() && i_integ == target_id)) {
                data.insert(QStringLiteral("progresso"), percent);
                const int c_tot = data.value(QStringLiteral("ciclos_totais"),
                                  data.value(QStringLiteral("ciclos_permitidos"),
                                  data.value(QStringLiteral("total_ciclos"),
                                  data.value(QStringLiteral("ciclos_planejados"),
                                  data.value(QStringLiteral("ciclos_alvo"), 4))))).toInt();
                const int effective_c_tot = c_tot > 0 ? c_tot : 4;
                const int done_pomos = qBound(0, qRound((percent / 100.0) * effective_c_tot), effective_c_tot);
                data.insert(QStringLiteral("pomodoros_completados"), done_pomos);
                data.insert(QStringLiteral("ciclos_completados"), done_pomos);
                data.insert(QStringLiteral("ciclos_concluidos"), done_pomos);
                data.insert(QStringLiteral("pomodoros"), done_pomos);
                item->setData(Qt::UserRole, data);

                const bool currently_completed = is_completed_list(list);
                const int quad = quadrant_from_priority(data.value(QStringLiteral("priority")), this);
                const int effective_quad = (quad >= 1 && quad <= 4) ? quad : 1;

                if (is_done && !currently_completed) {
                    QListWidget *target = quadrant_list(effective_quad, true);
                    if (target != nullptr && target != list) {
                        move_item_between_lists(item, list, target, Qt::Checked);
                    } else {
                        item->setCheckState(Qt::Checked);
                    }
                } else if (!is_done) {
                    if (percent > 0.0) {
                        QListWidget *target = quadrant_in_progress_list(effective_quad);
                        if (target != nullptr && target != list) {
                            move_item_between_lists(item, list, target, Qt::Unchecked);
                        } else {
                            item->setCheckState(Qt::Unchecked);
                        }
                    } else if (currently_completed) {
                        QListWidget *target = quadrant_list(effective_quad, false);
                        if (target != nullptr && target != list) {
                            move_item_between_lists(item, list, target, Qt::Unchecked);
                        } else {
                            item->setCheckState(Qt::Unchecked);
                        }
                    }
                }
                eisenhower_modified = true;
            }
        }
    }

    if (eisenhower_modified) {
        save_tasks();
    }

    // 2. Atualizar registros no UnifiedTaskStore
    QVariantList records = source::utils::UnifiedTaskStore::load_records(tasks_path);
    bool store_modified = false;
    for (int i = 0; i < records.size(); ++i) {
        QVariantMap rec = records.at(i).toMap();
        const QString rid = rec.value(QStringLiteral("id")).toString().trimmed();
        const QString rinteg = rec.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        if (rid == target_id || (!rinteg.isEmpty() && rinteg == target_id)) {
            rec.insert(QStringLiteral("progresso"), percent);
            const int c_tot = rec.value(QStringLiteral("ciclos_totais"),
                              rec.value(QStringLiteral("ciclos_permitidos"),
                              rec.value(QStringLiteral("total_ciclos"),
                              rec.value(QStringLiteral("ciclos_planejados"),
                              rec.value(QStringLiteral("ciclos_alvo"), 4))))).toInt();
            const int effective_c_tot = c_tot > 0 ? c_tot : 4;
            const int done_pomos = qBound(0, qRound((percent / 100.0) * effective_c_tot), effective_c_tot);
            rec.insert(QStringLiteral("pomodoros_completados"), done_pomos);
            rec.insert(QStringLiteral("ciclos_completados"), done_pomos);
            rec.insert(QStringLiteral("ciclos_concluidos"), done_pomos);
            rec.insert(QStringLiteral("pomodoros"), done_pomos);
            rec.insert(QStringLiteral("completed"), is_done);
            rec.insert(QStringLiteral("status"), is_done ? QStringLiteral("Done")
                                                         : (percent > 0.0 ? QStringLiteral("Doing")
                                                                          : QStringLiteral("Todo")));
            records[i] = rec;
            store_modified = true;
        }
    }
    if (store_modified) {
        source::utils::UnifiedTaskStore::save_records(tasks_path, records);
    }

    // 3. Atualizar painel de tarefas do calendário
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }

    // 4. Notificar e recarregar plugin Pomodoro
    if (pomodoro_plugin_ != nullptr) {
        pomodoro_plugin_->reload_shared_state();
        pomodoro_plugin_->refresh_task_views();
    }

    // 5. Notificar e recarregar plugin Feynman
    if (feynman_plugin_ != nullptr) {
        feynman_plugin_->reload_shared_state();
        feynman_plugin_->refresh_task_views();
    }
}

void EisenhowerMatrixApp::sync_task_duration(const QString &task_id, double duration_days)
{
    const QString target_id = task_id.trimmed();
    if (target_id.isEmpty()) return;
    if (duration_days <= 0.0) {
        duration_days = 1.0;
    }

    QSet<QString> matched_ids;
    QSet<QString> matched_titles;
    QSet<QString> matched_canonical_ids;
    matched_ids.insert(target_id);

    // 1. Inspect in-memory Eisenhower items for matching target_id
    const auto lists = all_task_lists(this);
    for (QListWidget *list : lists) {
        if (list == nullptr) continue;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || _is_group_header(item)) {
                continue;
            }
            const QVariantMap data = item->data(Qt::UserRole).toMap();
            const QString iid = data.value(QStringLiteral("id")).toString().trimmed();
            const QString i_integ = data.value(QStringLiteral("integracao_task_id")).toString().trimmed();
            if ((!iid.isEmpty() && iid == target_id) || (!i_integ.isEmpty() && i_integ == target_id)) {
                if (!iid.isEmpty()) matched_ids.insert(iid);
                if (!i_integ.isEmpty()) matched_ids.insert(i_integ);
                const QString title = item->text().trimmed();
                if (!title.isEmpty()) matched_titles.insert(title);
                const QString base_text = data.value(QStringLiteral("text")).toString().trimmed();
                if (!base_text.isEmpty()) matched_titles.insert(base_text);
            }
        }
    }

    // 2. Load records from UnifiedTaskStore to find linked tasks/records
    QVariantList records = source::utils::UnifiedTaskStore::load_records(tasks_path);
    for (const QVariant &vrec : records) {
        const QVariantMap rec = vrec.toMap();
        const QString rid = rec.value(QStringLiteral("id")).toString().trimmed();
        const QString rinteg = rec.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        const QString rcanon = rec.value(QStringLiteral("canonical_id")).toString().trimmed();
        const QString rtitle = rec.value(QStringLiteral("titulo"), rec.value(QStringLiteral("title"))).toString().trimmed();

        if (matched_ids.contains(rid) || matched_ids.contains(rinteg) ||
            (!rcanon.isEmpty() && matched_canonical_ids.contains(rcanon)) ||
            (!rtitle.isEmpty() && matched_titles.contains(rtitle))) {
            if (!rid.isEmpty()) matched_ids.insert(rid);
            if (!rinteg.isEmpty()) matched_ids.insert(rinteg);
            if (!rcanon.isEmpty()) matched_canonical_ids.insert(rcanon);
            if (!rtitle.isEmpty()) matched_titles.insert(rtitle);
        }
    }

    // Pass 2 over records to expand linked group by canonical_id or title
    for (const QVariant &vrec : records) {
        const QVariantMap rec = vrec.toMap();
        const QString rid = rec.value(QStringLiteral("id")).toString().trimmed();
        const QString rinteg = rec.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        const QString rcanon = rec.value(QStringLiteral("canonical_id")).toString().trimmed();
        const QString rtitle = rec.value(QStringLiteral("titulo"), rec.value(QStringLiteral("title"))).toString().trimmed();

        if ((!rcanon.isEmpty() && matched_canonical_ids.contains(rcanon)) ||
            (!rtitle.isEmpty() && matched_titles.contains(rtitle))) {
            if (!rid.isEmpty()) matched_ids.insert(rid);
            if (!rinteg.isEmpty()) matched_ids.insert(rinteg);
        }
    }

    // 3. Update matching in-memory Eisenhower items
    for (QListWidget *list : lists) {
        if (list == nullptr) continue;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || _is_group_header(item)) {
                continue;
            }
            QVariantMap data = item->data(Qt::UserRole).toMap();
            const QString iid = data.value(QStringLiteral("id")).toString().trimmed();
            const QString i_integ = data.value(QStringLiteral("integracao_task_id")).toString().trimmed();
            const QString ititle = item->text().trimmed();
            const QString base_text = data.value(QStringLiteral("text")).toString().trimmed();

            if (matched_ids.contains(iid) || matched_ids.contains(i_integ) ||
                (!ititle.isEmpty() && matched_titles.contains(ititle)) ||
                (!base_text.isEmpty() && matched_titles.contains(base_text))) {
                data.insert(QStringLiteral("duracao"), duration_days);
                data.insert(QStringLiteral("duration"), duration_days);
                data.insert(QStringLiteral("duracao_dias"), duration_days);
                item->setData(Qt::UserRole, data);
                _apply_task_item_appearance(item);
            }
        }
    }

    save_tasks();

    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }

    // 4. Update UnifiedTaskStore records
    bool store_modified = false;
    for (int i = 0; i < records.size(); ++i) {
        QVariantMap rec = records.at(i).toMap();
        const QString rid = rec.value(QStringLiteral("id")).toString().trimmed();
        const QString rinteg = rec.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        const QString rcanon = rec.value(QStringLiteral("canonical_id")).toString().trimmed();
        const QString rtitle = rec.value(QStringLiteral("titulo"), rec.value(QStringLiteral("title"))).toString().trimmed();

        if (matched_ids.contains(rid) || matched_ids.contains(rinteg) ||
            (!rcanon.isEmpty() && matched_canonical_ids.contains(rcanon)) ||
            (!rtitle.isEmpty() && matched_titles.contains(rtitle))) {
            rec.insert(QStringLiteral("duracao"), duration_days);
            rec.insert(QStringLiteral("duration"), duration_days);
            rec.insert(QStringLiteral("duracao_dias"), duration_days);
            records[i] = rec;
            store_modified = true;
        }
    }
    if (store_modified) {
        source::utils::UnifiedTaskStore::save_records(tasks_path, records);
    }

    // 5. Notify Gantt plugin
    if (gantt_plugin_ != nullptr) {
        for (const QString &mid : matched_ids) {
            gantt_plugin_->on_eisenhower_task_duration_changed(mid, duration_days);
        }
    }

    // 6. Notify Pomodoro plugin
    if (pomodoro_plugin_ != nullptr) {
        for (const QString &mid : matched_ids) {
            pomodoro_plugin_->on_task_duration_changed(mid, duration_days);
        }
        pomodoro_plugin_->reload_shared_state();
        pomodoro_plugin_->refresh_task_views();
    }

    // 7. Notify Feynman plugin
    if (feynman_plugin_ != nullptr) {
        for (const QString &mid : matched_ids) {
            feynman_plugin_->on_task_duration_changed(mid, duration_days);
        }
        feynman_plugin_->reload_shared_state();
        feynman_plugin_->refresh_task_views();
    }
}

void EisenhowerMatrixApp::sync_task_title(const QString &task_id, const QString &new_title)
{
    const QString target_id = task_id.trimmed();
    const QString cleaned_title = new_title.trimmed();
    if (target_id.isEmpty() || cleaned_title.isEmpty()) return;

    // 1. Identificar IDs e canônicos vinculados
    QSet<QString> matched_ids = {target_id};
    QSet<QString> matched_canonical_ids;
    QSet<QString> matched_titles;

    const auto lists = all_task_lists(this);
    for (QListWidget *list : lists) {
        if (list == nullptr) continue;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || _is_group_header(item)) {
                continue;
            }
            const QVariantMap data = item->data(Qt::UserRole).toMap();
            const QString iid = data.value(QStringLiteral("id")).toString().trimmed();
            const QString i_integ = data.value(QStringLiteral("integracao_task_id")).toString().trimmed();
            if (matched_ids.contains(iid) || (!i_integ.isEmpty() && matched_ids.contains(i_integ))) {
                if (!iid.isEmpty()) matched_ids.insert(iid);
                if (!i_integ.isEmpty()) matched_ids.insert(i_integ);
                const QString canon = data.value(QStringLiteral("canonical_id")).toString().trimmed();
                if (!canon.isEmpty()) matched_canonical_ids.insert(canon);
                const QString base_text = data.value(QStringLiteral("text")).toString().trimmed();
                if (!base_text.isEmpty()) matched_titles.insert(base_text);
            }
        }
    }

    // 2. Atualizar registros em UnifiedTaskStore
    QVariantList records = source::utils::UnifiedTaskStore::load_records(tasks_path);
    bool store_modified = false;
    for (int i = 0; i < records.size(); ++i) {
        QVariantMap rec = records.at(i).toMap();
        const QString rid = rec.value(QStringLiteral("id")).toString().trimmed();
        const QString rinteg = rec.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        const QString rcanon = rec.value(QStringLiteral("canonical_id")).toString().trimmed();
        const QString rtitle = rec.value(QStringLiteral("titulo"), rec.value(QStringLiteral("title"))).toString().trimmed();

        if (matched_ids.contains(rid) || (!rinteg.isEmpty() && matched_ids.contains(rinteg)) ||
            (!rcanon.isEmpty() && matched_canonical_ids.contains(rcanon)) ||
            (!rtitle.isEmpty() && matched_titles.contains(rtitle))) {
            rec.insert(QStringLiteral("text"), cleaned_title);
            rec.insert(QStringLiteral("titulo"), cleaned_title);
            rec.insert(QStringLiteral("title"), cleaned_title);
            records[i] = rec;
            store_modified = true;
            if (!rid.isEmpty()) matched_ids.insert(rid);
            if (!rinteg.isEmpty()) matched_ids.insert(rinteg);
        }
    }
    if (store_modified) {
        source::utils::UnifiedTaskStore::save_records(tasks_path, records);
    }

    // 3. Atualizar itens em memória do Eisenhower
    bool eisenhower_modified = false;
    for (QListWidget *list : lists) {
        if (list == nullptr) continue;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || _is_group_header(item)) {
                continue;
            }
            QVariantMap data = item->data(Qt::UserRole).toMap();
            const QString iid = data.value(QStringLiteral("id")).toString().trimmed();
            const QString i_integ = data.value(QStringLiteral("integracao_task_id")).toString().trimmed();
            const QString base_text = data.value(QStringLiteral("text")).toString().trimmed();

            if (matched_ids.contains(iid) || (!i_integ.isEmpty() && matched_ids.contains(i_integ)) ||
                (!base_text.isEmpty() && matched_titles.contains(base_text))) {
                data.insert(QStringLiteral("text"), cleaned_title);
                item->setData(Qt::UserRole, data);
                _apply_task_item_appearance(item);
                eisenhower_modified = true;
            }
        }
    }

    if (eisenhower_modified) {
        save_tasks();
    }

    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }

    if (pomodoro_plugin_ != nullptr) {
        pomodoro_plugin_->reload_shared_state();
        pomodoro_plugin_->refresh_task_views();
    }

    if (feynman_plugin_ != nullptr) {
        feynman_plugin_->reload_shared_state();
        feynman_plugin_->refresh_task_views();
    }
}

bool EisenhowerMatrixApp::remove_task_by_id(const QString &task_id)
{
    const QString target_id = task_id.trimmed();
    if (target_id.isEmpty()) return false;

    bool removed_any = false;
    const auto lists = all_task_lists(this);
    for (QListWidget *list : lists) {
        if (list == nullptr) continue;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable) || _is_group_header(item)) {
                continue;
            }
            const QVariantMap data = item->data(Qt::UserRole).toMap();
            const QString iid = data.value(QStringLiteral("id")).toString().trimmed();
            const QString i_integ = data.value(QStringLiteral("integracao_task_id")).toString().trimmed();
            if (iid == target_id || (!i_integ.isEmpty() && i_integ == target_id)) {
                if (remove_task(item, list, false)) {
                    removed_any = true;
                }
                break;
            }
        }
    }

    // Garantir remoção em UnifiedTaskStore e notificação aos plugins
    QVariantList records = source::utils::UnifiedTaskStore::load_records(tasks_path);
    bool store_modified = false;
    for (int i = records.size() - 1; i >= 0; --i) {
        const QVariantMap rec = records.at(i).toMap();
        const QString rid = rec.value(QStringLiteral("id")).toString().trimmed();
        const QString rinteg = rec.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        if (rid == target_id || (!rinteg.isEmpty() && rinteg == target_id)) {
            records.removeAt(i);
            store_modified = true;
            removed_any = true;
        }
    }
    if (store_modified) {
        source::utils::UnifiedTaskStore::save_records(tasks_path, records);
        if (pomodoro_plugin_ != nullptr) {
            pomodoro_plugin_->reload_shared_state();
            pomodoro_plugin_->refresh_task_views();
        }
        if (feynman_plugin_ != nullptr) {
            feynman_plugin_->reload_shared_state();
            feynman_plugin_->refresh_task_views();
        }
    }

    return removed_any;
}

QWidget *EisenhowerMatrixApp::as_widget()
{
    return this;
}

}  // namespace source
