#include "Interface_Grafica.hpp"

#include "source/modules/agenda/AgendaAvaliacoes/BancoDeDados/Banco_Dados.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/IconUtils.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/ComboBoxUtils.hpp"
#include "source/utils/LogManager.hpp"

#include <QCoreApplication>
#include <QEvent>
#include <QFile>
#include <QIcon>
#include <QMenuBar>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QTimer>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>
#include <QUuid>

namespace {
    QString texto_combo(QComboBox *combo) {
        return combo != nullptr ? combo->currentText().trimmed() : QString();
    }

    QString rotulo_sem_dois_pontos(QLabel *label, const QString &fallback) {
        QString text = label != nullptr ? label->text().trimmed() : fallback;
        while (text.endsWith(QLatin1Char(':'))) {
            text.chop(1);
        }
        return text.trimmed();
    }

    QString prioridade_sem_emoji(const QString &texto) {
        QString valor = texto.trimmed();
        while (!valor.isEmpty() && !valor.at(0).isLetterOrNumber()) {
            valor.remove(0, 1);
        }
        return valor.trimmed();
    }

    QString tr_agenda(const char *source) {
        return QCoreApplication::translate("InterfaceGerenciadorAtividades", source);
    }

    void restaurar_combo_padrao(QComboBox *combo, const QStringList &itens, int indice_padrao) {
        if (combo == nullptr) {
            return;
        }
        const QSignalBlocker blocker(combo);
        combo->clear();
        combo->addItems(itens);
        if (combo->count() > 0) {
            combo->setCurrentIndex(qBound(0, indice_padrao, combo->count() - 1));
        }
    }

    void restaurar_prioridade_padrao(QComboBox *combo) {
        restaurar_combo_padrao(combo,
                               {
                                   tr_agenda("🔴 Importante e Urgente"),
                                   tr_agenda("🟠 Importante, mas Não Urgente"),
                                   tr_agenda("🟡 Não Importante, mas Urgente"),
                                   tr_agenda("🟢 Não Importante e Não Urgente"),
                               },
                               0);
    }

    void restaurar_dominio_padrao(QComboBox *combo) {
        restaurar_combo_padrao(combo,
                               {
                                   tr_agenda("🥉 Iniciante (não consigo explicar)"),
                                   tr_agenda("🥈 Intermediário (explico com dificuldade)"),
                                   tr_agenda("🥇 Avançado (explico facilmente)"),
                               },
                               0);
    }
}

InterfaceGerenciadorAtividades::InterfaceGerenciadorAtividades(GerenciadorTraducao *traducao,
                                                               QWidget *parent,
                                                               bool painel_esquerdo_apenas_,
                                                               bool criar_menu_janela_)
    : InterfaceGerenciadorAtividades(traducao, parent, painel_esquerdo_apenas_, criar_menu_janela_, true) {
}

InterfaceGerenciadorAtividades::InterfaceGerenciadorAtividades(GerenciadorTraducao *traducao,
                                                               QWidget *parent,
                                                               bool painel_esquerdo_apenas_,
                                                               bool criar_menu_janela_,
                                                               bool persistir_atividades_locais_)
    : QMainWindow(parent),
      gerenciamento_atividades(persistir_atividades_locais_),
      painel_esquerdo_apenas(painel_esquerdo_apenas_),
      criar_menu_janela(criar_menu_janela_) {
    setWindowFlag(Qt::WindowMaximizeButtonHint, false);
    if (traducao) {
        gerenciador_traducao = traducao;
    } else {
        gerenciador_traducao_proprio = std::make_unique<GerenciadorTraducao>();
        gerenciador_traducao = gerenciador_traducao_proprio.get();
    }
    gerenciador_traducao->aplicar_traducao();
    connect(gerenciador_traducao, &GerenciadorTraducao::idioma_alterado, this,
            [this](const QString &) { retranslate_ui(); });
    QString iconPath = get_icon_path("ReviewsManager.ico");
    if (iconPath.isEmpty() || !QFile::exists(iconPath)) {
        iconPath = get_icon_path("Reviews-Manager_PySide6-pyppeteer.1.0.ico");
    }
    if (!iconPath.isEmpty() && QFile::exists(iconPath)) {
        setWindowIcon(QIcon(iconPath));
    }
    setWindowTitle(QString());
    create_widgets();
    if (criar_menu_janela) {
        create_menu();
    } else if (menuBar() != nullptr) {
        menuBar()->hide();
    }
    carregar_ultima_agenda_ativa();
    retranslate_ui();
    update_textbox();
}

