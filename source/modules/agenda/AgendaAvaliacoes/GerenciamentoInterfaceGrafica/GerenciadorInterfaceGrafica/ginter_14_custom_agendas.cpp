#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoInterfaceGrafica/Interface_Grafica.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/BancoDeDados/Banco_Dados.hpp"

#include "source/modules/agenda/AgendaAvaliacoes/utils/CaminhoPersistenteUtils.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/ComboBoxUtils.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/DialogTranslationUtils.hpp"
#include "source/utils/LogManager.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSignalBlocker>

namespace {
    QString tr_agenda_custom(const char *source) {
        return QCoreApplication::translate("InterfaceGerenciadorAtividades", source);
    }

    QString titulo_normalizado(const QString &titulo) {
        return titulo.trimmed().toCaseFolded();
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

    QDir diretorio_agendas_customizadas() {
        QDir dir(obter_caminho_persistente());
        dir.mkpath(QStringLiteral("agendas"));
        dir.cd(QStringLiteral("agendas"));
        return dir;
    }

    QString caminho_config_agenda() {
        QDir dir(obter_caminho_persistente());
        dir.mkpath(QStringLiteral("."));
        return dir.filePath(QStringLiteral("agenda_config.json"));
    }

    QString caminho_config_agenda_legado() {
        QDir dir(obter_caminho_persistente());
        dir.mkpath(QStringLiteral("."));
        return dir.filePath(QStringLiteral("config.json"));
    }

    QJsonObject carregar_config_agenda() {
        QFile file(caminho_config_agenda());
        if (!file.exists()) {
            file.setFileName(caminho_config_agenda_legado());
        }
        if (!file.open(QIODevice::ReadOnly)) {
            return QJsonObject();
        }

        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        return document.isObject() ? document.object() : QJsonObject();
    }

    void salvar_config_agenda(const QJsonObject &config) {
        QFile file(caminho_config_agenda());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            source::utils::LogManager::get_logger()->error(QStringLiteral("Erro ao salvar agenda_config.json: %1").arg(file.errorString()));
            return;
        }
        file.write(QJsonDocument(config).toJson(QJsonDocument::Indented));
    }

    bool contem_titulo(const QStringList &titulos, const QString &titulo) {
        const QString alvo = titulo_normalizado(titulo);
        for (const QString &existente : titulos) {
            if (titulo_normalizado(existente) == alvo) {
                return true;
            }
        }
        return false;
    }

    QString titulo_correspondente(const QStringList &titulos, const QString &titulo) {
        const QString alvo = titulo_normalizado(titulo);
        for (const QString &existente : titulos) {
            if (titulo_normalizado(existente) == alvo) {
                return existente;
            }
        }
        return QString();
    }

    QString titulo_linha_customizada(const InterfaceGerenciadorAtividades::CustomAgendaRow &linha) {
        if (linha.title_label != nullptr && !linha.title_label->text().trimmed().isEmpty()) {
            return linha.title_label->text().trimmed();
        }
        return QString();
    }

    QString texto_normalizado(const QString &texto) {
        return texto.trimmed().toCaseFolded();
    }

    bool contem_texto(const QStringList &lista, const QString &texto) {
        const QString alvo = texto_normalizado(texto);
        if (alvo.isEmpty()) {
            return false;
        }
        for (const QString &item : lista) {
            if (texto_normalizado(item) == alvo) {
                return true;
            }
        }
        return false;
    }

    void adicionar_texto_unico(QStringList &lista, const QString &texto) {
        const QString limpo = texto.trimmed();
        if (!limpo.isEmpty() && !contem_texto(lista, limpo)) {
            lista << limpo;
        }
    }

    void remover_texto(QStringList &lista, const QString &texto) {
        const QString alvo = texto_normalizado(texto);
        for (int i = lista.size() - 1; i >= 0; --i) {
            if (texto_normalizado(lista.at(i)) == alvo) {
                lista.removeAt(i);
            }
        }
    }

    QString chave_mapa_aninhado(const QHash<QString, QStringList> &mapa, const QString &pai) {
        const QString alvo = texto_normalizado(pai);
        for (auto it = mapa.cbegin(); it != mapa.cend(); ++it) {
            if (texto_normalizado(it.key()) == alvo) {
                return it.key();
            }
        }
        return pai.trimmed();
    }

    QStringList lista_de_json_array(const QJsonArray &array) {
        QStringList itens;
        for (const QJsonValue &value : array) {
            adicionar_texto_unico(itens, value.toString());
        }
        return itens;
    }

    QJsonArray json_array_de_lista(const QStringList &lista) {
        QJsonArray array;
        QStringList adicionados;
        for (const QString &item : lista) {
            const QString limpo = item.trimmed();
            if (!limpo.isEmpty() && !contem_texto(adicionados, limpo)) {
                array.append(limpo);
                adicionados << limpo;
            }
        }
        return array;
    }

    QStringList itens_mapa_aninhado(const QHash<QString, QStringList> &mapa) {
        QStringList itens;
        for (auto it = mapa.cbegin(); it != mapa.cend(); ++it) {
            for (const QString &item : it.value()) {
                adicionar_texto_unico(itens, item);
            }
        }
        return itens;
    }

