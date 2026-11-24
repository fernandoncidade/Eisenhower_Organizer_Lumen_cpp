#include "source/app/app_08_apply_theme_palette.hpp"

#include <QDir>
#include <QFileInfo>
#include <QScrollArea>

#include "source/ui/ui_17_OllamaInstrucoesDialog.hpp"
#include "source/ui/ui_18_ModelDownloadDialog.hpp"

namespace source {
using namespace app;

void EisenhowerMatrixApp::_apply_theme_palette()
{
    auto apply_window_bg = [](QWidget *widget, const QColor &color) {
        if (widget == nullptr) {
            return;
        }

        QPalette palette = widget->palette();
        palette.setColor(QPalette::Window, color);
        widget->setAutoFillBackground(true);
        widget->setPalette(palette);
    };

    auto apply_list_bg = [](QListWidget *widget, const QColor &color) {
        if (widget == nullptr) {
            return;
        }

        QPalette palette = widget->palette();
        palette.setColor(QPalette::Base, color);
        palette.setColor(QPalette::AlternateBase, color);
        widget->setPalette(palette);
        widget->setAutoFillBackground(true);
        if (widget->viewport() != nullptr) {
            widget->viewport()->setAutoFillBackground(true);
        }
    };

    auto apply_input_bg = [](QWidget *widget, const QColor &color) {
        if (widget == nullptr) {
            return;
        }

        QPalette palette = widget->palette();
        palette.setColor(QPalette::Base, color);
        palette.setColor(QPalette::AlternateBase, color);

        if (auto *combo = qobject_cast<QComboBox *>(widget)) {
            palette.setColor(QPalette::Button, color);
            palette.setColor(QPalette::Window, color);
            combo->setPalette(palette);
            if (combo->view() != nullptr) {
                QPalette view_palette = combo->view()->palette();
                view_palette.setColor(QPalette::Base, color);
                view_palette.setColor(QPalette::AlternateBase, color);
                combo->view()->setPalette(view_palette);
            }
        } else {
            widget->setPalette(palette);
        }
    };

    const QColor base_color = palette().color(QPalette::Window);
    const QColor main_color = QColor(base_color).lighter(120);
    const QColor list_color = QColor(base_color).darker(105);

    apply_window_bg(_root_container, main_color);
    apply_window_bg(module_tabs, main_color);
    apply_window_bg(eisenhower_module_page, main_color);
    apply_window_bg(pomodoro_module_page, main_color);
    apply_window_bg(audiolivro_module_page, main_color);
    apply_window_bg(_main_content, main_color);
    apply_window_bg(new_task_scroll, main_color);
    if (new_task_scroll != nullptr && new_task_scroll->viewport() != nullptr) {
        apply_window_bg(new_task_scroll->viewport(), main_color);
    }
    apply_window_bg(new_task_content, main_color);
    apply_window_bg(task_layout_host, main_color);
    apply_window_bg(quadrant_tabs_scroll, main_color);
    apply_window_bg(quadrants_scroll, main_color);
    apply_window_bg(matrix2x2_scroll, main_color);
    if (quadrant_tabs_scroll != nullptr && quadrant_tabs_scroll->viewport() != nullptr) {
        apply_window_bg(quadrant_tabs_scroll->viewport(), main_color);
    }
    if (quadrants_scroll != nullptr && quadrants_scroll->viewport() != nullptr) {
        apply_window_bg(quadrants_scroll->viewport(), main_color);
    }
    if (matrix2x2_scroll != nullptr && matrix2x2_scroll->viewport() != nullptr) {
        apply_window_bg(matrix2x2_scroll->viewport(), main_color);
    }
    apply_window_bg(quadrant_tabs_page, main_color);
    apply_window_bg(quadrants_page, main_color);
    apply_window_bg(matrix2x2_page, main_color);
    apply_window_bg(quadrant_tabs, main_color);
    apply_window_bg(quadrant1_status_tabs, main_color);
    apply_window_bg(quadrant2_status_tabs, main_color);
    apply_window_bg(quadrant3_status_tabs, main_color);
    apply_window_bg(quadrant4_status_tabs, main_color);
    apply_window_bg(quadrant1_tab, main_color);
    apply_window_bg(quadrant2_tab, main_color);
    apply_window_bg(quadrant3_tab, main_color);
    apply_window_bg(quadrant4_tab, main_color);

    for (QLabel *label : {quadrant1_label,
                          quadrant2_label,
                          quadrant3_label,
                          quadrant4_label,
                          quadrant1_in_progress_label,
                          quadrant2_in_progress_label,
                          quadrant3_in_progress_label,
                          quadrant4_in_progress_label,
                          quadrant1_completed_label,
                          quadrant2_completed_label,
                          quadrant3_completed_label,
                          quadrant4_completed_label}) {
        apply_window_bg(label, main_color);
    }

    for (QGroupBox *group : {new_task_group,
                             quadrant1_group,
                             quadrant2_group,
                             quadrant3_group,
                             quadrant4_group,
                             quadrant1_in_progress_group,
                             quadrant2_in_progress_group,
                             quadrant3_in_progress_group,
                             quadrant4_in_progress_group,
                             quadrant1_completed_group,
                             quadrant2_completed_group,
                             quadrant3_completed_group,
                             quadrant4_completed_group}) {
        apply_window_bg(group, main_color);
    }

    for (QListWidget *list : {quadrant1_list,
                              quadrant2_list,
                              quadrant3_list,
                              quadrant4_list,
                              quadrant1_in_progress_list,
                              quadrant2_in_progress_list,
                              quadrant3_in_progress_list,
                              quadrant4_in_progress_list,
                              quadrant1_completed_list,
                              quadrant2_completed_list,
                              quadrant3_completed_list,
                              quadrant4_completed_list}) {
        apply_list_bg(list, list_color);
    }

    for (QWidget *input : {static_cast<QWidget *>(task_input),
                           static_cast<QWidget *>(date_input),
                           static_cast<QWidget *>(time_input),
                           static_cast<QWidget *>(quadrant_selector),
                           static_cast<QWidget *>(domain_selector)}) {
        apply_input_bg(input, list_color);
    }

    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        apply_window_bg(calendar_pane->calendar_panel, main_color);
        apply_window_bg(calendar_pane->calendar_panel->tasks_label, main_color);
        apply_window_bg(calendar_pane->calendar_panel->tasks_group, main_color);
        apply_list_bg(calendar_pane->calendar_panel->tasks_list, list_color);
        apply_input_bg(calendar_pane->calendar_panel->filter_combo, list_color);
    }

