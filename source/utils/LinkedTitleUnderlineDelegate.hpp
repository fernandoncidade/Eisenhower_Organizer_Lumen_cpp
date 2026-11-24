#pragma once

#include <QColor>
#include <QFont>
#include <QList>
#include <QString>
#include <QVariant>
#include <Qt>

class QAbstractItemView;
class QListWidgetItem;
class QWidget;

namespace source::utils {

inline constexpr int kLinkedTitleUnderlineRole = Qt::UserRole + 700;
inline constexpr int kLinkedTitleUnderlineTextRole = Qt::UserRole + 701;
inline constexpr int kLinkedTitleUnderlineVisibleRole = Qt::UserRole + 702;
inline constexpr int kLinkedTitleStrikeOutVisibleRole = Qt::UserRole + 703;
inline constexpr int kCustomFontSettingsRole = Qt::UserRole + 704;
inline constexpr int kHasLinkedFileRole = Qt::UserRole + 705;

struct TaskTitleFontSettings {
    QString family;
    QString style;
    int point_size = 0;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strike_out = false;
};

using LinkedFileTitleFontSettings = TaskTitleFontSettings;

TaskTitleFontSettings default_general_task_title_font_settings();
TaskTitleFontSettings load_general_task_title_font_settings();
bool save_general_task_title_font_settings(const TaskTitleFontSettings &settings);
TaskTitleFontSettings default_linked_file_title_font_settings();
TaskTitleFontSettings load_linked_file_title_font_settings();
bool save_linked_file_title_font_settings(const TaskTitleFontSettings &settings);
enum class PluginFontContext {
    Global,
    Feynman,
    Pomodoro,
    MapaMental
};

TaskTitleFontSettings load_plugin_general_task_title_font_settings(PluginFontContext context);
bool save_plugin_general_task_title_font_settings(PluginFontContext context, const TaskTitleFontSettings &settings);
TaskTitleFontSettings load_plugin_linked_file_title_font_settings(PluginFontContext context);
bool save_plugin_linked_file_title_font_settings(PluginFontContext context, const TaskTitleFontSettings &settings);

QVariantMap task_title_font_settings_to_map(const TaskTitleFontSettings &settings);
TaskTitleFontSettings task_title_font_settings_from_map(const QVariantMap &map, const TaskTitleFontSettings &defaults = {});
bool item_has_custom_font_settings(const QListWidgetItem *item);
TaskTitleFontSettings get_item_current_font_settings(const QListWidgetItem *item, bool has_linked_file, PluginFontContext context = PluginFontContext::Global);
void set_item_custom_font_settings(QListWidgetItem *item, const TaskTitleFontSettings &settings);
void clear_item_custom_font_settings(QListWidgetItem *item);

bool open_task_font_settings_dialog(QWidget *parent, const QString &dialog_title, const QString &preview_text, const TaskTitleFontSettings &initial_settings, const TaskTitleFontSettings &default_settings, TaskTitleFontSettings *out_settings, PluginFontContext context = PluginFontContext::Global);
bool open_task_font_settings_dialog(QWidget *parent, const QList<QListWidgetItem *> &items, PluginFontContext context = PluginFontContext::Global);
QColor choose_task_color(QWidget *parent, const QColor &initial_color = QColor(), PluginFontContext context = PluginFontContext::Global, const QString &dialog_title = QString());

QFont task_title_font(const QFont &base_font, const TaskTitleFontSettings &settings);
QFont linked_file_title_font(const QFont &base_font, const TaskTitleFontSettings &settings);
void apply_task_title_font(QListWidgetItem *item, const QString &title, bool has_linked_file, PluginFontContext context = PluginFontContext::Global);
void apply_linked_file_title_font(QListWidgetItem *item, bool enabled, PluginFontContext context = PluginFontContext::Global);

void install_linked_title_underline_delegate(QAbstractItemView *view);
void set_linked_title_underline(QListWidgetItem *item, const QString &title, bool enabled);

}  // namespace source::utils