    QStringList itens_linha_customizada(const InterfaceGerenciadorAtividades::CustomAgendaRow &linha) {
        if (linha.nested) {
            QStringList itens = itens_mapa_aninhado(linha.nested_items);
            if (linha.combo != nullptr) {
                for (const QString &item : obter_itens_combo(linha.combo)) {
                    adicionar_texto_unico(itens, item);
                }
            }
            return itens;
        }
        return linha.combo != nullptr ? obter_itens_combo(linha.combo) : QStringList();
    }

    void substituir_itens_combo_customizado(QComboBox *combo, const QStringList &itens, const QString &texto_atual) {
        if (combo == nullptr) {
            return;
        }
        const QString atual = texto_atual.trimmed();
        const QSignalBlocker blocker(combo);
        combo->clear();
        combo->addItems(itens);
        if (!atual.isEmpty() && contem_texto(itens, atual)) {
            combo->setCurrentText(atual);
            return;
        }
        combo->setCurrentIndex(-1);
        if (combo->isEditable()) {
            combo->clearEditText();
        }
    }

    int indice_titulo_anterior(const QList<InterfaceGerenciadorAtividades::CustomAgendaRow> &linhas,
                               int index,
                               const QString &titulo) {
        for (int i = 0; i < index && i < linhas.size(); ++i) {
            if (titulo_normalizado(titulo_linha_customizada(linhas.at(i))) == titulo_normalizado(titulo)) {
                return i;
            }
        }
        return -1;
    }

    void limpar_linhas_customizadas(QList<InterfaceGerenciadorAtividades::CustomAgendaRow> &linhas) {
        for (auto &linha : linhas) {
            linha.nested = false;
            linha.parent_index = -1;
            linha.nested_items.clear();
            if (linha.enabled != nullptr) {
                const QSignalBlocker blocker(linha.enabled);
                linha.enabled->setChecked(false);
                linha.enabled->setEnabled(true);
                linha.enabled->setVisible(true);
            }
            if (linha.title_edit != nullptr) {
                const QSignalBlocker blocker(linha.title_edit);
                linha.title_edit->clear();
                linha.title_edit->setEnabled(false);
                linha.title_edit->setVisible(false);
            }
            if (linha.title_label != nullptr) {
                const QSignalBlocker blocker(linha.title_label);
                linha.title_label->clear();
                linha.title_label->setVisible(false);
            }
            if (linha.combo != nullptr) {
                const QSignalBlocker blocker(linha.combo);
                linha.combo->clear();
                linha.combo->clearEditText();
                linha.combo->setEnabled(false);
                linha.combo->setVisible(false);
            }
            if (linha.widget != nullptr) {
                linha.widget->setVisible(false);
            }
        }
    }
} // namespace

QString InterfaceGerenciadorAtividades::caminho_arquivo_agenda_customizada(const QString &titulo) const {
    return diretorio_agendas_customizadas().filePath(nome_arquivo_seguro(titulo));
}

QStringList InterfaceGerenciadorAtividades::itens_linha_agenda_customizada(int index) const {
    if (index < 0 || index >= custom_agenda_rows.size()) {
        return QStringList();
    }
    return itens_linha_customizada(custom_agenda_rows.at(index));
}

void InterfaceGerenciadorAtividades::atualizar_dependencias_agenda_customizada(int parent_index) {
    if (parent_index < 0 || parent_index >= custom_agenda_rows.size()) {
        return;
    }

    for (int i = 0; i < custom_agenda_rows.size(); ++i) {
        CustomAgendaRow &linha = custom_agenda_rows[i];
        if (!linha.nested || linha.parent_index != parent_index || linha.combo == nullptr) {
            continue;
        }

        const CustomAgendaRow &linha_pai = custom_agenda_rows.at(parent_index);
        const QString pai_atual = linha_pai.combo != nullptr ? linha_pai.combo->currentText().trimmed() : QString();
        const QString atual = linha.combo->currentText().trimmed();
        QStringList itens;
        if (!pai_atual.isEmpty()) {
            const QString chave = chave_mapa_aninhado(linha.nested_items, pai_atual);
            itens = linha.nested_items.value(chave);
        }
        substituir_itens_combo_customizado(linha.combo, itens, atual);
        atualizar_dependencias_agenda_customizada(i);
    }
}

