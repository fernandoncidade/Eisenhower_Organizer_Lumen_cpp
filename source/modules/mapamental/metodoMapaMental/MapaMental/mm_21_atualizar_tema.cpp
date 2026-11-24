#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_21_atualizar_tema.hpp"

#include <QApplication>
#include <QPalette>
#include <QWidget>

#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_07_SmoothGraphicsView.hpp"

namespace source::plugin::MapaMental {

void MapaMental::atualizar_tema()
{
    if (atualizando_tema_ || view_ == nullptr) {
        return;
    }

    atualizando_tema_ = true;
    const QPalette palette = QApplication::palette();
    const QColor fundo = palette.color(QPalette::Window);
    view_->setBackgroundBrush(fundo);
    view_->setPalette(palette);
    view_->setAutoFillBackground(true);
    if (view_->viewport() != nullptr) {
        view_->viewport()->setPalette(palette);
        view_->viewport()->setAutoFillBackground(true);
        view_->viewport()->update();
    }
    if (scene_ != nullptr) {
        scene_->setBackgroundBrush(fundo);
        scene_->update();
    }
    view_->update();
    atualizando_tema_ = false;
}

}  // namespace source::plugin::MapaMental
