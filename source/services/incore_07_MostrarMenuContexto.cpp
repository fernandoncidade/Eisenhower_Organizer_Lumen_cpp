#include "source/services/incore_07_MostrarMenuContexto.hpp"

#include <QDateEdit>
#include <QLocale>

#include "source/services/MostrarMenuContexto/agregador_01_MostrarMenuContexto.hpp"

namespace source::InterfaceCore::incore_07_MostrarMenuContexto {

int MostrarMenuContexto::_compute_widget_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name)
{
    return MostrarMenuContextoModules::compute_widget_height(app, widget, name);
}

void MostrarMenuContexto::_apply_widget_min_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name)
{
    MostrarMenuContextoModules::apply_widget_min_height(app, widget, name);
}

QLocale MostrarMenuContexto::_effective_locale(source::EisenhowerMatrixApp *app)
{
    return MostrarMenuContextoModules::effective_locale(app);
}

void MostrarMenuContexto::_apply_locale_to_dateedit(source::EisenhowerMatrixApp *app, QDateEdit *date_edit)
{
    MostrarMenuContextoModules::apply_locale_to_dateedit(app, date_edit);
}

bool MostrarMenuContexto::_is_completed_list(source::EisenhowerMatrixApp *app, QListWidget *list)
{
    return MostrarMenuContextoModules::is_completed_list(app, list);
}

QStringList MostrarMenuContexto::_quadrant_options(source::EisenhowerMatrixApp *app, bool include_completed)
{
    return MostrarMenuContextoModules::quadrant_options(app, include_completed);
}

QListWidget *MostrarMenuContexto::_target_list_for_quadrant(source::EisenhowerMatrixApp *app,
                                                            int quadrant_index,
                                                            bool keep_completed,
                                                            bool include_completed)
{
    return MostrarMenuContextoModules::target_list_for_quadrant(app, quadrant_index, keep_completed, include_completed);
}

QPair<QListWidget *, QListWidgetItem *> MostrarMenuContexto::_find_source_item_for_calendar(
    source::EisenhowerMatrixApp *app, const QVariantMap &task_dict)
{
    return MostrarMenuContextoModules::find_source_item_for_calendar(app, task_dict);
}

QString MostrarMenuContexto::_base_text_from_item(QListWidgetItem *item)
{
    return MostrarMenuContextoModules::base_text_from_item(item);
}

QPair<QString, QString> MostrarMenuContexto::_build_display_and_tooltip(source::EisenhowerMatrixApp *app,
                                                                        const QString &base_text,
                                                                        const QString &date_iso,
                                                                        const QString &time_str)
{
    return MostrarMenuContextoModules::build_display_and_tooltip(app, base_text, date_iso, time_str);
}

void MostrarMenuContexto::edit_task_dialog(source::EisenhowerMatrixApp *app, QListWidgetItem *item)
{
    MostrarMenuContextoModules::edit_task_dialog(app, item);
}

void MostrarMenuContexto::_move_to_quadrant(source::EisenhowerMatrixApp *app, QListWidgetItem *item, QListWidget *source_list)
{
    MostrarMenuContextoModules::move_to_quadrant(app, item, source_list);
}

void MostrarMenuContexto::mostrar_menu_contexto(source::EisenhowerMatrixApp *app,
                                                const QPoint &point,
                                                QListWidget *list_widget)
{
    MostrarMenuContextoModules::mostrar_menu_contexto(app, point, list_widget);
}

void show_context_menu(source::EisenhowerMatrixApp *app, const QPoint &point, QListWidget *list_widget)
{
    MostrarMenuContexto::mostrar_menu_contexto(app, point, list_widget);
}

QPair<QListWidget *, QListWidgetItem *> _find_source_item_for_calendar(source::EisenhowerMatrixApp *app,
                                                                       const QVariantMap &task_dict)
{
    return MostrarMenuContexto::_find_source_item_for_calendar(app, task_dict);
}

void edit_task_dialog(source::EisenhowerMatrixApp *app, QListWidgetItem *item)
{
    MostrarMenuContexto::edit_task_dialog(app, item);
}

}  // namespace source::InterfaceCore::incore_07_MostrarMenuContexto
