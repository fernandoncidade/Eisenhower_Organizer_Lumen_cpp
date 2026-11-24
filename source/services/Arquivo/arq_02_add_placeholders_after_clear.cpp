#include "source/services/Arquivo/arq_02_add_placeholders_after_clear.hpp"

#include <QCoreApplication>

#include "source/services/incore_09_Arquivo.hpp"
#include "source/eisenhower_app.hpp"

namespace source::InterfaceCore::ArquivoModules {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

}  // namespace

void add_placeholders_after_clear(Arquivo *self)
{
    if (self == nullptr || self->app() == nullptr) {
        return;
    }

    source::EisenhowerMatrixApp *app = self->app();
    app->add_placeholder(app->quadrant1_list, get_text(QStringLiteral("1º Quadrante")));
    app->add_placeholder(app->quadrant2_list, get_text(QStringLiteral("2º Quadrante")));
    app->add_placeholder(app->quadrant3_list, get_text(QStringLiteral("3º Quadrante")));
    app->add_placeholder(app->quadrant4_list, get_text(QStringLiteral("4º Quadrante")));
    app->add_placeholder(app->quadrant1_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
    app->add_placeholder(app->quadrant2_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
    app->add_placeholder(app->quadrant3_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
    app->add_placeholder(app->quadrant4_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
}

}  // namespace source::InterfaceCore::ArquivoModules
