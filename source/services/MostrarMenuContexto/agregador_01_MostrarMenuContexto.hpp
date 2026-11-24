#pragma once

#include <QPair>
#include <QLocale>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class QDateEdit;
class QListWidget;
class QListWidgetItem;
class QPoint;
class QWidget;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::MostrarMenuContextoModules {

int compute_widget_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name = QString());
void apply_widget_min_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name = QString());
QLocale effective_locale(source::EisenhowerMatrixApp *app);
void apply_locale_to_dateedit(source::EisenhowerMatrixApp *app, QDateEdit *date_edit);
bool is_completed_list(source::EisenhowerMatrixApp *app, QListWidget *list);
QStringList quadrant_options(source::EisenhowerMatrixApp *app, bool include_completed = false);
QListWidget *target_list_for_quadrant(source::EisenhowerMatrixApp *app,
                                      int quadrant_index,
                                      bool keep_completed,
                                      bool include_completed = false);
QPair<QListWidget *, QListWidgetItem *> find_source_item_for_calendar(source::EisenhowerMatrixApp *app,
                                                                      const QVariantMap &task_dict);
QString base_text_from_item(QListWidgetItem *item);
QPair<QString, QString> build_display_and_tooltip(source::EisenhowerMatrixApp *app,
                                                  const QString &base_text,
                                                  const QString &date_iso,
                                                  const QString &time_str);
void edit_task_dialog(source::EisenhowerMatrixApp *app, QListWidgetItem *item);
void move_to_quadrant(source::EisenhowerMatrixApp *app, QListWidgetItem *item, QListWidget *source_list);
void mostrar_menu_contexto(source::EisenhowerMatrixApp *app, const QPoint &point, QListWidget *list_widget);

}  // namespace source::InterfaceCore::MostrarMenuContextoModules
