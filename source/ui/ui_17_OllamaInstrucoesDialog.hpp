#pragma once

#include <QDialog>
#include <QList>
#include <QPointer>

class QDialogButtonBox;
class QEvent;
class QLabel;
class QListWidget;
class QPushButton;
class QShowEvent;
class QTabWidget;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI {

class OllamaInstrucoesDialog : public QDialog {
    Q_OBJECT

public:
    explicit OllamaInstrucoesDialog(source::EisenhowerMatrixApp *app, QWidget *parent = nullptr);
    ~OllamaInstrucoesDialog() override = default;

public slots:
    void retranslate_ui();
    void refresh_theme();

protected:
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void atualizar_status();
    void copiar_comando(const QString &comando);

private:
    void recriar_abas();
    void criar_aba_passo_a_passo();
    void criar_aba_modelos_recomendados();
    void criar_aba_gerenciamento();
    void criar_aba_status_local();
    QLabel *criar_label_comando(const QString &comando, QWidget *parent);
    void aplicar_estilos_tema();
    void agendar_refresh_tema();

    source::EisenhowerMatrixApp *app_ = nullptr;
    QLabel *title_label_ = nullptr;
    QLabel *sub_label_ = nullptr;
    QTabWidget *tab_widget_ = nullptr;
    QLabel *status_conexao_label_ = nullptr;
    QLabel *lista_modelos_header_label_ = nullptr;
    QPushButton *btn_refresh_ = nullptr;
    QListWidget *lista_modelos_widget_ = nullptr;
    QDialogButtonBox *button_box_ = nullptr;
    QList<QPointer<QLabel>> cmd_labels_;
    bool theme_refresh_pending_ = false;
    bool applying_theme_ = false;
};

}  // namespace source::GerenciamentoUI
