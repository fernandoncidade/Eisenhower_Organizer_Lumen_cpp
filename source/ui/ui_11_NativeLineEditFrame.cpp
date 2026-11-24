#include "source/ui/ui_11_NativeLineEditFrame.hpp"

#include <QEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QVBoxLayout>

#include "source/utils/LogManager.hpp"

namespace source::GerenciamentoUI {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

}  // namespace

NativeLineEditFrame::NativeLineEditFrame(QWidget *child, QWidget *parent)
    : QWidget(parent), _child(child)
{
    try {
        _layout = new QVBoxLayout(this);
        _layout->setSpacing(0);
        _layout->setContentsMargins(0, 0, 0, 0);
        _layout->addWidget(_child);
        _update_margins_from_style();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao inicializar NativeLineEditFrame."));
    }
}

int NativeLineEditFrame::_frame_width() const
{
    return style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, const_cast<NativeLineEditFrame *>(this));
}

void NativeLineEditFrame::_update_margins_from_style()
{
    try {
        const int fw = _frame_width();
        _layout->setContentsMargins(fw, fw, fw, fw);
        updateGeometry();
        update();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao atualizar margens do frame nativo."));
    }
}

void NativeLineEditFrame::changeEvent(QEvent *event)
{
    try {
        if (event != nullptr
            && (event->type() == QEvent::StyleChange || event->type() == QEvent::PaletteChange
                || event->type() == QEvent::ApplicationPaletteChange)) {
            _update_margins_from_style();
        }
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao processar changeEvent no frame nativo."));
    }

    QWidget::changeEvent(event);
}

void NativeLineEditFrame::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    try {
        QPainter painter(this);
        QStyleOptionFrame opt;
        opt.initFrom(this);
        opt.rect = rect();
        opt.lineWidth = _frame_width();
        opt.midLineWidth = 0;
        opt.state |= QStyle::State_Sunken;
        style()->drawPrimitive(QStyle::PE_FrameLineEdit, &opt, &painter, this);
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao pintar frame nativo."));
    }
}

}  // namespace source::GerenciamentoUI