    if (quadrant_tabs != nullptr && quadrant_tabs->tabBar() != nullptr) {
        quadrant_tabs->tabBar()->update();
    }
    for (QTabWidget *tabs : {quadrant1_status_tabs, quadrant2_status_tabs, quadrant3_status_tabs, quadrant4_status_tabs}) {
        if (tabs != nullptr && tabs->tabBar() != nullptr) {
            tabs->tabBar()->update();
        }
    }
    if (module_tabs != nullptr && module_tabs->tabBar() != nullptr) {
        module_tabs->tabBar()->update();
    }

    if (agenda_color_mode_ == QStringLiteral("preto")) {
        _apply_agenda_color_mode(agenda_color_mode_);
    }

    if (auto *ollama_dialog = qobject_cast<source::GerenciamentoUI::OllamaInstrucoesDialog *>(
            _ollama_instructions_dialog.data())) {
        ollama_dialog->refresh_theme();
    }

    if (auto *download_dialog = qobject_cast<source::GerenciamentoUI::ModelDownloadDialog *>(
            _model_download_dialog.data())) {
        download_dialog->refresh_theme();
    }
}

bool EisenhowerMatrixApp::_open_linked_file_path(const QString &path) const
{
    const QString cleaned = path.trimmed();
    if (cleaned.isEmpty()) {
        return false;
    }

    const QFileInfo file_info(cleaned);
    if (!file_info.exists()) {
        return false;
    }

    const QString absolute_path = file_info.absoluteFilePath();
    if (QDesktopServices::openUrl(QUrl::fromLocalFile(absolute_path))) {
        return true;
    }

#if defined(Q_OS_WIN)
    return QProcess::startDetached(QStringLiteral("explorer.exe"), {QDir::toNativeSeparators(absolute_path)});
#elif defined(Q_OS_MACOS)
    return QProcess::startDetached(QStringLiteral("open"), {absolute_path});
#else
    return QProcess::startDetached(QStringLiteral("xdg-open"), {absolute_path});
#endif
}

