#include "ginter_13_populate_host_menus.hpp"

#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoInterfaceGrafica/Interface_Grafica.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoInterfaceGrafica/GerenciadorInterfaceGrafica/ginter_12_excluir_item_lista_personalizada.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/BancoDeDados/Banco_Dados.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/CaminhoPersistenteUtils.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/DialogTranslationUtils.hpp"

#include <QAction>
#include <QActionGroup>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSet>

namespace {
    QString titulo_linha_customizada(const InterfaceGerenciadorAtividades::CustomAgendaRow &linha) {
        if (linha.title_label != nullptr && !linha.title_label->text().trimmed().isEmpty()) {
            return linha.title_label->text().trimmed();
        }
        return QString();
    }

    QString nome_arquivo_seguro(const QString &titulo) {
        QString nome = titulo.trimmed();
        nome.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]")), QStringLiteral("_"));
        nome.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
        nome = nome.trimmed();
        if (nome.isEmpty()) {
            nome = QStringLiteral("agenda");
        }
        return nome + QStringLiteral(".json");
    }

    QString caminho_arquivo_agenda_customizada(const QString &titulo) {
        QDir dir(obter_caminho_persistente());
        dir.mkpath(QStringLiteral("agendas"));
        return dir.filePath(QStringLiteral("agendas/") + nome_arquivo_seguro(titulo));
    }

    QStringList extrair_itens_lista_json(const QJsonObject &list_obj) {
        QStringList itens;
        QSet<QString> vistos;
        const auto adicionar = [&](const QString &texto) {
            const QString limpo = texto.trimmed();
            const QString normal = limpo.toCaseFolded();
            if (!limpo.isEmpty() && !vistos.contains(normal)) {
                vistos.insert(normal);
                itens.append(limpo);
            }
        };

        const QJsonArray arr = list_obj.value(QStringLiteral("items")).toArray();
        for (const QJsonValue &v : arr) {
            adicionar(v.toString());
        }

        const QJsonObject nested_map = list_obj.value(QStringLiteral("nested_items")).toObject();
        for (auto it = nested_map.begin(); it != nested_map.end(); ++it) {
            const QJsonArray filhos = it.value().toArray();
            for (const QJsonValue &fv : filhos) {
                adicionar(fv.toString());
            }
        }
        return itens;
    }

    void remover_item_de_arquivo_agenda_customizada(const QString &json_path, int list_index, const QString &item_texto) {
        QFile file(json_path);
        if (!file.open(QIODevice::ReadOnly)) {
            return;
        }
        QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
        file.close();

        QJsonArray lists = root.value(QStringLiteral("lists")).toArray();
        if (list_index < 0 || list_index >= lists.size()) {
            return;
        }

        const QString alvo = item_texto.trimmed().toCaseFolded();
        QStringList removidos;
        removidos << item_texto.trimmed();

        QJsonObject target_list = lists.at(list_index).toObject();
        QJsonArray items_arr = target_list.value(QStringLiteral("items")).toArray();
        QJsonArray new_items;
        for (const QJsonValue &v : items_arr) {
            if (v.toString().trimmed().toCaseFolded() != alvo) {
                new_items.append(v);
            }
        }
        target_list.insert(QStringLiteral("items"), new_items);

        if (target_list.value(QStringLiteral("nested")).toBool(false)) {
            QJsonObject nested_obj = target_list.value(QStringLiteral("nested_items")).toObject();
            for (auto it = nested_obj.begin(); it != nested_obj.end(); ++it) {
                QJsonArray filhos = it.value().toArray();
                QJsonArray new_filhos;
                for (const QJsonValue &fv : filhos) {
                    if (fv.toString().trimmed().toCaseFolded() != alvo) {
                        new_filhos.append(fv);
                    }
                }
                it.value() = new_filhos;
            }
            target_list.insert(QStringLiteral("nested_items"), nested_obj);
        }
        lists[list_index] = target_list;

        for (int child_index = list_index + 1; child_index < lists.size(); ++child_index) {
            QJsonObject child_list = lists.at(child_index).toObject();
            if (child_list.value(QStringLiteral("nested")).toBool(false)) {
                QJsonObject child_nested = child_list.value(QStringLiteral("nested_items")).toObject();
                bool mudou = false;
                for (const QString &rem : removidos) {
                    for (auto it = child_nested.begin(); it != child_nested.end(); ++it) {
                        if (it.key().trimmed().toCaseFolded() == rem.toCaseFolded()) {
                            child_nested.erase(it);
                            mudou = true;
                            break;
                        }
                    }
                }
                if (mudou) {
                    child_list.insert(QStringLiteral("nested_items"), child_nested);
                    lists[child_index] = child_list;
                }
            }
        }

        root.insert(QStringLiteral("lists"), lists);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        }
    }
}