void InterfaceGerenciadorAtividades::registrar_item_linha_agenda_customizada(int index) {
    if (updating_custom_agenda || index < 0 || index >= custom_agenda_rows.size()) {
        return;
    }

    CustomAgendaRow &linha = custom_agenda_rows[index];
    if (linha.combo == nullptr) {
        return;
    }

    const QString item = linha.combo->currentText().trimmed();
    if (item.isEmpty()) {
        return;
    }

    if (!linha.nested) {
        adicionar_item_combo_se_ausente(linha.combo, item);
        atualizar_dependencias_agenda_customizada(index);
        salvar_agenda_customizada_atual();
        return;
    }

    const int parent_index = linha.parent_index;
    if (parent_index < 0 || parent_index >= custom_agenda_rows.size()) {
        adicionar_item_combo_se_ausente(linha.combo, item);
        salvar_agenda_customizada_atual();
        return;
    }

    if (contem_texto(obter_itens_combo(linha.combo), item)) {
        atualizar_dependencias_agenda_customizada(index);
        salvar_agenda_customizada_atual();
        return;
    }

    CustomAgendaRow &linha_pai = custom_agenda_rows[parent_index];
    QStringList itens_pai = linha_pai.combo != nullptr ? obter_itens_combo(linha_pai.combo) : QStringList();
    const QString pai_atual = linha_pai.combo != nullptr ? linha_pai.combo->currentText().trimmed() : QString();
    if (!pai_atual.isEmpty()) {
        adicionar_texto_unico(itens_pai, pai_atual);
    }

    if (itens_pai.isEmpty()) {
        QMessageBox::warning(this,
                             tr_agenda_custom("Lista anterior vazia"),
                             tr_agenda_custom("Adicione ao menos um item na lista anterior antes de aninhar."));
        return;
    }

    const int indice_padrao = qMax(0, itens_pai.indexOf(pai_atual));
    bool ok = false;
    const QString pai_escolhido = obter_item_dialogo(
                                      this,
                                      tr_agenda_custom("Aninhar item"),
                                      tr_agenda_custom("Selecione o item da lista anterior ao qual este item pertence:"),
                                      itens_pai,
                                      indice_padrao,
                                      false,
                                      &ok)
                                      .trimmed();
    if (!ok || pai_escolhido.isEmpty()) {
        return;
    }

    QString chave = chave_mapa_aninhado(linha.nested_items, pai_escolhido);
    QStringList filhos = linha.nested_items.value(chave);
    adicionar_texto_unico(filhos, item);
    linha.nested_items.insert(chave, filhos);

    if (linha_pai.combo != nullptr && texto_normalizado(linha_pai.combo->currentText()) != texto_normalizado(pai_escolhido)) {
        linha_pai.combo->setCurrentText(pai_escolhido);
    }
    substituir_itens_combo_customizado(linha.combo, filhos, item);
    atualizar_dependencias_agenda_customizada(index);
    salvar_agenda_customizada_atual();
}

void InterfaceGerenciadorAtividades::remover_item_linha_agenda_customizada(int index, const QString &texto) {
    if (index < 0 || index >= custom_agenda_rows.size()) {
        return;
    }

    const QString item = texto.trimmed();
    if (item.isEmpty()) {
        return;
    }

    CustomAgendaRow &linha = custom_agenda_rows[index];
    QStringList removidos;
    adicionar_texto_unico(removidos, item);

    if (linha.nested) {
        for (auto it = linha.nested_items.begin(); it != linha.nested_items.end(); ++it) {
            QStringList filhos = it.value();
            remover_texto(filhos, item);
            it.value() = filhos;
        }
    }

    if (linha.combo != nullptr) {
        for (int combo_index = linha.combo->count() - 1; combo_index >= 0; --combo_index) {
            if (texto_normalizado(linha.combo->itemText(combo_index)) == texto_normalizado(item)) {
                linha.combo->removeItem(combo_index);
            }
        }
        if (texto_normalizado(linha.combo->currentText()) == texto_normalizado(item)) {
            linha.combo->setCurrentIndex(-1);
            if (linha.combo->isEditable()) {
                linha.combo->clearEditText();
            }
        }
    }

    for (int child_index = 0; child_index < custom_agenda_rows.size(); ++child_index) {
        CustomAgendaRow &filha = custom_agenda_rows[child_index];
        if (!filha.nested || filha.parent_index != index) {
            continue;
        }

        QStringList filhos_removidos;
        for (const QString &removido : removidos) {
            const QString chave = chave_mapa_aninhado(filha.nested_items, removido);
            const QStringList filhos = filha.nested_items.take(chave);
            for (const QString &filho : filhos) {
                adicionar_texto_unico(filhos_removidos, filho);
            }
        }
        for (const QString &filho : filhos_removidos) {
            remover_item_linha_agenda_customizada(child_index, filho);
        }
    }

    atualizar_dependencias_agenda_customizada(index);
    salvar_agenda_customizada_atual();
}

void InterfaceGerenciadorAtividades::carregar_agendas_customizadas() {
    custom_agenda_titles.clear();

    const QDir dir = diretorio_agendas_customizadas();
    const QStringList arquivos = dir.entryList({QStringLiteral("*.json")}, QDir::Files, QDir::Name);
    for (const QString &arquivo : arquivos) {
        QFile file(dir.filePath(arquivo));
        QString titulo = QFileInfo(arquivo).completeBaseName();
        if (file.open(QIODevice::ReadOnly)) {
            const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
            const QString titulo_json = document.object().value(QStringLiteral("title")).toString().trimmed();
            if (!titulo_json.isEmpty()) {
                titulo = titulo_json;
            }
        }
        if (!titulo.trimmed().isEmpty() && !contem_titulo(custom_agenda_titles, titulo)) {
            custom_agenda_titles << titulo.trimmed();
        }
    }
}

void InterfaceGerenciadorAtividades::salvar_ultima_agenda_ativa(const QString &tipo, const QString &titulo) const {
    QJsonObject config = carregar_config_agenda();
    const QString tipo_atual = config.value(QStringLiteral("agenda_ativa_tipo")).toString().trimmed();
    const QString titulo_atual = config.value(QStringLiteral("agenda_ativa_titulo")).toString().trimmed();
    const QString novo_titulo = (tipo == QStringLiteral("customizada")) ? titulo.trimmed() : QString();
    if (tipo_atual == tipo && titulo_atual == novo_titulo) {
        return;
    }
    config.insert(QStringLiteral("agenda_ativa_tipo"), tipo);
    if (!novo_titulo.isEmpty()) {
        config.insert(QStringLiteral("agenda_ativa_titulo"), novo_titulo);
    } else {
        config.remove(QStringLiteral("agenda_ativa_titulo"));
    }
    salvar_config_agenda(config);
}

