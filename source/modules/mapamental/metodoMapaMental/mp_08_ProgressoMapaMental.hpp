#pragma once

#include <QDialog>
#include <QString>

class QLabel;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QCloseEvent;

namespace source::plugin::MapaMental {

class ProgressoMapaMental final : public QDialog {
public:
    explicit ProgressoMapaMental(QWidget *parent = nullptr);

    void atualizar_ia(int valor, const QString &mensagem);
    void configurar_nos(int total);
    void atualizar_no(int indice, int total, const QString &titulo);
    void concluir_nos(const QString &mensagem);
    void registrar(const QString &mensagem);
    void finalizar(const QString &mensagem);

    void retranslate_ui();

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    QLabel *ia_label_ = nullptr;
    QProgressBar *ia_bar_ = nullptr;
    QPlainTextEdit *log_ = nullptr;
    QLabel *nos_label_ = nullptr;
    QProgressBar *nos_bar_ = nullptr;
    QPushButton *fechar_button_ = nullptr;
    bool finalizado_ = false;
};

}  // namespace source::plugin::MapaMental
