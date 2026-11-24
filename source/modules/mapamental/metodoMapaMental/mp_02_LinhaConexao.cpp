#include "source/modules/mapamental/metodoMapaMental/mp_02_LinhaConexao.hpp"

#include <cmath>

#include <QPainter>
#include <QPainterPathStroker>
#include <QPen>
#include <QStyleOptionGraphicsItem>

#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"

namespace source::plugin::MapaMental {

LinhaConexao::LinhaConexao(NoConceito *no_inicio, NoConceito *no_fim)
    : no_inicio_(no_inicio), no_fim_(no_fim)
{
    setZValue(-1.0);
    atualizar_posicao();
}

void LinhaConexao::atualizar_posicao()
{
    if (no_inicio_ == nullptr || no_fim_ == nullptr) {
        return;
    }

    const QPointF inicio = no_inicio_->scenePos();
    const QPointF fim = no_fim_->scenePos();
    const qreal dx = fim.x() - inicio.x();
    const qreal dy = fim.y() - inicio.y();

    QPainterPath path(inicio);
    if (std::abs(dx) >= std::abs(dy)) {
        const qreal bend = qBound<qreal>(80.0, std::abs(dx) * 0.48 + std::abs(dy) * 0.12, 260.0);
        const qreal direction = dx >= 0 ? 1.0 : -1.0;
        path.cubicTo(inicio + QPointF(direction * bend, dy * 0.06),
                     fim - QPointF(direction * bend, dy * 0.06),
                     fim);
    } else {
        const qreal bend = qBound<qreal>(72.0, std::abs(dy) * 0.48 + std::abs(dx) * 0.12, 260.0);
        const qreal direction = dy >= 0 ? 1.0 : -1.0;
        path.cubicTo(inicio + QPointF(dx * 0.06, direction * bend),
                     fim - QPointF(dx * 0.06, direction * bend),
                     fim);
    }

    prepareGeometryChange();
    full_path_ = path;
    setPath(full_path_);
}

void LinhaConexao::set_reveal_progress(qreal progress)
{
    reveal_progress_ = qBound<qreal>(0.0, progress, 1.0);
    update();
}

QRectF LinhaConexao::boundingRect() const
{
    return full_path_.boundingRect().adjusted(-34.0, -34.0, 34.0, 34.0);
}

QPainterPath LinhaConexao::shape() const
{
    QPainterPathStroker stroker;
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    stroker.setWidth(18.0);
    return stroker.createStroke(full_path_);
}

QPainterPath LinhaConexao::visible_path() const
{
    if (reveal_progress_ >= 0.995) {
        return full_path_;
    }

    QPainterPath result;
    const int segments = 54;
    const int visible_segments = qBound(1, static_cast<int>(std::ceil(segments * reveal_progress_)), segments);
    result.moveTo(full_path_.pointAtPercent(0.0));
    for (int i = 1; i <= visible_segments; ++i) {
        const qreal t = qMin<qreal>(1.0, static_cast<qreal>(i) / segments);
        result.lineTo(full_path_.pointAtPercent(t));
    }
    return result;
}

QColor LinhaConexao::branch_color() const
{
    const QColor start = no_inicio_ != nullptr ? no_inicio_->cor() : QColor("#4d8df7");
    const QColor end = no_fim_ != nullptr ? no_fim_->cor() : QColor("#6aa6ff");
    return QColor((start.red() + end.red()) / 2,
                  (start.green() + end.green()) / 2,
                  (start.blue() + end.blue()) / 2,
                  210);
}

void LinhaConexao::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    if (painter == nullptr || full_path_.isEmpty()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const QPainterPath current_path = visible_path();
    const QColor color = branch_color();

    QPen glow(QColor(color.red(), color.green(), color.blue(), 42), 8.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    glow.setCosmetic(true);
    painter->setPen(glow);
    painter->drawPath(current_path);

    QPen branch(color,
                option != nullptr && (option->state & QStyle::State_Selected) ? 3.4 : 2.4,
                Qt::SolidLine,
                Qt::RoundCap,
                Qt::RoundJoin);
    branch.setCosmetic(true);
    painter->setPen(branch);
    painter->drawPath(current_path);

    QPen highlight(QColor(255, 255, 255, 70), 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    highlight.setCosmetic(true);
    painter->setPen(highlight);
    painter->drawPath(current_path);

    if (reveal_progress_ > 0.92) {
        const QPointF end = full_path_.pointAtPercent(1.0);
        const qreal angle = full_path_.angleAtPercent(1.0);
        painter->translate(end);
        painter->rotate(-angle);
        painter->setPen(QPen(QColor(255, 255, 255, 72), 1));
        painter->setBrush(QColor(color.red(), color.green(), color.blue(), 150));
        QPainterPath leaf;
        leaf.moveTo(0, 0);
        leaf.cubicTo(-10, -9, -24, -5, -26, 5);
        leaf.cubicTo(-14, 11, -5, 8, 0, 0);
        painter->drawPath(leaf);
    }

    painter->restore();
}

}  // namespace source::plugin::MapaMental