void InterfaceGerenciadorAtividades::carregar_ultima_agenda_ativa() {
    carregar_agendas_customizadas();

    const QJsonObject config = carregar_config_agenda();
    const QString tipo = config.value(QStringLiteral("agenda_ativa_tipo")).toString().trimmed();
    const QString titulo = config.value(QStringLiteral("agenda_ativa_titulo")).toString().trimmed();

    if (tipo == QStringLiteral("customizada") && !titulo.isEmpty()) {
        const QString titulo_existente = titulo_correspondente(custom_agenda_titles, titulo);
        if (!titulo_existente.isEmpty() && QFile::exists(caminho_arquivo_agenda_customizada(titulo_existente))) {
            selecionar_agenda_customizada(titulo_existente);
            return;
        }
    }

    selecionar_agenda_exemplo();
}

void InterfaceGerenciadorAtividades::reconstruir_menus_agendas_customizadas() {
    if (menu_config == nullptr) {
        return;
    }

    for (auto it = custom_agenda_menus.begin(); it != custom_agenda_menus.end(); ++it) {
        if (it.value() != nullptr) {
            menu_config->removeAction(it.value()->menuAction());
            it.value()->deleteLater();
        }
    }
    custom_agenda_menus.clear();

    QAction *before = menu_idiomas != nullptr ? menu_idiomas->menuAction() : nullptr;
    for (const QString &titulo : custom_agenda_titles) {
        QMenu *menu = new QMenu(titulo, menu_config);
        QAction *abrir = menu->addAction(tr_agenda_custom("Abrir agenda"));
        QAction *editar = menu->addAction(tr_agenda_custom("Editar agenda"));
        QAction *remover = menu->addAction(tr_agenda_custom("Remover agenda"));
        menu->addSeparator();
        QAction *editar_titulo = menu->addAction(tr_agenda_custom("Editar título"));
        QAction *remover_titulo = menu->addAction(tr_agenda_custom("Remover título"));

        connect(abrir, &QAction::triggered, this, [this, titulo] { selecionar_agenda_customizada(titulo); });
        connect(editar, &QAction::triggered, this, [this, titulo] { editar_agenda_customizada(titulo); });
        connect(remover, &QAction::triggered, this, [this, titulo] { remover_agenda_customizada(titulo); });
        connect(editar_titulo, &QAction::triggered, this, [this, titulo] { editar_titulo_agenda_customizada(titulo); });
        connect(remover_titulo, &QAction::triggered, this, [this, titulo] { remover_titulo_agenda_customizada(titulo); });

        if (before != nullptr) {
            menu_config->insertMenu(before, menu);
        } else {
            menu_config->addMenu(menu);
        }
        custom_agenda_menus.insert(titulo, menu);
    }
}

void InterfaceGerenciadorAtividades::selecionar_agenda_exemplo() {
    custom_agenda_mode = false;
    current_custom_agenda_title.clear();
    if (agenda_stack != nullptr && agenda_exemplo_widget != nullptr) {
        agenda_stack->setCurrentWidget(agenda_exemplo_widget);
    }
    salvar_ultima_agenda_ativa(QStringLiteral("exemplo"));
    adjustSize();
}

void InterfaceGerenciadorAtividades::criar_agenda_customizada_do_menu() {
    if (line_novo_titulo_agenda == nullptr) {
        return;
    }

    const QString titulo = line_novo_titulo_agenda->text().trimmed();
    if (titulo.isEmpty()) {
        return;
    }

    line_novo_titulo_agenda->clear();
    if (!contem_titulo(custom_agenda_titles, titulo)) {
        custom_agenda_titles << titulo;
        salvar_agenda_customizada_atual();

        QFile file(caminho_arquivo_agenda_customizada(titulo));
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QJsonObject root;
            root.insert(QStringLiteral("title"), titulo);
            root.insert(QStringLiteral("lists"), QJsonArray());
            file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        }
    }

    reconstruir_menus_agendas_customizadas();
}

void InterfaceGerenciadorAtividades::selecionar_agenda_customizada(const QString &titulo) {
    const QString titulo_limpo = titulo.trimmed();
    if (titulo_limpo.isEmpty()) {
        return;
    }

    if (!contem_titulo(custom_agenda_titles, titulo_limpo)) {
        custom_agenda_titles << titulo_limpo;
    }

    updating_custom_agenda = true;
    custom_agenda_mode = true;
    current_custom_agenda_title = titulo_limpo;
    if (agenda_stack != nullptr && agenda_custom_widget != nullptr) {
        agenda_stack->setCurrentWidget(agenda_custom_widget);
    }
    limpar_linhas_customizadas(custom_agenda_rows);

    QFile file(caminho_arquivo_agenda_customizada(titulo_limpo));
    if (file.open(QIODevice::ReadOnly)) {
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        aplicar_listas_agenda_customizada(document.object().value(QStringLiteral("lists")).toArray());
    }

    updating_custom_agenda = false;
    atualizar_visibilidade_linhas_customizadas();
    salvar_ultima_agenda_ativa(QStringLiteral("customizada"), titulo_limpo);
    adjustSize();
}