void InterfaceGerenciadorAtividades::sync_date_time_controls() {
    const bool data_vinculada = check_vincular_data == nullptr || check_vincular_data->isChecked();
    if (calendar != nullptr) {
        calendar->setEnabled(data_vinculada);
    }
    if (check_vincular_horario != nullptr) {
        check_vincular_horario->setEnabled(data_vinculada);
        if (!data_vinculada) {
            check_vincular_horario->setChecked(false);
        }
    }
    if (time_edit != nullptr) {
        time_edit->setEnabled(data_vinculada && check_vincular_horario != nullptr && check_vincular_horario->isChecked());
    }
}

void InterfaceGerenciadorAtividades::changeEvent(QEvent *event) {
    if (event != nullptr && (event->type() == QEvent::ApplicationPaletteChange ||
                             event->type() == QEvent::PaletteChange ||
                             event->type() == QEvent::StyleChange)) {
        update_textbox();
    }

    QMainWindow::changeEvent(event);
}

void InterfaceGerenciadorAtividades::limpar_definicoes_selecionadas() {
    const auto limpar_combo = [](QComboBox *combo) {
        if (combo == nullptr) {
            return;
        }
        const QSignalBlocker blocker(combo);
        combo->setCurrentIndex(-1);
        if (combo->isEditable()) {
            combo->clearEditText();
        }
    };

    limpar_combo(combo_curso);
    limpar_combo(entry_ementa);
    limpar_combo(entry_semestre);
    limpar_combo(entry_disciplina);
    limpar_combo(entry_codigo);
    limpar_combo(combo_tipo);
    limpar_combo(combo_sequencia);
    if (custom_agenda_mode) {
        for (const CustomAgendaRow &linha : custom_agenda_rows) {
            limpar_combo(linha.combo);
        }
        for (int i = 0; i < custom_agenda_rows.size(); ++i) {
            atualizar_dependencias_agenda_customizada(i);
        }
        salvar_agenda_customizada_atual();
    }
    restaurar_prioridade_padrao(combo_prioridade);
    restaurar_dominio_padrao(combo_dominio);

    if (check_vincular_data != nullptr) {
        check_vincular_data->setChecked(true);
    }
    if (check_vincular_horario != nullptr) {
        check_vincular_horario->setChecked(true);
    }
    if (calendar != nullptr) {
        calendar->setSelectedDate(QDate::currentDate());
    }
    if (time_edit != nullptr) {
        time_edit->setTime(QTime::currentTime());
    }
    if (arquivo_vinculado_edit != nullptr) {
        arquivo_vinculado_edit->clear();
        arquivo_vinculado_edit->setToolTip(QString());
    }
    if (button_abrir_arquivo != nullptr) {
        button_abrir_arquivo->setEnabled(false);
    }
    sync_date_time_controls();
}

