#include "source/modules/mapamental/metodoMapaMental/NoConceito/nocon_04_mouseDoubleClickEvent.hpp"

#include <functional>
#include <utility>

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

namespace source::plugin::MapaMental {
namespace {

QString primeiro_arquivo_local(const QMimeData *mime_data)
{
    if (mime_data == nullptr || !mime_data->hasUrls()) {
        return QString();
    }

    const QList<QUrl> urls = mime_data->urls();
    for (const QUrl &url : urls) {
        const QString caminho = url.toLocalFile().trimmed();
        if (!caminho.isEmpty() && QFileInfo(caminho).isFile()) {
            return caminho;
        }
    }

    return QString();
}

class ArquivoVinculadoDropFilter final : public QObject {
public:
    explicit ArquivoVinculadoDropFilter(QLineEdit *target, std::function<void(const QString &)> vincular)
        : QObject(target), target_(target), vincular_(std::move(vincular))
    {
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != target_ || event == nullptr) {
            return QObject::eventFilter(watched, event);
        }

        if (event->type() == QEvent::DragEnter) {
            auto *drag = static_cast<QDragEnterEvent *>(event);
            if (!primeiro_arquivo_local(drag->mimeData()).isEmpty()) {
                drag->acceptProposedAction();
                return true;
            }
        } else if (event->type() == QEvent::DragMove) {
            auto *drag = static_cast<QDragMoveEvent *>(event);
            if (!primeiro_arquivo_local(drag->mimeData()).isEmpty()) {
                drag->acceptProposedAction();
                return true;
            }
        } else if (event->type() == QEvent::Drop) {
            auto *drop = static_cast<QDropEvent *>(event);
            const QString caminho = primeiro_arquivo_local(drop->mimeData());
            if (!caminho.isEmpty()) {
                if (vincular_) {
                    vincular_(caminho);
                }
                drop->acceptProposedAction();
                return true;
            }
        }

        return QObject::eventFilter(watched, event);
    }

private:
    QLineEdit *target_ = nullptr;
    std::function<void(const QString &)> vincular_;
};

}  // namespace

bool NoConceito::abrir_editor_conceito()
{
    QDialog dialog(owner_);
    dialog.setWindowTitle(QCoreApplication::translate("MapaMentalPlugin", "Editor conceito"));
    dialog.setMinimumSize(520, 420);

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout;
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    auto *input_texto = new QLineEdit(texto_, &dialog);
    input_texto->setPlaceholderText(QCoreApplication::translate("MapaMentalPlugin", "Nome do conceito"));
    form->addRow(QCoreApplication::translate("MapaMentalPlugin", "Conceito:"), input_texto);

    auto *input_notas = new QTextEdit(&dialog);
    input_notas->setPlainText(notas_);
    input_notas->setAcceptRichText(false);
    input_notas->setPlaceholderText(QCoreApplication::translate("MapaMentalPlugin", "Notas"));
    form->addRow(QCoreApplication::translate("MapaMentalPlugin", "Notas:"), input_notas);

    QString arquivo_local = arquivo_vinculado_;
    auto *arquivo_label = new QLineEdit(&dialog);
    arquivo_label->setReadOnly(true);
    arquivo_label->setAcceptDrops(true);
    arquivo_label->setPlaceholderText(QCoreApplication::translate("MapaMentalPlugin", "Nenhum arquivo vinculado"));

    auto *btn_vincular = new QPushButton(QCoreApplication::translate("MapaMentalPlugin", "Vincular arquivo"), &dialog);
    auto *btn_abrir = new QPushButton(QCoreApplication::translate("MapaMentalPlugin", "Abrir arquivo"), &dialog);
    auto *btn_remover = new QPushButton(QCoreApplication::translate("MapaMentalPlugin", "Remover arquivo"), &dialog);

    auto atualizar_arquivo = [&]() {
        arquivo_label->setText(arquivo_local);
        arquivo_label->setToolTip(arquivo_local);
        const bool possui_arquivo = !arquivo_local.trimmed().isEmpty();
        btn_abrir->setEnabled(possui_arquivo);
        btn_remover->setEnabled(possui_arquivo);
    };

    auto *arquivo_drop_filter = new ArquivoVinculadoDropFilter(arquivo_label, [&](const QString &caminho) {
        arquivo_local = caminho;
        atualizar_arquivo();
    });
    arquivo_label->installEventFilter(arquivo_drop_filter);

    auto *arquivo_layout = new QHBoxLayout;
    arquivo_layout->addWidget(arquivo_label, 1);
    arquivo_layout->addWidget(btn_vincular);
    arquivo_layout->addWidget(btn_abrir);
    arquivo_layout->addWidget(btn_remover);
    form->addRow(QCoreApplication::translate("MapaMentalPlugin", "Arquivo:"), arquivo_layout);

    layout->addLayout(form);

    auto *botoes = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(botoes);
    if (owner_ != nullptr) {
        owner_->traduzir_botoes_padrao(&dialog);
    }

    auto atualizar_ok = [input_texto, botoes]() {
        if (auto *ok = botoes->button(QDialogButtonBox::Ok)) {
            ok->setEnabled(!input_texto->text().trimmed().isEmpty());
        }
    };

    QObject::connect(input_texto, &QLineEdit::textChanged, &dialog, atualizar_ok);
    QObject::connect(btn_vincular, &QPushButton::clicked, &dialog, [&]() {
        const QString caminho = QFileDialog::getOpenFileName(
            &dialog, QCoreApplication::translate("MapaMentalPlugin", "Vincular arquivo"));
        if (!caminho.isEmpty()) {
            arquivo_local = caminho;
            atualizar_arquivo();
        }
    });
    QObject::connect(btn_abrir, &QPushButton::clicked, &dialog, [&]() {
        if (!arquivo_local.trimmed().isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(arquivo_local));
        }
    });
    QObject::connect(btn_remover, &QPushButton::clicked, &dialog, [&]() {
        arquivo_local.clear();
        atualizar_arquivo();
    });
    QObject::connect(botoes, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(botoes, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    atualizar_arquivo();
    atualizar_ok();

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    const QString novo_texto = input_texto->text().trimmed();
    if (novo_texto.isEmpty()) {
        return false;
    }

    set_texto(novo_texto);
    set_notas(input_notas->toPlainText());
    set_arquivo_vinculado(arquivo_local);
    return true;
}

void NoConceito::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event != nullptr && (event->modifiers() & Qt::ControlModifier) && !arquivo_vinculado_.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(arquivo_vinculado_));
        event->accept();
        return;
    }

    if (abrir_editor_conceito()) {
        if (owner_ != nullptr) {
            owner_->salvar_backup_automatico();
            owner_->refresh_counter();
        }
    }
}

}  // namespace source::plugin::MapaMental
