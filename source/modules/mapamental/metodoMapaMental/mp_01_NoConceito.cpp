#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"

#include <QBrush>
#include <QCoreApplication>
#include <QCursor>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRadialGradient>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

#include "source/modules/mapamental/metodoMapaMental/mp_02_LinhaConexao.hpp"
#include "source/utils/LinkedTitleUnderlineDelegate.hpp"

namespace source::plugin::MapaMental {

NoConceito::NoConceito(qreal x, qreal y, const QString &texto, const QColor &cor, MapaMental *owner)
    : QGraphicsEllipseItem(-64, -42, 128, 84), owner_(owner), texto_(texto), cor_(cor)
{
    setPos(x, y);
    setBrush(QBrush(QColor(cor.red(), cor.green(), cor.blue(), 190)));
    setPen(QPen(QColor(255, 255, 255, 48), 2));

    auto *shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 3);
    shadow->setColor(QColor(0, 0, 0, 130));
    setGraphicsEffect(shadow);

    texto_item_ = new QGraphicsTextItem(texto_, this);
    texto_item_->setDefaultTextColor(QColor(245, 245, 245));
    texto_item_->setTextWidth(108);
    texto_item_->setAcceptedMouseButtons(Qt::NoButton);
    update_font();
    update_text_geometry();

    badge_ = new QGraphicsEllipseItem(-4, -35, 8, 8, this);
    badge_->setBrush(QBrush(QColor("#ffb900")));
    badge_->setPen(QPen(Qt::NoPen));
    badge_->setVisible(false);
    badge_->setAcceptedMouseButtons(Qt::NoButton);

    setFlag(QGraphicsEllipseItem::ItemIsMovable);
    setFlag(QGraphicsEllipseItem::ItemIsSelectable);
    setFlag(QGraphicsEllipseItem::ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
}

void NoConceito::set_texto(const QString &texto)
{
    texto_ = texto.trimmed();
    if (texto_item_ != nullptr) {
        texto_item_->setPlainText(texto_);
        update_text_geometry();
    }
}

void NoConceito::update_font()
{
    if (texto_item_ != nullptr) {
        source::utils::TaskTitleFontSettings settings;
        if (!font_config_.isEmpty()) {
            settings = source::utils::task_title_font_settings_from_map(font_config_);
        } else {
            settings = !arquivo_vinculado_.isEmpty() ? source::utils::load_plugin_linked_file_title_font_settings(source::utils::PluginFontContext::MapaMental)
                                                     : source::utils::load_plugin_general_task_title_font_settings(source::utils::PluginFontContext::MapaMental);
        }
        
        QFont base_font(QStringLiteral("Segoe UI"), 10, QFont::Bold);
        QFont new_font = source::utils::task_title_font(base_font, settings);
        texto_item_->setFont(new_font);
        
        const bool active = !texto_.isEmpty() && (settings.underline || settings.strike_out);
        QFont f = texto_item_->font();
        f.setUnderline(active ? settings.underline : false);
        f.setStrikeOut(active ? settings.strike_out : false);
        texto_item_->setFont(f);
        
        update_text_geometry();
    }
}

void NoConceito::set_cor(const QColor &cor)
{
    cor_ = cor;
    setBrush(QBrush(QColor(cor.red(), cor.green(), cor.blue(), 190)));
}

void NoConceito::set_arquivo_vinculado(const QString &caminho)
{
    arquivo_vinculado_ = caminho.trimmed();
    if (badge_ != nullptr) {
        badge_->setVisible(!arquivo_vinculado_.isEmpty());
    }
    update_font();
    setToolTip(arquivo_vinculado_.isEmpty()
                   ? QString()
                   : QCoreApplication::translate("MapaMentalPlugin",
                                                 "Ctrl + duplo clique abre o arquivo vinculado.\n%1")
                         .arg(arquivo_vinculado_));
}

void NoConceito::set_font_config(const QVariantMap &config)
{
    font_config_ = config;
    update_font();
}

QRectF NoConceito::boundingRect() const
{
    return QGraphicsEllipseItem::boundingRect().adjusted(-14.0, -14.0, 14.0, 14.0);
}

void NoConceito::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    if (painter == nullptr) {
        return;
    }

    const bool active = hovered_ || (option != nullptr && (option->state & QStyle::State_Selected));
    const QRectF body = rect().adjusted(2.0, 2.0, -2.0, -2.0);
    const QColor base = cor_.isValid() ? cor_ : QColor("#4d8df7");
    QColor light = base.lighter(active ? 152 : 134);
    QColor deep = base.darker(active ? 118 : 132);
    light.setAlpha(active ? 242 : 226);
    deep.setAlpha(220);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (active) {
        QColor glow_color(base.red(), base.green(), base.blue(), 78);
        painter->setPen(QPen(glow_color, 10.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(body.adjusted(-4.0, -4.0, 4.0, 4.0));
    }

    QRadialGradient fill(body.center() + QPointF(-body.width() * 0.24, -body.height() * 0.32),
                         body.width() * 0.84);
    fill.setColorAt(0.0, QColor(255, 255, 255, active ? 118 : 88));
    fill.setColorAt(0.24, light);
    fill.setColorAt(1.0, deep);
    painter->setBrush(fill);
    painter->setPen(QPen(QColor(255, 255, 255, active ? 180 : 92), active ? 2.2 : 1.4));
    painter->drawEllipse(body);

    QPainterPath highlight;
    highlight.moveTo(body.left() + body.width() * 0.22, body.top() + body.height() * 0.32);
    highlight.cubicTo(body.left() + body.width() * 0.40,
                      body.top() + body.height() * 0.08,
                      body.left() + body.width() * 0.72,
                      body.top() + body.height() * 0.14,
                      body.right() - body.width() * 0.18,
                      body.top() + body.height() * 0.38);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(QColor(255, 255, 255, active ? 106 : 70), 1.1, Qt::SolidLine, Qt::RoundCap));
    painter->drawPath(highlight);

    painter->restore();
}

void NoConceito::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hovered_ = true;
    setScale(1.035);
    setCursor(QCursor(Qt::PointingHandCursor));
    update();
    QGraphicsEllipseItem::hoverEnterEvent(event);
}

void NoConceito::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hovered_ = false;
    setScale(1.0);
    unsetCursor();
    update();
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

void NoConceito::add_conexao(LinhaConexao *linha)
{
    if (linha != nullptr && !conexoes_.contains(linha)) {
        conexoes_.append(linha);
    }
}

void NoConceito::remove_conexao(LinhaConexao *linha)
{
    conexoes_.removeAll(linha);
}

void NoConceito::update_text_geometry()
{
    if (texto_item_ == nullptr) {
        return;
    }

    const QRectF rect = texto_item_->boundingRect();
    texto_item_->setPos(-rect.width() / 2.0, -rect.height() / 2.0);
}

}  // namespace source::plugin::MapaMental
