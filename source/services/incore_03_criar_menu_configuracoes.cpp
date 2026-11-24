#include "source/services/incore_03_criar_menu_configuracoes.hpp"

#include <functional>
#include <memory>

#include <QAction>
#include <QActionGroup>
#include <QAbstractButton>
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QColorDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QCursor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPointer>
#include <QPushButton>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStringList>
#include <QTimer>
#include <QTranslator>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>

#include "source/services/incore_08_exibir_sobre.hpp"
#include "source/services/incore_11_atualizar_itens_tarefas.hpp"
#include "source/services/incore_15_ProductivityFeatures.hpp"
#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"
#include "source/utils/LogManager.hpp"

namespace source::InterfaceCore::incore_03_criar_menu_configuracoes {
namespace {

class StickyMenu final : public QMenu {
public:
    explicit StickyMenu(const QString &title, QWidget *parent = nullptr) : QMenu(title, parent)
    {
        close_watcher_.setInterval(50);
        close_watcher_.setSingleShot(false);
        QObject::connect(&close_watcher_, &QTimer::timeout, this, [this]() { update_close_on_mouse_exit(); });
    }

    void set_sticky_parent(StickyMenu *parent_menu) { sticky_parent_menu_ = parent_menu; }

protected:
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event == nullptr) {
            QMenu::mouseReleaseEvent(event);
            return;
        }

        QAction *action = actionAt(event->pos());
        if (event->button() != Qt::LeftButton || action == nullptr || !action->isEnabled() || action->isSeparator()
            || action->menu() != nullptr) {
            QMenu::mouseReleaseEvent(event);
            return;
        }

        setActiveAction(action);
        action->trigger();
        arm_close_on_mouse_exit_chain();
        event->accept();
    }

    void hideEvent(QHideEvent *event) override
    {
        disarm_close_on_mouse_exit();
        QMenu::hideEvent(event);
    }

private:
    void arm_close_on_mouse_exit()
    {
        close_on_mouse_exit_ = true;
        if (isVisible() && !close_watcher_.isActive()) {
            close_watcher_.start();
        }
    }

    void arm_close_on_mouse_exit_chain()
    {
        StickyMenu *branch_root = this;
        while (branch_root->sticky_parent_menu_ != nullptr) {
            branch_root = branch_root->sticky_parent_menu_.data();
        }

        for (StickyMenu *menu = this; menu != nullptr; menu = menu->sticky_parent_menu_.data()) {
            menu->close_branch_root_ = branch_root;
            menu->arm_close_on_mouse_exit();
        }
    }

    void disarm_close_on_mouse_exit()
    {
        close_on_mouse_exit_ = false;
        close_branch_root_.clear();
        close_watcher_.stop();
    }

    void update_close_on_mouse_exit()
    {
        if (!close_on_mouse_exit_) {
            close_watcher_.stop();
            return;
        }
        if (!isVisible()) {
            disarm_close_on_mouse_exit();
            return;
        }
        StickyMenu *branch_root = close_branch_root_.data();
        if (cursor_over_menu_tree(branch_root != nullptr ? branch_root : this)) {
            return;
        }

        disarm_close_on_mouse_exit();
        close();
    }

    static bool cursor_over_menu_tree(const StickyMenu *menu)
    {
        if (menu == nullptr || !menu->isVisible()) {
            return false;
        }

        const QPoint cursor_pos = QCursor::pos();
        if (menu->frameGeometry().contains(cursor_pos)) {
            return true;
        }

        for (QAction *action : menu->actions()) {
            QMenu *submenu = action != nullptr ? action->menu() : nullptr;
            if (submenu == nullptr || !submenu->isVisible()) {
                continue;
            }

            if (auto *sticky_submenu = dynamic_cast<StickyMenu *>(submenu)) {
                if (cursor_over_menu_tree(sticky_submenu)) {
                    return true;
                }
                continue;
            }

            if (submenu->frameGeometry().contains(cursor_pos)) {
                return true;
            }
        }

        return false;
    }

    QPointer<StickyMenu> sticky_parent_menu_;
    QPointer<StickyMenu> close_branch_root_;
    QTimer close_watcher_;
    bool close_on_mouse_exit_ = false;
};

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

void log_menu_stage(const QString &stage)
{
    source::utils::LogManager::get_logger()->debug(QStringLiteral("Menu de configurações: %1").arg(stage));
}

QString interface_menu_text()
{
    return get_text(QStringLiteral("Interface do Usuário"));
}

QString eisenhower_matrix_menu_text()
{
    return get_text(QStringLiteral("Eisenhower"));
}

QString pomodoro_interface_simple_text()
{
    return get_text(QStringLiteral("Ao lado"));
}

QString pomodoro_interface_robust_text()
{
    return get_text(QStringLiteral("Embaixo"));
}

QString pomodoro_interface_tabs_text()
{
    return get_text(QStringLiteral("Abas"));
}

QString pomodoro_interface_vertical_text()
{
    return get_text(QStringLiteral("Vertical"));
}

QString pomodoro_interface_horizontal_text()
{
    return get_text(QStringLiteral("Horizontal"));
}

QString feynman_interface_tabs_text()
{
    return get_text(QStringLiteral("Abas"));
}

QString feynman_interface_vertical_text()
{
    return get_text(QStringLiteral("Vertical"));
}

QString feynman_interface_horizontal_text()
{
    return get_text(QStringLiteral("Horizontal"));
}

QString color_tasks_menu_path()
{
    return get_text(QStringLiteral("Configurações>Configurações de Fonte>Colorir tarefas"));
}

QString general_task_font_menu_path()
{
    return get_text(QStringLiteral("Configurações>Configurações de Fonte>Eisenhower>Tarefas em Geral"));
}

QString linked_file_font_menu_path()
{
    return get_text(QStringLiteral("Configurações>Configurações de Fonte>Eisenhower>Arquivo Vinculado"));
}

QString audiolivro_font_menu_path()
{
    return get_text(QStringLiteral("Configurações>Configurações de Fonte>Audiolivro>Texto Audiolivro"));
}

QString feynman_interface_tabs_menu_path()
{
    return get_text(QStringLiteral("Configurações>Layout de Tarefas>Interface do Usuário>Feynman>Abas"));
}

QString feynman_interface_vertical_menu_path()
{
    return get_text(QStringLiteral("Configurações>Layout de Tarefas>Interface do Usuário>Feynman>Vertical"));
}

QString feynman_interface_horizontal_menu_path()
{
    return get_text(QStringLiteral("Configurações>Layout de Tarefas>Interface do Usuário>Feynman>Horizontal"));
}

QString pomodoro_interface_simple_tabs_menu_path()
{
    return get_text(QStringLiteral("Configurações>Layout de Tarefas>Interface do Usuário>Pomodoro>Ao lado>Abas"));
}

QString pomodoro_interface_simple_vertical_menu_path()
{
    return get_text(QStringLiteral("Configurações>Layout de Tarefas>Interface do Usuário>Pomodoro>Ao lado>Vertical"));
}

QString pomodoro_interface_simple_horizontal_menu_path()
{
    return get_text(QStringLiteral("Configurações>Layout de Tarefas>Interface do Usuário>Pomodoro>Ao lado>Horizontal"));
}

QString pomodoro_interface_robust_tabs_menu_path()
{
    return get_text(QStringLiteral("Configurações>Layout de Tarefas>Interface do Usuário>Pomodoro>Embaixo>Abas"));
}

QString pomodoro_interface_robust_vertical_menu_path()
{
    return get_text(QStringLiteral("Configurações>Layout de Tarefas>Interface do Usuário>Pomodoro>Embaixo>Vertical"));
}

QString pomodoro_interface_robust_horizontal_menu_path()
{
    return get_text(QStringLiteral("Configurações>Layout de Tarefas>Interface do Usuário>Pomodoro>Embaixo>Horizontal"));
}

void set_menu_path(QAction *action, const QString &path)
{
    if (action == nullptr) {
        return;
    }
    action->setProperty("menu_path", path);
}

QString pomodoro_interface_layout_applied_message(const QString &layout_name)
{
    return get_text(QStringLiteral("Layout da interface do Pomodoro aplicado: %1.")).arg(layout_name);
}

QString feynman_interface_layout_applied_message(const QString &layout_name)
{
    return get_text(QStringLiteral("Layout da interface do Feynman aplicado: %1.")).arg(layout_name);
}

QString mapamental_interface_layout_applied_message(const QString &layout_name)
{
    return get_text(QStringLiteral("Layout da interface do Mapa Mental aplicado: %1.")).arg(layout_name);
}

void retranslate_pomodoro_interface_actions(QMenu *simple_menu,
                                            QAction *simple_tabs_action,
                                            QAction *simple_vertical_action,
                                            QAction *simple_horizontal_action,
                                            QMenu *robust_menu,
                                            QAction *robust_tabs_action,
                                            QAction *robust_vertical_action,
                                            QAction *robust_horizontal_action)
{
    if (simple_menu != nullptr) {
        simple_menu->setTitle(pomodoro_interface_simple_text());
    }
    if (robust_menu != nullptr) {
        robust_menu->setTitle(pomodoro_interface_robust_text());
    }

    if (simple_tabs_action != nullptr) {
        simple_tabs_action->setText(pomodoro_interface_tabs_text());
        set_menu_path(simple_tabs_action, pomodoro_interface_simple_tabs_menu_path());
    }
    if (simple_vertical_action != nullptr) {
        simple_vertical_action->setText(pomodoro_interface_vertical_text());
        set_menu_path(simple_vertical_action, pomodoro_interface_simple_vertical_menu_path());
    }
    if (simple_horizontal_action != nullptr) {
        simple_horizontal_action->setText(pomodoro_interface_horizontal_text());
        set_menu_path(simple_horizontal_action, pomodoro_interface_simple_horizontal_menu_path());
    }

    if (robust_tabs_action != nullptr) {
        robust_tabs_action->setText(pomodoro_interface_tabs_text());
        set_menu_path(robust_tabs_action, pomodoro_interface_robust_tabs_menu_path());
    }
    if (robust_vertical_action != nullptr) {
        robust_vertical_action->setText(pomodoro_interface_vertical_text());
        set_menu_path(robust_vertical_action, pomodoro_interface_robust_vertical_menu_path());
    }
    if (robust_horizontal_action != nullptr) {
        robust_horizontal_action->setText(pomodoro_interface_horizontal_text());
        set_menu_path(robust_horizontal_action, pomodoro_interface_robust_horizontal_menu_path());
    }
}

void retranslate_feynman_interface_actions(QAction *tabs_action, QAction *vertical_action, QAction *horizontal_action)
{
    if (tabs_action != nullptr) {
        tabs_action->setText(feynman_interface_tabs_text());
        set_menu_path(tabs_action, feynman_interface_tabs_menu_path());
    }
    if (vertical_action != nullptr) {
        vertical_action->setText(feynman_interface_vertical_text());
        set_menu_path(vertical_action, feynman_interface_vertical_menu_path());
    }
    if (horizontal_action != nullptr) {
        horizontal_action->setText(feynman_interface_horizontal_text());
        set_menu_path(horizontal_action, feynman_interface_horizontal_menu_path());
    }
}

void delete_shortcut(QShortcut *&shortcut)
{
    if (shortcut != nullptr) {
        shortcut->deleteLater();
        shortcut = nullptr;
    }
}

void popup_menu(QMenu *menu, QMenuBar *menu_bar)
{
    if (menu == nullptr || menu_bar == nullptr) {
        return;
    }
    const QRect geometry = menu_bar->actionGeometry(menu->menuAction());
    menu->popup(menu_bar->mapToGlobal(geometry.bottomLeft()));
}

StickyMenu *create_menu(const QString &title, QWidget *parent = nullptr)
{
    return new StickyMenu(title, parent);
}

void add_submenu(QMenu *parent_menu, StickyMenu *submenu)
{
    if (parent_menu == nullptr || submenu == nullptr) {
        return;
    }

    if (auto *sticky_parent = dynamic_cast<StickyMenu *>(parent_menu)) {
        submenu->set_sticky_parent(sticky_parent);
    }

    parent_menu->addMenu(submenu);
}

QString eisenhower_layout_name(source::EisenhowerMatrixApp::TaskLayoutMode mode)
{
    switch (mode) {
    case source::EisenhowerMatrixApp::TaskLayoutMode::Quadrants:
        return get_text(QStringLiteral("Layout em Quadrantes"));
    case source::EisenhowerMatrixApp::TaskLayoutMode::Matrix2x2:
        return get_text(QStringLiteral("Layout Matriz 2x2"));
    case source::EisenhowerMatrixApp::TaskLayoutMode::TabsHorizontal:
        return QStringLiteral("%1 / %2").arg(get_text(QStringLiteral("Layout em Abas")), get_text(QStringLiteral("Horizontal")));
    case source::EisenhowerMatrixApp::TaskLayoutMode::Tabs:
    default:
        return QStringLiteral("%1 / %2").arg(get_text(QStringLiteral("Layout em Abas")), get_text(QStringLiteral("Vertical")));
    }
}

QString layout_success_message(source::EisenhowerMatrixApp::TaskLayoutMode mode)
{
    return get_text(QStringLiteral("Layout da interface do Eisenhower aplicado: %1.")).arg(eisenhower_layout_name(mode));
}

QString layout_error_message(source::EisenhowerMatrixApp::TaskLayoutMode mode)
{
    switch (mode) {
    case source::EisenhowerMatrixApp::TaskLayoutMode::Quadrants:
        return get_text(QStringLiteral("Não foi possível configurar a Interface Gráfica para janela de tarefas em Quadrantes."));
    case source::EisenhowerMatrixApp::TaskLayoutMode::Matrix2x2:
        return get_text(QStringLiteral("Não foi possível configurar a Interface Gráfica para janela de tarefas em Matriz 2x2."));
    case source::EisenhowerMatrixApp::TaskLayoutMode::TabsHorizontal:
        return get_text(QStringLiteral("Não foi possível configurar a Interface Gráfica para janela de tarefas em Abas (Horizontal)."));
    case source::EisenhowerMatrixApp::TaskLayoutMode::Tabs:
    default:
        return get_text(QStringLiteral("Não foi possível configurar a Interface Gráfica para janela de tarefas em Abas (Vertical)."));
    }
}

void show_feedback_message(source::EisenhowerMatrixApp *app,
                           QMessageBox::Icon icon,
                           const QString &title,
                           const QString &text)
{
    if (app == nullptr) {
        return;
    }

    QMessageBox box(app);
    box.setIcon(icon);
    box.setWindowTitle(title);
    box.setText(text);
    box.setStandardButtons(QMessageBox::Ok);
    if (app->gerenciador_traducao != nullptr) {
        app->gerenciador_traducao->traduzir_botoes_padrao(&box);
    }
    box.exec();
}

QList<QListWidget *> eisenhower_task_lists(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return {};
    }

    return {
        app->quadrant1_list,
        app->quadrant2_list,
        app->quadrant3_list,
        app->quadrant4_list,
        app->quadrant1_in_progress_list,
        app->quadrant2_in_progress_list,
        app->quadrant3_in_progress_list,
        app->quadrant4_in_progress_list,
        app->quadrant1_completed_list,
        app->quadrant2_completed_list,
        app->quadrant3_completed_list,
        app->quadrant4_completed_list,
    };
}

bool is_real_task_item(QListWidgetItem *item)
{
    if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
        return false;
    }

    return item->data(Qt::UserRole + 1).toString() != QStringLiteral("group_header");
}

QList<QListWidgetItem *> selected_task_items(source::EisenhowerMatrixApp *app)
{
    QList<QListWidgetItem *> items;
    for (QListWidget *list : eisenhower_task_lists(app)) {
        if (list == nullptr) {
            continue;
        }

        for (QListWidgetItem *item : list->selectedItems()) {
            if (is_real_task_item(item) && !items.contains(item)) {
                items.append(item);
            }
        }
    }
    return items;
}

QColor initial_color_for_tasks(const QList<QListWidgetItem *> &items)
{
    for (QListWidgetItem *item : items) {
        if (item == nullptr) {
            continue;
        }

        const QColor color(item->data(Qt::UserRole).toMap().value(QStringLiteral("cor_texto")).toString().trimmed());
        if (color.isValid()) {
            return color;
        }
    }
    return QColor(QStringLiteral("#1f6feb"));
}

QString normalized_dialog_text(QString text)
{
    return text.remove(QLatin1Char('&')).replace(QStringLiteral("..."), QString()).trimmed();
}