void InterfaceGerenciadorAtividades::submiter() {
    const auto reativar_janela = [this]() {
        QTimer::singleShot(0, this, [this]() {
            if (!isVisible()) {
                return;
            }
            show();
            raise();
            activateWindow();
        });
    };

    const QString curso = texto_combo(combo_curso);
    const QString ementa = texto_combo(entry_ementa);
    const QString semestre = texto_combo(entry_semestre);
    const QString disciplina = texto_combo(entry_disciplina);
    const QString turma = texto_combo(entry_codigo);
    const QString tipo = texto_combo(combo_tipo);
    const QString sequencia = texto_combo(combo_sequencia);
    const QDate data = calendar != nullptr ? calendar->selectedDate() : QDate::currentDate();
    const bool data_vinculada = check_vincular_data == nullptr || check_vincular_data->isChecked();
    const bool horario_vinculado =
        data_vinculada && check_vincular_horario != nullptr && check_vincular_horario->isChecked() && time_edit != nullptr;
    const int prioridade =
        combo_prioridade != nullptr && combo_prioridade->currentIndex() >= 0 ? combo_prioridade->currentIndex() + 1 : 1;
    const int dominio = combo_dominio != nullptr && combo_dominio->currentIndex() >= 0 ? combo_dominio->currentIndex() : 0;
    const QString titulo_prioridade = combo_prioridade != nullptr ? combo_prioridade->currentText().trimmed() : QString();
    const QString titulo_dominio = combo_dominio != nullptr ? combo_dominio->currentText().trimmed() : QString();
    const QString arquivo_vinculado =
        arquivo_vinculado_edit != nullptr ? arquivo_vinculado_edit->toolTip().trimmed() : QString();

    if (custom_agenda_mode) {
        QStringList titulo_partes;
        QStringList descricao;
        const auto contem_valor = [](const QStringList &itens, const QString &valor) {
            const QString alvo = valor.trimmed().toCaseFolded();
            for (const QString &item : itens) {
                if (item.trimmed().toCaseFolded() == alvo) {
                    return true;
                }
            }
            return false;
        };
        if (!current_custom_agenda_title.trimmed().isEmpty()) {
            titulo_partes << current_custom_agenda_title.trimmed();
            descricao << QStringLiteral("%1: %2").arg(tr_agenda("Agenda"), current_custom_agenda_title.trimmed());
        }

        for (int i = 0; i < custom_agenda_rows.size(); ++i) {
            CustomAgendaRow &linha = custom_agenda_rows[i];
            QString label;
            if (linha.title_label != nullptr && !linha.title_label->text().trimmed().isEmpty()) {
                label = linha.title_label->text().trimmed();
            }
            const QString value = linha.combo != nullptr ? linha.combo->currentText().trimmed() : QString();
            if (label.isEmpty() || value.isEmpty()) {
                continue;
            }
            if (linha.nested) {
                registrar_item_linha_agenda_customizada(i);
                if (!contem_valor(itens_linha_agenda_customizada(i), value)) {
                    QMessageBox::warning(
                        this,
                        tr_agenda("Erro"),
                        tr_agenda("Selecione um item da lista anterior para concluir o aninhamento."));
                    return;
                }
            } else {
                adicionar_item_combo_se_ausente(linha.combo, value);
            }
            titulo_partes << value;
            descricao << QStringLiteral("%1: %2").arg(label, value);
        }

        if (descricao.size() <= (current_custom_agenda_title.trimmed().isEmpty() ? 0 : 1)) {
            QMessageBox::warning(this,
                                 tr_agenda("Erro"),
                                 tr_agenda("Por favor, preencha ao menos uma lista antes de registrar a atividade."));
            return;
        }

        salvar_agenda_customizada_atual();

        const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QVariantMap task;
        task.insert(QStringLiteral("origin"), QStringLiteral("agenda"));
        task.insert(QStringLiteral("integration_id"), uuid);
        task.insert(QStringLiteral("id"), uuid);
        task.insert(QStringLiteral("titulo"), titulo_partes.join(QStringLiteral(" - ")).trimmed());
        task.insert(QStringLiteral("prioridade"), prioridade);
        task.insert(QStringLiteral("titulo_prioridade"), prioridade_sem_emoji(titulo_prioridade));
        task.insert(QStringLiteral("dominio"), dominio);
        task.insert(QStringLiteral("titulo_dominio"), titulo_dominio);
        task.insert(QStringLiteral("description"), descricao.join(QLatin1Char('\n')));
        task.insert(QStringLiteral("date"), data_vinculada ? data.toString(Qt::ISODate) : QString());
        task.insert(QStringLiteral("time"), horario_vinculado ? time_edit->time().toString(QStringLiteral("HH:mm")) : QString());
        task.insert(QStringLiteral("completed"), false);
        task.insert(QStringLiteral("status"), QStringLiteral("Todo"));
        if (!arquivo_vinculado.isEmpty()) {
            task.insert(QStringLiteral("file_path"), arquivo_vinculado);
        }

        QString chave_disciplina;
        for (const CustomAgendaRow &linha : custom_agenda_rows) {
            if (linha.combo != nullptr && !linha.combo->currentText().trimmed().isEmpty()) {
                const QString label = linha.title_label != nullptr ? linha.title_label->text().trimmed().toCaseFolded() : QString();
                if (label.contains(QStringLiteral("disciplina")) || label.contains(QStringLiteral("subject")) || label.contains(QStringLiteral("matéria")) || label.contains(QStringLiteral("materia"))) {
                    chave_disciplina = linha.combo->currentText().trimmed();
                    break;
                }
            }
        }
        if (chave_disciplina.isEmpty()) {
            for (const CustomAgendaRow &linha : custom_agenda_rows) {
                if (linha.combo != nullptr && !linha.combo->currentText().trimmed().isEmpty()) {
                    chave_disciplina = linha.combo->currentText().trimmed();
                    break;
                }
            }
        }
        const QString cor_texto_colorida = !chave_disciplina.isEmpty()
                                               ? obter_ou_atribuir_cor_disciplina(chave_disciplina)
                                               : proxima_cor_agenda_customizada();
        if (!cor_texto_colorida.isEmpty()) {
            task.insert(QStringLiteral("cor_texto_original"), cor_texto_colorida);
            task.insert(QStringLiteral("agenda_cor_texto"), cor_texto_colorida);
        }
        const QString cor_texto_efetiva =
            gerenciamento_atividades.obter_modo_cores() == QStringLiteral("coloridas")
                ? cor_texto_colorida
                : gerenciamento_atividades.cor_texto_do_sistema();
        if (!cor_texto_efetiva.isEmpty()) {
            task.insert(QStringLiteral("cor_texto"), cor_texto_efetiva);
            task.insert(QStringLiteral("task_color"), cor_texto_efetiva);
        }

        emit atividade_registrada(task);
        reativar_janela();
        return;
    }

    const QString cor_texto_colorida = obter_ou_atribuir_cor_disciplina(disciplina);
    const QString cor_texto = gerenciamento_atividades.obter_modo_cores() == QStringLiteral("coloridas")
                                  ? cor_texto_colorida
                                  : gerenciamento_atividades.cor_texto_do_sistema();

    if (!gerenciamento_atividades.submiter()) {
        return;
    }

    const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString avaliacao = tipo;
    if (!sequencia.isEmpty()) {
        avaliacao = avaliacao.isEmpty() ? sequencia : QStringLiteral("%1 %2").arg(avaliacao, sequencia);
    }

    QStringList titulo_partes;
    if (!disciplina.isEmpty()) {
        titulo_partes << disciplina;
    }
    if (!avaliacao.isEmpty()) {
        titulo_partes << avaliacao;
    }
    if (!turma.isEmpty()) {
        titulo_partes << turma;
    }

    QStringList descricao;
    const auto append_field = [&descricao](const QString &label, const QString &value) {
        if (!value.trimmed().isEmpty()) {
            descricao << QStringLiteral("%1: %2").arg(label, value.trimmed());
        }
    };
    append_field(rotulo_sem_dois_pontos(label_curso, QStringLiteral("Curso")), curso);
    append_field(rotulo_sem_dois_pontos(label_ementa, QStringLiteral("Ementa")), ementa);
    append_field(rotulo_sem_dois_pontos(label_semestre, QStringLiteral("Semestre")), semestre);
    append_field(rotulo_sem_dois_pontos(label_disciplina, QStringLiteral("Disciplina")), disciplina);
    append_field(rotulo_sem_dois_pontos(label_codigo, QStringLiteral("Turma da Disciplina")), turma);
    append_field(rotulo_sem_dois_pontos(label_tipo, QStringLiteral("Tipo de Atividade Avaliativa")), tipo);
    append_field(rotulo_sem_dois_pontos(label_sequencia, QStringLiteral("Sequência da Atividade")), sequencia);

    QVariantMap task;
    task.insert(QStringLiteral("origin"), QStringLiteral("agenda"));
    task.insert(QStringLiteral("integration_id"), uuid);
    task.insert(QStringLiteral("id"), uuid);
    task.insert(QStringLiteral("titulo"), titulo_partes.join(QStringLiteral(" - ")).trimmed());
    task.insert(QStringLiteral("prioridade"), prioridade);
    task.insert(QStringLiteral("titulo_prioridade"), prioridade_sem_emoji(titulo_prioridade));
    task.insert(QStringLiteral("dominio"), dominio);
    task.insert(QStringLiteral("titulo_dominio"), titulo_dominio);
    task.insert(QStringLiteral("description"), descricao.join(QLatin1Char('\n')));
    task.insert(QStringLiteral("date"), data_vinculada ? data.toString(Qt::ISODate) : QString());
    task.insert(QStringLiteral("time"), horario_vinculado ? time_edit->time().toString(QStringLiteral("HH:mm")) : QString());
    task.insert(QStringLiteral("completed"), false);
    task.insert(QStringLiteral("status"), QStringLiteral("Todo"));
    if (!arquivo_vinculado.isEmpty()) {
        task.insert(QStringLiteral("file_path"), arquivo_vinculado);
    }
    if (!cor_texto_colorida.isEmpty()) {
        task.insert(QStringLiteral("cor_texto_original"), cor_texto_colorida);
        task.insert(QStringLiteral("agenda_cor_texto"), cor_texto_colorida);
    }
    if (!cor_texto.isEmpty()) {
        task.insert(QStringLiteral("cor_texto"), cor_texto);
        task.insert(QStringLiteral("task_color"), cor_texto);
    }

    emit atividade_registrada(task);
    reativar_janela();
}