void InterfaceGerenciadorAtividades::aplicar_listas_agenda_customizada(const QJsonArray &listas) {
    const bool estado_anterior = updating_custom_agenda;
    updating_custom_agenda = true;
    limpar_linhas_customizadas(custom_agenda_rows);

    int linha_destino = 0;
    for (const QJsonValue &value : listas) {
        if (linha_destino >= custom_agenda_rows.size()) {
            break;
        }

        const QJsonObject object = value.toObject();
        const QString titulo_linha = object.value(QStringLiteral("title")).toString().trimmed();
        if (titulo_linha.isEmpty()) {
            continue;
        }

        const int index_linha = linha_destino++;
        CustomAgendaRow &linha = custom_agenda_rows[index_linha];
        if (linha.enabled != nullptr) {
            const QSignalBlocker blocker(linha.enabled);
            linha.enabled->setChecked(true);
            linha.enabled->setEnabled(false);
            linha.enabled->setVisible(false);
        }
        if (linha.title_edit != nullptr) {
            const QSignalBlocker blocker(linha.title_edit);
            linha.title_edit->setText(titulo_linha);
            linha.title_edit->setEnabled(false);
            linha.title_edit->setVisible(false);
        }
        if (linha.title_label != nullptr) {
            linha.title_label->setText(titulo_linha);
            linha.title_label->setVisible(true);
        }
        if (linha.combo != nullptr) {
            const QSignalBlocker blocker(linha.combo);
            linha.combo->clear();
            linha.nested = object.value(QStringLiteral("nested")).toBool(false);
            linha.parent_index = -1;
            linha.nested_items.clear();

            const QStringList flat_items = lista_de_json_array(object.value(QStringLiteral("items")).toArray());
            if (linha.nested) {
                int parent_index = object.value(QStringLiteral("parent_index")).toInt(-1);
                if (parent_index < 0 || parent_index >= index_linha) {
                    parent_index = indice_titulo_anterior(custom_agenda_rows,
                                                          index_linha,
                                                          object.value(QStringLiteral("parent_title")).toString());
                }
                if (parent_index < 0 && index_linha > 0) {
                    parent_index = index_linha - 1;
                }
                linha.parent_index = parent_index;

                QJsonObject children = object.value(QStringLiteral("children")).toObject();
                if (children.isEmpty()) {
                    children = object.value(QStringLiteral("dictionary")).toObject();
                }
                for (auto it = children.constBegin(); it != children.constEnd(); ++it) {
                    const QString pai = it.key().trimmed();
                    const QStringList filhos = lista_de_json_array(it.value().toArray());
                    if (!pai.isEmpty() && !filhos.isEmpty()) {
                        linha.nested_items.insert(pai, filhos);
                    }
                }

                if (linha.nested_items.isEmpty() && !flat_items.isEmpty() && parent_index >= 0 &&
                    parent_index < custom_agenda_rows.size()) {
                    const CustomAgendaRow &linha_pai = custom_agenda_rows.at(parent_index);
                    QString pai = linha_pai.combo != nullptr ? linha_pai.combo->currentText().trimmed() : QString();
                    if (pai.isEmpty()) {
                        const QStringList itens_pai = itens_linha_customizada(linha_pai);
                        pai = itens_pai.value(0).trimmed();
                    }
                    if (!pai.isEmpty()) {
                        linha.nested_items.insert(pai, flat_items);
                    }
                }

                const QString pai_atual =
                    parent_index >= 0 && parent_index < custom_agenda_rows.size() &&
                            custom_agenda_rows.at(parent_index).combo != nullptr
                        ? custom_agenda_rows.at(parent_index).combo->currentText().trimmed()
                        : QString();
                if (!pai_atual.isEmpty()) {
                    const QString chave = chave_mapa_aninhado(linha.nested_items, pai_atual);
                    linha.combo->addItems(linha.nested_items.value(chave));
                }
            } else {
                for (const QString &item : flat_items) {
                    linha.combo->addItem(item);
                }
            }
            const QString current = object.value(QStringLiteral("current")).toString().trimmed();
            if (!current.isEmpty() && (!linha.nested || contem_texto(obter_itens_combo(linha.combo), current))) {
                adicionar_item_combo_se_ausente(linha.combo, current);
            } else {
                linha.combo->setCurrentIndex(-1);
                linha.combo->clearEditText();
            }
            linha.combo->setEnabled(true);
            linha.combo->setVisible(true);
        }
    }

    for (int i = 0; i < custom_agenda_rows.size(); ++i) {
        atualizar_dependencias_agenda_customizada(i);
    }

    updating_custom_agenda = estado_anterior;
    atualizar_visibilidade_linhas_customizadas();
}