void EisenhowerMatrixApp::_set_linked_file_for_item(QListWidgetItem *item, const QString &path)
{
    if (item == nullptr) {
        return;
    }

    QListWidget *owner_list = nullptr;
    for (QListWidget *list : {quadrant1_list,
                              quadrant2_list,
                              quadrant3_list,
                              quadrant4_list,
                              quadrant1_in_progress_list,
                              quadrant2_in_progress_list,
                              quadrant3_in_progress_list,
                              quadrant4_in_progress_list,
                              quadrant1_completed_list,
                              quadrant2_completed_list,
                              quadrant3_completed_list,
                              quadrant4_completed_list}) {
        if (list != nullptr && list->row(item) >= 0) {
            owner_list = list;
            break;
        }
    }

    QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString trimmed_path = path.trimmed();
    if (trimmed_path.isEmpty()) {
        data.remove(QStringLiteral("file_path"));
    } else {
        data.insert(QStringLiteral("file_path"), trimmed_path);
    }
    if (owner_list != nullptr) {
        const QSignalBlocker blocker(owner_list);
        item->setData(Qt::UserRole, data);
    } else {
        item->setData(Qt::UserRole, data);
    }

    save_tasks();
    source::InterfaceCore::incore_11_atualizar_itens_tarefas::atualizar_itens_tarefas(this);
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }
}

bool EisenhowerMatrixApp::handle_linked_file_open_request(QListWidgetItem *item, QWidget *parent)
{
    if (item == nullptr) {
        return false;
    }

    const QVariantMap data = item->data(Qt::UserRole).toMap();
    const QString current_path = data.value(QStringLiteral("file_path")).toString().trimmed();
    if (current_path.isEmpty()) {
        return false;
    }

    if (QFileInfo::exists(current_path)) {
        return _open_linked_file_path(current_path);
    }

    QMessageBox message_box(parent != nullptr ? parent : this);
    message_box.setIcon(QMessageBox::Warning);
    message_box.setWindowTitle(get_text(QStringLiteral("Aviso")));
    message_box.setText(get_text(QStringLiteral("O arquivo vinculado não foi encontrado ou não existe mais.")));
    QAbstractButton *remove_button =
        message_box.addButton(get_text(QStringLiteral("Remover Vínculo")), QMessageBox::ActionRole);
    QAbstractButton *search_button = message_box.addButton(get_text(QStringLiteral("Buscar")), QMessageBox::ActionRole);
    message_box.addButton(get_text(QStringLiteral("OK")), QMessageBox::AcceptRole);
    message_box.exec();

    if (message_box.clickedButton() == remove_button) {
        _set_linked_file_for_item(item, QString());

        QMessageBox success_box(parent != nullptr ? parent : this);
        success_box.setIcon(QMessageBox::Information);
        success_box.setWindowTitle(get_text(QStringLiteral("Sucesso")));
        success_box.setText(get_text(QStringLiteral("Vínculo removido com sucesso!")));
        success_box.setStandardButtons(QMessageBox::Ok);
        if (gerenciador_traducao != nullptr) {
            gerenciador_traducao->traduzir_botoes_padrao(&success_box);
        }
        success_box.exec();
        return true;
    }

    if (message_box.clickedButton() == search_button) {
        const QString initial_dir = QFileInfo(current_path).absolutePath();
        const QString selected_path = QFileDialog::getOpenFileName(parent != nullptr ? parent : this,
                                                                   get_text(QStringLiteral("Buscar arquivo")),
                                                                   initial_dir,
                                                                   QStringLiteral("*"));
        if (!selected_path.trimmed().isEmpty()) {
            _set_linked_file_for_item(item, selected_path);
            return true;
        }
    }

    return false;
}

bool EisenhowerMatrixApp::sync_eisenhower_enabled() const
{
    return sync_eisenhower_enabled_;
}

bool EisenhowerMatrixApp::sync_pomodoro_enabled() const
{
    return sync_pomodoro_enabled_;
}

bool EisenhowerMatrixApp::sync_feynman_enabled() const
{
    return sync_feynman_enabled_;
}

bool EisenhowerMatrixApp::reserved_space_eisenhower_enabled() const
{
    return reserved_space_eisenhower_enabled_;
}

bool EisenhowerMatrixApp::reserved_space_pomodoro_enabled() const
{
    return reserved_space_pomodoro_enabled_;
}

bool EisenhowerMatrixApp::reserved_space_feynman_enabled() const
{
    return reserved_space_feynman_enabled_;
}