void popular_menu_excluir_item_agenda(InterfaceGerenciadorAtividades *window, QMenu *menu) {
    if (menu == nullptr) {
        return;
    }

    auto tr = [](const char *context, const char *source) { return QCoreApplication::translate(context, source); };

    bool is_custom = false;
    QString custom_title;
    if (window != nullptr) {
        is_custom = window->custom_agenda_mode;
        custom_title = window->current_custom_agenda_title;
    } else {
        QDir dir(obter_caminho_persistente());
        QFile cfg(dir.filePath(QStringLiteral("agenda_config.json")));
        if (!cfg.exists()) {
            cfg.setFileName(dir.filePath(QStringLiteral("config.json")));
        }
        if (cfg.open(QIODevice::ReadOnly)) {
            const QJsonObject obj = QJsonDocument::fromJson(cfg.readAll()).object();
            if (obj.value(QStringLiteral("agenda_ativa_tipo")).toString().trimmed() == QStringLiteral("customizada")) {
                is_custom = true;
                custom_title = obj.value(QStringLiteral("agenda_ativa_titulo")).toString().trimmed();
            }
        }
    }

    const QString current_lang = obter_idioma_atual();
    const QString signature = QStringLiteral("%1|%2|%3")
                                .arg(is_custom ? QStringLiteral("custom") : QStringLiteral("exemplo"),
                                     is_custom ? custom_title : QString(),
                                     current_lang);

    if (menu->property("_agenda_delete_sig").toString() == signature && !menu->actions().isEmpty()) {
        return;
    }
    menu->setProperty("_agenda_delete_sig", signature);
    menu->clear();

    if (is_custom) {
        if (window != nullptr) {
            bool adicionou_titulo = false;
            for (int i = 0; i < window->custom_agenda_rows.size(); ++i) {
                const auto &linha = window->custom_agenda_rows.at(i);
                const QString titulo = titulo_linha_customizada(linha);
                if (titulo.isEmpty()) {
                    continue;
                }

                adicionou_titulo = true;
                QMenu *submenu = menu->addMenu(titulo);
                QObject::connect(submenu, &QMenu::aboutToShow, submenu, [window, submenu, i]() {
                    auto tr = [](const char *context, const char *source) {
                        return QCoreApplication::translate(context, source);
                    };
                    submenu->clear();
                    if (window == nullptr || i < 0 || i >= window->custom_agenda_rows.size()) {
                        return;
                    }

                    const QStringList itens = window->itens_linha_agenda_customizada(i);
                    if (itens.isEmpty()) {
                        QAction *empty = submenu->addAction(
                            tr("InterfaceGerenciadorAtividades", "Nenhum item personalizado encontrado."));
                        empty->setEnabled(false);
                        return;
                    }

                    for (const QString &texto_item_bruto : itens) {
                        const QString texto_item = texto_item_bruto.trimmed();
                        if (texto_item.isEmpty()) {
                            continue;
                        }
                        QAction *action = submenu->addAction(texto_item);
                        QObject::connect(action, &QAction::triggered, window, [window, submenu, i, texto_item]() {
                            if (window == nullptr || i < 0 || i >= window->custom_agenda_rows.size()) {
                                return;
                            }
                            const auto resposta = perguntar_sim_nao(
                                window,
                                QCoreApplication::translate("InterfaceGerenciadorAtividades", "Confirmar exclusão"),
                                QCoreApplication::translate("InterfaceGerenciadorAtividades", "Deseja remover o item selecionado?"));
                            if (resposta != QMessageBox::Yes) {
                                return;
                            }

                            window->remover_item_linha_agenda_customizada(i, texto_item);
                            submenu->clear();
                        });
                    }

                    if (submenu->actions().isEmpty()) {
                        QAction *empty = submenu->addAction(
                            tr("InterfaceGerenciadorAtividades", "Nenhum item personalizado encontrado."));
                        empty->setEnabled(false);
                    }
                });
            }

            if (!adicionou_titulo) {
                QAction *empty =
                    menu->addAction(tr("InterfaceGerenciadorAtividades", "Nenhum título personalizado encontrado."));
                empty->setEnabled(false);
            }
            return;
        } else {
            const QString json_path = caminho_arquivo_agenda_customizada(custom_title);
            QFile file(json_path);
            bool adicionou_titulo = false;
            if (file.open(QIODevice::ReadOnly)) {
                const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
                const QJsonArray lists = root.value(QStringLiteral("lists")).toArray();
                for (int i = 0; i < lists.size(); ++i) {
                    const QJsonObject list_obj = lists.at(i).toObject();
                    const QString titulo = list_obj.value(QStringLiteral("title")).toString().trimmed();
                    if (titulo.isEmpty()) {
                        continue;
                    }
                    adicionou_titulo = true;
                    QMenu *submenu = menu->addMenu(titulo);
                    QObject::connect(submenu, &QMenu::aboutToShow, submenu, [submenu, json_path, i]() {
                        auto tr = [](const char *context, const char *source) {
                            return QCoreApplication::translate(context, source);
                        };
                        submenu->clear();
                        QFile rf(json_path);
                        if (!rf.open(QIODevice::ReadOnly)) {
                            return;
                        }
                        const QJsonObject r_root = QJsonDocument::fromJson(rf.readAll()).object();
                        const QJsonArray r_lists = r_root.value(QStringLiteral("lists")).toArray();
                        if (i < 0 || i >= r_lists.size()) {
                            return;
                        }
                        const QStringList itens = extrair_itens_lista_json(r_lists.at(i).toObject());
                        if (itens.isEmpty()) {
                            QAction *empty = submenu->addAction(
                                tr("InterfaceGerenciadorAtividades", "Nenhum item personalizado encontrado."));
                            empty->setEnabled(false);
                            return;
                        }
                        for (const QString &texto_item : itens) {
                            QAction *action = submenu->addAction(texto_item);
                            QObject::connect(action, &QAction::triggered, submenu, [submenu, json_path, i, texto_item]() {
                                QWidget *parent_widget = submenu != nullptr ? submenu->window() : nullptr;
                                const auto resposta = perguntar_sim_nao(
                                    parent_widget,
                                    QCoreApplication::translate("InterfaceGerenciadorAtividades", "Confirmar exclusão"),
                                    QCoreApplication::translate("InterfaceGerenciadorAtividades", "Deseja remover o item selecionado?"));
                                if (resposta != QMessageBox::Yes) {
                                    return;
                                }
                                remover_item_de_arquivo_agenda_customizada(json_path, i, texto_item);
                                submenu->clear();
                            });
                        }
                    });
                }
            }
            if (!adicionou_titulo) {
                QAction *empty =
                    menu->addAction(tr("InterfaceGerenciadorAtividades", "Nenhum título personalizado encontrado."));
                empty->setEnabled(false);
            }
            return;
        }
    }

    // Standard agenda mode (exemplo):
    for (const auto &categoria : CATEGORIAS_EXCLUSAO_LISTAS) {
        QMenu *submenu = menu->addMenu(tr("InterfaceGerenciadorAtividades", categoria.second.toUtf8().constData()));
        QObject::connect(submenu, &QMenu::aboutToShow, submenu, [window, submenu, codigo = categoria.first]() {
            popular_menu_itens_personalizados(window, submenu, codigo);
        });
    }
}