void InterfaceGerenciadorAtividades::limpar_entradas() {
    gerenciamento_atividades.limpar_entradas();
}

void InterfaceGerenciadorAtividades::limpar_ultima_entrada() {
    gerenciamento_atividades.limpar_ultima_entrada();
}

void InterfaceGerenciadorAtividades::excluir_item() {
    gerenciamento_atividades.excluir_item();
}

void InterfaceGerenciadorAtividades::editar_item() {
    gerenciamento_atividades.editar_item();
}

void InterfaceGerenciadorAtividades::exportar_para_pdf() {
    gerenciamento_atividades.exportar_para_pdf();
}

void InterfaceGerenciadorAtividades::update_textbox() {
    gerenciamento_atividades.update_textbox();
}

void InterfaceGerenciadorAtividades::update_ementa() {
    if (_atualizando_ementa)
        return;
    _atualizando_ementa = true;
    gerenciamento_atividades.update_ementa();
    _atualizando_ementa = false;
}

void InterfaceGerenciadorAtividades::update_semestre() {
    if (_atualizando_semestre)
        return;
    _atualizando_semestre = true;
    gerenciamento_atividades.update_semestre();
    _atualizando_semestre = false;
}

void InterfaceGerenciadorAtividades::update_disciplinas() {
    if (_atualizando_disciplinas)
        return;
    _atualizando_disciplinas = true;
    gerenciamento_atividades.update_disciplinas();
    _atualizando_disciplinas = false;
}
