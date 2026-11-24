#include "DialogTranslationUtils.hpp"

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QPushButton>
#include <QtGlobal>

QString traduzir_botao_dialogo(const char *source) {
    return QCoreApplication::translate("InterfaceGerenciadorAtividades", source);
}

void traduzir_botoes_padrao(QDialogButtonBox *dialogo) {
    if (dialogo == nullptr) {
        return;
    }
    if (auto *botao = dialogo->button(QDialogButtonBox::Ok)) {
        botao->setText(traduzir_botao_dialogo("OK"));
    }
    if (auto *botao = dialogo->button(QDialogButtonBox::Cancel)) {
        botao->setText(traduzir_botao_dialogo("Cancelar"));
    }
    if (auto *botao = dialogo->button(QDialogButtonBox::Yes)) {
        botao->setText(traduzir_botao_dialogo("Sim"));
    }
    if (auto *botao = dialogo->button(QDialogButtonBox::No)) {
        botao->setText(traduzir_botao_dialogo("Não"));
    }
}

void traduzir_botoes_mensagem(QMessageBox *mensagem) {
    if (mensagem == nullptr) {
        return;
    }
    if (auto *botao = mensagem->button(QMessageBox::Ok)) {
        botao->setText(traduzir_botao_dialogo("OK"));
    }
    if (auto *botao = mensagem->button(QMessageBox::Cancel)) {
        botao->setText(traduzir_botao_dialogo("Cancelar"));
    }
    if (auto *botao = mensagem->button(QMessageBox::Yes)) {
        botao->setText(traduzir_botao_dialogo("Sim"));
    }
    if (auto *botao = mensagem->button(QMessageBox::No)) {
        botao->setText(traduzir_botao_dialogo("Não"));
    }
}

QMessageBox::StandardButton executar_mensagem_padrao(QWidget *parent,
                                                     QMessageBox::Icon icon,
                                                     const QString &titulo,
                                                     const QString &texto,
                                                     QMessageBox::StandardButtons botoes,
                                                     QMessageBox::StandardButton botao_padrao,
                                                     const QString &texto_informativo) {
    QMessageBox mensagem(parent);
    mensagem.setIcon(icon);
    mensagem.setWindowTitle(titulo);
    mensagem.setText(texto);
    if (!texto_informativo.trimmed().isEmpty()) {
        mensagem.setInformativeText(texto_informativo);
    }
    mensagem.setStandardButtons(botoes);
    mensagem.setDefaultButton(botao_padrao);
    traduzir_botoes_mensagem(&mensagem);
    return static_cast<QMessageBox::StandardButton>(mensagem.exec());
}

QMessageBox::StandardButton perguntar_sim_nao(QWidget *parent,
                                              const QString &titulo,
                                              const QString &texto,
                                              const QString &texto_informativo,
                                              QMessageBox::StandardButton botao_padrao) {
    return executar_mensagem_padrao(parent,
                                    QMessageBox::Question,
                                    titulo,
                                    texto,
                                    QMessageBox::Yes | QMessageBox::No,
                                    botao_padrao,
                                    texto_informativo);
}

QString obter_texto_dialogo(QWidget *parent,
                            const QString &titulo,
                            const QString &rotulo,
                            const QString &valor_inicial,
                            bool *ok) {
    QInputDialog dialog(parent);
    dialog.setWindowTitle(titulo);
    dialog.setLabelText(rotulo);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setTextValue(valor_inicial);
    dialog.setOkButtonText(traduzir_botao_dialogo("OK"));
    dialog.setCancelButtonText(traduzir_botao_dialogo("Cancelar"));

    const bool aceito = dialog.exec() == QDialog::Accepted;
    if (ok != nullptr) {
        *ok = aceito;
    }
    return aceito ? dialog.textValue() : QString();
}

QString obter_item_dialogo(QWidget *parent,
                           const QString &titulo,
                           const QString &rotulo,
                           const QStringList &itens,
                           int item_atual,
                           bool editavel,
                           bool *ok) {
    QInputDialog dialog(parent);
    dialog.setWindowTitle(titulo);
    dialog.setLabelText(rotulo);
    dialog.setComboBoxItems(itens);
    dialog.setComboBoxEditable(editavel);
    if (!itens.isEmpty()) {
        dialog.setTextValue(itens.value(qBound(0, item_atual, itens.size() - 1)));
    }
    dialog.setOkButtonText(traduzir_botao_dialogo("OK"));
    dialog.setCancelButtonText(traduzir_botao_dialogo("Cancelar"));

    const bool aceito = dialog.exec() == QDialog::Accepted;
    if (ok != nullptr) {
        *ok = aceito;
    }
    return aceito ? dialog.textValue() : QString();
}