QString color_dialog_source_text(const QString &text)
{
    static const QMap<QString, QString> sources = {
        {QStringLiteral("Basic colors"), QStringLiteral("Cores básicas")},
        {QStringLiteral("Cores básicas"), QStringLiteral("Cores básicas")},
        {QStringLiteral("Custom colors"), QStringLiteral("Cores personalizadas")},
        {QStringLiteral("Cores personalizadas"), QStringLiteral("Cores personalizadas")},
        {QStringLiteral("Add to Custom Colors"), QStringLiteral("Adicionar às cores personalizadas")},
        {QStringLiteral("Adicionar às cores personalizadas"), QStringLiteral("Adicionar às cores personalizadas")},
        {QStringLiteral("Pick Screen Color"), QStringLiteral("Selecionar cor na tela")},
        {QStringLiteral("Escolher cor da tela"), QStringLiteral("Selecionar cor na tela")},
        {QStringLiteral("Selecionar cor na tela"), QStringLiteral("Selecionar cor na tela")},
        {QStringLiteral("Select Color"), QStringLiteral("Colorir tarefas")},
        {QStringLiteral("Hue:"), QStringLiteral("Matiz:")},
        {QStringLiteral("Matiz:"), QStringLiteral("Matiz:")},
        {QStringLiteral("Sat:"), QStringLiteral("Saturação:")},
        {QStringLiteral("Saturação:"), QStringLiteral("Saturação:")},
        {QStringLiteral("Val:"), QStringLiteral("Valor:")},
        {QStringLiteral("Valor:"), QStringLiteral("Valor:")},
        {QStringLiteral("Red:"), QStringLiteral("Vermelho:")},
        {QStringLiteral("Vermelho:"), QStringLiteral("Vermelho:")},
        {QStringLiteral("Green:"), QStringLiteral("Verde:")},
        {QStringLiteral("Verde:"), QStringLiteral("Verde:")},
        {QStringLiteral("Blue:"), QStringLiteral("Azul:")},
        {QStringLiteral("Azul:"), QStringLiteral("Azul:")},
        {QStringLiteral("Alpha channel:"), QStringLiteral("Canal alfa:")},
        {QStringLiteral("Canal alfa:"), QStringLiteral("Canal alfa:")},
        {QStringLiteral("Alpha:"), QStringLiteral("Alfa:")},
        {QStringLiteral("Alfa:"), QStringLiteral("Alfa:")},
        {QStringLiteral("HTML:"), QStringLiteral("HTML:")},
        {QStringLiteral("Cursor at %1, %2\nPress ESC to cancel"),
         QStringLiteral("Cursor em %1, %2\nPressione ESC para cancelar")},
        {QStringLiteral("Cursor at %1, %2, color: %3\nPress ESC to cancel"),
         QStringLiteral("Cursor em %1, %2, cor: %3\nPressione ESC para cancelar")},
    };
    return sources.value(normalized_dialog_text(text));
}

class ColorDialogScopedTranslator final : public QTranslator {
public:
    explicit ColorDialogScopedTranslator(QObject *parent = nullptr)
        : QTranslator(parent)
    {
    }

    QString translate(const char *context,
                      const char *source_text,
                      const char *disambiguation = nullptr,
                      int n = -1) const override
    {
        Q_UNUSED(disambiguation)
        Q_UNUSED(n)

        if (context == nullptr || source_text == nullptr || QString::fromLatin1(context) != QStringLiteral("QColorDialog")) {
            return {};
        }

        const QString source = color_dialog_source_text(QString::fromUtf8(source_text));
        return source.isEmpty() ? QString() : get_text(source);
    }
};

QString color_dialog_text(const QString &text)
{
    const QString source = color_dialog_source_text(text);
    return source.isEmpty() ? QString() : get_text(source);
}

void translate_color_dialog(QColorDialog *dialog, source::EisenhowerMatrixApp *app)
{
    if (dialog == nullptr) {
        return;
    }

    dialog->setWindowTitle(get_text(QStringLiteral("Colorir tarefas")));

    if (app != nullptr && app->gerenciador_traducao != nullptr) {
        const auto button_boxes = dialog->findChildren<QDialogButtonBox *>();
        for (QDialogButtonBox *button_box : button_boxes) {
            app->gerenciador_traducao->traduzir_botoes_padrao(button_box);
        }
    }

    const auto labels = dialog->findChildren<QLabel *>();
    for (QLabel *label : labels) {
        const QString text = color_dialog_text(label->text());
        if (!text.isEmpty()) {
            label->setText(text);
        }
    }

    const auto buttons = dialog->findChildren<QAbstractButton *>();
    for (QAbstractButton *button : buttons) {
        const QString text = color_dialog_text(button->text());
        if (!text.isEmpty()) {
            button->setText(text);
        }
    }

    const auto groups = dialog->findChildren<QGroupBox *>();
    for (QGroupBox *group : groups) {
        const QString text = color_dialog_text(group->title());
        if (!text.isEmpty()) {
            group->setTitle(text);
        }
    }
}

enum class TaskFontProfile {
    General,
    LinkedFile,
    FeynmanGeneral,
    FeynmanLinkedFile,
    PomodoroGeneral,
    PomodoroLinkedFile,
    MapaMentalGeneral,
    MapaMentalLinkedFile
};

QString task_font_profile_title(TaskFontProfile profile)
{
    switch (profile) {
        case TaskFontProfile::LinkedFile:
        case TaskFontProfile::FeynmanLinkedFile:
        case TaskFontProfile::PomodoroLinkedFile:
        case TaskFontProfile::MapaMentalLinkedFile:
            return get_text(QStringLiteral("Arquivo Vinculado"));
        default:
            return get_text(QStringLiteral("Tarefas em Geral"));
    }
}

QString task_font_profile_preview(TaskFontProfile profile)
{
    switch (profile) {
        case TaskFontProfile::LinkedFile:
        case TaskFontProfile::FeynmanLinkedFile:
        case TaskFontProfile::PomodoroLinkedFile:
        case TaskFontProfile::MapaMentalLinkedFile:
            return get_text(QStringLiteral("Tarefa com arquivo externo vinculado"));
        default:
            return get_text(QStringLiteral("Tarefa comum sem arquivo externo vinculado"));
    }
}

source::utils::TaskTitleFontSettings task_font_profile_defaults(TaskFontProfile profile)
{
    switch (profile) {
        case TaskFontProfile::LinkedFile:
        case TaskFontProfile::FeynmanLinkedFile:
        case TaskFontProfile::PomodoroLinkedFile:
        case TaskFontProfile::MapaMentalLinkedFile:
            return source::utils::default_linked_file_title_font_settings();
        default:
            return source::utils::default_general_task_title_font_settings();
    }
}

source::utils::TaskTitleFontSettings load_task_font_profile(TaskFontProfile profile)
{
    switch (profile) {
        case TaskFontProfile::FeynmanGeneral: return source::utils::load_plugin_general_task_title_font_settings(source::utils::PluginFontContext::Feynman);
        case TaskFontProfile::PomodoroGeneral: return source::utils::load_plugin_general_task_title_font_settings(source::utils::PluginFontContext::Pomodoro);
        case TaskFontProfile::MapaMentalGeneral: return source::utils::load_plugin_general_task_title_font_settings(source::utils::PluginFontContext::MapaMental);
        case TaskFontProfile::General: return source::utils::load_general_task_title_font_settings();
        case TaskFontProfile::FeynmanLinkedFile: return source::utils::load_plugin_linked_file_title_font_settings(source::utils::PluginFontContext::Feynman);
        case TaskFontProfile::PomodoroLinkedFile: return source::utils::load_plugin_linked_file_title_font_settings(source::utils::PluginFontContext::Pomodoro);
        case TaskFontProfile::MapaMentalLinkedFile: return source::utils::load_plugin_linked_file_title_font_settings(source::utils::PluginFontContext::MapaMental);
        case TaskFontProfile::LinkedFile: return source::utils::load_linked_file_title_font_settings();
    }
    return source::utils::default_general_task_title_font_settings();
}

bool save_task_font_profile(TaskFontProfile profile, const source::utils::TaskTitleFontSettings &settings)
{
    switch (profile) {
        case TaskFontProfile::FeynmanGeneral: return source::utils::save_plugin_general_task_title_font_settings(source::utils::PluginFontContext::Feynman, settings);
        case TaskFontProfile::PomodoroGeneral: return source::utils::save_plugin_general_task_title_font_settings(source::utils::PluginFontContext::Pomodoro, settings);
        case TaskFontProfile::MapaMentalGeneral: return source::utils::save_plugin_general_task_title_font_settings(source::utils::PluginFontContext::MapaMental, settings);
        case TaskFontProfile::General: return source::utils::save_general_task_title_font_settings(settings);
        case TaskFontProfile::FeynmanLinkedFile: return source::utils::save_plugin_linked_file_title_font_settings(source::utils::PluginFontContext::Feynman, settings);
        case TaskFontProfile::PomodoroLinkedFile: return source::utils::save_plugin_linked_file_title_font_settings(source::utils::PluginFontContext::Pomodoro, settings);
        case TaskFontProfile::MapaMentalLinkedFile: return source::utils::save_plugin_linked_file_title_font_settings(source::utils::PluginFontContext::MapaMental, settings);
        case TaskFontProfile::LinkedFile: return source::utils::save_linked_file_title_font_settings(settings);
    }
    return false;
}

QString task_font_profile_success_message(TaskFontProfile profile)
{
    switch (profile) {
        case TaskFontProfile::LinkedFile:
        case TaskFontProfile::FeynmanLinkedFile:
        case TaskFontProfile::PomodoroLinkedFile:
        case TaskFontProfile::MapaMentalLinkedFile:
            return get_text(QStringLiteral("Configurações de fonte aplicadas às tarefas com arquivo vinculado."));
        default:
            return get_text(QStringLiteral("Configurações de fonte aplicadas às tarefas em geral."));
    }
}

