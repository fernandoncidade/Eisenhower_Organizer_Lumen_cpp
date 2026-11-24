#include "source/services/incore_04_definir_idioma.hpp"

#include <QAction>
#include <QCoreApplication>
#include <QMenu>
#include <QMessageBox>

#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"

namespace source::InterfaceCore::incore_04_definir_idioma {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

}  // namespace

bool definir_idioma(source::EisenhowerMatrixApp *app, const QString &codigo_idioma)
{
    QString idioma_nome = codigo_idioma.trimmed();

    if (app == nullptr || app->gerenciador_traducao == nullptr) {
        return false;
    }

    idioma_nome = app->gerenciador_traducao->idiomas_disponiveis().value(codigo_idioma, idioma_nome);
    const bool resultado = app->gerenciador_traducao->definir_idioma(codigo_idioma);

    if (app->idioma_menu != nullptr) {
        const auto actions = app->idioma_menu->actions();
        for (QAction *acao : actions) {
            if (acao != nullptr) {
                acao->setChecked(acao->text() == idioma_nome);
            }
        }
    }

    try {
        if (resultado) {
            QMessageBox::information(
                app,
                get_text(QStringLiteral("Sucesso")),
                (get_text(QStringLiteral("Idioma \"{language}\" aplicado com sucesso!"))
                     .replace(QStringLiteral("{language}"), idioma_nome)));
        } else {
            QMessageBox::warning(
                app,
                get_text(QStringLiteral("Erro")),
                (get_text(QStringLiteral("Não foi possível aplicar o Idioma \"{language}\"!"))
                     .replace(QStringLiteral("{language}"), idioma_nome)));
        }
    } catch (...) {
    }

    return resultado;
}

}  // namespace source::InterfaceCore::incore_04_definir_idioma
