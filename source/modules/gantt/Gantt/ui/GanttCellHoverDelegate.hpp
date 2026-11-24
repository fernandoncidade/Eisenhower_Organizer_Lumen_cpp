#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QPalette>
#include <QColor>
#include <QBrush>
#include <QVariant>
#include <algorithm>

namespace source::gantt {

/// Delegate que personaliza o realce ao passar o mouse ou selecionar células das tabelas.
///
/// Substitui o realce azul nativo do Windows 11 por:
///   • Tema Escuro: uma tonalidade mais clara da cor padrão de preenchimento do fundo da célula.
///   • Tema Claro: uma tonalidade mais escura da cor padrão de preenchimento do fundo da célula.
///
/// Este realce se propaga tanto para células sob o mouse (hover) quanto para células selecionadas
/// (célula individual, múltiplas células, coluna inteira, múltiplas colunas, linha inteira, múltiplas linhas).
class GanttCellHoverDelegate : public QStyledItemDelegate {
public:
    explicit GanttCellHoverDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        const bool is_hovered = (opt.state & QStyle::State_MouseOver);
        const bool is_selected = (opt.state & QStyle::State_Selected);

        if (is_hovered || is_selected) {
            // Remove os estados nativos para suprimir o azul padrão do Windows 11 (LISS_HOT e seleção azul)
            opt.state &= ~QStyle::State_MouseOver;
            opt.state &= ~QStyle::State_Selected;

            // 1. Detecta confiavelmente se o tema ativo do Windows 11 é escuro ou claro
            const QColor base_color = opt.palette.color(QPalette::Base);
            const QColor window_color = opt.palette.color(QPalette::Window);
            const bool is_dark = (base_color.lightnessF() < 0.5) || (window_color.lightnessF() < 0.5);

            // 2. Obtém a cor padrão de preenchimento do fundo desta célula específica
            QColor cell_bg;
            const QVariant bg_var = index.data(Qt::BackgroundRole);
            if (bg_var.isValid() && bg_var.canConvert<QBrush>() && bg_var.value<QBrush>().style() != Qt::NoBrush) {
                cell_bg = bg_var.value<QBrush>().color();
            } else {
                const bool is_alternate = (opt.features & QStyleOptionViewItem::Alternate)
                                          || (index.row() % 2 == 1);
                if (!is_alternate) {
                    // Linha padrão: usa a cor Base da tabela
                    cell_bg = base_color;
                } else {
                    // Linha alternada:
                    // No tema escuro: tonalidade de cinza ligeiramente mais clara que a Base
                    // No tema claro: tonalidade de cinza ligeiramente mais escura que a Base
                    const QColor alt_candidate = opt.palette.color(QPalette::AlternateBase);
                    if (is_dark) {
                        if (alt_candidate.isValid() && alt_candidate.lightnessF() < 0.5 && alt_candidate.lightnessF() > base_color.lightnessF()) {
                            cell_bg = alt_candidate;
                        } else {
                            // Linha alternada no tema escuro: tom ligeiramente mais claro (~ +12)
                            cell_bg = QColor(
                                std::min(255, base_color.red() + 12),
                                std::min(255, base_color.green() + 12),
                                std::min(255, base_color.blue() + 12)
                            );
                        }
                    } else {
                        if (alt_candidate.isValid() && alt_candidate.lightnessF() >= 0.5 && alt_candidate.lightnessF() < base_color.lightnessF()) {
                            cell_bg = alt_candidate;
                        } else {
                            // Linha alternada no tema claro: tom ligeiramente mais escuro (~ -16)
                            cell_bg = QColor(
                                std::max(0, base_color.red() - 16),
                                std::max(0, base_color.green() - 16),
                                std::max(0, base_color.blue() - 16)
                            );
                        }
                    }
                }
            }

            // 3. Calcula a intensidade do realce:
            //    • Selecionada + Hover: maior destaque
            //    • Selecionada: destaque intermediário
            //    • Hover simples: destaque suave
            int delta = 0;
            if (is_selected && is_hovered) {
                delta = 30;
            } else if (is_selected) {
                delta = 24;
            } else {
                delta = 18;
            }

            QColor highlight_color;
            if (is_dark) {
                // Tema Escuro: realce com uma tonalidade mais clara que o fundo da célula
                highlight_color = QColor(
                    std::min(255, cell_bg.red() + delta),
                    std::min(255, cell_bg.green() + delta),
                    std::min(255, cell_bg.blue() + delta)
                );
            } else {
                // Tema Claro: realce com uma tonalidade mais escura que o fundo da célula
                highlight_color = QColor(
                    std::max(0, cell_bg.red() - delta),
                    std::max(0, cell_bg.green() - delta),
                    std::max(0, cell_bg.blue() - delta)
                );
            }

            // 4. Pinta o fundo da célula com a cor de realce calculada
            painter->save();
            painter->fillRect(opt.rect, highlight_color);
            painter->restore();

            // 5. Limpa o brush de fundo da opção para que a pintura de texto e ícones não sobrescreva o fundo
            opt.backgroundBrush = QBrush(Qt::NoBrush);
        }

        QStyledItemDelegate::paint(painter, opt, index);
    }
};

} // namespace source::gantt
