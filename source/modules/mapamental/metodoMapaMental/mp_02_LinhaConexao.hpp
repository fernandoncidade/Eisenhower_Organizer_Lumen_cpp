#pragma once

#include <QColor>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QRectF>

namespace source::plugin::MapaMental {

class NoConceito;

class LinhaConexao final : public QGraphicsPathItem {
public:
    LinhaConexao(NoConceito *no_inicio, NoConceito *no_fim);

    NoConceito *no_inicio() const { return no_inicio_; }
    NoConceito *no_fim() const { return no_fim_; }
    void atualizar_posicao();
    void set_reveal_progress(qreal progress);
    qreal reveal_progress() const { return reveal_progress_; }

private:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QPainterPath visible_path() const;
    QColor branch_color() const;

    NoConceito *no_inicio_ = nullptr;
    NoConceito *no_fim_ = nullptr;
    QPainterPath full_path_;
    qreal reveal_progress_ = 1.0;
};

}  // namespace source::plugin::MapaMental