void InterfaceGerenciadorAtividades::atualizar_visibilidade_linhas_customizadas() {
    bool mostrar_proxima_linha = true;
    for (int i = 0; i < custom_agenda_rows.size(); ++i) {
        CustomAgendaRow &linha = custom_agenda_rows[i];
        const QString titulo = titulo_linha_customizada(linha);
        const bool titulo_confirmado = !titulo.isEmpty();
        const bool editando_titulo = !titulo_confirmado && linha.enabled != nullptr && linha.enabled->isChecked();
        const bool mostrar_linha = titulo_confirmado || mostrar_proxima_linha;

        if (linha.widget != nullptr) {
            linha.widget->setVisible(mostrar_linha);
        }
        if (linha.enabled != nullptr) {
            const QSignalBlocker blocker(linha.enabled);
            linha.enabled->setChecked(titulo_confirmado || editando_titulo);
            linha.enabled->setEnabled(mostrar_linha && !titulo_confirmado);
            linha.enabled->setVisible(mostrar_linha && !titulo_confirmado);
        }
        if (linha.title_edit != nullptr) {
            linha.title_edit->setVisible(mostrar_linha && !titulo_confirmado);
            linha.title_edit->setEnabled(mostrar_linha && editando_titulo);
        }
        if (linha.title_label != nullptr) {
            linha.title_label->setVisible(mostrar_linha && titulo_confirmado);
        }
        if (linha.combo != nullptr) {
            linha.combo->setVisible(mostrar_linha && titulo_confirmado);
            linha.combo->setEnabled(mostrar_linha && titulo_confirmado);
        }

        if (mostrar_linha && !titulo_confirmado) {
            mostrar_proxima_linha = false;
        } else if (!titulo_confirmado) {
            if (linha.enabled != nullptr) {
                const QSignalBlocker blocker(linha.enabled);
                linha.enabled->setChecked(false);
                linha.enabled->setEnabled(true);
                linha.enabled->setVisible(false);
            }
            if (linha.title_edit != nullptr) {
                linha.title_edit->setVisible(false);
                linha.title_edit->setEnabled(false);
            }
            if (linha.combo != nullptr) {
                linha.combo->setVisible(false);
                linha.combo->setEnabled(false);
            }
        }
    }

    if (painel_esquerdo_apenas) {
        adjustSize();
    }
}

void InterfaceGerenciadorAtividades::editar_agenda_customizada(const QString &titulo) {
    const QString atual = titulo.trimmed();
    if (atual.isEmpty()) {
        return;
    }

    bool ok = false;
    const QString novo = obter_texto_dialogo(this,
                                             tr_agenda_custom("Editar agenda"),
                                             tr_agenda_custom("Novo título para sua agenda"),
                                             atual,
                                             &ok)
                             .trimmed();
    if (!ok || novo.isEmpty() || titulo_normalizado(novo) == titulo_normalizado(atual)) {
        return;
    }
    if (contem_titulo(custom_agenda_titles, novo)) {
        QMessageBox::warning(this,
                             tr_agenda_custom("Agenda existente"),
                             tr_agenda_custom("Já existe uma agenda com este título."));
        return;
    }

    const QString antigo_path = caminho_arquivo_agenda_customizada(atual);
    const QString novo_path = caminho_arquivo_agenda_customizada(novo);
    if (QFile::exists(antigo_path) && !QFile::rename(antigo_path, novo_path)) {
        QMessageBox::warning(this,
                             tr_agenda_custom("Erro"),
                             tr_agenda_custom("Não foi possível renomear a agenda."));
        return;
    }

    for (QString &item : custom_agenda_titles) {
        if (titulo_normalizado(item) == titulo_normalizado(atual)) {
            item = novo;
            break;
        }
    }
    const bool agenda_ativa = custom_agenda_mode && titulo_normalizado(current_custom_agenda_title) == titulo_normalizado(atual);
    if (agenda_ativa) {
        current_custom_agenda_title = novo;
        selecionar_agenda_customizada(novo);
        salvar_agenda_customizada_atual();
    } else {
        QFile file(novo_path);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
            file.close();
            root.insert(QStringLiteral("title"), novo);
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
            }
        }
    }
    reconstruir_menus_agendas_customizadas();
}

void InterfaceGerenciadorAtividades::remover_agenda_customizada(const QString &titulo) {
    const QString titulo_limpo = titulo.trimmed();
    if (titulo_limpo.isEmpty()) {
        return;
    }

    const auto resposta = perguntar_sim_nao(this,
                                            tr_agenda_custom("Remover agenda"),
                                            tr_agenda_custom("Deseja remover a agenda selecionada?"));
    if (resposta != QMessageBox::Yes) {
        return;
    }

    QFile::remove(caminho_arquivo_agenda_customizada(titulo_limpo));
    for (int i = custom_agenda_titles.size() - 1; i >= 0; --i) {
        if (titulo_normalizado(custom_agenda_titles.at(i)) == titulo_normalizado(titulo_limpo)) {
            custom_agenda_titles.removeAt(i);
        }
    }

    reconstruir_menus_agendas_customizadas();
    if (titulo_normalizado(current_custom_agenda_title) == titulo_normalizado(titulo_limpo)) {
        selecionar_agenda_exemplo();
    }
}

