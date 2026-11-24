#pragma once

#include <QPair>
#include <QLocale>
#include <QStringList>
#include <QVariantMap>

class QListWidget;
class QListWidgetItem;
class QPoint;
class QDateEdit;
class QWidget;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore {
class EditTaskDialog;
}

namespace source::InterfaceCore::incore_07_MostrarMenuContexto {

class MostrarMenuContexto {
public:
    static int _compute_widget_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name = QString());
    static void _apply_widget_min_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name = QString());
    static QLocale _effective_locale(source::EisenhowerMatrixApp *app);
    static void _apply_locale_to_dateedit(source::EisenhowerMatrixApp *app, QDateEdit *date_edit);
    static bool _is_completed_list(source::EisenhowerMatrixApp *app, QListWidget *list);
    static QStringList _quadrant_options(source::EisenhowerMatrixApp *app, bool include_completed = false);
    static QListWidget *_target_list_for_quadrant(source::EisenhowerMatrixApp *app,
                                                  int quadrant_index,
                                                  bool keep_completed,
                                                  bool include_completed = false);
    static QPair<QListWidget *, QListWidgetItem *> _find_source_item_for_calendar(source::EisenhowerMatrixApp *app,
                                                                                  const QVariantMap &task_dict);
    static QString _base_text_from_item(QListWidgetItem *item);
    static QPair<QString, QString> _build_display_and_tooltip(source::EisenhowerMatrixApp *app,
                                                              const QString &base_text,
                                                              const QString &date_iso,
                                                              const QString &time_str);
    static void edit_task_dialog(source::EisenhowerMatrixApp *app, QListWidgetItem *item);
    static void _move_to_quadrant(source::EisenhowerMatrixApp *app, QListWidgetItem *item, QListWidget *source_list);
    static void mostrar_menu_contexto(source::EisenhowerMatrixApp *app, const QPoint &point, QListWidget *list_widget);
};

void show_context_menu(source::EisenhowerMatrixApp *app, const QPoint &point, QListWidget *list_widget);
QPair<QListWidget *, QListWidgetItem *> _find_source_item_for_calendar(source::EisenhowerMatrixApp *app,
                                                                       const QVariantMap &task_dict);
void edit_task_dialog(source::EisenhowerMatrixApp *app, QListWidgetItem *item);

}  // namespace source::InterfaceCore::incore_07_MostrarMenuContexto
