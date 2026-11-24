#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_16_ativar_regua_foco.hpp"

#include "source/modules/audiolivro/metodoLeitor/Foco/agregador_01_Foco.hpp"

#include <QCursor>
#include <QGuiApplication>
#include <QPushButton>
#include <QScreen>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::ativar_regua_foco()
{
    if (regua_ == nullptr) {
        regua_ = new FocusRuler();
        connect(regua_, &FocusRuler::closed, this, &LeitorAcessivel::regua_fechada);
    }

    const QRect screen = QGuiApplication::screenAt(QCursor::pos()) != nullptr
                             ? QGuiApplication::screenAt(QCursor::pos())->availableGeometry()
                             : QGuiApplication::primaryScreen()->availableGeometry();
    regua_->move(screen.center() - QPoint(regua_->width() / 2, regua_->height() / 2));
    regua_->show();
    regua_->raise();
    regua_->activateWindow();
    if (btn_regua_ != nullptr) {
        btn_regua_->setChecked(true);
        update_regua_foco_button();
    }
}

}  // namespace source::plugin::Audiolivro
