#include "source/services/Arquivo/arq_04_limpar_tudo.hpp"

#include <QCoreApplication>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include "source/services/Arquivo/arq_01_clear_all_lists.hpp"
#include "source/services/Arquivo/arq_02_add_placeholders_after_clear.hpp"
#include "source/services/incore_09_Arquivo.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/ui/ui_09_Calendar.hpp"
#include "source/ui/ui_14_CalendarPanel.hpp"
#include "source/eisenhower_app.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/UnifiedTaskStore.hpp"
#include <QLineEdit>

namespace source::InterfaceCore::ArquivoModules {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

QStringList legacy_task_datastore_paths()
{
    const QString persistent_dir = source::utils::obter_caminho_persistente();
    return {
        QDir(persistent_dir).filePath(QStringLiteral("eisenhower_tarefas.json")),
        QDir(persistent_dir).filePath(QStringLiteral("tasks.json")),
        QDir(persistent_dir).filePath(QStringLiteral("tarefas_pomodoro.json")),
        QDir(persistent_dir).filePath(QStringLiteral("conceitos_feynman.json")),
    };
}

void remove_legacy_task_datastores(const source::EisenhowerMatrixApp *app)
{
    const QString current_tasks_path =
        app != nullptr ? QFileInfo(app->tasks_path).absoluteFilePath() : QString();
    for (const QString &candidate : legacy_task_datastore_paths()) {
        const QString path = QFileInfo(candidate).absoluteFilePath();
        if (path.isEmpty() || path == current_tasks_path) {
            continue;
        }

        QFile::remove(path);
        QFile::remove(path + QStringLiteral(".bak"));
    }
}

}  // namespace

void limpar_tudo(Arquivo *self)
{
    if (self == nullptr || self->app() == nullptr) {
        return;
    }

    source::EisenhowerMatrixApp *app = self->app();
    QMessageBox confirmation(app);
    confirmation.setIcon(QMessageBox::Question);
    confirmation.setWindowTitle(get_text(QStringLiteral("Limpar")));
    confirmation.setText(get_text(QStringLiteral(
        "Deseja remover todos os dados das tarefas dos métodos sincronizados? Esta ação não pode ser desfeita.")));
    confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmation.setDefaultButton(QMessageBox::No);
    if (app->gerenciador_traducao != nullptr) {
        app->gerenciador_traducao->traduzir_botoes_padrao(&confirmation);
    }
    if (confirmation.exec() != QMessageBox::Yes) {
        return;
    }

    clear_all_lists(self);
    add_placeholders_after_clear(self);
    if (app->task_input != nullptr) {
        app->task_input->clear();
    }
    if (app->domain_selector != nullptr) {
        app->domain_selector->setCurrentIndex(0);
    }
    app->clear_pomodoro_tasks_if_available();
    app->clear_feynman_tasks_if_available();
    source::utils::UnifiedTaskStore::save_records(app->tasks_path, {});
    remove_legacy_task_datastores(app);
    app->reload_all_modules_from_shared_store();
}

}  // namespace source::InterfaceCore::ArquivoModules