bool EisenhowerMatrixApp::reserved_watermark_eisenhower_enabled() const
{
    return reserved_watermark_eisenhower_enabled_;
}

bool EisenhowerMatrixApp::reserved_watermark_pomodoro_enabled() const
{
    return reserved_watermark_pomodoro_enabled_;
}

bool EisenhowerMatrixApp::reserved_watermark_feynman_enabled() const
{
    return reserved_watermark_feynman_enabled_;
}

void EisenhowerMatrixApp::set_sync_eisenhower_enabled(bool enabled)
{
    if (sync_eisenhower_enabled_ == enabled) {
        return;
    }

    sync_eisenhower_enabled_ = enabled;
    _save_task_layout_preference();
    if (enabled) {
        _queue_runtime_sync(QStringLiteral("configuracoes"));
    }
}

void EisenhowerMatrixApp::set_sync_pomodoro_enabled(bool enabled)
{
    if (sync_pomodoro_enabled_ == enabled) {
        return;
    }

    sync_pomodoro_enabled_ = enabled;
    _save_task_layout_preference();
    if (enabled) {
        _queue_runtime_sync(QStringLiteral("configuracoes"));
    }
}

void EisenhowerMatrixApp::set_sync_feynman_enabled(bool enabled)
{
    if (sync_feynman_enabled_ == enabled) {
        return;
    }

    sync_feynman_enabled_ = enabled;
    _save_task_layout_preference();
    if (enabled) {
        _queue_runtime_sync(QStringLiteral("configuracoes"));
    }
}

void EisenhowerMatrixApp::set_reserved_space_eisenhower_enabled(bool enabled)
{
    if (reserved_space_eisenhower_enabled_ == enabled) {
        return;
    }

    reserved_space_eisenhower_enabled_ = enabled;
    _save_task_layout_preference();
    source::InterfaceCore::incore_11_atualizar_itens_tarefas::atualizar_itens_tarefas(this);
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }
}

void EisenhowerMatrixApp::set_reserved_space_pomodoro_enabled(bool enabled)
{
    if (reserved_space_pomodoro_enabled_ == enabled) {
        return;
    }

    reserved_space_pomodoro_enabled_ = enabled;
    _save_task_layout_preference();
    if (pomodoro_plugin_ != nullptr) {
        pomodoro_plugin_->refresh_task_views();
    }
}

void EisenhowerMatrixApp::set_reserved_space_feynman_enabled(bool enabled)
{
    if (reserved_space_feynman_enabled_ == enabled) {
        return;
    }

    reserved_space_feynman_enabled_ = enabled;
    _save_task_layout_preference();
    if (feynman_plugin_ != nullptr) {
        feynman_plugin_->refresh_task_views();
    }

    if (mapamental_plugin_ != nullptr) {
        mapamental_plugin_->refresh_task_views();
    }
}

void EisenhowerMatrixApp::set_reserved_watermark_eisenhower_enabled(bool enabled)
{
    if (reserved_watermark_eisenhower_enabled_ == enabled) {
        return;
    }

    reserved_watermark_eisenhower_enabled_ = enabled;
    _save_task_layout_preference();
    source::InterfaceCore::incore_11_atualizar_itens_tarefas::atualizar_itens_tarefas(this);
    if (calendar_pane != nullptr && calendar_pane->calendar_panel != nullptr) {
        calendar_pane->calendar_panel->update_task_list();
    }
}

void EisenhowerMatrixApp::set_reserved_watermark_pomodoro_enabled(bool enabled)
{
    if (reserved_watermark_pomodoro_enabled_ == enabled) {
        return;
    }

    reserved_watermark_pomodoro_enabled_ = enabled;
    _save_task_layout_preference();
    if (pomodoro_plugin_ != nullptr) {
        pomodoro_plugin_->refresh_task_views();
    }
}

void EisenhowerMatrixApp::set_reserved_watermark_feynman_enabled(bool enabled)
{
    if (reserved_watermark_feynman_enabled_ == enabled) {
        return;
    }

    reserved_watermark_feynman_enabled_ = enabled;
    _save_task_layout_preference();
    if (feynman_plugin_ != nullptr) {
        feynman_plugin_->refresh_task_views();
    }

    if (audiolivro_plugin_ != nullptr) {
        audiolivro_plugin_->refresh_task_views();
    }
}

