#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_13_adicionar_no.hpp"

#include <memory>

#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QEasingCurve>
#include <QGraphicsView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QVariantAnimation>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_32_expandir_area_se_necessario.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_24_gerar_mapa_de_hierarquia.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_07_SmoothGraphicsView.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_08_ProgressoMapaMental.hpp"

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

class NovoConceitoDropFilter final : public QObject {
public:
    NovoConceitoDropFilter(QLineEdit *input, ProcessadorIA *processador, MapaMental *owner)
        : QObject(input), input_(input), processador_(processador), owner_(owner)
    {
    }

    bool tem_documento_carregado() const
    {
        return !texto_extraido_.trimmed().isEmpty();
    }

    QString texto_extraido() const
    {
        return texto_extraido_;
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != input_ || event == nullptr) {
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
                carregar_arquivo(caminho);
                drop->acceptProposedAction();
                return true;
            }
        }

        return QObject::eventFilter(watched, event);
    }

private:
    void carregar_arquivo(const QString &caminho)
    {
        if (input_ == nullptr || processador_ == nullptr) {
            return;
        }

        ProgressoMapaMental progresso(owner_);
        progresso.setWindowTitle(QCoreApplication::translate("MapaMentalPlugin", "Novo conceito"));
        progresso.show();
        progresso.atualizar_ia(10, QCoreApplication::translate("MapaMentalPlugin", "Arquivo recebido."));
        progresso.registrar(QCoreApplication::translate("MapaMentalPlugin", "Arquivo: %1")
                                .arg(QDir::toNativeSeparators(QFileInfo(caminho).absoluteFilePath())));
        progresso.atualizar_ia(45, QCoreApplication::translate("MapaMentalPlugin", "Extraindo texto do arquivo..."));

        const QString texto = processador_->extrair_texto(caminho);
        if (texto.trimmed().isEmpty()) {
            texto_extraido_.clear();
            progresso.atualizar_ia(100,
                                   QCoreApplication::translate("MapaMentalPlugin",
                                                               "Nao foi possivel extrair texto do arquivo."));
            progresso.configurar_nos(0);
            progresso.finalizar(QCoreApplication::translate("MapaMentalPlugin", "Processamento encerrado."));

            QMessageBox box(&progresso);
            box.setIcon(QMessageBox::Warning);
            box.setWindowTitle(QCoreApplication::translate("MapaMentalPlugin", "Novo conceito"));
            box.setText(QCoreApplication::translate("MapaMentalPlugin", "Nao foi possivel extrair texto do arquivo."));
            box.setStandardButtons(QMessageBox::Ok);
            if (owner_ != nullptr) {
                owner_->traduzir_botoes_padrao(&box);
            }
            box.exec();
            progresso.accept();
            return;
        }

        texto_extraido_ = texto;
        input_->setText(QFileInfo(caminho).fileName());
        input_->setToolTip(QDir::toNativeSeparators(caminho));
        progresso.atualizar_ia(100, QCoreApplication::translate("MapaMentalPlugin", "Texto carregado com sucesso."));
        progresso.configurar_nos(0);
        progresso.finalizar(QCoreApplication::translate("MapaMentalPlugin",
                                                        "Arquivo carregado. Clique em OK para gerar o mapa."));
        progresso.accept();
    }

    QLineEdit *input_ = nullptr;
    ProcessadorIA *processador_ = nullptr;
    MapaMental *owner_ = nullptr;
    QString texto_extraido_;
};

}  // namespace

void MapaMental::adicionar_no()
{
    QInputDialog dialog(this);
    dialog.setWindowTitle(QCoreApplication::translate("MapaMentalPlugin", "Novo conceito"));
    dialog.setLabelText(QCoreApplication::translate("MapaMentalPlugin", "Texto:"));
    dialog.setOkButtonText(QCoreApplication::translate("MapaMentalPlugin", "OK"));
    dialog.setCancelButtonText(QCoreApplication::translate("MapaMentalPlugin", "Cancelar"));
    dialog.setTextEchoMode(QLineEdit::Normal);
    traduzir_botoes_padrao(&dialog);

    NovoConceitoDropFilter *drop_filter = nullptr;
    if (QLineEdit *input_texto = dialog.findChild<QLineEdit *>()) {
        input_texto->setAcceptDrops(true);
        drop_filter = new NovoConceitoDropFilter(input_texto, processador_ia_, this);
        input_texto->installEventFilter(drop_filter);
    }

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    if (drop_filter != nullptr && drop_filter->tem_documento_carregado()) {
        processar_texto_com_ia(drop_filter->texto_extraido(),
                               QCoreApplication::translate("MapaMentalPlugin", "Novo conceito"));
        return;
    }

    const QString texto = dialog.textValue().trimmed();
    if (texto.isEmpty()) {
        return;
    }

    QPointF pos(0, 0);
    if (view_ != nullptr) {
        pos = view_->mapToScene(view_->viewport()->rect().center());
    }
    if (!nos_.isEmpty()) {
        pos += QPointF((nos_.size() % 5) * 34, (nos_.size() % 7) * 28);
    }

    adicionar_no(texto, pos, cor_atual_);
}

NoConceito *MapaMental::adicionar_no(const QString &texto, const QPointF &pos, const QColor &cor)
{
    if (scene_ == nullptr || texto.trimmed().isEmpty()) {
        return nullptr;
    }

    auto *no = new NoConceito(pos.x(), pos.y(), texto.trimmed(), cor, this);
    no->setOpacity(0.0);
    no->setScale(0.82);
    scene_->addItem(no);
    nos_.append(no);

    auto animation_guard = std::make_shared<bool>(true);
    no_animation_guards_.insert(no, animation_guard);

    auto *animacao = new QVariantAnimation(this);
    animacao->setDuration(260);
    animacao->setEasingCurve(QEasingCurve::OutCubic);
    animacao->setStartValue(0.0);
    animacao->setEndValue(1.0);
    connect(animacao, &QVariantAnimation::valueChanged, this, [no, animation_guard](const QVariant &valor) {
        if (!*animation_guard) {
            return;
        }
        const qreal t = valor.toReal();
        no->setOpacity(t);
        no->setScale(0.82 + 0.18 * t);
    });
    connect(animacao, &QVariantAnimation::finished, this, [this, no, animation_guard, animacao]() {
        if (*animation_guard) {
            no_animation_guards_.remove(no);
        }
        animacao->deleteLater();
    });
    animacao->start();

    expandir_area_se_necessario(no);
    if (!carregando_em_lote_) {
        salvar_backup_automatico();
        refresh_counter();
    }
    return no;
}

}  // namespace source::plugin::MapaMental
