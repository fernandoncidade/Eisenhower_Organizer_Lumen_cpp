#pragma once

#include <QMessageBox>
#include <QString>
#include <QStringList>

class QDialogButtonBox;
class QInputDialog;
class QWidget;

QString traduzir_botao_dialogo(const char *source);
void traduzir_botoes_padrao(QDialogButtonBox *dialogo);
void traduzir_botoes_mensagem(QMessageBox *mensagem);
QMessageBox::StandardButton executar_mensagem_padrao(QWidget *parent,
                                                     QMessageBox::Icon icon,
                                                     const QString &titulo,
                                                     const QString &texto,
                                                     QMessageBox::StandardButtons botoes = QMessageBox::Ok,
                                                     QMessageBox::StandardButton botao_padrao = QMessageBox::Ok,
                                                     const QString &texto_informativo = QString());
QMessageBox::StandardButton perguntar_sim_nao(QWidget *parent,
                                              const QString &titulo,
                                              const QString &texto,
                                              const QString &texto_informativo = QString(),
                                              QMessageBox::StandardButton botao_padrao = QMessageBox::No);
QString obter_texto_dialogo(QWidget *parent,
                            const QString &titulo,
                            const QString &rotulo,
                            const QString &valor_inicial,
                            bool *ok);
QString obter_item_dialogo(QWidget *parent,
                           const QString &titulo,
                           const QString &rotulo,
                           const QStringList &itens,
                           int item_atual,
                           bool editavel,
                           bool *ok);