void InterfaceGerenciadorAtividades::populate_delete_item_menu(QMenu *menu) {
    popular_menu_excluir_item_agenda(menu);
}

void InterfaceGerenciadorAtividades::popular_menu_excluir_item_agenda(QMenu *menu) {
    ::popular_menu_excluir_item_agenda(this, menu);
}

void InterfaceGerenciadorAtividades::populate_color_menu(QMenu *menu) {
    if (menu == nullptr) {
        return;
    }

    auto tr = [](const char *context, const char *source) { return QCoreApplication::translate(context, source); };
    menu->clear();

    const bool is_own_menu = (menu == menu_cores);
    if (is_own_menu) {
        color_actions.clear();
    }

    QMenu *sub_preto = menu->addMenu(tr("InterfaceGerenciadorAtividades", "Preto"));
    QMenu *sub_coloridas = menu->addMenu(tr("InterfaceGerenciadorAtividades", "Coloridas"));

    if (is_own_menu) {
        menu_preto = sub_preto;
        menu_coloridas = sub_coloridas;
    }

    const QString modo_eisenhower = gerenciamento_atividades.obter_modo_cores(QStringLiteral("eisenhower"));
    const QString modo_pomodoro = gerenciamento_atividades.obter_modo_cores(QStringLiteral("pomodoro"));
    const QString modo_feynman = gerenciamento_atividades.obter_modo_cores(QStringLiteral("feynman"));

    const auto setup_action = [&](QMenu *parent_menu, const QString &code, const QString &label, bool checked) {
        QAction *action = parent_menu->addAction(tr("InterfaceGerenciadorAtividades", label.toUtf8().constData()));
        action->setCheckable(true);
        action->setData(code);
        action->setChecked(checked);
        connect(action, &QAction::triggered, this, [this, code] {
            change_color_mode(code);
        });
        if (is_own_menu) {
            color_actions.insert(code, action);
        }
    };

    setup_action(sub_preto, QStringLiteral("preto_eisenhower"), QStringLiteral("Eisenhower"), modo_eisenhower == QStringLiteral("preto"));
    setup_action(sub_preto, QStringLiteral("preto_pomodoro"), QStringLiteral("Pomodoro"), modo_pomodoro == QStringLiteral("preto"));
    setup_action(sub_preto, QStringLiteral("preto_feynman"), QStringLiteral("Feynman"), modo_feynman == QStringLiteral("preto"));
    setup_action(sub_preto, QStringLiteral("preto_geral"), QStringLiteral("Geral"), modo_eisenhower == QStringLiteral("preto") && modo_pomodoro == QStringLiteral("preto") && modo_feynman == QStringLiteral("preto"));

    setup_action(sub_coloridas, QStringLiteral("coloridas_eisenhower"), QStringLiteral("Eisenhower"), modo_eisenhower == QStringLiteral("coloridas"));
    setup_action(sub_coloridas, QStringLiteral("coloridas_pomodoro"), QStringLiteral("Pomodoro"), modo_pomodoro == QStringLiteral("coloridas"));
    setup_action(sub_coloridas, QStringLiteral("coloridas_feynman"), QStringLiteral("Feynman"), modo_feynman == QStringLiteral("coloridas"));
    setup_action(sub_coloridas, QStringLiteral("coloridas_geral"), QStringLiteral("Geral"), modo_eisenhower == QStringLiteral("coloridas") && modo_pomodoro == QStringLiteral("coloridas") && modo_feynman == QStringLiteral("coloridas"));
}