void InterfaceGerenciadorAtividades::confirmar_titulo_linha_customizada(int index) {
    if (index < 0 || index >= custom_agenda_rows.size()) {
        return;
    }

    CustomAgendaRow &linha = custom_agenda_rows[index];
    const QString titulo = linha.title_edit != nullptr ? linha.title_edit->text().trimmed() : QString();
    if (titulo.isEmpty()) {
        return;
    }

    for (int i = 0; i < custom_agenda_rows.size(); ++i) {
        if (i == index) {
            continue;
        }
        if (titulo_normalizado(titulo_linha_customizada(custom_agenda_rows.at(i))) == titulo_normalizado(titulo)) {
            QMessageBox::warning(this,
                                 tr_agenda_custom("Título existente"),
                                 tr_agenda_custom("Já existe uma lista com este título."));
            return;
        }
    }

    linha.nested = false;
    linha.parent_index = -1;
    linha.nested_items.clear();
    if (index > 0) {
        const int parent_index = index - 1;
        const QString titulo_pai = titulo_linha_customizada(custom_agenda_rows.at(parent_index));
        const QStringList itens_pai = itens_linha_agenda_customizada(parent_index);
        if (!titulo_pai.isEmpty() && !itens_pai.isEmpty()) {
            const auto resposta = perguntar_sim_nao(
                this,
                tr_agenda_custom("Aninhar lista"),
                tr_agenda_custom("Deseja aninhar os itens desta lista aos itens da lista anterior?"));
            if (resposta == QMessageBox::Yes) {
                linha.nested = true;
                linha.parent_index = parent_index;
            }
        }
    }

    if (linha.enabled != nullptr) {
        linha.enabled->setChecked(true);
        linha.enabled->setEnabled(false);
        linha.enabled->setVisible(false);
    }
    if (linha.title_label != nullptr) {
        linha.title_label->setText(titulo);
        linha.title_label->setVisible(true);
    }
    if (linha.title_edit != nullptr) {
        linha.title_edit->setVisible(false);
        linha.title_edit->setEnabled(false);
    }
    if (linha.combo != nullptr) {
        linha.combo->setEnabled(true);
        linha.combo->setVisible(true);
    }
    salvar_agenda_customizada_atual();
    atualizar_visibilidade_linhas_customizadas();
}

void InterfaceGerenciadorAtividades::editar_titulo_agenda_customizada(const QString &titulo_agenda) {
    const QString path = caminho_arquivo_agenda_customizada(titulo_agenda);
    QFile file(path);
    QJsonObject root;
    if (file.open(QIODevice::ReadOnly)) {
        root = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    }

    QJsonArray listas = root.value(QStringLiteral("lists")).toArray();
    QStringList titulos;
    for (const QJsonValue &value : listas) {
        const QString titulo = value.toObject().value(QStringLiteral("title")).toString().trimmed();
        if (!titulo.isEmpty()) {
            titulos << titulo;
        }
    }
    if (titulos.isEmpty()) {
        QMessageBox::information(this,
                                 tr_agenda_custom("Editar título"),
                                 tr_agenda_custom("Nenhum título personalizado encontrado."));
        return;
    }

    bool ok = false;
    const QString escolhido = obter_item_dialogo(this,
                                                 tr_agenda_custom("Editar título"),
                                                 tr_agenda_custom("Selecione o título:"),
                                                 titulos,
                                                 0,
                                                 false,
                                                 &ok);
    if (!ok || escolhido.trimmed().isEmpty()) {
        return;
    }
    int index = -1;
    for (int i = 0; i < listas.size(); ++i) {
        const QString titulo = listas.at(i).toObject().value(QStringLiteral("title")).toString().trimmed();
        if (titulo_normalizado(titulo) == titulo_normalizado(escolhido)) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        return;
    }

    const QString novo = obter_texto_dialogo(this,
                                             tr_agenda_custom("Editar título"),
                                             tr_agenda_custom("Novo título:"),
                                             escolhido,
                                             &ok)
                             .trimmed();
    if (!ok || novo.isEmpty() || titulo_normalizado(novo) == titulo_normalizado(escolhido)) {
        return;
    }
    for (int i = 0; i < listas.size(); ++i) {
        const QString titulo = listas.at(i).toObject().value(QStringLiteral("title")).toString().trimmed();
        if (i != index && titulo_normalizado(titulo) == titulo_normalizado(novo)) {
            QMessageBox::warning(this,
                                 tr_agenda_custom("Título existente"),
                                 tr_agenda_custom("Já existe uma lista com este título."));
            return;
        }
    }

    QJsonObject object = listas.at(index).toObject();
    object.insert(QStringLiteral("title"), novo);
    object.insert(QStringLiteral("enabled"), true);
    listas.replace(index, object);
    for (int i = 0; i < listas.size(); ++i) {
        if (i == index) {
            continue;
        }
        QJsonObject filha = listas.at(i).toObject();
        const int parent_index = filha.value(QStringLiteral("parent_index")).toInt(-1);
        const QString parent_title = filha.value(QStringLiteral("parent_title")).toString();
        if (parent_index == index || titulo_normalizado(parent_title) == titulo_normalizado(escolhido)) {
            filha.insert(QStringLiteral("parent_title"), novo);
            listas.replace(i, filha);
        }
    }
    root.insert(QStringLiteral("title"), titulo_agenda.trimmed());
    root.insert(QStringLiteral("lists"), listas);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this,
                             tr_agenda_custom("Erro"),
                             tr_agenda_custom("Não foi possível renomear a agenda."));
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));

    if (custom_agenda_mode && titulo_normalizado(current_custom_agenda_title) == titulo_normalizado(titulo_agenda)) {
        aplicar_listas_agenda_customizada(listas);
        adjustSize();
    }
}

