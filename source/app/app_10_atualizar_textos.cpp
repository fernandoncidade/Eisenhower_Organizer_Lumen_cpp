#include "source/app/app_10_atualizar_textos.hpp"
#include "source/services/RecursosProdutividade/recpro_07_filtros.hpp"
#include "source/ui/ui_17_OllamaInstrucoesDialog.hpp"
#include "source/ui/ui_18_ModelDownloadDialog.hpp"

namespace source {
using namespace app;

void EisenhowerMatrixApp::atualizar_textos()
{
    QTimer::singleShot(0, this, [this]() {
        _sync_feynman_host_bridge();
        if (feynman_plugin_ != nullptr) {
            feynman_plugin_->retranslate_ui();
        }
        _sync_pomodoro_host_bridge();
        if (pomodoro_plugin_ != nullptr) {
            pomodoro_plugin_->retranslate_ui();
        }
        _sync_mapamental_host_bridge();
        if (mapamental_plugin_ != nullptr) {
            mapamental_plugin_->retranslate_ui();
        }
        _sync_audiolivro_host_bridge();
        if (audiolivro_plugin_ != nullptr) {
            audiolivro_plugin_->retranslate_ui();
        }
        _sync_agenda_host_bridge();
        if (agenda_plugin_ != nullptr) {
            agenda_plugin_->retranslate_ui();
        }
        _sync_gantt_host_bridge();
        if (gantt_plugin_ != nullptr) {
            gantt_plugin_->retranslate_ui();
        }
        source::InterfaceCore::atualizar_textos(this);
        source::InterfaceCore::ProductivityFeatures::apply_active_filters(this);
        _attach_module_task_counter_to_menu_bar();
        _refresh_module_tabs();
        if (calendar_pane != nullptr) {
            calendar_pane->on_language_changed();
        }
        if (auto *ollama_dialog = qobject_cast<source::GerenciamentoUI::OllamaInstrucoesDialog *>(
                _ollama_instructions_dialog.data())) {
            ollama_dialog->retranslate_ui();
        }
        if (auto *download_dialog = qobject_cast<source::GerenciamentoUI::ModelDownloadDialog *>(
                _model_download_dialog.data())) {
            download_dialog->retranslate_ui();
        }
    });
}

void EisenhowerMatrixApp::atualizar_placeholders()
{
    source::InterfaceCore::atualizar_placeholders(this);
}

void EisenhowerMatrixApp::exibir_sobre()
{
    source::InterfaceCore::exibir_sobre(this);
}

void EisenhowerMatrixApp::show_context_menu(const QPoint &point, QListWidget *list_widget)
{
    source::InterfaceCore::show_context_menu(this, point, list_widget);
}

void EisenhowerMatrixApp::open_calendar()
{
    if (module_tabs != nullptr && eisenhower_module_page != nullptr) {
        set_module_tab_visible(QStringLiteral("eisenhower"), true);
        module_tabs->setCurrentWidget(eisenhower_module_page);
    }

    if (calendar_pane != nullptr) {
        calendar_pane->toggle_panel(true);
        _sync_splitter_for_calendar();
    }
}

void EisenhowerMatrixApp::_sync_splitter_for_calendar()
{
    if (main_splitter == nullptr || calendar_pane == nullptr) {
        return;
    }

    const QList<int> sizes = main_splitter->sizes();
    if (sizes.size() < 2) {
        return;
    }

    const int calendar_w = qMax(qMax(25, calendar_pane->minimumWidth()),
                                qMax(calendar_pane->width(), calendar_pane->sizeHint().width()));
    const int total_w = (sizes.at(0) + sizes.at(1) > 0) ? sizes.at(0) + sizes.at(1) : qMax(width(), 400);
    const int quadrant_w = qMax(qMax(120, quadrants_min_width), total_w - calendar_w);
    main_splitter->setSizes({calendar_w, quadrant_w});
}

void EisenhowerMatrixApp::nova_sessao()
{
    arquivo->novo();
}

void EisenhowerMatrixApp::abrir_arquivo()
{
    arquivo->abrir_arquivo();
}

void EisenhowerMatrixApp::salvar()
{
    arquivo->salvar();
}

void EisenhowerMatrixApp::salvar_como()
{
    arquivo->salvar_como();
}

void EisenhowerMatrixApp::limpar_tudo()
{
    arquivo->limpar_tudo();
}

void EisenhowerMatrixApp::sair_app()
{
    arquivo->sair();
}

void EisenhowerMatrixApp::edit_task_datetime(QListWidgetItem *item, QListWidget *list_widget)
{
    source::GerenciamentoUI::edit_task_datetime(this, item, list_widget);
}

void EisenhowerMatrixApp::move_task_to_quadrant(QListWidgetItem *item,
                                                QListWidget *source_list,
                                                QListWidget *target_list)
{
    source::GerenciamentoUI::move_task_to_quadrant(this, item, source_list, target_list);
}

QListWidget *EisenhowerMatrixApp::quadrant_list(int quadrant, bool completed) const
{
    switch (quadrant) {
    case 1:
        return completed ? quadrant1_completed_list : quadrant1_list;
    case 2:
        return completed ? quadrant2_completed_list : quadrant2_list;
    case 3:
        return completed ? quadrant3_completed_list : quadrant3_list;
    case 4:
        return completed ? quadrant4_completed_list : quadrant4_list;
    default:
        return nullptr;
    }
}

bool EisenhowerMatrixApp::is_completed_list(const QListWidget *list) const
{
    return list == quadrant1_completed_list || list == quadrant2_completed_list || list == quadrant3_completed_list
           || list == quadrant4_completed_list;
}

bool EisenhowerMatrixApp::is_in_progress_list(const QListWidget *list) const
{
    return list == quadrant1_in_progress_list || list == quadrant2_in_progress_list || list == quadrant3_in_progress_list
           || list == quadrant4_in_progress_list;
}

QListWidget *EisenhowerMatrixApp::quadrant_in_progress_list(int quadrant) const
{
    switch (quadrant) {
    case 1:
        return quadrant1_in_progress_list;
    case 2:
        return quadrant2_in_progress_list;
    case 3:
        return quadrant3_in_progress_list;
    case 4:
        return quadrant4_in_progress_list;
    default:
        return nullptr;
    }
}

}  // namespace source
