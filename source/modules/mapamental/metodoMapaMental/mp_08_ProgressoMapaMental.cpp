#include "source/modules/mapamental/metodoMapaMental/mp_08_ProgressoMapaMental.hpp"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QEventLoop>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

namespace source::plugin::MapaMental {

ProgressoMapaMental::ProgressoMapaMental(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(QCoreApplication::translate("MapaMentalPlugin", "Processando mapa mental"));
    setWindowModality(Qt::ApplicationModal);
    setMinimumSize(520, 320);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(8);

    ia_label_ = new QLabel(QCoreApplication::translate("MapaMentalPlugin", "Preparando processamento de IA..."), this);
    ia_label_->setWordWrap(true);
    layout->addWidget(ia_label_);

    ia_bar_ = new QProgressBar(this);
    ia_bar_->setRange(0, 100);
    ia_bar_->setValue(0);
    layout->addWidget(ia_bar_);

    log_ = new QPlainTextEdit(this);
    log_->setReadOnly(true);
    log_->setMinimumHeight(150);
    layout->addWidget(log_, 1);

    nos_label_ = new QLabel(QCoreApplication::translate("MapaMentalPlugin", "Aguardando geracao dos nos..."), this);
    nos_label_->setWordWrap(true);
    layout->addWidget(nos_label_);

    nos_bar_ = new QProgressBar(this);
    nos_bar_->setRange(0, 1);
    nos_bar_->setValue(0);
    nos_bar_->setFormat(QStringLiteral("%v/%m"));
    layout->addWidget(nos_bar_);

    fechar_button_ = new QPushButton(QCoreApplication::translate("MapaMentalPlugin", "Fechar"), this);
    fechar_button_->setEnabled(false);
    connect(fechar_button_, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(fechar_button_, 0, Qt::AlignRight);
}

void ProgressoMapaMental::atualizar_ia(int valor, const QString &mensagem)
{
    if (ia_label_ != nullptr) {
        ia_label_->setText(mensagem);
    }
    if (ia_bar_ != nullptr) {
        ia_bar_->setValue(qBound(0, valor, 100));
    }
    registrar(mensagem);
}

void ProgressoMapaMental::configurar_nos(int total)
{
    const int limite = qMax(1, total);
    if (nos_bar_ != nullptr) {
        nos_bar_->setRange(0, limite);
        nos_bar_->setValue(0);
    }
    if (nos_label_ != nullptr) {
        nos_label_->setText(
            QCoreApplication::translate("MapaMentalPlugin", "Preparando geracao de %1 no(s)...").arg(total));
    }
    registrar(QCoreApplication::translate("MapaMentalPlugin", "Total de nos previstos: %1").arg(total));
}

void ProgressoMapaMental::atualizar_no(int indice, int total, const QString &titulo)
{
    (void)titulo;

    const int limite = qMax(1, total);
    const int valor = qBound(0, indice, limite);
    const QString mensagem =
        QCoreApplication::translate("MapaMentalPlugin", "Gerando nos... %1/%2").arg(valor).arg(total);

    if (nos_bar_ != nullptr) {
        nos_bar_->setRange(0, limite);
        nos_bar_->setValue(valor);
    }
    if (nos_label_ != nullptr) {
        nos_label_->setText(mensagem);
    }
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void ProgressoMapaMental::concluir_nos(const QString &mensagem)
{
    if (nos_bar_ != nullptr) {
        nos_bar_->setValue(nos_bar_->maximum());
    }
    if (nos_label_ != nullptr) {
        nos_label_->setText(mensagem);
    }
    registrar(mensagem);
}

void ProgressoMapaMental::registrar(const QString &mensagem)
{
    if (mensagem.trimmed().isEmpty()) {
        return;
    }
    if (log_ != nullptr) {
        log_->appendPlainText(mensagem.trimmed());
        log_->verticalScrollBar()->setValue(log_->verticalScrollBar()->maximum());
    }
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void ProgressoMapaMental::finalizar(const QString &mensagem)
{
    finalizado_ = true;
    registrar(mensagem);
    if (fechar_button_ != nullptr) {
        fechar_button_->setEnabled(true);
        fechar_button_->setFocus();
    }
}

void ProgressoMapaMental::closeEvent(QCloseEvent *event)
{
    if (finalizado_) {
        QDialog::closeEvent(event);
        return;
    }
    event->ignore();
}

void ProgressoMapaMental::changeEvent(QEvent *event)
{
    if (event != nullptr && event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    QDialog::changeEvent(event);
}

void ProgressoMapaMental::retranslate_ui()
{
    setWindowTitle(QCoreApplication::translate("MapaMentalPlugin", "Processando mapa mental"));
    if (fechar_button_ != nullptr) {
        fechar_button_->setText(QCoreApplication::translate("MapaMentalPlugin", "Fechar"));
    }
}

}  // namespace source::plugin::MapaMental