void InterfaceGerenciadorAtividades::remover_titulo_agenda_customizada(const QString &titulo_agenda) {
    const QString path = caminho_arquivo_agenda_customizada(titulo_agenda);
    QFile file(path);
    QJsonObject root;
    if (file.open(QIODevice::ReadOnly)) {
        root = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    }

    QJsonArray listas = root.value(QStringLiteral("lists")).toArray();
    QStringList titulos;
    for (const QJsonValue &value : listas) {
        const QString titulo = value.toObject().value(QStringLiteral("title")).toString().trimmed();
        if (!titulo.isEmpty()) {
            titulos << titulo;
        }
    }
    if (titulos.isEmpty()) {
        QMessageBox::information(this,
                                 tr_agenda_custom("Remover título"),
                                 tr_agenda_custom("Nenhum título personalizado encontrado."));
        return;
    }

    bool ok = false;
    const QString escolhido = obter_item_dialogo(this,
                                                 tr_agenda_custom("Remover título"),
                                                 tr_agenda_custom("Selecione o título:"),
                                                 titulos,
                                                 0,
                                                 false,
                                                 &ok);
    if (!ok || escolhido.trimmed().isEmpty()) {
        return;
    }
    int index = -1;
    for (int i = 0; i < listas.size(); ++i) {
        const QString titulo = listas.at(i).toObject().value(QStringLiteral("title")).toString().trimmed();
        if (titulo_normalizado(titulo) == titulo_normalizado(escolhido)) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        return;
    }

    const auto resposta = perguntar_sim_nao(this,
                                            tr_agenda_custom("Remover título"),
                                            tr_agenda_custom("Deseja remover o título selecionado?"));
    if (resposta != QMessageBox::Yes) {
        return;
    }

    listas.removeAt(index);
    for (int i = 0; i < listas.size(); ++i) {
        QJsonObject object = listas.at(i).toObject();
        const int parent_index = object.value(QStringLiteral("parent_index")).toInt(-1);
        if (object.value(QStringLiteral("nested")).toBool(false) && parent_index == index) {
            object.insert(QStringLiteral("nested"), false);
            object.remove(QStringLiteral("parent_index"));
            object.remove(QStringLiteral("parent_title"));
            object.remove(QStringLiteral("children"));
        } else if (parent_index > index) {
            object.insert(QStringLiteral("parent_index"), parent_index - 1);
        }
        listas.replace(i, object);
    }
    root.insert(QStringLiteral("title"), titulo_agenda.trimmed());
    root.insert(QStringLiteral("lists"), listas);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this,
                             tr_agenda_custom("Erro"),
                             tr_agenda_custom("Não foi possível renomear a agenda."));
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));

    if (custom_agenda_mode && titulo_normalizado(current_custom_agenda_title) == titulo_normalizado(titulo_agenda)) {
        aplicar_listas_agenda_customizada(listas);
        adjustSize();
    }
}

void InterfaceGerenciadorAtividades::salvar_agenda_customizada_atual() {
    if (updating_custom_agenda || current_custom_agenda_title.trimmed().isEmpty()) {
        return;
    }

    QJsonObject root;
    root.insert(QStringLiteral("title"), current_custom_agenda_title.trimmed());
    QJsonArray listas;
    for (const CustomAgendaRow &linha : custom_agenda_rows) {
        QJsonObject object;
        const QString title = titulo_linha_customizada(linha);
        if (title.isEmpty()) {
            continue;
        }
        object.insert(QStringLiteral("enabled"), true);
        object.insert(QStringLiteral("title"), title);
        object.insert(QStringLiteral("nested"), linha.nested);
        if (linha.combo != nullptr) {
            const QString current = linha.combo->currentText().trimmed();
            object.insert(QStringLiteral("current"), current);
        } else {
            object.insert(QStringLiteral("current"), QString());
        }
        if (linha.nested && linha.parent_index >= 0 && linha.parent_index < custom_agenda_rows.size()) {
            object.insert(QStringLiteral("parent_index"), linha.parent_index);
            object.insert(QStringLiteral("parent_title"), titulo_linha_customizada(custom_agenda_rows.at(linha.parent_index)));
            QJsonObject children;
            for (auto it = linha.nested_items.cbegin(); it != linha.nested_items.cend(); ++it) {
                const QString pai = it.key().trimmed();
                const QJsonArray filhos = json_array_de_lista(it.value());
                if (!pai.isEmpty() && !filhos.isEmpty()) {
                    children.insert(pai, filhos);
                }
            }
            object.insert(QStringLiteral("children"), children);
        }
        const QJsonArray items = json_array_de_lista(itens_linha_customizada(linha));
        object.insert(QStringLiteral("items"), items);
        listas.append(object);
    }

    if (listas.isEmpty()) {
        QFile existing(caminho_arquivo_agenda_customizada(current_custom_agenda_title));
        if (existing.open(QIODevice::ReadOnly)) {
            const QJsonArray existing_lists =
                QJsonDocument::fromJson(existing.readAll()).object().value(QStringLiteral("lists")).toArray();
            if (!existing_lists.isEmpty()) {
                return;
            }
        }
    }

    root.insert(QStringLiteral("lists"), listas);

    QFile file(caminho_arquivo_agenda_customizada(current_custom_agenda_title));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        source::utils::LogManager::get_logger()->error(QStringLiteral("Erro ao salvar agenda customizada: %1").arg(file.errorString()));
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

QString InterfaceGerenciadorAtividades::proxima_cor_agenda_customizada() {
    QString chave = current_custom_agenda_title.trimmed();
    if (chave.isEmpty()) {
        for (const CustomAgendaRow &linha : custom_agenda_rows) {
            if (linha.combo != nullptr && !linha.combo->currentText().trimmed().isEmpty()) {
                chave = linha.combo->currentText().trimmed();
                break;
            }
        }
    }
    if (chave.isEmpty()) {
        chave = QStringLiteral("agenda_customizada");
    }
    ultima_cor_agenda_customizada = obter_ou_atribuir_cor_disciplina(chave);
    return ultima_cor_agenda_customizada;
}
