#include "source/modules/audiolivro/metodoLeitor/Foco/agregador_01_Foco.hpp"

#include <Qt>

namespace source::plugin::Audiolivro {

FocusRuler::FocusRuler(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    resize(680, 72);
}

}  // namespace source::plugin::Audiolivro
