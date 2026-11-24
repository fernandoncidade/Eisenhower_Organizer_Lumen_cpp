#include "source/ui/ui_13_RotatedTabButton.hpp"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QSize>

#include "source/utils/LogManager.hpp"

namespace source::GerenciamentoUI {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

}  // namespace

RotatedTabButton::RotatedTabButton(const QString &text, QWidget *parent)
    : QWidget(parent), _text(text)
{
    try {
        setMinimumWidth(25);
        _recompute_size();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao inicializar RotatedTabButton."));
    }
}

void RotatedTabButton::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event != nullptr && event->type() == QEvent::LanguageChange) {
        _recompute_size();
        update();
    }
}

void RotatedTabButton::setText(const QString &text)
{
    try {
        _text = text;
        _recompute_size();
        update();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao definir texto do botão de aba."));
    }
}

QString RotatedTabButton::text() const
{
    return _text;
}

void RotatedTabButton::setSizeReferenceTexts(const QStringList &texts)
{
    _size_reference_texts = texts;
    _recompute_size();
    update();
}

void RotatedTabButton::_recompute_size()
{
    try {
        QFontMetrics fm(font());
        int text_w = fm.horizontalAdvance(_text);
        for (const QString &reference_text : _size_reference_texts) {
            const int candidate_width = fm.horizontalAdvance(reference_text);
            if (candidate_width > text_w) {
                text_w = candidate_width;
            }
        }
        const int text_h = fm.height();

        if (_rotation_direction == RotationDirection::None) {
            const int width = text_w + 20;
            const int height = text_h + 10;
            setFixedSize(QSize(width, height));
        } else {
            const int width = text_h + 5;
            const int height = text_w + 20;
            setFixedSize(QSize(width, height));
        }
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao recalcular tamanho do botão de aba."));
    }
}

void RotatedTabButton::setClickHandler(std::function<void()> handler)
{
    _click_handler = std::move(handler);
}

void RotatedTabButton::setRotationDirection(RotationDirection direction)
{
    if (_rotation_direction == direction) {
        return;
    }

    _rotation_direction = direction;
    _recompute_size();
    update();
}

void RotatedTabButton::mousePressEvent(QMouseEvent *event)
{
    try {
        if (_click_handler) {
            _click_handler();
        } else if (parentWidget() != nullptr) {
            QMetaObject::invokeMethod(parentWidget(), "toggle_panel");
        }
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao processar clique no botão de aba."));
    }

    QWidget::mousePressEvent(event);
}

void RotatedTabButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    try {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.save();
        painter.fillRect(rect(), palette().button().color());
        painter.setPen(palette().dark().color());
        painter.drawRect(0, 0, width() - 1, height() - 1);

        if (_rotation_direction == RotationDirection::None) {
            painter.setPen(palette().buttonText().color());
            painter.drawText(rect(), Qt::AlignCenter, _text);
        } else {
            if (_rotation_direction == RotationDirection::CounterClockwise90) {
                painter.translate(width(), 0);
                painter.rotate(90);
            } else {
                painter.translate(0, height());
                painter.rotate(270);
            }

            painter.setPen(palette().buttonText().color());
            painter.drawText(QRectF(0, 0, height(), width()), Qt::AlignCenter, _text);
        }
        painter.restore();
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao pintar botão de aba."));
    }
}

}  // namespace source::GerenciamentoUI
