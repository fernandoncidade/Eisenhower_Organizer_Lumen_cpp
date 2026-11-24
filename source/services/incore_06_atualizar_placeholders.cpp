#include "source/services/incore_06_atualizar_placeholders.hpp"

#include <QCoreApplication>
#include <QListWidget>

#include "source/services/incore_02_add_placeholder.hpp"
#include "source/eisenhower_app.hpp"

namespace source::InterfaceCore::incore_06_atualizar_placeholders {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

}  // namespace

void atualizar_placeholders(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    const QList<QPair<QListWidget *, QString>> placeholders = {
        {app->quadrant1_list, get_text(QStringLiteral("1º Quadrante"))},
        {app->quadrant2_list, get_text(QStringLiteral("2º Quadrante"))},
        {app->quadrant3_list, get_text(QStringLiteral("3º Quadrante"))},
        {app->quadrant4_list, get_text(QStringLiteral("4º Quadrante"))},
        {app->quadrant1_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso"))},
        {app->quadrant2_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso"))},
        {app->quadrant3_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso"))},
        {app->quadrant4_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso"))},
        {app->quadrant1_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída"))},
        {app->quadrant2_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída"))},
        {app->quadrant3_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída"))},
        {app->quadrant4_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída"))},
    };

    for (const auto &entry : placeholders) {
        QListWidget *list = entry.first;
        const QString text = entry.second;
        if (list == nullptr) {
            continue;
        }
        source::InterfaceCore::incore_02_add_placeholder::add_placeholder(app, list, text);
    }
}

}  // namespace source::InterfaceCore::incore_06_atualizar_placeholders
