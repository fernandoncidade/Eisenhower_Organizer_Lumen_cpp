#pragma once

#include <QColor>
#include <QGraphicsEllipseItem>
#include <QRectF>
#include <QStringList>
#include <QVariantMap>

class QGraphicsSceneHoverEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

namespace source::plugin::MapaMental {

class LinhaConexao;
class MapaMental;

class NoConceito final : public QGraphicsEllipseItem {
public:
    NoConceito(qreal x, qreal y, const QString &texto, const QColor &cor, MapaMental *owner);

    QString texto() const { return texto_; }
    void set_texto(const QString &texto);

    QColor cor() const { return cor_; }
    void set_cor(const QColor &cor);

    QString notas() const { return notas_; }
    void set_notas(const QString &notas) { notas_ = notas; }

    void set_layout_animating(bool animating) { layout_animating_ = animating; }

    QString arquivo_vinculado() const { return arquivo_vinculado_; }
    void set_arquivo_vinculado(const QString &caminho);

    QStringList info_associada() const { return info_associada_; }
    void set_info_associada(const QStringList &info) { info_associada_ = info; }

    QVariantMap font_config() const { return font_config_; }
    void set_font_config(const QVariantMap &config);

    void add_conexao(LinhaConexao *linha);
    void remove_conexao(LinhaConexao *linha);
    QList<LinhaConexao *> conexoes() const { return conexoes_; }

    void update_font();

protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    bool abrir_editor_conceito();
    void update_text_geometry();

    MapaMental *owner_ = nullptr;
    QGraphicsTextItem *texto_item_ = nullptr;
    QGraphicsEllipseItem *badge_ = nullptr;
    QList<LinhaConexao *> conexoes_;
    QString texto_;
    QColor cor_;
    QString notas_;
    QStringList info_associada_;
    QString arquivo_vinculado_;
    QVariantMap font_config_;
    bool hovered_ = false;
    bool layout_animating_ = false;
    bool ctrl_selection_click_ = false;
};

}  // namespace source::plugin::MapaMental
