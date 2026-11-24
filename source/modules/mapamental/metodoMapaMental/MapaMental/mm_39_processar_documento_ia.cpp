#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_39_processar_documento_ia.hpp"

#include <memory>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_24_gerar_mapa_de_hierarquia.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_08_ProgressoMapaMental.hpp"
#include "source/modules/mapamental/mapamental_host_bridge.hpp"
#include "source/modules/mapamental/mapamental_support.hpp"

namespace source::plugin::MapaMental {

bool MapaMental::processar_documento_com_ia(const QString &caminho, const QString &titulo_janela)
{
    if (caminho.trimmed().isEmpty()) {
        return false;
    }

    ProgressoMapaMental progresso(this);
    progresso.setWindowTitle(titulo_janela);
    progresso.show();
    progresso.atualizar_ia(5, QCoreApplication::translate("MapaMentalPlugin", "Arquivo recebido."));
    progresso.registrar(QCoreApplication::translate("MapaMentalPlugin", "Arquivo: %1")
                            .arg(QDir::toNativeSeparators(QFileInfo(caminho).absoluteFilePath())));

    if (processador_ia_ == nullptr) {
        progresso.atualizar_ia(100,
                               QCoreApplication::translate("MapaMentalPlugin",
                                                           "Processador de IA indisponivel."));
        progresso.configurar_nos(0);
        progresso.finalizar(QCoreApplication::translate("MapaMentalPlugin", "Processamento encerrado."));
        progresso.exec();
        return false;
    }

    progresso.atualizar_ia(25, QCoreApplication::translate("MapaMentalPlugin", "Extraindo texto do arquivo..."));
    const QString texto = processador_ia_->extrair_texto(caminho);
    if (texto.trimmed().isEmpty()) {
        progresso.atualizar_ia(100,
                               QCoreApplication::translate("MapaMentalPlugin",
                                                           "Nao foi possivel extrair texto do arquivo."));
        progresso.configurar_nos(0);
        progresso.finalizar(QCoreApplication::translate("MapaMentalPlugin", "Processamento encerrado."));

        QMessageBox box(&progresso);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle(titulo_janela);
        box.setText(QCoreApplication::translate("MapaMentalPlugin", "Nao foi possivel extrair texto do arquivo."));
        box.setStandardButtons(QMessageBox::Ok);
        traduzir_botoes_padrao(&box);
        box.exec();
        progresso.exec();
        return false;
    }

    progresso.atualizar_ia(45, QCoreApplication::translate("MapaMentalPlugin", "Texto extraido com sucesso."));
    const bool sucesso = processar_texto_com_ia(texto, titulo_janela, &progresso);
    progresso.exec();
    return sucesso;
}

bool MapaMental::processar_texto_com_ia(const QString &texto,
                                        const QString &titulo_janela,
                                        ProgressoMapaMental *progresso)
{
    std::unique_ptr<ProgressoMapaMental> progresso_local;
    if (progresso == nullptr) {
        progresso_local = std::make_unique<ProgressoMapaMental>(this);
        progresso = progresso_local.get();
        progresso->setWindowTitle(titulo_janela);
        progresso->show();
        progresso->atualizar_ia(5, QCoreApplication::translate("MapaMentalPlugin", "Texto recebido."));
    } else if (!progresso->isVisible()) {
        progresso->show();
    }

    if (processador_ia_ == nullptr) {
        progresso->atualizar_ia(100,
                                QCoreApplication::translate("MapaMentalPlugin",
                                                            "Processador de IA indisponivel."));
        progresso->configurar_nos(0);
        progresso->finalizar(QCoreApplication::translate("MapaMentalPlugin", "Processamento encerrado."));
        if (progresso_local != nullptr) {
            progresso->exec();
        }
        return false;
    }

    if (texto.trimmed().isEmpty()) {
        progresso->atualizar_ia(100, QCoreApplication::translate("MapaMentalPlugin", "Texto vazio."));
        progresso->configurar_nos(0);
        progresso->finalizar(QCoreApplication::translate("MapaMentalPlugin", "Processamento encerrado."));
        if (progresso_local != nullptr) {
            progresso->exec();
        }
        return false;
    }

    source::plugin::MapaMentalHostBridge *host_bridge = compat::host_bridge();
    const bool modelo_disponivel = host_bridge != nullptr && host_bridge->model_available();
    const QString method_key = summary_method_key();
    const bool is_ollama = method_key.startsWith(QStringLiteral("ollama:"));
    const bool usar_modelo = modelo_disponivel && (method_key == QStringLiteral("qwen") || is_ollama);
    const QVariantMap diagnostico_modelo =
        modelo_disponivel ? host_bridge->model_diagnostic() : QVariantMap();
    if (usar_modelo) {
        QString model_display = diagnostico_modelo.value(QStringLiteral("modelo")).toString();
        if (is_ollama) {
            model_display = method_key.mid(7);
            progresso->registrar(
                QCoreApplication::translate("MapaMentalPlugin", "Plugin Modelo disponivel: Ollama (%1).")
                    .arg(model_display));
            progresso->atualizar_ia(
                60,
                QCoreApplication::translate(
                    "MapaMentalPlugin",
                    "O modelo Ollama (%1) esta sintetizando a obra em camadas hierarquicas...").arg(model_display));
        } else {
            if (model_display.trimmed().isEmpty()) {
                model_display = QStringLiteral("qwen2.5-3b");
            }
            progresso->registrar(
                QCoreApplication::translate("MapaMentalPlugin", "Plugin Modelo disponivel: %1 (%2).")
                    .arg(model_display,
                         diagnostico_modelo.value(QStringLiteral("provider"), QStringLiteral("qwen")).toString()));
            progresso->atualizar_ia(
                60,
                QCoreApplication::translate(
                    "MapaMentalPlugin",
                    "O modelo Qwen esta sintetizando a obra em camadas hierarquicas..."));
        }
    } else {
        if (method_key.compare(QStringLiteral("qwen"), Qt::CaseInsensitive) == 0 || is_ollama) {
            progresso->registrar(
                QCoreApplication::translate(
                    "MapaMentalPlugin",
                    "O modelo de IA selecionado (%1) nao esta instalado ou disponivel no momento; usando o metodo heuristico.")
                    .arg(is_ollama ? method_key.mid(7) : QStringLiteral("qwen2.5-3b")));
        }
        progresso->atualizar_ia(
            60,
            QCoreApplication::translate(
                "MapaMentalPlugin",
                "Analisando a hierarquia e sintetizando os resumos..."));
    }
    const QVariantMap hierarquia = processador_ia_->analisar_estrutura(texto, usar_modelo);
    if (hierarquia.isEmpty()) {
        progresso->atualizar_ia(100,
                                QCoreApplication::translate("MapaMentalPlugin",
                                                            "A IA nao retornou uma estrutura valida."));
        progresso->configurar_nos(0);
        progresso->finalizar(QCoreApplication::translate("MapaMentalPlugin", "Processamento encerrado."));
        if (progresso_local != nullptr) {
            progresso->exec();
        }
        return false;
    }

    const QString motor_ia = hierarquia.value(QStringLiteral("motor_ia")).toString();
    if (motor_ia == QStringLiteral("modelo.qwen")) {
        progresso->registrar(
            QCoreApplication::translate(
                "MapaMentalPlugin",
                "Sintese Qwen concluida: %1 niveis, %2 nos estruturados e %3 resumos gerados.")
                .arg(hierarquia.value(QStringLiteral("profundidade_hierarquia")).toInt())
                .arg(hierarquia.value(QStringLiteral("nos_estruturados")).toInt())
                .arg(hierarquia.value(QStringLiteral("resumos_gerados")).toInt()));
    } else if (motor_ia.startsWith(QStringLiteral("ollama"))) {
        const QString model_name = motor_ia.startsWith(QStringLiteral("ollama:"))
                                       ? motor_ia.mid(7)
                                       : method_key.mid(7);
        progresso->registrar(
            QCoreApplication::translate(
                "MapaMentalPlugin",
                "Sintese com Ollama (%1) concluida: %2 niveis, %3 nos estruturados e %4 resumos gerados.")
                .arg(model_name)
                .arg(hierarquia.value(QStringLiteral("profundidade_hierarquia")).toInt())
                .arg(hierarquia.value(QStringLiteral("nos_estruturados")).toInt())
                .arg(hierarquia.value(QStringLiteral("resumos_gerados")).toInt()));
    } else {
        if (usar_modelo) {
            progresso->registrar(QCoreApplication::translate(
                "MapaMentalPlugin",
                "O plugin Modelo nao retornou uma estrutura valida; retomando com o metodo heuristico."));
        }
        progresso->registrar(
            QCoreApplication::translate(
                "MapaMentalPlugin",
                "Sintese heuristica concluida: %1 niveis, %2 nos estruturados e %3 resumos gerados.")
                .arg(hierarquia.value(QStringLiteral("profundidade_hierarquia")).toInt())
                .arg(hierarquia.value(QStringLiteral("nos_estruturados")).toInt())
                .arg(hierarquia.value(QStringLiteral("resumos_gerados")).toInt()));
    }

    progresso->atualizar_ia(85, QCoreApplication::translate("MapaMentalPlugin", "Gerando mapa mental..."));
    gerar_mapa_de_hierarquia(hierarquia, progresso);
    progresso->atualizar_ia(100, QCoreApplication::translate("MapaMentalPlugin", "Processamento concluido."));
    progresso->finalizar(QCoreApplication::translate("MapaMentalPlugin", "Mapa mental gerado."));

    if (progresso_local != nullptr) {
        progresso->exec();
    }
    return true;
}

}  // namespace source::plugin::MapaMental