QString translate_font_style(const QString &raw_style)
{
    const QString trimmed = raw_style.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }
    const QString translated = get_text(trimmed);
    if (!translated.isEmpty() && translated != trimmed) {
        return translated;
    }

    const QString qt_tr = QCoreApplication::translate("QFontDatabase", trimmed.toUtf8().constData());
    if (!qt_tr.isEmpty() && qt_tr != trimmed) {
        return qt_tr;
    }

    const QString language = QLocale().name();
    const bool is_english = language.startsWith(QStringLiteral("en"), Qt::CaseInsensitive);

    if (is_english) {
        return trimmed;
    }

    // ── Step 3: Exhaustive exact map for all known styles on this system ──
    static const QMap<QString, QString> exact_pt = {
        // ── Basic weights ──
        {QStringLiteral("Normal"), QStringLiteral("Normal")},
        {QStringLiteral("Regular"), QStringLiteral("Normal")},
        {QStringLiteral("Bold"), QStringLiteral("Negrito")},
        {QStringLiteral("Italic"), QStringLiteral("Itálico")},
        {QStringLiteral("Bold Italic"), QStringLiteral("Negrito Itálico")},
        {QStringLiteral("Oblique"), QStringLiteral("Oblíquo")},
        {QStringLiteral("Bold Oblique"), QStringLiteral("Negrito Oblíquo")},
        {QStringLiteral("BoldOblique"), QStringLiteral("Negrito Oblíquo")},
        {QStringLiteral("Slanted"), QStringLiteral("Inclinado")},
        {QStringLiteral("Upright"), QStringLiteral("Vertical")},
        {QStringLiteral("Roman"), QStringLiteral("Romano")},
        // ── Black ──
        {QStringLiteral("Black"), QStringLiteral("Preto")},
        {QStringLiteral("Black Italic"), QStringLiteral("Preto Itálico")},
        // ── Light ──
        {QStringLiteral("Light"), QStringLiteral("Fina")},
        {QStringLiteral("Light Italic"), QStringLiteral("Fina Itálico")},
        // ── Semilight ──
        {QStringLiteral("Semilight"), QStringLiteral("Semileve")},
        {QStringLiteral("SemiLight"), QStringLiteral("Semileve")},
        {QStringLiteral("Semi Light"), QStringLiteral("Semileve")},
        {QStringLiteral("Semi-Light"), QStringLiteral("Semileve")},
        {QStringLiteral("Semilight Italic"), QStringLiteral("Semileve Itálico")},
        {QStringLiteral("SemiLight Italic"), QStringLiteral("Semileve Itálico")},
        {QStringLiteral("Semi Light Italic"), QStringLiteral("Semileve Itálico")},
        {QStringLiteral("Semi-Light Italic"), QStringLiteral("Semileve Itálico")},
        // ── Medium ──
        {QStringLiteral("Medium"), QStringLiteral("Média")},
        {QStringLiteral("Medium Italic"), QStringLiteral("Média Itálico")},
        // ── Thin / Hairline / Hair ──
        {QStringLiteral("Thin"), QStringLiteral("Muito Fina")},
        {QStringLiteral("Thin Italic"), QStringLiteral("Muito Fina Itálico")},
        {QStringLiteral("Hairline"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Hair"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Hair Italic"), QStringLiteral("Ultrafina Itálico")},
        // ── ExtraLight ──
        {QStringLiteral("Extra Light"), QStringLiteral("Extra Fina")},
        {QStringLiteral("ExtraLight"), QStringLiteral("Extra Fina")},
        {QStringLiteral("Extralight"), QStringLiteral("Extra Fina")},
        {QStringLiteral("Extra-Light"), QStringLiteral("Extra Fina")},
        {QStringLiteral("Extra Light Italic"), QStringLiteral("Extra Fina Itálico")},
        {QStringLiteral("ExtraLight Italic"), QStringLiteral("Extra Fina Itálico")},
        {QStringLiteral("Extralight Italic"), QStringLiteral("Extra Fina Itálico")},
        {QStringLiteral("Extra-Light Italic"), QStringLiteral("Extra Fina Itálico")},
        // ── Semibold ──
        {QStringLiteral("Semibold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("SemiBold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Semi Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Semi-Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Semibold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("SemiBold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Semi Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Semi-Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        // ── Demi ──
        {QStringLiteral("Demi"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demibold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demi Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demi-Bold"), QStringLiteral("Seminegrito")},
        {QStringLiteral("Demi Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demibold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demi Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demi-Bold Italic"), QStringLiteral("Seminegrito Itálico")},
        {QStringLiteral("Demibold Roman"), QStringLiteral("Seminegrito Romano")},
        // ── Extra Bold ──
        {QStringLiteral("Extra Bold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("ExtraBold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("Extrabold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("Extra-Bold"), QStringLiteral("Extra Negrito")},
        {QStringLiteral("Extra Bold Italic"), QStringLiteral("Extra Negrito Itálico")},
        {QStringLiteral("ExtraBold Italic"), QStringLiteral("Extra Negrito Itálico")},
        {QStringLiteral("Extrabold Italic"), QStringLiteral("Extra Negrito Itálico")},
        {QStringLiteral("Extra-Bold Italic"), QStringLiteral("Extra Negrito Itálico")},
        // ── Ultra Light ──
        {QStringLiteral("Ultra Light"), QStringLiteral("Ultrafina")},
        {QStringLiteral("UltraLight"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Ultralight"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Ultra-Light"), QStringLiteral("Ultrafina")},
        {QStringLiteral("Ultra Light Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("UltraLight Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("Ultralight Italic"), QStringLiteral("Ultrafina Itálico")},
        {QStringLiteral("Ultra-Light Italic"), QStringLiteral("Ultrafina Itálico")},
        // ── Ultra Bold ──
        {QStringLiteral("Ultra Bold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("UltraBold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("Ultrabold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("Ultra-Bold"), QStringLiteral("Ultra Negrito")},
        {QStringLiteral("Ultra Bold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        {QStringLiteral("UltraBold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        {QStringLiteral("Ultrabold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        {QStringLiteral("Ultra-Bold Italic"), QStringLiteral("Ultra Negrito Itálico")},
        // ── Heavy ──
        {QStringLiteral("Heavy"), QStringLiteral("Pesada")},
        {QStringLiteral("Heavy Italic"), QStringLiteral("Pesada Itálico")},
        // ── Book ──
        {QStringLiteral("Book"), QStringLiteral("Livro")},
        {QStringLiteral("Book Italic"), QStringLiteral("Livro Itálico")},
        // ── Bold Outline ──
        {QStringLiteral("Bold Outline"), QStringLiteral("Negrito Contorno")},
        // ── Condensed ──
        {QStringLiteral("Condensed"), QStringLiteral("Condensada")},
        {QStringLiteral("Condensed Italic"), QStringLiteral("Condensada Itálico")},
        {QStringLiteral("Condensed Bold"), QStringLiteral("Negrito Condensado")},
        {QStringLiteral("Bold Condensed"), QStringLiteral("Negrito Condensado")},
        {QStringLiteral("Condensed Bold Italic"), QStringLiteral("Negrito Condensado Itálico")},
        {QStringLiteral("Bold Condensed Italic"), QStringLiteral("Negrito Condensado Itálico")},
        {QStringLiteral("Condensed Extra Bold"), QStringLiteral("Condensada Extra Negrito")},
        {QStringLiteral("Condensed Ultra Bold"), QStringLiteral("Condensada Ultra Negrito")},
        {QStringLiteral("Light Condensed"), QStringLiteral("Fina Condensada")},
        // ── SemiCondensed ──
        {QStringLiteral("SemiCondensed"), QStringLiteral("Semicondensada")},
        {QStringLiteral("Semi Condensed"), QStringLiteral("Semicondensada")},
        {QStringLiteral("Semi-Condensed"), QStringLiteral("Semicondensada")},
        {QStringLiteral("Bold SemiCondensed"), QStringLiteral("Negrito Semicondensado")},
        {QStringLiteral("Bold Semi Condensed"), QStringLiteral("Negrito Semicondensado")},
        {QStringLiteral("Light SemiCondensed"), QStringLiteral("Fina Semicondensada")},
        {QStringLiteral("Light Semi Condensed"), QStringLiteral("Fina Semicondensada")},
        {QStringLiteral("SemiBold Condensed"), QStringLiteral("Seminegrito Condensado")},
        {QStringLiteral("SemiBold SemiCondensed"), QStringLiteral("Seminegrito Semicondensado")},
        {QStringLiteral("SemiLight Condensed"), QStringLiteral("Semileve Condensada")},
        {QStringLiteral("SemiLight SemiCondensed"), QStringLiteral("Semileve Semicondensada")},
        // ── Ext / Extra / Ultra Condensed (abbreviations) ──
        {QStringLiteral("Ext Condensed Bold"), QStringLiteral("Extra Condensada Negrito")},
        {QStringLiteral("Extra Condensed"), QStringLiteral("Extra Condensada")},
        {QStringLiteral("Extra-Condensed"), QStringLiteral("Extra Condensada")},
        {QStringLiteral("Ultra Condensed"), QStringLiteral("Ultra Condensada")},
        {QStringLiteral("Ultra-Condensed"), QStringLiteral("Ultra Condensada")},
        // ── Cond (abbreviation) ──
        {QStringLiteral("Cond Demi"), QStringLiteral("Condensada Seminegrito")},
        {QStringLiteral("Cond Medium"), QStringLiteral("Condensada Média")},
        // ── Compressed ──
        {QStringLiteral("Compressed"), QStringLiteral("Comprimida")},
        {QStringLiteral("Compressed Italic"), QStringLiteral("Comprimida Itálico")},
        {QStringLiteral("Poster Compressed"), QStringLiteral("Poster Comprimida")},
        // ── Narrow ──
        {QStringLiteral("Narrow"), QStringLiteral("Estreita")},
        {QStringLiteral("Narrow Italic"), QStringLiteral("Estreita Itálico")},
        {QStringLiteral("Narrow Bold"), QStringLiteral("Negrito Estreito")},
        {QStringLiteral("Narrow Bold Italic"), QStringLiteral("Negrito Estreito Itálico")},
        // ── Expanded / Extended ──
        {QStringLiteral("Expanded"), QStringLiteral("Expandida")},
        {QStringLiteral("Expanded Italic"), QStringLiteral("Expandida Itálico")},
        {QStringLiteral("Extended"), QStringLiteral("Estendida")},
        {QStringLiteral("Extended Italic"), QStringLiteral("Estendida Itálico")},
        {QStringLiteral("SemiExpanded Italic"), QStringLiteral("Semiexpandida Itálico")},
        {QStringLiteral("Semi Expanded"), QStringLiteral("Semiexpandida")},
        {QStringLiteral("Semi-Expanded"), QStringLiteral("Semiexpandida")},
        {QStringLiteral("Extra Expanded"), QStringLiteral("Extra Expandida")},
        {QStringLiteral("Extra-Expanded"), QStringLiteral("Extra Expandida")},
        {QStringLiteral("Ultra Expanded"), QStringLiteral("Ultra Expandida")},
        {QStringLiteral("Ultra-Expanded"), QStringLiteral("Ultra Expandida")},
        // ── Optical sizes (Sitka) ──
        {QStringLiteral("Small"), QStringLiteral("Pequena")},
        {QStringLiteral("Small Italic"), QStringLiteral("Pequena Itálico")},
        {QStringLiteral("Small Bold"), QStringLiteral("Pequena Negrito")},
        {QStringLiteral("Small Bold Italic"), QStringLiteral("Pequena Negrito Itálico")},
        {QStringLiteral("Small Semibold"), QStringLiteral("Pequena Seminegrito")},
        {QStringLiteral("Small Semibold Italic"), QStringLiteral("Pequena Seminegrito Itálico")},
        {QStringLiteral("Small Light"), QStringLiteral("Pequena Fina")},
        {QStringLiteral("Small Semilight"), QStringLiteral("Pequena Semileve")},
        {QStringLiteral("Text"), QStringLiteral("Texto")},
        {QStringLiteral("Text Italic"), QStringLiteral("Texto Itálico")},
        {QStringLiteral("Text Bold"), QStringLiteral("Texto Negrito")},
        {QStringLiteral("Text Bold Italic"), QStringLiteral("Texto Negrito Itálico")},
        {QStringLiteral("Text Semibold"), QStringLiteral("Texto Seminegrito")},
        {QStringLiteral("Text Semibold Italic"), QStringLiteral("Texto Seminegrito Itálico")},
        {QStringLiteral("Text Light"), QStringLiteral("Texto Fina")},
        {QStringLiteral("Text Semilight"), QStringLiteral("Texto Semileve")},
        {QStringLiteral("Heading"), QStringLiteral("Título")},
        {QStringLiteral("Heading Italic"), QStringLiteral("Título Itálico")},
        {QStringLiteral("Heading Bold"), QStringLiteral("Título Negrito")},
        {QStringLiteral("Heading Bold Italic"), QStringLiteral("Título Negrito Itálico")},
        {QStringLiteral("Heading Semibold"), QStringLiteral("Título Seminegrito")},
        {QStringLiteral("Heading Semibold Italic"), QStringLiteral("Título Seminegrito Itálico")},
        {QStringLiteral("Subheading"), QStringLiteral("Subtítulo")},
        {QStringLiteral("Subheading Italic"), QStringLiteral("Subtítulo Itálico")},
        {QStringLiteral("Subheading Bold"), QStringLiteral("Subtítulo Negrito")},
        {QStringLiteral("Subheading Bold Italic"), QStringLiteral("Subtítulo Negrito Itálico")},
        {QStringLiteral("Subheading Semibold"), QStringLiteral("Subtítulo Seminegrito")},
        {QStringLiteral("Subheading Semibold Italic"), QStringLiteral("Subtítulo Seminegrito Itálico")},
        {QStringLiteral("Display"), QStringLiteral("Exibição")},
        {QStringLiteral("Display Italic"), QStringLiteral("Exibição Itálico")},
        {QStringLiteral("Display Bold"), QStringLiteral("Exibição Negrito")},
        {QStringLiteral("Display Bold Italic"), QStringLiteral("Exibição Negrito Itálico")},
        {QStringLiteral("Display Semibold"), QStringLiteral("Exibição Seminegrito")},
        {QStringLiteral("Display Semibold Italic"), QStringLiteral("Exibição Seminegrito Itálico")},
        {QStringLiteral("Display Light"), QStringLiteral("Exibição Fina")},
        {QStringLiteral("Display Semilight"), QStringLiteral("Exibição Semileve")},
        {QStringLiteral("Banner"), QStringLiteral("Faixa")},
        {QStringLiteral("Banner Italic"), QStringLiteral("Faixa Itálico")},
        {QStringLiteral("Banner Bold"), QStringLiteral("Faixa Negrito")},
        {QStringLiteral("Banner Bold Italic"), QStringLiteral("Faixa Negrito Itálico")},
        {QStringLiteral("Banner Semibold"), QStringLiteral("Faixa Seminegrito")},
        {QStringLiteral("Banner Semibold Italic"), QStringLiteral("Faixa Seminegrito Itálico")},
        {QStringLiteral("Caption"), QStringLiteral("Legenda")},
        {QStringLiteral("Poster"), QStringLiteral("Poster")},
        {QStringLiteral("Headline"), QStringLiteral("Manchete")},
        {QStringLiteral("Subhead"), QStringLiteral("Subtítulo")},
        // ── 18pt (Noto Sans CJK special) ──
        {QStringLiteral("18pt"), QStringLiteral("18pt")},
        {QStringLiteral("18pt Bold"), QStringLiteral("18pt Negrito")},
        {QStringLiteral("18pt Light"), QStringLiteral("18pt Fina")},
        {QStringLiteral("18pt Medium"), QStringLiteral("18pt Média")},
        {QStringLiteral("18pt Thin"), QStringLiteral("18pt Muito Fina")},
    };

    auto it = exact_pt.constFind(trimmed);
    if (it != exact_pt.constEnd()) {
        return it.value();
    }
    for (auto map_it = exact_pt.constBegin(); map_it != exact_pt.constEnd(); ++map_it) {
        if (map_it.key().compare(trimmed, Qt::CaseInsensitive) == 0) {
            return map_it.value();
        }
    }

    // ── Step 4: Component-based regex tokenizer for unknown compound styles ──
    // Rules are ordered longest-first to prevent partial matches.
    struct ReplacementRule {
        const char *en;
        const char *pt;
    };
    static const ReplacementRule rules[] = {
        // ── Multi-word weight+width combos (longest first) ──
        {"Ext Condensed", "Extra Condensada"},
        {"Ultra Light", "Ultrafina"},
        {"Ultra-Light", "Ultrafina"},
        {"Ultralight", "Ultrafina"},
        {"Extra Light", "Extra Fina"},
        {"Extra-Light", "Extra Fina"},
        {"ExtraLight", "Extra Fina"},
        {"Extralight", "Extra Fina"},
        {"Ultra Bold", "Ultra Negrito"},
        {"Ultra-Bold", "Ultra Negrito"},
        {"UltraBold", "Ultra Negrito"},
        {"Ultrabold", "Ultra Negrito"},
        {"Extra Bold", "Extra Negrito"},
        {"Extra-Bold", "Extra Negrito"},
        {"ExtraBold", "Extra Negrito"},
        {"Extrabold", "Extra Negrito"},
        {"Semi Bold", "Seminegrito"},
        {"Semi-Bold", "Seminegrito"},
        {"SemiBold", "Seminegrito"},
        {"Semibold", "Seminegrito"},
        {"Semi Light", "Semileve"},
        {"Semi-Light", "Semileve"},
        {"SemiLight", "Semileve"},
        {"Semilight", "Semileve"},
        {"Demi Bold", "Seminegrito"},
        {"Demi-Bold", "Seminegrito"},
        {"Demibold", "Seminegrito"},
        {"Semi Condensed", "Semicondensada"},
        {"Semi-Condensed", "Semicondensada"},
        {"SemiCondensed", "Semicondensada"},
        {"Semicondensed", "Semicondensada"},
        {"Extra Condensed", "Extra Condensada"},
        {"Extra-Condensed", "Extra Condensada"},
        {"Extracondensed", "Extra Condensada"},
        {"Ultra Condensed", "Ultra Condensada"},
        {"Ultra-Condensed", "Ultra Condensada"},
        {"Ultracondensed", "Ultra Condensada"},
        {"Semi Expanded", "Semiexpandida"},
        {"Semi-Expanded", "Semiexpandida"},
        {"SemiExpanded", "Semiexpandida"},
        {"Extra Expanded", "Extra Expandida"},
        {"Extra-Expanded", "Extra Expandida"},
        {"Extraexpanded", "Extra Expandida"},
        {"Ultra Expanded", "Ultra Expandida"},
        {"Ultra-Expanded", "Ultra Expandida"},
        {"Ultraexpanded", "Ultra Expandida"},
        {"Bold Outline", "Negrito Contorno"},
        // ── Abbreviations ──
        {"Cond", "Condensada"},
        {"Ext", "Extra"},
        // ── Single-word tokens ──
        {"Subheading", "Subtítulo"},
        {"Thin", "Muito Fina"},
        {"Hairline", "Ultrafina"},
        {"Hair", "Ultrafina"},
        {"Light", "Fina"},
        {"Medium", "Média"},
        {"Regular", "Normal"},
        {"Normal", "Normal"},
        {"Demi", "Seminegrito"},
        {"Bold", "Negrito"},
        {"Black", "Preto"},
        {"Heavy", "Pesada"},
        {"Book", "Livro"},
        {"Roman", "Romano"},
        {"Italic", "Itálico"},
        {"Oblique", "Oblíquo"},
        {"Slanted", "Inclinado"},
        {"Upright", "Vertical"},
        {"Outline", "Contorno"},
        {"Condensed", "Condensada"},
        {"Compressed", "Comprimida"},
        {"Narrow", "Estreita"},
        {"Expanded", "Expandida"},
        {"Extended", "Estendida"},
        {"Small", "Pequena"},
        {"Text", "Texto"},
        {"Display", "Exibição"},
        {"Banner", "Faixa"},
        {"Heading", "Título"},
        {"Subhead", "Subtítulo"},
        {"Caption", "Legenda"},
        {"Poster", "Poster"},
        {"Headline", "Manchete"},
    };

    QString result = trimmed;
    for (const auto &r : rules) {
        const QString pattern = QString::fromUtf8(r.en);
        const QString replacement = QString::fromUtf8(r.pt);
        result.replace(QRegularExpression(QStringLiteral("\\b") + QRegularExpression::escape(pattern) + QStringLiteral("\\b"),
                                           QRegularExpression::CaseInsensitiveOption),
                       replacement);
    }

    return result;
}

class TaskTitleFontDialog final : public QDialog {
public:
    explicit TaskTitleFontDialog(TaskFontProfile profile,
                                 const source::utils::TaskTitleFontSettings &initial_settings,
                                 const source::utils::TaskTitleFontSettings &default_settings,
                                 QWidget *parent = nullptr)
        : QDialog(parent)
        , profile_(profile)
        , defaults_(default_settings)
    {
        setModal(true);
        setMinimumWidth(480);

        auto *main_layout = new QVBoxLayout(this);
        auto *form_layout = new QGridLayout();
        form_layout->setColumnStretch(1, 1);

        family_label_ = new QLabel(this);
        family_combo_ = new QFontComboBox(this);

        style_label_ = new QLabel(this);
        style_combo_ = new QComboBox(this);

        size_label_ = new QLabel(this);
        size_spin_ = new QSpinBox(this);
        size_spin_->setRange(6, 72);
        size_spin_->setSuffix(QStringLiteral(" pt"));

        format_label_ = new QLabel(this);
        bold_check_ = new QCheckBox(this);
        italic_check_ = new QCheckBox(this);
        underline_check_ = new QCheckBox(this);
        strike_out_check_ = new QCheckBox(this);

        auto *format_layout = new QHBoxLayout();
        format_layout->addWidget(bold_check_);
        format_layout->addWidget(italic_check_);
        format_layout->addWidget(underline_check_);
        format_layout->addWidget(strike_out_check_);
        format_layout->addStretch();

        form_layout->addWidget(family_label_, 0, 0);
        form_layout->addWidget(family_combo_, 0, 1);
        form_layout->addWidget(style_label_, 1, 0);
        form_layout->addWidget(style_combo_, 1, 1);
        form_layout->addWidget(size_label_, 2, 0);
        form_layout->addWidget(size_spin_, 2, 1);
        form_layout->addWidget(format_label_, 3, 0);
        form_layout->addLayout(format_layout, 3, 1);
        main_layout->addLayout(form_layout);

        preview_title_label_ = new QLabel(this);
        main_layout->addWidget(preview_title_label_);

        auto *preview_frame = new QFrame(this);
        preview_frame->setFrameShape(QFrame::StyledPanel);
        preview_frame->setMinimumHeight(110);
        auto *preview_layout = new QVBoxLayout(preview_frame);
        preview_label_ = new QLabel(preview_frame);
        preview_label_->setAlignment(Qt::AlignCenter);
        preview_label_->setWordWrap(true);
        preview_layout->addWidget(preview_label_, 1);
        main_layout->addWidget(preview_frame);

        auto *button_layout = new QHBoxLayout();
        restore_button_ = new QPushButton(this);
        ok_button_ = new QPushButton(this);
        cancel_button_ = new QPushButton(this);
        button_layout->addWidget(restore_button_);
        button_layout->addStretch();
        button_layout->addWidget(ok_button_);
        button_layout->addWidget(cancel_button_);
        main_layout->addLayout(button_layout);

        connect(family_combo_, &QFontComboBox::currentFontChanged, this, [this](const QFont &) {
            update_style_options(QString(), true);
            update_preview();
        });
        connect(style_combo_, &QComboBox::currentTextChanged, this, [this](const QString &) { update_preview(); });
        connect(size_spin_, &QSpinBox::valueChanged, this, [this](int) { update_preview(); });
        connect(bold_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
        connect(italic_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
        connect(underline_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
        connect(strike_out_check_, &QCheckBox::toggled, this, [this](bool) { update_preview(); });
        connect(restore_button_, &QPushButton::clicked, this, [this]() {
            set_settings(defaults_);
            update_preview();
        });
        connect(ok_button_, &QPushButton::clicked, this, &QDialog::accept);
        connect(cancel_button_, &QPushButton::clicked, this, &QDialog::reject);

        set_settings(initial_settings);
        retranslate_ui();
        update_preview();
    }

    source::utils::TaskTitleFontSettings selected_settings() const
    {
        source::utils::TaskTitleFontSettings settings;
        settings.family = family_combo_ != nullptr ? family_combo_->currentFont().family().trimmed() : QString();
        if (style_combo_ != nullptr) {
            const QString raw_style = style_combo_->currentData(Qt::UserRole).toString().trimmed();
            settings.style = !raw_style.isEmpty() ? raw_style : style_combo_->currentText().trimmed();
        }
        settings.point_size = size_spin_ != nullptr ? size_spin_->value() : 0;
        settings.bold = bold_check_ != nullptr && bold_check_->isChecked();
        settings.italic = italic_check_ != nullptr && italic_check_->isChecked();
        settings.underline = underline_check_ != nullptr && underline_check_->isChecked();
        settings.strike_out = strike_out_check_ != nullptr && strike_out_check_->isChecked();
        return settings;
    }

protected:
    void changeEvent(QEvent *event) override
    {
        if (event != nullptr && event->type() == QEvent::LanguageChange) {
            retranslate_ui();
        }
        QDialog::changeEvent(event);
    }

private:
    void retranslate_ui()
    {
        setWindowTitle(task_font_profile_title(profile_));
        preview_text_ = task_font_profile_preview(profile_);
        if (family_label_ != nullptr) family_label_->setText(get_text(QStringLiteral("Tipo da fonte:")));
        if (style_label_ != nullptr) style_label_->setText(get_text(QStringLiteral("Estilo da fonte:")));
        if (size_label_ != nullptr) size_label_->setText(get_text(QStringLiteral("Tamanho:")));
        if (format_label_ != nullptr) format_label_->setText(get_text(QStringLiteral("Formatação:")));
        if (bold_check_ != nullptr) bold_check_->setText(get_text(QStringLiteral("Negrito")));
        if (italic_check_ != nullptr) italic_check_->setText(get_text(QStringLiteral("Itálico")));
        if (underline_check_ != nullptr) underline_check_->setText(get_text(QStringLiteral("Sublinhado")));
        if (strike_out_check_ != nullptr) strike_out_check_->setText(get_text(QStringLiteral("Tachado")));
        if (preview_title_label_ != nullptr) preview_title_label_->setText(get_text(QStringLiteral("Prévia")));
        if (restore_button_ != nullptr) restore_button_->setText(get_text(QStringLiteral("Restaurar")));
        if (ok_button_ != nullptr) ok_button_->setText(get_text(QStringLiteral("OK")));
        if (cancel_button_ != nullptr) cancel_button_->setText(get_text(QStringLiteral("Cancelar")));

        if (style_combo_ != nullptr) {
            const QSignalBlocker blocker(style_combo_);
            for (int i = 0; i < style_combo_->count(); ++i) {
                const QString raw_style = style_combo_->itemData(i, Qt::UserRole).toString();
                if (!raw_style.isEmpty()) {
                    style_combo_->setItemText(i, translate_font_style(raw_style));
                }
            }
        }
        update_preview();
    }

    void set_settings(const source::utils::TaskTitleFontSettings &settings)
    {
        const QFont base_font = QApplication::font();
        QFont selected_font = base_font;
        if (!settings.family.trimmed().isEmpty()) {
            selected_font.setFamily(settings.family.trimmed());
        }

        if (family_combo_ != nullptr) {
            const QSignalBlocker blocker(family_combo_);
            family_combo_->setCurrentFont(selected_font);
        }
        update_style_options(settings.style, false);

        if (size_spin_ != nullptr) {
            const QSignalBlocker blocker(size_spin_);
            const int point_size =
                settings.point_size >= 6 && settings.point_size <= 72 ? settings.point_size : base_font.pointSize();
            size_spin_->setValue(qBound(6, point_size > 0 ? point_size : 10, 72));
        }
        if (bold_check_ != nullptr) {
            const QSignalBlocker blocker(bold_check_);
            bold_check_->setChecked(settings.bold);
        }
        if (italic_check_ != nullptr) {
            const QSignalBlocker blocker(italic_check_);
            italic_check_->setChecked(settings.italic);
        }
        if (underline_check_ != nullptr) {
            const QSignalBlocker blocker(underline_check_);
            underline_check_->setChecked(settings.underline);
        }
        if (strike_out_check_ != nullptr) {
            const QSignalBlocker blocker(strike_out_check_);
            strike_out_check_->setChecked(settings.strike_out);
        }
    }

    void update_style_options(const QString &preferred_style, bool keep_current_style_when_empty)
    {
        if (family_combo_ == nullptr || style_combo_ == nullptr) {
            return;
        }

        const QString trimmed_preferred_style = preferred_style.trimmed();
        const QString current_raw_style = style_combo_->currentData(Qt::UserRole).toString().trimmed();
        const QString target_style = trimmed_preferred_style.isEmpty() && keep_current_style_when_empty
                                         ? (current_raw_style.isEmpty() ? style_combo_->currentText().trimmed() : current_raw_style)
                                         : trimmed_preferred_style;
        const QString family = family_combo_->currentFont().family();
        QFontDatabase database;
        QStringList styles = database.styles(family);
        if (styles.isEmpty()) {
            styles << QStringLiteral("Normal");
        }

        const QSignalBlocker blocker(style_combo_);
        style_combo_->clear();
        for (const QString &raw_style : styles) {
            style_combo_->addItem(translate_font_style(raw_style), raw_style);
        }

        int target_index = -1;
        if (!target_style.isEmpty()) {
            for (int i = 0; i < style_combo_->count(); ++i) {
                const QString item_raw = style_combo_->itemData(i, Qt::UserRole).toString();
                if (item_raw == target_style || style_combo_->itemText(i) == target_style) {
                    target_index = i;
                    break;
                }
            }
        }

        if (target_index >= 0) {
            style_combo_->setCurrentIndex(target_index);
        } else {
            int regular_index = -1;
            for (int i = 0; i < style_combo_->count(); ++i) {
                const QString item_raw = style_combo_->itemData(i, Qt::UserRole).toString();
                if (item_raw == QStringLiteral("Regular") || item_raw == QStringLiteral("Normal")) {
                    regular_index = i;
                    break;
                }
            }
            style_combo_->setCurrentIndex(regular_index >= 0 ? regular_index : 0);
        }
    }

    void update_preview()
    {
        if (preview_label_ == nullptr) {
            return;
        }

        const source::utils::TaskTitleFontSettings settings = selected_settings();
        QFont font = source::utils::task_title_font(QApplication::font(), settings);
        font.setUnderline(settings.underline);
        font.setStrikeOut(settings.strike_out);
        preview_label_->setFont(font);
        preview_label_->setText(preview_text_);
    }

    TaskFontProfile profile_;
    source::utils::TaskTitleFontSettings defaults_;
    QString preview_text_;
    QLabel *family_label_ = nullptr;
    QLabel *style_label_ = nullptr;
    QLabel *size_label_ = nullptr;
    QLabel *format_label_ = nullptr;
    QLabel *preview_title_label_ = nullptr;
    QFontComboBox *family_combo_ = nullptr;
    QComboBox *style_combo_ = nullptr;
    QSpinBox *size_spin_ = nullptr;
    QCheckBox *bold_check_ = nullptr;
    QCheckBox *italic_check_ = nullptr;
    QCheckBox *underline_check_ = nullptr;
    QCheckBox *strike_out_check_ = nullptr;
    QLabel *preview_label_ = nullptr;
    QPushButton *restore_button_ = nullptr;
    QPushButton *ok_button_ = nullptr;
    QPushButton *cancel_button_ = nullptr;
};

static void clear_profile_task_font_overrides(source::EisenhowerMatrixApp *app, TaskFontProfile profile)
{
    if (app == nullptr) {
        return;
    }

    if (profile == TaskFontProfile::General || profile == TaskFontProfile::LinkedFile) {
        const bool target_linked = (profile == TaskFontProfile::LinkedFile);
        for (QListWidget *list : eisenhower_task_lists(app)) {
            if (list == nullptr) continue;
            for (int i = 0; i < list->count(); ++i) {
                QListWidgetItem *item = list->item(i);
                if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) continue;
                QVariantMap data = item->data(Qt::UserRole).toMap();
                const bool has_linked = !data.value(QStringLiteral("file_path")).toString().trimmed().isEmpty() ||
                                         !data.value(QStringLiteral("arquivo_vinculado")).toString().trimmed().isEmpty();
                if (has_linked == target_linked) {
                    source::utils::clear_item_custom_font_settings(item);
                }
            }
        }
        app->save_tasks_now();
    } else if (profile == TaskFontProfile::PomodoroGeneral || profile == TaskFontProfile::PomodoroLinkedFile) {
        if (app->has_pomodoro_plugin()) {
            app->clear_pomodoro_custom_font_settings(profile == TaskFontProfile::PomodoroLinkedFile);
        }
    } else if (profile == TaskFontProfile::FeynmanGeneral || profile == TaskFontProfile::FeynmanLinkedFile) {
        if (app->has_feynman_plugin()) {
            app->clear_feynman_custom_font_settings(profile == TaskFontProfile::FeynmanLinkedFile);
        }
    } else if (profile == TaskFontProfile::MapaMentalGeneral || profile == TaskFontProfile::MapaMentalLinkedFile) {
        if (app->has_mapamental_plugin()) {
            app->clear_mapamental_custom_font_settings(profile == TaskFontProfile::MapaMentalLinkedFile);
        }
    }
}

static bool is_default_font_settings_profile(const source::utils::TaskTitleFontSettings &s, const source::utils::TaskTitleFontSettings &defaults)
{
    if (s.bold != defaults.bold || s.italic != defaults.italic ||
        s.underline != defaults.underline || s.strike_out != defaults.strike_out) {
        return false;
    }

    const QString default_family = !defaults.family.trimmed().isEmpty() ? defaults.family.trimmed() : QApplication::font().family().trimmed();
    if (!s.family.trimmed().isEmpty() && s.family.trimmed().compare(default_family, Qt::CaseInsensitive) != 0) {
        return false;
    }

    const int default_size = defaults.point_size > 0 ? defaults.point_size : qMax(6, QApplication::font().pointSize());
    if (s.point_size > 0 && s.point_size != default_size) {
        return false;
    }

    const QString st = s.style.trimmed();
    const QString def_st = defaults.style.trimmed();
    if (!def_st.isEmpty()) {
        if (!st.isEmpty() && st.compare(def_st, Qt::CaseInsensitive) != 0) {
            return false;
        }
    } else {
        if (!st.isEmpty() && st.compare(QStringLiteral("Normal"), Qt::CaseInsensitive) != 0 &&
            st.compare(QStringLiteral("Regular"), Qt::CaseInsensitive) != 0) {
            return false;
        }
    }

    return true;
}

static source::utils::TaskTitleFontSettings get_synced_task_font_profile(source::EisenhowerMatrixApp *app, TaskFontProfile profile)
{
    const source::utils::TaskTitleFontSettings global_settings = load_task_font_profile(profile);
    const source::utils::TaskTitleFontSettings defaults = task_font_profile_defaults(profile);
    if (app == nullptr || !is_default_font_settings_profile(global_settings, defaults)) {
        return global_settings;
    }

    QList<source::utils::TaskTitleFontSettings> active_settings_list;
    const bool is_linked = (profile == TaskFontProfile::LinkedFile || profile == TaskFontProfile::FeynmanLinkedFile ||
                            profile == TaskFontProfile::PomodoroLinkedFile || profile == TaskFontProfile::MapaMentalLinkedFile);

    if (profile == TaskFontProfile::General || profile == TaskFontProfile::LinkedFile) {
        for (QListWidget *list : eisenhower_task_lists(app)) {
            if (list == nullptr) continue;
            for (int i = 0; i < list->count(); ++i) {
                QListWidgetItem *item = list->item(i);
                if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) continue;
                QVariantMap data = item->data(Qt::UserRole).toMap();
                const bool has_linked = !data.value(QStringLiteral("file_path")).toString().trimmed().isEmpty() ||
                                         !data.value(QStringLiteral("arquivo_vinculado")).toString().trimmed().isEmpty();
                if (has_linked == is_linked) {
                    active_settings_list.append(source::utils::get_item_current_font_settings(item, is_linked, source::utils::PluginFontContext::Global));
                }
            }
        }
    } else {
        QVariantList maps;
        if (profile == TaskFontProfile::PomodoroGeneral || profile == TaskFontProfile::PomodoroLinkedFile) {
            if (app->has_pomodoro_plugin()) {
                maps = app->get_pomodoro_task_font_overrides(is_linked);
            }
        } else if (profile == TaskFontProfile::FeynmanGeneral || profile == TaskFontProfile::FeynmanLinkedFile) {
            if (app->has_feynman_plugin()) {
                maps = app->get_feynman_task_font_overrides(is_linked);
            }
        } else if (profile == TaskFontProfile::MapaMentalGeneral || profile == TaskFontProfile::MapaMentalLinkedFile) {
            if (app->has_mapamental_plugin()) {
                maps = app->get_mapamental_task_font_overrides(is_linked);
            }
        }

        for (const QVariant &var_map : maps) {
            const QVariantMap m = var_map.toMap();
            if (!m.isEmpty() && (m.contains(QStringLiteral("family")) || m.value(QStringLiteral("is_custom")).toBool())) {
                active_settings_list.append(source::utils::task_title_font_settings_from_map(m, global_settings));
            } else {
                active_settings_list.append(global_settings);
            }
        }
    }

    if (active_settings_list.isEmpty()) {
        return global_settings;
    }

    for (const source::utils::TaskTitleFontSettings &s : active_settings_list) {
        if (!is_default_font_settings_profile(s, defaults)) {
            return s;
        }
    }

    return defaults;
}

void configurar_fonte_tarefas(source::EisenhowerMatrixApp *app, TaskFontProfile profile)
{
    if (app == nullptr) {
        return;
    }

    TaskTitleFontDialog dialog(profile,
                               get_synced_task_font_profile(app, profile),
                               task_font_profile_defaults(profile),
                               app);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    if (!save_task_font_profile(profile, dialog.selected_settings())) {
        QMessageBox::warning(app,
                              get_text(QStringLiteral("Configurações de Fonte")),
                              get_text(QStringLiteral("Não foi possível salvar a configuração de fonte.")));
        return;
    }

    clear_profile_task_font_overrides(app, profile);
    app->refresh_task_title_styles();
    QMessageBox::information(app,
                             get_text(QStringLiteral("Configurações de Fonte")),
                             task_font_profile_success_message(profile));
}

QColor choose_task_color(source::EisenhowerMatrixApp *app, const QColor &initial_color)
{
    ColorDialogScopedTranslator color_dialog_translator;
    QCoreApplication::installTranslator(&color_dialog_translator);

    QColorDialog dialog(initial_color.isValid() ? initial_color : QColor(QStringLiteral("#1f6feb")), app);
    dialog.setOption(QColorDialog::DontUseNativeDialog, true);
    dialog.setOption(QColorDialog::ShowAlphaChannel, false);
    translate_color_dialog(&dialog, app);

    const int result = dialog.exec();
    const QColor selected_color = result == QDialog::Accepted ? dialog.selectedColor() : QColor();
    QCoreApplication::removeTranslator(&color_dialog_translator);
    return selected_color;
}

void colorir_tarefas_selecionadas(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    const QList<QListWidgetItem *> items = selected_task_items(app);
    const int plugin_selected_count = app->selected_plugin_task_count();
    if (items.isEmpty() && plugin_selected_count <= 0) {
        show_feedback_message(app,
                              QMessageBox::Information,
                              get_text(QStringLiteral("Colorir tarefas")),
                              get_text(QStringLiteral("Selecione uma ou mais tarefas antes de escolher a cor.")));
        return;
    }

    QColor initial_color = initial_color_for_tasks(items);
    if (items.isEmpty()) {
        const QColor plugin_color(app->selected_plugin_task_color().trimmed());
        if (plugin_color.isValid()) {
            initial_color = plugin_color;
        }
    }

    const QColor color = choose_task_color(app, initial_color);
    if (!color.isValid()) {
        return;
    }

    const QString color_name = color.name(QColor::HexRgb);
    int eisenhower_updated = 0;
    for (QListWidgetItem *item : items) {
        if (!is_real_task_item(item)) {
            continue;
        }

        QVariantMap data = item->data(Qt::UserRole).toMap();
        if (data.isEmpty()) {
            continue;
        }

        const QString current = data.value(QStringLiteral("cor_texto")).toString().trimmed();
        if (data.value(QStringLiteral("cor_texto_original")).toString().trimmed().isEmpty()
            && !current.isEmpty() && QColor(current).isValid()
            && current.compare(QStringLiteral("#000000"), Qt::CaseInsensitive) != 0
            && current.compare(QStringLiteral("#ffffff"), Qt::CaseInsensitive) != 0) {
            data.insert(QStringLiteral("cor_texto_original"), QColor(current).name(QColor::HexRgb));
        }

        data.insert(QStringLiteral("cor_texto"), color_name);
        data.insert(QStringLiteral("task_color"), color_name);
        data.insert(QStringLiteral("cor_texto_customizada"), color_name);
        item->setData(Qt::UserRole, data);
        item->setForeground(QBrush(color));
        ++eisenhower_updated;
    }

    const int plugin_updated = app->apply_color_to_selected_plugin_tasks(color_name);
    const int updated = eisenhower_updated + plugin_updated;

    if (updated <= 0) {
        return;
    }

    if (eisenhower_updated > 0) {
        source::InterfaceCore::incore_11_atualizar_itens_tarefas::atualizar_itens_tarefas(app);
        if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
            app->calendar_pane->calendar_panel->update_task_list();
        }
        app->save_tasks_now();
    }

    show_feedback_message(app,
                          QMessageBox::Information,
                          get_text(QStringLiteral("Colorir tarefas")),
                          updated == 1 ? get_text(QStringLiteral("Cor aplicada à tarefa selecionada."))
                                       : get_text(QStringLiteral("Cor aplicada às tarefas selecionadas.")));
}

}  // namespace

void criar_menu_configuracoes(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    log_menu_stage(QStringLiteral("início"));
    QMenuBar *menu_bar = app->menu_bar != nullptr ? app->menu_bar : app->menuBar();
    log_menu_stage(QStringLiteral("limpando barra de menu"));
    menu_bar->clear();
    app->menu_bar = menu_bar;

    log_menu_stage(QStringLiteral("criando menu Arquivo"));
    auto *arquivo_menu = create_menu(get_text(QStringLiteral("Arquivo")), app);
    QAction *acao_novo = arquivo_menu->addAction(get_text(QStringLiteral("Novo")));
    acao_novo->setShortcut(QKeySequence::New);
    QObject::connect(acao_novo, &QAction::triggered, app, [app]() { app->nova_sessao(); });

    QAction *acao_abrir = arquivo_menu->addAction(get_text(QStringLiteral("Abrir")));
    acao_abrir->setShortcut(QKeySequence::Open);
    QObject::connect(acao_abrir, &QAction::triggered, app, [app]() { app->abrir_arquivo(); });

    QAction *acao_salvar = arquivo_menu->addAction(get_text(QStringLiteral("Salvar")));
    acao_salvar->setShortcut(QKeySequence::Save);
    QObject::connect(acao_salvar, &QAction::triggered, app, [app]() { app->salvar(); });

    QAction *acao_salvar_como = arquivo_menu->addAction(get_text(QStringLiteral("Salvar como")));
    acao_salvar_como->setShortcut(QKeySequence::SaveAs);
    QObject::connect(acao_salvar_como, &QAction::triggered, app, [app]() { app->salvar_como(); });

    QAction *acao_limpar = arquivo_menu->addAction(get_text(QStringLiteral("Limpar")));
    acao_limpar->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+Del")));
    QObject::connect(acao_limpar, &QAction::triggered, app, [app]() { app->limpar_tudo(); });

    arquivo_menu->addSeparator();
    QAction *acao_sair = arquivo_menu->addAction(get_text(QStringLiteral("Sair")));
    acao_sair->setShortcut(QKeySequence(QStringLiteral("Ctrl+Q")));
    QObject::connect(acao_sair, &QAction::triggered, app, [app]() { app->sair_app(); });
    menu_bar->addMenu(arquivo_menu);

    log_menu_stage(QStringLiteral("criando menu Configurações"));
    auto *config_menu = create_menu(get_text(QStringLiteral("Configurações")), app);

    log_menu_stage(QStringLiteral("criando menu Abrir"));
    auto *abrir_menu = create_menu(get_text(QStringLiteral("Abrir")), app);

    QAction *acao_abrir_eisenhower = abrir_menu->addAction(get_text(QStringLiteral("Eisenhower")));
    acao_abrir_eisenhower->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+I")));
    QObject::connect(acao_abrir_eisenhower, &QAction::triggered, app, [app]() { app->open_eisenhower(); });

    if (app->has_pomodoro_plugin()) {
        QAction *acao_abrir_pomodoro = abrir_menu->addAction(app->pomodoro_display_name());
        acao_abrir_pomodoro->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+P")));
        QObject::connect(acao_abrir_pomodoro, &QAction::triggered, app, [app]() { app->open_pomodoro(); });
    }

    if (app->has_feynman_plugin()) {
        QAction *acao_abrir_feynman = abrir_menu->addAction(app->feynman_display_name());
        acao_abrir_feynman->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+F")));
        QObject::connect(acao_abrir_feynman, &QAction::triggered, app, [app]() { app->open_feynman(); });
    }

    if (app->has_mapamental_plugin()) {
        QAction *acao_abrir_mapamental = abrir_menu->addAction(app->mapamental_display_name());
        acao_abrir_mapamental->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+N")));
        QObject::connect(acao_abrir_mapamental, &QAction::triggered, app, [app]() { app->open_mapamental(); });
    }

    if (app->has_audiolivro_plugin()) {
        QAction *acao_abrir_audiolivro = abrir_menu->addAction(app->audiolivro_display_name());
        acao_abrir_audiolivro->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+L")));
        QObject::connect(acao_abrir_audiolivro, &QAction::triggered, app, [app]() { app->open_audiolivro(); });
    }
    add_submenu(config_menu, abrir_menu);

    log_menu_stage(QStringLiteral("criando menu Idioma"));
    auto *idioma_menu = create_menu(get_text(QStringLiteral("Idioma")), app);
    if (app->gerenciador_traducao != nullptr) {
        const auto idiomas = app->gerenciador_traducao->idiomas_disponiveis();
        for (auto it = idiomas.constBegin(); it != idiomas.constEnd(); ++it) {
            QAction *acao_idioma = idioma_menu->addAction(it.value());
            acao_idioma->setCheckable(true);
            acao_idioma->setChecked(app->gerenciador_traducao->obter_idioma_atual() == it.key());
            QObject::connect(acao_idioma, &QAction::triggered, app, [app, code = it.key()]() { app->definir_idioma(code); });
        }
    }

    log_menu_stage(QStringLiteral("criando seção Configurações de Fonte"));
    auto *font_settings_menu = create_menu(get_text(QStringLiteral("Configurações de Fonte")), app);

    auto *eisenhower_font_menu = create_menu(get_text(QStringLiteral("Eisenhower")), app);
    QAction *acao_eisenhower_tarefas = eisenhower_font_menu->addAction(get_text(QStringLiteral("Tarefas em Geral")));
    set_menu_path(acao_eisenhower_tarefas, general_task_font_menu_path());
    QObject::connect(acao_eisenhower_tarefas, &QAction::triggered, app, [app]() { configurar_fonte_tarefas(app, TaskFontProfile::General); });

    QAction *acao_eisenhower_vinculado = eisenhower_font_menu->addAction(get_text(QStringLiteral("Arquivo Vinculado")));
    set_menu_path(acao_eisenhower_vinculado, linked_file_font_menu_path());
    QObject::connect(acao_eisenhower_vinculado, &QAction::triggered, app, [app]() { configurar_fonte_tarefas(app, TaskFontProfile::LinkedFile); });
    add_submenu(font_settings_menu, eisenhower_font_menu);

    if (app->has_pomodoro_plugin()) {
        auto *pomodoro_font_menu = create_menu(app->pomodoro_display_name(), app);
        QAction *acao_pomodoro_tarefas = pomodoro_font_menu->addAction(get_text(QStringLiteral("Tarefas em Geral")));
        QObject::connect(acao_pomodoro_tarefas, &QAction::triggered, app, [app]() { configurar_fonte_tarefas(app, TaskFontProfile::PomodoroGeneral); });

        QAction *acao_pomodoro_vinculado = pomodoro_font_menu->addAction(get_text(QStringLiteral("Arquivo Vinculado")));
        QObject::connect(acao_pomodoro_vinculado, &QAction::triggered, app, [app]() { configurar_fonte_tarefas(app, TaskFontProfile::PomodoroLinkedFile); });
        add_submenu(font_settings_menu, pomodoro_font_menu);
    }

    if (app->has_feynman_plugin()) {
        auto *feynman_font_menu = create_menu(app->feynman_display_name(), app);
        QAction *acao_feynman_tarefas = feynman_font_menu->addAction(get_text(QStringLiteral("Tarefas em Geral")));
        QObject::connect(acao_feynman_tarefas, &QAction::triggered, app, [app]() { configurar_fonte_tarefas(app, TaskFontProfile::FeynmanGeneral); });

        QAction *acao_feynman_vinculado = feynman_font_menu->addAction(get_text(QStringLiteral("Arquivo Vinculado")));
        QObject::connect(acao_feynman_vinculado, &QAction::triggered, app, [app]() { configurar_fonte_tarefas(app, TaskFontProfile::FeynmanLinkedFile); });
        add_submenu(font_settings_menu, feynman_font_menu);
    }

    if (app->has_mapamental_plugin()) {
        auto *mapamental_font_menu = create_menu(app->mapamental_display_name(), app);
        QAction *acao_mapamental_tarefas = mapamental_font_menu->addAction(get_text(QStringLiteral("Tarefas em Geral")));
        QObject::connect(acao_mapamental_tarefas, &QAction::triggered, app, [app]() { configurar_fonte_tarefas(app, TaskFontProfile::MapaMentalGeneral); });

        QAction *acao_mapamental_vinculado = mapamental_font_menu->addAction(get_text(QStringLiteral("Arquivo Vinculado")));
        QObject::connect(acao_mapamental_vinculado, &QAction::triggered, app, [app]() { configurar_fonte_tarefas(app, TaskFontProfile::MapaMentalLinkedFile); });
        add_submenu(font_settings_menu, mapamental_font_menu);
    }

    if (app->has_audiolivro_plugin()) {
        auto *audiolivro_font_menu = create_menu(app->audiolivro_display_name(), app);
        QAction *acao_texto_audiolivro = audiolivro_font_menu->addAction(get_text(QStringLiteral("Texto Audiolivro")));
        set_menu_path(acao_texto_audiolivro, audiolivro_font_menu_path());
        QObject::connect(acao_texto_audiolivro, &QAction::triggered, app, [app]() { app->open_audiolivro_font_settings(app); });
        add_submenu(font_settings_menu, audiolivro_font_menu);
    }

    QAction *acao_colorir_tarefas = font_settings_menu->addAction(get_text(QStringLiteral("Colorir tarefas")));
    set_menu_path(acao_colorir_tarefas, color_tasks_menu_path());
    QObject::connect(acao_colorir_tarefas, &QAction::triggered, app, [app]() { colorir_tarefas_selecionadas(app); });

    log_menu_stage(QStringLiteral("criando seção Layout de Tarefas"));
    auto *layout_menu = create_menu(get_text(QStringLiteral("Layout de Tarefas")), app);
    auto *interface_menu = create_menu(interface_menu_text(), app);
    auto *eisenhower_interface_menu = create_menu(eisenhower_matrix_menu_text(), app);
    auto *layout_group = new QActionGroup(eisenhower_interface_menu);
    layout_group->setExclusive(true);

    auto *eisenhower_tabs_menu = create_menu(get_text(QStringLiteral("Layout em Abas")), app);

    QAction *acao_layout_abas_vertical = eisenhower_tabs_menu->addAction(get_text(QStringLiteral("Vertical")));
    acao_layout_abas_vertical->setCheckable(true);
    acao_layout_abas_vertical->setActionGroup(layout_group);
    acao_layout_abas_vertical->setChecked(app->current_task_layout() == source::EisenhowerMatrixApp::TaskLayoutMode::Tabs);

    QAction *acao_layout_abas_horizontal = eisenhower_tabs_menu->addAction(get_text(QStringLiteral("Horizontal")));
    acao_layout_abas_horizontal->setCheckable(true);
    acao_layout_abas_horizontal->setActionGroup(layout_group);
    acao_layout_abas_horizontal->setChecked(app->current_task_layout() == source::EisenhowerMatrixApp::TaskLayoutMode::TabsHorizontal);

    add_submenu(eisenhower_interface_menu, eisenhower_tabs_menu);

    QAction *acao_layout_quadrantes =
        eisenhower_interface_menu->addAction(get_text(QStringLiteral("Layout em Quadrantes")));
    acao_layout_quadrantes->setCheckable(true);
    acao_layout_quadrantes->setActionGroup(layout_group);
    acao_layout_quadrantes->setChecked(app->current_task_layout()
                                       == source::EisenhowerMatrixApp::TaskLayoutMode::Quadrants);

    QAction *acao_layout_matriz_2x2 = eisenhower_interface_menu->addAction(get_text(QStringLiteral("Layout Matriz 2x2")));
    acao_layout_matriz_2x2->setCheckable(true);
    acao_layout_matriz_2x2->setActionGroup(layout_group);
    acao_layout_matriz_2x2->setChecked(app->current_task_layout()
                                       == source::EisenhowerMatrixApp::TaskLayoutMode::Matrix2x2);

    eisenhower_interface_menu->addSeparator();

    auto add_reserved_space_toggle = [app](StickyMenu *parent_menu, const QString &title, bool enabled, auto setter) {
        auto *option_menu = create_menu(title, app);
        auto *group = new QActionGroup(option_menu);
        group->setExclusive(true);

        QAction *action_enable = option_menu->addAction(get_text(QStringLiteral("Ativar")));
        action_enable->setCheckable(true);
        action_enable->setActionGroup(group);

        QAction *action_disable = option_menu->addAction(get_text(QStringLiteral("Desativar")));
        action_disable->setCheckable(true);
        action_disable->setActionGroup(group);

        action_enable->setChecked(enabled);
        action_disable->setChecked(!enabled);

        QObject::connect(action_enable, &QAction::triggered, app, [app, setter]() { (app->*setter)(true); });
        QObject::connect(action_disable, &QAction::triggered, app, [app, setter]() { (app->*setter)(false); });

        add_submenu(parent_menu, option_menu);
    };

    add_reserved_space_toggle(eisenhower_interface_menu,
                              get_text(QStringLiteral("Título")),
                              app->reserved_space_eisenhower_enabled(),
                              &source::EisenhowerMatrixApp::set_reserved_space_eisenhower_enabled);
    add_reserved_space_toggle(eisenhower_interface_menu,
                              get_text(QStringLiteral("Marca d'Água")),
                              app->reserved_watermark_eisenhower_enabled(),
                              &source::EisenhowerMatrixApp::set_reserved_watermark_eisenhower_enabled);

    add_submenu(interface_menu, eisenhower_interface_menu);

    if (app->has_pomodoro_plugin()) {
        auto *pomodoro_interface_menu = create_menu(app->pomodoro_display_name(), app);
        auto *pomodoro_interface_group = new QActionGroup(pomodoro_interface_menu);
        pomodoro_interface_group->setExclusive(true);

        auto *pomodoro_simple_menu = create_menu(pomodoro_interface_simple_text(), app);
        QAction *acao_simples_abas = pomodoro_simple_menu->addAction(pomodoro_interface_tabs_text());
        acao_simples_abas->setCheckable(true);
        acao_simples_abas->setActionGroup(pomodoro_interface_group);
        acao_simples_abas->setData(QStringLiteral("simple:tabs"));
        set_menu_path(acao_simples_abas, pomodoro_interface_simple_tabs_menu_path());

        QAction *acao_simples_vertical = pomodoro_simple_menu->addAction(pomodoro_interface_vertical_text());
        acao_simples_vertical->setCheckable(true);
        acao_simples_vertical->setActionGroup(pomodoro_interface_group);
        acao_simples_vertical->setData(QStringLiteral("simple:vertical"));
        set_menu_path(acao_simples_vertical, pomodoro_interface_simple_vertical_menu_path());

        QAction *acao_simples_horizontal = pomodoro_simple_menu->addAction(pomodoro_interface_horizontal_text());
        acao_simples_horizontal->setCheckable(true);
        acao_simples_horizontal->setActionGroup(pomodoro_interface_group);
        acao_simples_horizontal->setData(QStringLiteral("simple:horizontal"));
        set_menu_path(acao_simples_horizontal, pomodoro_interface_simple_horizontal_menu_path());

        add_submenu(pomodoro_interface_menu, pomodoro_simple_menu);

        auto *pomodoro_robust_menu = create_menu(pomodoro_interface_robust_text(), app);
        QAction *acao_robusta_abas = pomodoro_robust_menu->addAction(pomodoro_interface_tabs_text());
        acao_robusta_abas->setCheckable(true);
        acao_robusta_abas->setActionGroup(pomodoro_interface_group);
        acao_robusta_abas->setData(QStringLiteral("robust:tabs"));
        set_menu_path(acao_robusta_abas, pomodoro_interface_robust_tabs_menu_path());

        QAction *acao_robusta_vertical = pomodoro_robust_menu->addAction(pomodoro_interface_vertical_text());
        acao_robusta_vertical->setCheckable(true);
        acao_robusta_vertical->setActionGroup(pomodoro_interface_group);
        acao_robusta_vertical->setData(QStringLiteral("robust:vertical"));
        set_menu_path(acao_robusta_vertical, pomodoro_interface_robust_vertical_menu_path());

        QAction *acao_robusta_horizontal = pomodoro_robust_menu->addAction(pomodoro_interface_horizontal_text());
        acao_robusta_horizontal->setCheckable(true);
        acao_robusta_horizontal->setActionGroup(pomodoro_interface_group);
        acao_robusta_horizontal->setData(QStringLiteral("robust:horizontal"));
        set_menu_path(acao_robusta_horizontal, pomodoro_interface_robust_horizontal_menu_path());

        add_submenu(pomodoro_interface_menu, pomodoro_robust_menu);

        const auto apply_pomodoro_layout =
            [app](const QString &mode_key, const QString &mode_text, const QString &panel_text) {
                if (app->set_pomodoro_ui_layout_mode_key(mode_key)) {
                    app->open_pomodoro();
                    show_feedback_message(
                        app,
                        QMessageBox::Information,
                        get_text(QStringLiteral("Sucesso")),
                        pomodoro_interface_layout_applied_message(QStringLiteral("%1 / %2").arg(mode_text, panel_text)));
                }
            };

        QObject::connect(acao_simples_abas, &QAction::triggered, app, [apply_pomodoro_layout]() {
            apply_pomodoro_layout(QStringLiteral("simple:tabs"), pomodoro_interface_simple_text(), pomodoro_interface_tabs_text());
        });
        QObject::connect(acao_simples_vertical, &QAction::triggered, app, [apply_pomodoro_layout]() {
            apply_pomodoro_layout(QStringLiteral("simple:vertical"),
                                  pomodoro_interface_simple_text(),
                                  pomodoro_interface_vertical_text());
        });
        QObject::connect(acao_simples_horizontal, &QAction::triggered, app, [apply_pomodoro_layout]() {
            apply_pomodoro_layout(QStringLiteral("simple:horizontal"),
                                  pomodoro_interface_simple_text(),
                                  pomodoro_interface_horizontal_text());
        });

        QObject::connect(acao_robusta_abas, &QAction::triggered, app, [apply_pomodoro_layout]() {
            apply_pomodoro_layout(QStringLiteral("robust:tabs"), pomodoro_interface_robust_text(), pomodoro_interface_tabs_text());
        });
        QObject::connect(acao_robusta_vertical, &QAction::triggered, app, [apply_pomodoro_layout]() {
            apply_pomodoro_layout(QStringLiteral("robust:vertical"),
                                  pomodoro_interface_robust_text(),
                                  pomodoro_interface_vertical_text());
        });
        QObject::connect(acao_robusta_horizontal, &QAction::triggered, app, [apply_pomodoro_layout]() {
            apply_pomodoro_layout(QStringLiteral("robust:horizontal"),
                                  pomodoro_interface_robust_text(),
                                  pomodoro_interface_horizontal_text());
        });

        QObject::connect(pomodoro_interface_menu,
                         &QMenu::aboutToShow,
                         app,
                         [app,
                          pomodoro_simple_menu,
                          acao_simples_abas,
                          acao_simples_vertical,
                          acao_simples_horizontal,
                          pomodoro_robust_menu,
                          acao_robusta_abas,
                          acao_robusta_vertical,
                          acao_robusta_horizontal]() {
                             retranslate_pomodoro_interface_actions(pomodoro_simple_menu,
                                                                   acao_simples_abas,
                                                                   acao_simples_vertical,
                                                                   acao_simples_horizontal,
                                                                   pomodoro_robust_menu,
                                                                   acao_robusta_abas,
                                                                   acao_robusta_vertical,
                                                                   acao_robusta_horizontal);
                             const QString current_layout = app->pomodoro_ui_layout_mode_key();
                             const QStringList parts = current_layout.split(QLatin1Char(':'), Qt::SkipEmptyParts);
                             const QString mode = parts.value(0, QStringLiteral("simple"));
                             const QString panel = parts.value(1, QStringLiteral("tabs"));

                             acao_simples_abas->setChecked(mode == QStringLiteral("simple")
                                                           && panel == QStringLiteral("tabs"));
                             acao_simples_vertical->setChecked(mode == QStringLiteral("simple")
                                                               && panel == QStringLiteral("vertical"));
                             acao_simples_horizontal->setChecked(mode == QStringLiteral("simple")
                                                                 && panel == QStringLiteral("horizontal"));

                             acao_robusta_abas->setChecked(mode == QStringLiteral("robust")
                                                           && panel == QStringLiteral("tabs"));
                             acao_robusta_vertical->setChecked(mode == QStringLiteral("robust")
                                                               && panel == QStringLiteral("vertical"));
                             acao_robusta_horizontal->setChecked(mode == QStringLiteral("robust")
                                                                 && panel == QStringLiteral("horizontal"));
                         });

        pomodoro_interface_menu->addSeparator();
        add_reserved_space_toggle(pomodoro_interface_menu,
                                  get_text(QStringLiteral("Título")),
                                  app->reserved_space_pomodoro_enabled(),
                                  &source::EisenhowerMatrixApp::set_reserved_space_pomodoro_enabled);
        add_reserved_space_toggle(pomodoro_interface_menu,
                                  get_text(QStringLiteral("Marca d'Água")),
                                  app->reserved_watermark_pomodoro_enabled(),
                                  &source::EisenhowerMatrixApp::set_reserved_watermark_pomodoro_enabled);

        add_submenu(interface_menu, pomodoro_interface_menu);
    }

    if (app->has_feynman_plugin()) {
        auto *feynman_interface_menu = create_menu(app->feynman_display_name(), app);
        auto *feynman_interface_group = new QActionGroup(feynman_interface_menu);
        feynman_interface_group->setExclusive(true);

        QAction *acao_interface_abas = feynman_interface_menu->addAction(feynman_interface_tabs_text());
        acao_interface_abas->setCheckable(true);
        acao_interface_abas->setActionGroup(feynman_interface_group);
        acao_interface_abas->setData(QStringLiteral("tabs"));
        acao_interface_abas->setChecked(app->feynman_ui_layout_mode_key() == QStringLiteral("tabs"));
        set_menu_path(acao_interface_abas, feynman_interface_tabs_menu_path());

        QAction *acao_interface_vertical = feynman_interface_menu->addAction(feynman_interface_vertical_text());
        acao_interface_vertical->setCheckable(true);
        acao_interface_vertical->setActionGroup(feynman_interface_group);
        acao_interface_vertical->setData(QStringLiteral("vertical"));
        acao_interface_vertical->setChecked(app->feynman_ui_layout_mode_key() == QStringLiteral("vertical"));
        set_menu_path(acao_interface_vertical, feynman_interface_vertical_menu_path());

        QAction *acao_interface_horizontal = feynman_interface_menu->addAction(feynman_interface_horizontal_text());
        acao_interface_horizontal->setCheckable(true);
        acao_interface_horizontal->setActionGroup(feynman_interface_group);
        acao_interface_horizontal->setData(QStringLiteral("horizontal"));
        acao_interface_horizontal->setChecked(app->feynman_ui_layout_mode_key() == QStringLiteral("horizontal"));
        set_menu_path(acao_interface_horizontal, feynman_interface_horizontal_menu_path());

        const auto apply_feynman_layout = [app](const QString &mode_key, const QString &layout_name) {
            if (app->set_feynman_ui_layout_mode_key(mode_key)) {
                app->open_feynman();
                show_feedback_message(app,
                                      QMessageBox::Information,
                                      get_text(QStringLiteral("Sucesso")),
                                      feynman_interface_layout_applied_message(layout_name));
            }
        };

        QObject::connect(acao_interface_abas, &QAction::triggered, app, [apply_feynman_layout]() {
            apply_feynman_layout(QStringLiteral("tabs"), feynman_interface_tabs_text());
        });
        QObject::connect(acao_interface_vertical, &QAction::triggered, app, [apply_feynman_layout]() {
            apply_feynman_layout(QStringLiteral("vertical"), feynman_interface_vertical_text());
        });
        QObject::connect(acao_interface_horizontal, &QAction::triggered, app, [apply_feynman_layout]() {
            apply_feynman_layout(QStringLiteral("horizontal"), feynman_interface_horizontal_text());
        });
        QObject::connect(feynman_interface_menu,
                         &QMenu::aboutToShow,
                         app,
                         [app, acao_interface_abas, acao_interface_vertical, acao_interface_horizontal]() {
                             retranslate_feynman_interface_actions(
                                 acao_interface_abas, acao_interface_vertical, acao_interface_horizontal);
                             const QString current_layout = app->feynman_ui_layout_mode_key();
                             acao_interface_abas->setChecked(current_layout == QStringLiteral("tabs"));
                             acao_interface_vertical->setChecked(current_layout == QStringLiteral("vertical"));
                             acao_interface_horizontal->setChecked(current_layout == QStringLiteral("horizontal"));
                         });

        feynman_interface_menu->addSeparator();
        add_reserved_space_toggle(feynman_interface_menu,
                                  get_text(QStringLiteral("Título")),
                                  app->reserved_space_feynman_enabled(),
                                  &source::EisenhowerMatrixApp::set_reserved_space_feynman_enabled);
        add_reserved_space_toggle(feynman_interface_menu,
                                  get_text(QStringLiteral("Marca d'Água")),
                                  app->reserved_watermark_feynman_enabled(),
                                  &source::EisenhowerMatrixApp::set_reserved_watermark_feynman_enabled);

        add_submenu(interface_menu, feynman_interface_menu);
    }

    if (app->has_mapamental_plugin()) {
        auto *mapamental_interface_menu = create_menu(app->mapamental_display_name(), app);
        auto *layout_mapa_group = new QActionGroup(mapamental_interface_menu);
        layout_mapa_group->setExclusive(true);

        const auto add_layout_action = [app, mapamental_interface_menu, layout_mapa_group](const QString &title,
                                                                                  const QString &mode_key) {
            QAction *action = mapamental_interface_menu->addAction(title);
            action->setCheckable(true);
            action->setActionGroup(layout_mapa_group);
            action->setData(mode_key);
            action->setChecked(app->mapamental_layout_mode_key() == mode_key);
            QObject::connect(action, &QAction::triggered, app, [app, mode_key, title]() {
                app->set_mapamental_layout_mode_key(mode_key);
                app->open_mapamental();
                show_feedback_message(app,
                                      QMessageBox::Information,
                                      get_text(QStringLiteral("Sucesso")),
                                      mapamental_interface_layout_applied_message(title));
            });
            return action;
        };

        QAction *acao_layout_vertical =
            add_layout_action(get_text(QStringLiteral("Vertical")), QStringLiteral("vertical"));
        QAction *acao_layout_horizontal =
            add_layout_action(get_text(QStringLiteral("Horizontal")), QStringLiteral("horizontal"));
        QAction *acao_layout_radial =
            add_layout_action(get_text(QStringLiteral("Radial")), QStringLiteral("radial"));
        QAction *acao_layout_arvore =
            add_layout_action(get_text(QStringLiteral("Árvore")), QStringLiteral("tree"));

        QObject::connect(mapamental_interface_menu,
                         &QMenu::aboutToShow,
                         app,
                         [app,
                          mapamental_interface_menu,
                          acao_layout_vertical,
                          acao_layout_horizontal,
                          acao_layout_radial,
                          acao_layout_arvore]() {
                             mapamental_interface_menu->setTitle(app->mapamental_display_name());
                             const QString current_mode = app->mapamental_layout_mode_key();
                             acao_layout_vertical->setChecked(current_mode == QStringLiteral("vertical"));
                             acao_layout_horizontal->setChecked(current_mode == QStringLiteral("horizontal"));
                             acao_layout_radial->setChecked(current_mode == QStringLiteral("radial"));
                             acao_layout_arvore->setChecked(current_mode == QStringLiteral("tree"));
                         });

        add_submenu(interface_menu, mapamental_interface_menu);
    }

    auto *abas_interface_menu = create_menu(get_text(QStringLiteral("Abas da Interface")), app);
    auto *abas_interface_group = new QActionGroup(abas_interface_menu);
    abas_interface_group->setExclusive(true);

    QAction *acao_bloquear_abas = abas_interface_menu->addAction(get_text(QStringLiteral("Bloquear ajuste")));
    acao_bloquear_abas->setCheckable(true);
    acao_bloquear_abas->setActionGroup(abas_interface_group);
    acao_bloquear_abas->setChecked(app->module_tab_adjustment_locked());

    QAction *acao_desbloquear_abas = abas_interface_menu->addAction(get_text(QStringLiteral("Desbloquear ajuste")));
    acao_desbloquear_abas->setCheckable(true);
    acao_desbloquear_abas->setActionGroup(abas_interface_group);
    acao_desbloquear_abas->setChecked(!app->module_tab_adjustment_locked());

    abas_interface_menu->addSeparator();
    QAction *acao_restaurar_abas = abas_interface_menu->addAction(get_text(QStringLiteral("Restaurar")));

    QObject::connect(acao_bloquear_abas, &QAction::triggered, app, [app, acao_bloquear_abas, acao_desbloquear_abas]() {
        app->set_module_tab_adjustment_locked(true);
        acao_bloquear_abas->setChecked(true);
        acao_desbloquear_abas->setChecked(false);
    });
    QObject::connect(acao_desbloquear_abas, &QAction::triggered, app, [app, acao_bloquear_abas, acao_desbloquear_abas]() {
        app->set_module_tab_adjustment_locked(false);
        acao_bloquear_abas->setChecked(false);
        acao_desbloquear_abas->setChecked(true);
    });
    QObject::connect(acao_restaurar_abas, &QAction::triggered, app, [app]() {
        app->restore_module_tab_order();
    });
    QObject::connect(abas_interface_menu, &QMenu::aboutToShow, app, [app, acao_bloquear_abas, acao_desbloquear_abas]() {
        const bool locked = app->module_tab_adjustment_locked();
        acao_bloquear_abas->setChecked(locked);
        acao_desbloquear_abas->setChecked(!locked);
    });

    auto *mostrar_abas_interface_menu = create_menu(get_text(QStringLiteral("Mostrar Abas de Interface")), app);
    const auto add_module_tab_visibility_action =
        [app, mostrar_abas_interface_menu](const QString &key, const QString &title) {
            QAction *action = mostrar_abas_interface_menu->addAction(title);
            action->setCheckable(true);
            action->setEnabled(app->module_tab_available(key));
            action->setChecked(app->module_tab_visible(key));
            QObject::connect(action, &QAction::triggered, app, [app, action, key](bool checked) {
                if (!app->set_module_tab_visible(key, checked)) {
                    action->setChecked(app->module_tab_visible(key));
                }
            });
            return action;
        };

    QAction *acao_aba_eisenhower =
        add_module_tab_visibility_action(QStringLiteral("eisenhower"), get_text(QStringLiteral("Eisenhower")));
    QAction *acao_aba_pomodoro =
        add_module_tab_visibility_action(QStringLiteral("pomodoro"), get_text(QStringLiteral("Pomodoro")));
    QAction *acao_aba_feynman =
        add_module_tab_visibility_action(QStringLiteral("feynman"), get_text(QStringLiteral("Feynman")));
    QAction *acao_aba_mapamental =
        add_module_tab_visibility_action(QStringLiteral("mapamental"), get_text(QStringLiteral("Mapa Mental")));
    QAction *acao_aba_audiolivro =
        add_module_tab_visibility_action(QStringLiteral("audiolivro"), app->audiolivro_display_name());

    const auto sync_module_tab_visibility_action = [app](QAction *action, const QString &key) {
        if (action == nullptr) {
            return;
        }
        if (key == QStringLiteral("audiolivro")) {
            action->setText(app->audiolivro_display_name());
        }
        action->setEnabled(app->module_tab_available(key));
        action->setChecked(app->module_tab_visible(key));
    };

    QObject::connect(mostrar_abas_interface_menu,
                     &QMenu::aboutToShow,
                     app,
                     [sync_module_tab_visibility_action,
                      acao_aba_eisenhower,
                      acao_aba_pomodoro,
                      acao_aba_feynman,
                      acao_aba_mapamental,
                      acao_aba_audiolivro]() {
                         sync_module_tab_visibility_action(acao_aba_eisenhower, QStringLiteral("eisenhower"));
                         sync_module_tab_visibility_action(acao_aba_pomodoro, QStringLiteral("pomodoro"));
                         sync_module_tab_visibility_action(acao_aba_feynman, QStringLiteral("feynman"));
                         sync_module_tab_visibility_action(acao_aba_mapamental, QStringLiteral("mapamental"));
                         sync_module_tab_visibility_action(acao_aba_audiolivro, QStringLiteral("audiolivro"));
                     });

    const bool has_plugin_panels = app->has_feynman_plugin() || app->has_pomodoro_plugin();
    auto *panel_menu = create_menu(
        has_plugin_panels ? get_text(QStringLiteral("Mostrar Painéis")) : get_text(QStringLiteral("Mostrar Painel")),
        app);

    QAction *acao_painel_calendario = panel_menu->addAction(get_text(QStringLiteral("Calendário")));
    acao_painel_calendario->setCheckable(true);
    acao_painel_calendario->setChecked(app->calendar_panel_visible());
    acao_painel_calendario->setShortcut(QKeySequence(QStringLiteral("Ctrl+M")));
    QObject::connect(acao_painel_calendario, &QAction::triggered, app, [app](bool checked) {
        app->set_calendar_panel_visible(checked);
    });

    QAction *acao_painel_feynman = nullptr;
    if (app->has_feynman_plugin()) {
        acao_painel_feynman = panel_menu->addAction(get_text(QStringLiteral("Minhas Tarefas")));
        acao_painel_feynman->setCheckable(true);
        acao_painel_feynman->setChecked(app->feynman_task_panel_visible());
        acao_painel_feynman->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+T")));
        QObject::connect(acao_painel_feynman, &QAction::triggered, app, [app](bool checked) {
            app->set_feynman_task_panel_visible(checked);
        });
    }

    QAction *acao_painel_pomodoro = nullptr;
    if (app->has_pomodoro_plugin()) {
        acao_painel_pomodoro = panel_menu->addAction(get_text(QStringLiteral("Em Progresso")));
        acao_painel_pomodoro->setCheckable(true);
        acao_painel_pomodoro->setChecked(app->pomodoro_in_progress_panel_visible());
        acao_painel_pomodoro->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+E")));
        QObject::connect(acao_painel_pomodoro, &QAction::triggered, app, [app](bool checked) {
            app->set_pomodoro_in_progress_panel_visible(checked);
        });
    }

    QObject::connect(panel_menu, &QMenu::aboutToShow, app, [app, acao_painel_calendario, acao_painel_feynman, acao_painel_pomodoro]() {
        if (acao_painel_calendario != nullptr) {
            acao_painel_calendario->setChecked(app->calendar_panel_visible());
        }
        if (acao_painel_feynman != nullptr) {
            acao_painel_feynman->setChecked(app->feynman_task_panel_visible());
        }
        if (acao_painel_pomodoro != nullptr) {
            acao_painel_pomodoro->setChecked(app->pomodoro_in_progress_panel_visible());
        }
    });

    add_submenu(layout_menu, interface_menu);
    add_submenu(layout_menu, panel_menu);
    add_submenu(layout_menu, mostrar_abas_interface_menu);
    add_submenu(layout_menu, abas_interface_menu);
    layout_menu->addSeparator();

    auto *ordenar_data_menu = create_menu(get_text(QStringLiteral("Organizar por Data")), app);
    auto *ordenar_horario_menu = create_menu(get_text(QStringLiteral("Organizar por Horário")), app);
    auto *ordenar_alfabetica_menu = create_menu(get_text(QStringLiteral("Organizar por Ordem Alfabética")), app);
    ordenar_data_menu->menuAction()->setCheckable(true);
    ordenar_horario_menu->menuAction()->setCheckable(true);
    ordenar_alfabetica_menu->menuAction()->setCheckable(true);

    QAction *acao_data_crescente = ordenar_data_menu->addAction(get_text(QStringLiteral("Crescente")));
    acao_data_crescente->setCheckable(true);

    QAction *acao_data_decrescente = ordenar_data_menu->addAction(get_text(QStringLiteral("Decrescente")));
    acao_data_decrescente->setCheckable(true);

    QAction *acao_horario_crescente = ordenar_horario_menu->addAction(get_text(QStringLiteral("Crescente")));
    acao_horario_crescente->setCheckable(true);

    QAction *acao_horario_decrescente = ordenar_horario_menu->addAction(get_text(QStringLiteral("Decrescente")));
    acao_horario_decrescente->setCheckable(true);

    QAction *acao_alfabetica_crescente = ordenar_alfabetica_menu->addAction(get_text(QStringLiteral("Crescente")));
    acao_alfabetica_crescente->setCheckable(true);

    QAction *acao_alfabetica_decrescente = ordenar_alfabetica_menu->addAction(get_text(QStringLiteral("Decrescente")));
    acao_alfabetica_decrescente->setCheckable(true);

    add_submenu(layout_menu, ordenar_data_menu);
    add_submenu(layout_menu, ordenar_horario_menu);
    add_submenu(layout_menu, ordenar_alfabetica_menu);

    const auto sync_layout_actions = [app,
                                      eisenhower_tabs_menu,
                                      acao_layout_abas_vertical,
                                      acao_layout_abas_horizontal,
                                      acao_layout_quadrantes,
                                      acao_layout_matriz_2x2]() {
        eisenhower_tabs_menu->setTitle(get_text(QStringLiteral("Layout em Abas")));
        acao_layout_abas_vertical->setText(get_text(QStringLiteral("Vertical")));
        acao_layout_abas_horizontal->setText(get_text(QStringLiteral("Horizontal")));
        acao_layout_quadrantes->setText(get_text(QStringLiteral("Layout em Quadrantes")));
        acao_layout_matriz_2x2->setText(get_text(QStringLiteral("Layout Matriz 2x2")));

        const auto current_layout = app->current_task_layout();
        acao_layout_abas_vertical->setChecked(current_layout == source::EisenhowerMatrixApp::TaskLayoutMode::Tabs);
        acao_layout_abas_horizontal->setChecked(current_layout == source::EisenhowerMatrixApp::TaskLayoutMode::TabsHorizontal);
        acao_layout_quadrantes->setChecked(current_layout == source::EisenhowerMatrixApp::TaskLayoutMode::Quadrants);
        acao_layout_matriz_2x2->setChecked(current_layout == source::EisenhowerMatrixApp::TaskLayoutMode::Matrix2x2);
    };
    QObject::connect(eisenhower_interface_menu, &QMenu::aboutToShow, app, sync_layout_actions);

    const auto sync_sort_actions = [app,
                                    ordenar_data_menu,
                                    ordenar_horario_menu,
                                    ordenar_alfabetica_menu,
                                    acao_data_crescente,
                                    acao_data_decrescente,
                                    acao_horario_crescente,
                                    acao_horario_decrescente,
                                    acao_alfabetica_crescente,
                                    acao_alfabetica_decrescente]() {
        const auto data_criterion = source::EisenhowerMatrixApp::TaskSortCriterion::Date;
        const auto time_criterion = source::EisenhowerMatrixApp::TaskSortCriterion::Time;
        const auto alpha_criterion = source::EisenhowerMatrixApp::TaskSortCriterion::Alphabetical;
        const bool data_ativa = app->has_task_sort_criterion(data_criterion);
        const bool horario_ativo = app->has_task_sort_criterion(time_criterion);
        const bool alfabetica_ativa = app->has_task_sort_criterion(alpha_criterion);
        const auto title_with_priority = [app](const QString &base_text,
                                               source::EisenhowerMatrixApp::TaskSortCriterion criterion) {
            const int priority = app->task_sort_priority(criterion);
            return priority >= 0 ? QStringLiteral("%1 (%2)").arg(base_text).arg(priority + 1) : base_text;
        };

        ordenar_data_menu->setTitle(title_with_priority(get_text(QStringLiteral("Organizar por Data")), data_criterion));
        ordenar_horario_menu->setTitle(title_with_priority(get_text(QStringLiteral("Organizar por Horário")), time_criterion));
        ordenar_alfabetica_menu->setTitle(
            title_with_priority(get_text(QStringLiteral("Organizar por Ordem Alfabética")), alpha_criterion));
        ordenar_data_menu->menuAction()->setChecked(data_ativa);
        ordenar_horario_menu->menuAction()->setChecked(horario_ativo);
        ordenar_alfabetica_menu->menuAction()->setChecked(alfabetica_ativa);
        acao_data_crescente->setChecked(data_ativa && app->task_sort_order(data_criterion) == Qt::AscendingOrder);
        acao_data_decrescente->setChecked(data_ativa && app->task_sort_order(data_criterion) == Qt::DescendingOrder);
        acao_horario_crescente->setChecked(horario_ativo && app->task_sort_order(time_criterion) == Qt::AscendingOrder);
        acao_horario_decrescente->setChecked(horario_ativo
                                             && app->task_sort_order(time_criterion) == Qt::DescendingOrder);
        acao_alfabetica_crescente->setChecked(alfabetica_ativa
                                              && app->task_sort_order(alpha_criterion) == Qt::AscendingOrder);
        acao_alfabetica_decrescente->setChecked(alfabetica_ativa
                                                && app->task_sort_order(alpha_criterion) == Qt::DescendingOrder);
    };

    const auto handle_sort_toggle = [app, sync_sort_actions](
                                        bool checked,
                                        source::EisenhowerMatrixApp::TaskSortMode target_mode,
                                        source::EisenhowerMatrixApp::TaskSortCriterion criterion) {
        if (checked) {
            app->set_task_sort_mode(target_mode);
        } else {
            app->clear_task_sort_criterion(criterion);
        }
        sync_sort_actions();
    };

    sync_sort_actions();

    const auto apply_eisenhower_layout = [app, sync_layout_actions](source::EisenhowerMatrixApp::TaskLayoutMode mode) {
        if (app->current_task_layout() == mode) {
            sync_layout_actions();
            return;
        }

        if (app->set_task_layout(mode)) {
            show_feedback_message(app, QMessageBox::Information, get_text(QStringLiteral("Sucesso")), layout_success_message(mode));
        } else {
            show_feedback_message(app, QMessageBox::Warning, get_text(QStringLiteral("Erro")), layout_error_message(mode));
        }
        sync_layout_actions();
    };

    QObject::connect(acao_layout_abas_vertical, &QAction::triggered, app, [apply_eisenhower_layout]() {
        apply_eisenhower_layout(source::EisenhowerMatrixApp::TaskLayoutMode::Tabs);
    });

    QObject::connect(acao_layout_abas_horizontal, &QAction::triggered, app, [apply_eisenhower_layout]() {
        apply_eisenhower_layout(source::EisenhowerMatrixApp::TaskLayoutMode::TabsHorizontal);
    });

    QObject::connect(acao_layout_quadrantes, &QAction::triggered, app, [apply_eisenhower_layout]() {
        apply_eisenhower_layout(source::EisenhowerMatrixApp::TaskLayoutMode::Quadrants);
    });

    QObject::connect(acao_layout_matriz_2x2, &QAction::triggered, app, [apply_eisenhower_layout]() {
        apply_eisenhower_layout(source::EisenhowerMatrixApp::TaskLayoutMode::Matrix2x2);
    });

    QObject::connect(acao_data_crescente, &QAction::triggered, app, [handle_sort_toggle](bool checked) {
        handle_sort_toggle(checked,
                           source::EisenhowerMatrixApp::TaskSortMode::DateAscending,
                           source::EisenhowerMatrixApp::TaskSortCriterion::Date);
    });

    QObject::connect(acao_data_decrescente, &QAction::triggered, app, [handle_sort_toggle](bool checked) {
        handle_sort_toggle(checked,
                           source::EisenhowerMatrixApp::TaskSortMode::DateDescending,
                           source::EisenhowerMatrixApp::TaskSortCriterion::Date);
    });

    QObject::connect(acao_horario_crescente, &QAction::triggered, app, [handle_sort_toggle](bool checked) {
        handle_sort_toggle(checked,
                           source::EisenhowerMatrixApp::TaskSortMode::TimeAscending,
                           source::EisenhowerMatrixApp::TaskSortCriterion::Time);
    });

    QObject::connect(acao_horario_decrescente, &QAction::triggered, app, [handle_sort_toggle](bool checked) {
        handle_sort_toggle(checked,
                           source::EisenhowerMatrixApp::TaskSortMode::TimeDescending,
                           source::EisenhowerMatrixApp::TaskSortCriterion::Time);
    });

    QObject::connect(acao_alfabetica_crescente, &QAction::triggered, app, [handle_sort_toggle](bool checked) {
        handle_sort_toggle(checked,
                           source::EisenhowerMatrixApp::TaskSortMode::AlphabeticalAscending,
                           source::EisenhowerMatrixApp::TaskSortCriterion::Alphabetical);
    });

    QObject::connect(acao_alfabetica_decrescente, &QAction::triggered, app, [handle_sort_toggle](bool checked) {
        handle_sort_toggle(checked,
                           source::EisenhowerMatrixApp::TaskSortMode::AlphabeticalDescending,
                           source::EisenhowerMatrixApp::TaskSortCriterion::Alphabetical);
    });

    log_menu_stage(QStringLiteral("seção Layout de Tarefas criada"));
    add_submenu(config_menu, layout_menu);
    add_submenu(config_menu, font_settings_menu);

    log_menu_stage(QStringLiteral("criando seção Sincronizar e Plugins"));
    StickyMenu *sync_menu = nullptr;
    if (app->has_pomodoro_plugin() || app->has_feynman_plugin()) {
        sync_menu = create_menu(get_text(QStringLiteral("Sincronizar")), app);

        QAction *acao_sync_eisenhower = sync_menu->addAction(get_text(QStringLiteral("Eisenhower")));
        acao_sync_eisenhower->setCheckable(true);
        acao_sync_eisenhower->setChecked(app->sync_eisenhower_enabled());
        QObject::connect(acao_sync_eisenhower, &QAction::toggled, app, [app](bool checked) {
            app->set_sync_eisenhower_enabled(checked);
        });

        if (app->has_pomodoro_plugin()) {
            QAction *acao_sync_pomodoro = sync_menu->addAction(get_text(QStringLiteral("Pomodoro")));
            acao_sync_pomodoro->setCheckable(true);
            acao_sync_pomodoro->setChecked(app->sync_pomodoro_enabled());
            QObject::connect(acao_sync_pomodoro, &QAction::toggled, app, [app](bool checked) {
                app->set_sync_pomodoro_enabled(checked);
            });
        }

        if (app->has_feynman_plugin()) {
            QAction *acao_sync_feynman = sync_menu->addAction(get_text(QStringLiteral("Feynman")));
            acao_sync_feynman->setCheckable(true);
            acao_sync_feynman->setChecked(app->sync_feynman_enabled());
            QObject::connect(acao_sync_feynman, &QAction::toggled, app, [app](bool checked) {
                app->set_sync_feynman_enabled(checked);
            });
        }
    }

    const bool has_specific_settings = app->has_pomodoro_plugin() || app->has_mapamental_plugin() || app->has_audiolivro_plugin();
    if (has_specific_settings) {
        auto *specific_settings_menu = create_menu(get_text(QStringLiteral("Configurações Específicas")), app);

        if (app->has_pomodoro_plugin()) {
            auto *pomodoro_config_menu = create_menu(app->pomodoro_display_name(), app);

            QAction *acao_alarme = pomodoro_config_menu->addAction(app->pomodoro_alarm_menu_text());
            QObject::connect(acao_alarme, &QAction::triggered, app, [app]() { app->open_pomodoro_alarm_settings(app); });

            QAction *acao_modo_automatico = pomodoro_config_menu->addAction(app->pomodoro_automatic_mode_menu_text());
            QObject::connect(acao_modo_automatico, &QAction::triggered, app, [app]() {
                app->open_pomodoro_automatic_mode_settings(app);
            });

            QObject::connect(pomodoro_config_menu, &QMenu::aboutToShow, app, [app, pomodoro_config_menu, acao_alarme, acao_modo_automatico]() {
                pomodoro_config_menu->setTitle(app->pomodoro_display_name());
                acao_alarme->setText(app->pomodoro_alarm_menu_text());
                acao_modo_automatico->setText(app->pomodoro_automatic_mode_menu_text());
            });

            add_submenu(specific_settings_menu, pomodoro_config_menu);
        }

        if (app->has_mapamental_plugin()) {
            auto *mapamental_config_menu = create_menu(app->mapamental_display_name(), app);
            auto *summary_group = new QActionGroup(mapamental_config_menu);
            summary_group->setExclusive(true);

            QAction *acao_resumo_heuristico = mapamental_config_menu->addAction(
                get_text(QStringLiteral("Resumo método heurístico")));
            acao_resumo_heuristico->setCheckable(true);
            acao_resumo_heuristico->setActionGroup(summary_group);
            acao_resumo_heuristico->setData(QStringLiteral("heuristic"));

            const auto close_mapamental_menus = [mapamental_config_menu, specific_settings_menu, config_menu]() {
                if (mapamental_config_menu != nullptr) {
                    mapamental_config_menu->close();
                }
                if (specific_settings_menu != nullptr) {
                    specific_settings_menu->close();
                }
                if (config_menu != nullptr) {
                    config_menu->close();
                }
            };

            if (app->has_modelo_plugin()) {
                acao_resumo_heuristico->setChecked(
                    app->mapamental_summary_method_key().compare(QStringLiteral("heuristic"), Qt::CaseInsensitive) == 0);
                QObject::connect(acao_resumo_heuristico, &QAction::triggered, app, [app]() {
                    app->set_mapamental_summary_method_key(QStringLiteral("heuristic"));
                });

                const bool qwen_ok_inicial = app->is_qwen_model_installed();
                QAction *acao_resumo_qwen = mapamental_config_menu->addAction(
                    qwen_ok_inicial
                        ? get_text(QStringLiteral("Resumo com modelo qwen2.5-3b [Instalado]"))
                        : get_text(QStringLiteral("Resumo com modelo qwen2.5-3b [Não instalado]")));
                acao_resumo_qwen->setCheckable(true);
                acao_resumo_qwen->setActionGroup(summary_group);
                acao_resumo_qwen->setData(QStringLiteral("qwen"));
                acao_resumo_qwen->setChecked(
                    app->mapamental_summary_method_key().compare(QStringLiteral("qwen"), Qt::CaseInsensitive) == 0);
                QObject::connect(acao_resumo_qwen, &QAction::triggered, app, [app, close_mapamental_menus]() {
                    if (!app->is_qwen_model_installed()) {
                        close_mapamental_menus();
                        QMessageBox box(app);
                        box.setIcon(QMessageBox::Question);
                        box.setWindowTitle(get_text(QStringLiteral("Modelo Qwen Não Instalado")));
                        box.setText(get_text(QStringLiteral("O modelo local Qwen 2.5 (2,1 GB) ainda não foi instalado neste computador.\n\n"
                                                            "Deseja abrir o Gerenciador de Download para baixá-lo agora?")));
                        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                        box.setDefaultButton(QMessageBox::Yes);
                        if (app->gerenciador_traducao != nullptr) {
                            app->gerenciador_traducao->traduzir_botoes_padrao(&box);
                        }
                        if (box.exec() == QMessageBox::Yes) {
                            app->show_model_download_dialog();
                        }
                    }
                    app->set_mapamental_summary_method_key(QStringLiteral("qwen"));
                });

                QAction *acao_gerenciar_qwen = mapamental_config_menu->addAction(
                    get_text(QStringLiteral("Como baixar ou gerenciar modelo Qwen (GGUF)...")));
                QObject::connect(acao_gerenciar_qwen, &QAction::triggered, app, [app, close_mapamental_menus]() {
                    close_mapamental_menus();
                    app->show_model_download_dialog();
                });

                mapamental_config_menu->addSeparator();

                auto *ollama_submenu = create_menu(get_text(QStringLiteral("Modelos Ollama")), app);
                add_submenu(mapamental_config_menu, ollama_submenu);

                QAction *acao_instrucoes_ollama = mapamental_config_menu->addAction(
                    get_text(QStringLiteral("Como instalar modelos no Ollama...")));
                QObject::connect(acao_instrucoes_ollama, &QAction::triggered, app, [app, close_mapamental_menus]() {
                    close_mapamental_menus();
                    app->show_ollama_instructions_dialog();
                });

                auto ollama_populating = std::make_shared<bool>(false);
                auto ollama_loaded = std::make_shared<bool>(false);
                auto popular_menu_ollama = std::make_shared<std::function<void(bool)>>();

                QAction *acao_recarregar_inicial = ollama_submenu->addAction(
                    get_text(QStringLiteral("Atualizar lista de modelos")));
                QObject::connect(acao_recarregar_inicial, &QAction::triggered, app, [popular_menu_ollama]() {
                    (*popular_menu_ollama)(true);
                });

                *popular_menu_ollama = [app, ollama_submenu, summary_group, acao_resumo_heuristico, acao_resumo_qwen, ollama_populating, ollama_loaded, popular_menu_ollama](bool forcar_recarga) {
                    if (*ollama_populating) {
                        return;
                    }
                    *ollama_populating = true;

                    if (forcar_recarga) {
                        app->refresh_ollama_models();
                    }
                    const QVariantList modelos_disponiveis = app->ollama_installed_models();
                    const QString current_method = app->mapamental_summary_method_key();

                    for (QAction *action : ollama_submenu->actions()) {
                        summary_group->removeAction(action);
                    }
                    ollama_submenu->clear();

                    QAction *acao_recarregar = ollama_submenu->addAction(
                        get_text(QStringLiteral("Atualizar lista de modelos")));
                    QObject::connect(acao_recarregar, &QAction::triggered, app, [popular_menu_ollama]() {
                        (*popular_menu_ollama)(true);
                    });
                    ollama_submenu->addSeparator();

                    bool modelo_ativo_encontrado = false;
                    int modelos_ollama_adicionados = 0;

                    for (const QVariant &item_val : modelos_disponiveis) {
                        const QVariantMap item = item_val.toMap();
                        const QString key = item.value(QStringLiteral("key")).toString();
                        if (!key.startsWith(QStringLiteral("ollama:"), Qt::CaseInsensitive)) {
                            continue;
                        }
                        const QString display = item.value(QStringLiteral("display")).toString();
                        QAction *model_action = ollama_submenu->addAction(display);
                        model_action->setCheckable(true);
                        model_action->setActionGroup(summary_group);
                        model_action->setData(key);
                        const bool is_checked = (current_method.compare(key, Qt::CaseInsensitive) == 0);
                        model_action->setChecked(is_checked);
                        if (is_checked) {
                            modelo_ativo_encontrado = true;
                        }
                        QObject::connect(model_action, &QAction::triggered, app, [app, key]() {
                            app->set_mapamental_summary_method_key(key);
                        });
                        ++modelos_ollama_adicionados;
                    }

                    if (modelos_ollama_adicionados == 0) {
                        QAction *empty_action = ollama_submenu->addAction(
                            get_text(QStringLiteral("(Nenhum modelo Ollama instalado)")));
                        empty_action->setEnabled(false);
                    }

                    // Se o método configurado era um modelo Ollama que foi desinstalado, fazer fallback seguro
                    if (current_method.startsWith(QStringLiteral("ollama:"), Qt::CaseInsensitive) && !modelo_ativo_encontrado) {
                        const QString fallback = app->is_qwen_model_installed() ? QStringLiteral("qwen") : QStringLiteral("heuristic");
                        app->set_mapamental_summary_method_key(fallback);
                        acao_resumo_heuristico->setChecked(fallback == QStringLiteral("heuristic"));
                        acao_resumo_qwen->setChecked(fallback == QStringLiteral("qwen"));
                    } else {
                        acao_resumo_heuristico->setChecked(
                            current_method.compare(QStringLiteral("heuristic"), Qt::CaseInsensitive) == 0);
                        acao_resumo_qwen->setChecked(
                            current_method.compare(QStringLiteral("qwen"), Qt::CaseInsensitive) == 0);
                    }

                    *ollama_loaded = true;
                    *ollama_populating = false;
                };

                QObject::connect(mapamental_config_menu,
                                 &QMenu::aboutToShow,
                                 app,
                                 [mapamental_config_menu, popular_menu_ollama, ollama_submenu, acao_instrucoes_ollama, acao_resumo_heuristico, acao_resumo_qwen, acao_gerenciar_qwen, ollama_loaded, app]() {
                                     mapamental_config_menu->setTitle(app->mapamental_display_name());
                                     acao_resumo_heuristico->setText(get_text(QStringLiteral("Resumo método heurístico")));
                                     const bool qwen_ok = app->is_qwen_model_installed();
                                     acao_resumo_qwen->setText(qwen_ok
                                         ? get_text(QStringLiteral("Resumo com modelo qwen2.5-3b [Instalado]"))
                                         : get_text(QStringLiteral("Resumo com modelo qwen2.5-3b [Não instalado]")));
                                     acao_gerenciar_qwen->setText(get_text(QStringLiteral("Como baixar ou gerenciar modelo Qwen (GGUF)...")));
                                     ollama_submenu->setTitle(get_text(QStringLiteral("Modelos Ollama")));
                                     acao_instrucoes_ollama->setText(get_text(QStringLiteral("Como instalar modelos no Ollama...")));

                                     const QString current_method = app->mapamental_summary_method_key();
                                     acao_resumo_heuristico->setChecked(
                                         current_method.compare(QStringLiteral("heuristic"), Qt::CaseInsensitive) == 0);
                                     acao_resumo_qwen->setChecked(
                                         current_method.compare(QStringLiteral("qwen"), Qt::CaseInsensitive) == 0);

                                     if (*ollama_loaded || current_method.startsWith(QStringLiteral("ollama:"), Qt::CaseInsensitive)) {
                                         (*popular_menu_ollama)(false);
                                     }
                                 });

                QObject::connect(ollama_submenu,
                                 &QMenu::aboutToShow,
                                 app,
                                 [popular_menu_ollama, ollama_submenu]() {
                                     ollama_submenu->setTitle(get_text(QStringLiteral("Modelos Ollama")));
                                     (*popular_menu_ollama)(false);
                                 });
            } else {
                acao_resumo_heuristico->setChecked(true);
                QObject::connect(acao_resumo_heuristico, &QAction::triggered, app, [app]() {
                    app->set_mapamental_summary_method_key(QStringLiteral("heuristic"));
                });
                QObject::connect(mapamental_config_menu, &QMenu::aboutToShow, app, [app, mapamental_config_menu, acao_resumo_heuristico]() {
                    mapamental_config_menu->setTitle(app->mapamental_display_name());
                    acao_resumo_heuristico->setText(get_text(QStringLiteral("Resumo método heurístico")));
                    acao_resumo_heuristico->setChecked(true);
                });
            }

            add_submenu(specific_settings_menu, mapamental_config_menu);
        }

        QObject::connect(specific_settings_menu, &QMenu::aboutToShow, app, [specific_settings_menu]() {
            specific_settings_menu->setTitle(get_text(QStringLiteral("Configurações Específicas")));
        });

        if (app->has_audiolivro_plugin()) {
            auto *audiolivro_config_menu = create_menu(app->audiolivro_display_name(), app);

            auto *vozes_menu = create_menu(get_text(QStringLiteral("Vozes")), app);
            auto *vozes_group = new QActionGroup(vozes_menu);
            vozes_group->setExclusive(true);
            QAction *acao_carregar_vozes = vozes_menu->addAction(get_text(QStringLiteral("Carregar vozes")));
            acao_carregar_vozes->setEnabled(false);

            QObject::connect(vozes_menu, &QMenu::aboutToShow, app, [app, vozes_menu, vozes_group, populated = false, populating = false]() mutable {
                if (populating) {
                    return;
                }
                if (!populated) {
                    populating = true;
                    log_menu_stage(QStringLiteral("populando vozes do Audiolivro"));
                    const QVariantList vozes = app->audiolivro_voice_options();

                    for (QAction *acao : vozes_group->actions()) {
                        vozes_group->removeAction(acao);
                    }
                    for (QMenu *submenu : vozes_menu->findChildren<QMenu *>()) {
                        submenu->deleteLater();
                    }
                    vozes_menu->clear();

                    QMap<QString, StickyMenu *> menus_por_idioma;
                    if (vozes.isEmpty()) {
                        QAction *sem_vozes = vozes_menu->addAction(get_text(QStringLiteral("Nenhuma voz encontrada")));
                        sem_vozes->setEnabled(false);
                    }

                    for (const QVariant &item : vozes) {
                        const QVariantMap voz = item.toMap();
                        const QString id = voz.value(QStringLiteral("id")).toString();
                        if (id.trimmed().isEmpty()) {
                            continue;
                        }

                        const QString idioma = voz.value(QStringLiteral("language")).toString();
                        const QString locale = voz.value(QStringLiteral("locale")).toString();
                        QString grupo = idioma.isEmpty() ? locale : idioma;
                        if (!locale.isEmpty() && !grupo.contains(locale)) {
                            grupo = QStringLiteral("%1 (%2)").arg(grupo, locale);
                        }
                        if (grupo.trimmed().isEmpty()) {
                            grupo = get_text(QStringLiteral("Outras"));
                        }

                        StickyMenu *idioma_menu = menus_por_idioma.value(grupo, nullptr);
                        if (idioma_menu == nullptr) {
                            idioma_menu = create_menu(grupo, vozes_menu);
                            menus_por_idioma.insert(grupo, idioma_menu);
                            add_submenu(vozes_menu, idioma_menu);
                        }

                        const QString nome = voz.value(QStringLiteral("name")).toString();
                        const QString genero = voz.value(QStringLiteral("gender")).toString();
                        const QString mecanismo = voz.value(QStringLiteral("engine")).toString();
                        QString rotulo = nome.trimmed().isEmpty() ? id : nome.trimmed();
                        if (!genero.trimmed().isEmpty()) {
                            rotulo += QStringLiteral(" - %1").arg(genero.trimmed());
                        }
                        if (!mecanismo.trimmed().isEmpty()) {
                            rotulo += QStringLiteral(" - %1").arg(mecanismo.trimmed());
                        }
                        rotulo += QStringLiteral(" [%1]").arg(id);

                        QAction *acao_voz = idioma_menu->addAction(rotulo);
                        acao_voz->setCheckable(true);
                        acao_voz->setActionGroup(vozes_group);
                        acao_voz->setData(id);
                        acao_voz->setToolTip(id);
                        QObject::connect(acao_voz, &QAction::triggered, app, [app, id]() {
                            app->set_audiolivro_voice_id(id);
                            app->open_audiolivro();
                        });
                    }
                    if (vozes.size() > 100) {
                        populated = true;
                    }
                    populating = false;
                    log_menu_stage(QStringLiteral("vozes do Audiolivro populadas"));
                }

                const QString voz_atual = app->audiolivro_current_voice_id();
                for (QAction *acao : vozes_group->actions()) {
                    acao->setChecked(acao->data().toString() == voz_atual);
                }
            });

            add_submenu(audiolivro_config_menu, vozes_menu);

            QObject::connect(audiolivro_config_menu, &QMenu::aboutToShow, app, [app, audiolivro_config_menu]() {
                audiolivro_config_menu->setTitle(app->audiolivro_display_name());
            });

            add_submenu(specific_settings_menu, audiolivro_config_menu);
        }

        add_submenu(config_menu, specific_settings_menu);
    }

    if (app->has_agenda_plugin()) {
        auto *agenda_menu = create_menu(app->agenda_display_name(), app);

        auto *agenda_excluir_menu = create_menu(get_text(QStringLiteral("Excluir item")), app);
        QObject::connect(agenda_excluir_menu, &QMenu::aboutToShow, app, [app, agenda_excluir_menu]() {
            agenda_excluir_menu->setTitle(get_text(QStringLiteral("Excluir item")));
            app->populate_agenda_delete_item_menu(agenda_excluir_menu);
        });
        add_submenu(agenda_menu, agenda_excluir_menu);

        auto *agenda_cores_menu = create_menu(get_text(QStringLiteral("Cores")), app);
        QObject::connect(agenda_cores_menu, &QMenu::aboutToShow, app, [app, agenda_cores_menu]() {
            agenda_cores_menu->setTitle(get_text(QStringLiteral("Cores")));
            app->populate_agenda_color_menu(agenda_cores_menu);
        });
        add_submenu(agenda_menu, agenda_cores_menu);

        QObject::connect(agenda_menu, &QMenu::aboutToShow, app, [app, agenda_menu]() {
            agenda_menu->setTitle(app->agenda_display_name());
        });

        add_submenu(config_menu, agenda_menu);
    }

    if (sync_menu != nullptr) {
        add_submenu(config_menu, sync_menu);
    }

    if (idioma_menu != nullptr) {
        add_submenu(config_menu, idioma_menu);
    }

    log_menu_stage(QStringLiteral("adicionando menu Configurações"));
    menu_bar->addMenu(config_menu);

    log_menu_stage(QStringLiteral("criando menu Ferramentas"));
    auto *ferramentas_menu = create_menu(get_text(QStringLiteral("Ferramentas")), app);
    QAction *acao_busca_global = ferramentas_menu->addAction(get_text(QStringLiteral("Busca global")));
    acao_busca_global->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+G")));
    QObject::connect(acao_busca_global, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::show_global_search(app);
    });

    QAction *acao_hoje = ferramentas_menu->addAction(get_text(QStringLiteral("Hoje / Próximos 7 dias")));
    acao_hoje->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+H")));
    QObject::connect(acao_hoje, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::show_today_panel(app);
    });

    if (app->has_agenda_plugin()) {
        QAction *acao_agenda = ferramentas_menu->addAction(app->agenda_open_menu_text());
        acao_agenda->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+R")));
        QObject::connect(acao_agenda, &QAction::triggered, app, [app]() { app->open_agenda(); });
    }

    if (app->has_gantt_plugin()) {
        QAction *acao_gantt = ferramentas_menu->addAction(app->gantt_open_menu_text());
        acao_gantt->setShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+G")));
        QObject::connect(acao_gantt, &QAction::triggered, app, [app]() { app->open_gantt(); });
    }

    QAction *acao_filtros = ferramentas_menu->addAction(get_text(QStringLiteral("Filtros")));
    acao_filtros->setShortcut(QKeySequence(QStringLiteral("Ctrl+Alt+F")));
    QObject::connect(acao_filtros, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::show_filters_dialog(app);
    });

    ferramentas_menu->addSeparator();
    QAction *acao_recorrencias = ferramentas_menu->addAction(get_text(QStringLiteral("Gerar próximas recorrências")));
    QObject::connect(acao_recorrencias, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::materialize_recurring_tasks(app);
    });

    QAction *acao_lembretes = ferramentas_menu->addAction(get_text(QStringLiteral("Ativar lembretes locais")));
    QObject::connect(acao_lembretes, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::schedule_local_reminders(app);
    });

    QAction *acao_ics = ferramentas_menu->addAction(get_text(QStringLiteral("Exportar calendário .ics")));
    QObject::connect(acao_ics, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::export_calendar_ics(app);
    });

    ferramentas_menu->addSeparator();
    QAction *acao_historico = ferramentas_menu->addAction(get_text(QStringLiteral("Histórico de produtividade")));
    QObject::connect(acao_historico, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::show_productivity_history(app);
    });

    QAction *acao_fluxo_estudo = ferramentas_menu->addAction(get_text(QStringLiteral("Fluxo integrado de estudo")));
    QObject::connect(acao_fluxo_estudo, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::run_study_flow(app);
    });

    QAction *acao_templates = ferramentas_menu->addAction(get_text(QStringLiteral("Templates")));
    QObject::connect(acao_templates, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::show_templates_dialog(app);
    });

    QAction *acao_revisao_espacada = ferramentas_menu->addAction(get_text(QStringLiteral("Criar revisões espaçadas")));
    QObject::connect(acao_revisao_espacada, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::create_spaced_reviews(app);
    });

    ferramentas_menu->addSeparator();
    QAction *acao_plugins = ferramentas_menu->addAction(get_text(QStringLiteral("Gerenciador de plugins")));
    QObject::connect(acao_plugins, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::show_plugin_manager(app);
    });
    QAction *acao_logs = ferramentas_menu->addAction(get_text(QStringLiteral("Gerenciador de logs")));
    QObject::connect(acao_logs, &QAction::triggered, app, [app]() {
        source::InterfaceCore::ProductivityFeatures::show_log_manager(app);
    });
    menu_bar->addMenu(ferramentas_menu);

    log_menu_stage(QStringLiteral("criando menu Opções"));
    auto *opcoes_menu = create_menu(get_text(QStringLiteral("Opções")), app);
    QAction *acao_sobre = opcoes_menu->addAction(get_text(QStringLiteral("Sobre")));
    acao_sobre->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+A")));
    QObject::connect(acao_sobre, &QAction::triggered, app, [app]() { app->exibir_sobre(); });

    QAction *acao_manual = opcoes_menu->addAction(get_text(QStringLiteral("Manual")));
    acao_manual->setShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+M")));
    QObject::connect(acao_manual, &QAction::triggered, app, [app]() { source::InterfaceCore::incore_08_exibir_sobre::exibir_manual(app); });
    menu_bar->addMenu(opcoes_menu);

    app->idioma_menu = idioma_menu;
    log_menu_stage(QStringLiteral("configurando atalhos principais"));
    delete_shortcut(app->atalho_arquivo);
    delete_shortcut(app->atalho_idioma);
    delete_shortcut(app->atalho_opcoes);

    app->atalho_arquivo = new QShortcut(QKeySequence(QStringLiteral("Alt+A")), app);
    app->atalho_arquivo->setContext(Qt::ApplicationShortcut);
    QObject::connect(app->atalho_arquivo, &QShortcut::activated, app, [arquivo_menu, menu_bar]() {
        popup_menu(arquivo_menu, menu_bar);
    });

    app->atalho_idioma = new QShortcut(QKeySequence(QStringLiteral("Alt+I")), app);
    app->atalho_idioma->setContext(Qt::ApplicationShortcut);
    QObject::connect(app->atalho_idioma, &QShortcut::activated, app, [idioma_menu, menu_bar]() {
        popup_menu(idioma_menu, menu_bar);
    });

    app->atalho_opcoes = new QShortcut(QKeySequence(QStringLiteral("Alt+O")), app);
    app->atalho_opcoes->setContext(Qt::ApplicationShortcut);
    QObject::connect(app->atalho_opcoes, &QShortcut::activated, app, [opcoes_menu, menu_bar]() {
        popup_menu(opcoes_menu, menu_bar);
    });
    log_menu_stage(QStringLiteral("fim"));
}

}  // namespace source::InterfaceCore::incore_03_criar_menu_configuracoes
