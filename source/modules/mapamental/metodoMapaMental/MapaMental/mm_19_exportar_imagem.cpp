#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_19_exportar_imagem.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QImage>
#include <QPainter>

#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"

namespace source::plugin::MapaMental {

void MapaMental::exportar_imagem()
{
    if (scene_ == nullptr) {
        return;
    }

    const QString caminho = QFileDialog::getSaveFileName(
        this,
        QCoreApplication::translate("MapaMentalPlugin", "Exportar imagem"),
        QDir::home().filePath(QStringLiteral("mapa_mental.png")),
        QCoreApplication::translate("MapaMentalPlugin", "PNG (*.png)"));
    if (caminho.isEmpty()) {
        return;
    }

    QRectF rect = scene_->itemsBoundingRect().adjusted(-80, -80, 80, 80);
    if (rect.isEmpty()) {
        rect = QRectF(-400, -300, 800, 600);
    }

    QImage image(rect.size().toSize().expandedTo(QSize(640, 480)), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    scene_->render(&painter, QRectF(QPointF(0, 0), image.size()), rect);
    image.save(caminho, "PNG");
}

}  // namespace source::plugin::MapaMental