void EisenhowerMatrixApp::_hide_legacy_calendar_button()
{
    for (QPushButton *button : findChildren<QPushButton *>()) {
        const QString text = button->text().trimmed().toLower();
        if (text == get_text(QStringLiteral("Calendário")).trimmed().toLower() || text == QStringLiteral("calendário")
            || text == QStringLiteral("calendario") || text == QStringLiteral("calendar")) {
            button->hide();
        }
    }
}

bool EisenhowerMatrixApp::_window_geometry_adjustment_deferred() const
{
    if (!isVisible()) {
        return false;
    }

    const Qt::WindowStates states = windowState();
    if (states.testFlag(Qt::WindowMaximized) || states.testFlag(Qt::WindowFullScreen)
        || states.testFlag(Qt::WindowMinimized)) {
        return true;
    }

    return QApplication::mouseButtons().testFlag(Qt::LeftButton);
}

void EisenhowerMatrixApp::_queue_window_geometry_stabilization(int delay_ms)
{
    if (window_geometry_stabilization_queued_) {
        return;
    }

    window_geometry_stabilization_queued_ = true;
    QTimer::singleShot(qMax(0, delay_ms), this, [this]() {
        window_geometry_stabilization_queued_ = false;

        if (_window_geometry_adjustment_deferred()) {
            if (QApplication::mouseButtons().testFlag(Qt::LeftButton)) {
                _queue_window_geometry_stabilization(180);
            }
            return;
        }

        _apply_startup_geometry_constraints();
        _clamp_window_to_screen();
    });
}

void EisenhowerMatrixApp::_clamp_window_to_screen()
{
    if (_window_geometry_adjustment_deferred()) {
        if (QApplication::mouseButtons().testFlag(Qt::LeftButton)) {
            _queue_window_geometry_stabilization(180);
        }
        return;
    }

    QScreen *screen = effective_screen_for_widget(this);
    if (screen == nullptr) {
        return;
    }

    const QRect available = screen->availableGeometry();
    const QSize max_client_size = max_client_size_for_screen(this, available);
    const QRect geometry = this->geometry();
    const QRect frame = frameGeometry();
    const QMargins margins = top_level_frame_margins(this);
    const QSize minimum_size = initial_window_minimum_for_screen(max_client_size);
    const int min_w = minimum_size.width();
    const int min_h = minimum_size.height();
    setMinimumSize(min_w, min_h);

    const int new_w = qBound(min_w, geometry.width(), max_client_size.width());
    const int new_h = qBound(min_h, geometry.height(), max_client_size.height());
    const int frame_w = new_w + margins.left() + margins.right();
    const int frame_h = new_h + margins.top() + margins.bottom();
    const int new_x = qMax(available.x(), qMin(frame.x(), available.x() + available.width() - frame_w));
    const int new_y = qMax(available.y(), qMin(frame.y(), available.y() + available.height() - frame_h));

    if (!isMaximized() && !isFullScreen() && (new_w != geometry.width() || new_h != geometry.height())) {
        resize(new_w, new_h);
    }
    if (!isMaximized() && !isFullScreen() && (new_x != frame.x() || new_y != frame.y())) {
        move(new_x, new_y);
    }

    setMinimumSize(min_w, min_h);
}

void EisenhowerMatrixApp::_apply_startup_geometry_constraints()
{
    if (_window_geometry_adjustment_deferred()) {
        if (QApplication::mouseButtons().testFlag(Qt::LeftButton)) {
            _queue_window_geometry_stabilization(180);
        }
        return;
    }

    QScreen *screen = effective_screen_for_widget(this);
    if (screen == nullptr) {
        return;
    }

    const QRect available = screen->availableGeometry();
    const QSize max_client_size = max_client_size_for_screen(this, available);
    const QSize minimum_size = initial_window_minimum_for_screen(max_client_size);
    const int min_w = minimum_size.width();
    const int min_h = minimum_size.height();

    setMinimumSize(min_w, min_h);
    if (!isMaximized() && !isFullScreen()) {
        const int target_w = qBound(min_w, qMin(width() > 0 ? width() : 1380, max_client_size.width()), max_client_size.width());
        const int target_h = qBound(min_h, qMin(height() > 0 ? height() : 840, max_client_size.height()), max_client_size.height());
        if (width() != target_w || height() != target_h) {
            resize(target_w, target_h);
        }
    }
}

}  // namespace source
