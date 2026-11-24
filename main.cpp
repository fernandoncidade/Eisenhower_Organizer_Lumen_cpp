#include <QApplication>
#include <QIcon>
#include <stdexcept>

#include "source/eisenhower_app.hpp"
#include "source/language/tr_01_gerenciadorTraducao.hpp"
#include "source/utils/AppConfig.hpp"
#include "source/utils/FontUtils.hpp"
#include "source/utils/IconUtils.hpp"
#include "source/utils/LogManager.hpp"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
    source::utils::AppConfig::apply_runtime_build_target_to_environment();
    source::utils::configure_emoji_flat_rendering();

    QApplication app(argc, argv);
    auto *logger = source::utils::LogManager::get_logger();
    source::utils::LogManager::install_qt_message_handler();

    int final_exit_code = 0;

    try {
        logger->info(QStringLiteral("Inicializando interface principal."));

        const QString icon_path = source::utils::get_default_window_icon_path();
        if (!icon_path.isEmpty()) {
            logger->debug(QStringLiteral("Aplicando ícone da janela principal: %1").arg(icon_path));
            app.setWindowIcon(QIcon(icon_path));
        }

        source::language::GerenciadorTraducao gerenciador_traducao;
        logger->debug(QStringLiteral("Aplicando tradução inicial."));
        gerenciador_traducao.aplicar_traducao();

        logger->debug(QStringLiteral("Criando janela principal."));
        source::EisenhowerMatrixApp window(&gerenciador_traducao);
        logger->debug(QStringLiteral("Janela principal criada; exibindo janela."));
        window.show();
        logger->info(QStringLiteral("Janela principal exibida; entrando no loop de eventos."));

        final_exit_code = app.exec();
        logger->debug(QStringLiteral("Aplicação encerrada com código de saída: %1").arg(final_exit_code));
        source::utils::LogManager::uninstall_qt_message_handler();

    } catch (const std::exception &e) {
        logger->critical(QStringLiteral("Erro fatal (std::exception) ao iniciar aplicação: %1").arg(e.what()));
        source::utils::LogManager::uninstall_qt_message_handler();
        return 1;
    } catch (...) {
        logger->critical(QStringLiteral("Erro fatal desconhecido ao iniciar aplicação."));
        source::utils::LogManager::uninstall_qt_message_handler();
        return 1;
    }

    return final_exit_code;
}
