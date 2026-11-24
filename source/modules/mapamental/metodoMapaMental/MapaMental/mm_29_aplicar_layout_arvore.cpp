#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_29_aplicar_layout_arvore.hpp"

#include <algorithm>
#include <cmath>

#include <QEasingCurve>
#include <QHash>
#include <QRectF>
#include <QSet>
#include <QSizeF>
#include <QVariantAnimation>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_02_LinhaConexao.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_07_SmoothGraphicsView.hpp"

namespace source::plugin::MapaMental {
namespace {

constexpr qreal kSiblingSpacing = 220.0;
constexpr qreal kLevelSpacing = 180.0;
constexpr qreal kRadialLevelSpacing = 260.0;
constexpr qreal kCompactSiblingSpacing = 178.0;
constexpr qreal kCompactLevelSpacing = 148.0;
constexpr qreal kCompactRadialLevelSpacing = 190.0;
constexpr qreal kNodeSafetyWidth = 190.0;
constexpr qreal kCompactNodeSafetyWidth = 168.0;
constexpr qreal kPi = 3.14159265358979323846;

struct LayoutGraph {
    QHash<NoConceito *, QList<NoConceito *>> children;
    QList<NoConceito *> roots;
};

struct LayoutMetrics {
    qreal sibling_spacing = kSiblingSpacing;
    qreal level_spacing = kLevelSpacing;
    qreal radial_level_spacing = kRadialLevelSpacing;
    qreal node_safety_width = kNodeSafetyWidth;
};

struct LayoutState {
    QHash<NoConceito *, qreal> slot;
    QHash<NoConceito *, int> depth;
    QSet<NoConceito *> assigned;
    int max_depth = 0;
    qreal next_slot = 0.0;
};

void append_unique(QList<NoConceito *> &items, NoConceito *node)
{
    if (node != nullptr && !items.contains(node)) {
        items.append(node);
    }
}

LayoutGraph build_layout_graph(const QList<NoConceito *> &nodes, const QList<LinhaConexao *> &connections)
{
    LayoutGraph graph;
    QHash<NoConceito *, NoConceito *> primary_parent;
    QSet<NoConceito *> valid_nodes;

    for (NoConceito *node : nodes) {
        if (node != nullptr) {
            valid_nodes.insert(node);
        }
    }

    for (LinhaConexao *line : connections) {
        if (line == nullptr || line->no_inicio() == nullptr || line->no_fim() == nullptr) {
            continue;
        }

        NoConceito *parent = line->no_inicio();
        NoConceito *child = line->no_fim();
        if (!valid_nodes.contains(parent) || !valid_nodes.contains(child) || parent == child) {
            continue;
        }

        if (primary_parent.contains(child)) {
            continue;
        }

        primary_parent.insert(child, parent);
        append_unique(graph.children[parent], child);
    }

    for (NoConceito *node : nodes) {
        if (node != nullptr && !primary_parent.contains(node)) {
            graph.roots.append(node);
        }
    }
    if (graph.roots.isEmpty() && !nodes.isEmpty()) {
        graph.roots.append(nodes.first());
    }

    return graph;
}

QList<NoConceito *> visible_nodes(const QList<NoConceito *> &nodes)
{
    QList<NoConceito *> visible;
    visible.reserve(nodes.size());
    for (NoConceito *node : nodes) {
        if (node != nullptr && node->isVisible()) {
            visible.append(node);
        }
    }
    return visible;
}

LayoutGraph build_compact_hierarchy_graph(const QList<NoConceito *> &nodes,
                                          const QHash<NoConceito *, QList<NoConceito *>> &hierarchy_children,
                                          const QSet<NoConceito *> &expanded,
                                          NoConceito *root)
{
    LayoutGraph graph;
    QSet<NoConceito *> visible;
    QSet<NoConceito *> visible_children;

    for (NoConceito *node : nodes) {
        if (node != nullptr) {
            visible.insert(node);
        }
    }

    for (NoConceito *parent : nodes) {
        if (parent == nullptr || !expanded.contains(parent)) {
            continue;
        }

        const QList<NoConceito *> child_nodes = hierarchy_children.value(parent);
        for (NoConceito *child : child_nodes) {
            if (visible.contains(child)) {
                append_unique(graph.children[parent], child);
                visible_children.insert(child);
            }
        }
    }

    if (root != nullptr && visible.contains(root)) {
        append_unique(graph.roots, root);
    }
    for (NoConceito *node : nodes) {
        if (node != nullptr && !visible_children.contains(node)) {
            append_unique(graph.roots, node);
        }
    }
    if (graph.roots.isEmpty() && !nodes.isEmpty()) {
        graph.roots.append(nodes.first());
    }

    return graph;
}

void assign_tree_slots(NoConceito *node,
                       int depth,
                       const QHash<NoConceito *, QList<NoConceito *>> &children,
                       LayoutState &state,
                       QSet<NoConceito *> &stack)
{
    if (node == nullptr || state.assigned.contains(node) || stack.contains(node)) {
        return;
    }

    stack.insert(node);
    state.depth.insert(node, depth);
    state.max_depth = std::max(state.max_depth, depth);

    QList<qreal> child_slots;
    const QList<NoConceito *> child_nodes = children.value(node);
    for (NoConceito *child : child_nodes) {
        if (child == nullptr || stack.contains(child) || state.assigned.contains(child)) {
            continue;
        }

        assign_tree_slots(child, depth + 1, children, state, stack);
        if (state.slot.contains(child)) {
            child_slots.append(state.slot.value(child));
        }
    }

    if (child_slots.isEmpty()) {
        state.slot.insert(node, state.next_slot);
        state.next_slot += 1.0;
    } else {
        const auto [min_it, max_it] = std::minmax_element(child_slots.constBegin(), child_slots.constEnd());
        state.slot.insert(node, (*min_it + *max_it) / 2.0);
    }

    stack.remove(node);
    state.assigned.insert(node);
}

LayoutState assign_layout_slots(const QList<NoConceito *> &nodes, const LayoutGraph &graph)
{
    LayoutState state;
    QSet<NoConceito *> stack;

    for (NoConceito *root : graph.roots) {
        const qreal before = state.next_slot;
        assign_tree_slots(root, 0, graph.children, state, stack);
        if (state.next_slot > before) {
            state.next_slot += 1.0;
        }
    }

    for (NoConceito *node : nodes) {
        if (node == nullptr || state.assigned.contains(node)) {
            continue;
        }

        assign_tree_slots(node, 0, graph.children, state, stack);
        state.next_slot += 1.0;
    }

    state.next_slot = std::max<qreal>(1.0, state.next_slot);
    return state;
}

LayoutMetrics layout_metrics(bool compact)
{
    if (!compact) {
        return {};
    }

    return LayoutMetrics{kCompactSiblingSpacing,
                         kCompactLevelSpacing,
                         kCompactRadialLevelSpacing,
                         kCompactNodeSafetyWidth};
}

QPointF axis_position(MapaMental::LayoutMode mode,
                      NoConceito *node,
                      const LayoutState &state,
                      const LayoutMetrics &metrics)
{
    const qreal slot = state.slot.value(node);
    const int depth = state.depth.value(node);

    switch (mode) {
    case MapaMental::LayoutMode::Horizontal:
        return QPointF(depth * metrics.level_spacing, slot * metrics.sibling_spacing);
    case MapaMental::LayoutMode::Tree:
        return QPointF(slot * metrics.sibling_spacing, (state.max_depth - depth) * metrics.level_spacing);
    case MapaMental::LayoutMode::Vertical:
    default:
        return QPointF(slot * metrics.sibling_spacing, depth * metrics.level_spacing);
    }
}

QHash<NoConceito *, QPointF> axis_layout_positions(const QList<NoConceito *> &nodes,
                                                   MapaMental::LayoutMode mode,
                                                   const LayoutState &state,
                                                   const LayoutMetrics &metrics)
{
    QHash<NoConceito *, QPointF> positions;
    for (NoConceito *node : nodes) {
        if (node != nullptr && state.slot.contains(node)) {
            positions.insert(node, axis_position(mode, node, state, metrics));
        }
    }
    return positions;
}

QHash<NoConceito *, QPointF> radial_layout_positions(const QList<NoConceito *> &nodes,
                                                     const LayoutGraph &graph,
                                                     const LayoutState &state,
                                                     const LayoutMetrics &metrics)
{
    QHash<NoConceito *, QPointF> positions;
    const qreal leaf_slots = std::max<qreal>(1.0, state.next_slot - 1.0);
    const qreal radial_spacing =
        std::max(metrics.radial_level_spacing, leaf_slots * metrics.node_safety_width / (2.0 * kPi));
    const bool single_root = graph.roots.size() == 1;
    NoConceito *root = single_root ? graph.roots.first() : nullptr;

    for (NoConceito *node : nodes) {
        if (node == nullptr || !state.slot.contains(node)) {
            continue;
        }

        const int depth = state.depth.value(node);
        if (single_root && node == root) {
            positions.insert(node, QPointF(0.0, 0.0));
            continue;
        }

        const qreal slot = state.slot.value(node);
        const qreal angle = ((slot + 0.5) / leaf_slots) * 2.0 * kPi - (kPi / 2.0);
        const qreal radius = (depth + (single_root ? 0 : 1)) * radial_spacing;
        positions.insert(node, QPointF(std::cos(angle) * radius, std::sin(angle) * radius));
    }

    return positions;
}

void center_positions(QHash<NoConceito *, QPointF> &positions)
{
    if (positions.isEmpty()) {
        return;
    }

    QRectF bounds;
    bool first = true;
    for (auto it = positions.constBegin(); it != positions.constEnd(); ++it) {
        const QRectF node_rect(it.value() - QPointF(80.0, 56.0), QSizeF(160.0, 112.0));
        bounds = first ? node_rect : bounds.united(node_rect);
        first = false;
    }

    const QPointF offset = -bounds.center();
    for (auto it = positions.begin(); it != positions.end(); ++it) {
        it.value() += offset;
    }
}

}  // namespace

void MapaMental::aplicar_layout_arvore()
{
    if (nos_.isEmpty()) {
        return;
    }

    const int animation_generation = ++layout_animation_generation_;
    const bool compact_hierarchy = navegacao_hierarquia_ativa_;
    const QList<NoConceito *> layout_nodes = compact_hierarchy ? visible_nodes(nos_) : nos_;
    if (layout_nodes.isEmpty()) {
        return;
    }

    const LayoutGraph graph = compact_hierarchy
                                  ? build_compact_hierarchy_graph(layout_nodes,
                                                                  hierarquia_filhos_,
                                                                  hierarquia_expandida_,
                                                                  hierarquia_raiz_)
                                  : build_layout_graph(layout_nodes, conexoes_);
    const LayoutState state = assign_layout_slots(layout_nodes, graph);
    const LayoutMetrics metrics = layout_metrics(compact_hierarchy);

    QHash<NoConceito *, QPointF> target_positions =
        layout_mode_ == LayoutMode::Radial ? radial_layout_positions(layout_nodes, graph, state, metrics)
                                           : axis_layout_positions(layout_nodes, layout_mode_, state, metrics);
    center_positions(target_positions);

    for (NoConceito *node : nos_) {
        if (node != nullptr) {
            node->set_layout_animating(false);
        }
    }

    QHash<NoConceito *, QPointF> start_positions;
    for (NoConceito *node : layout_nodes) {
        if (node == nullptr || !target_positions.contains(node)) {
            continue;
        }
        start_positions.insert(node, node->pos());
        node->set_layout_animating(true);
    }

    auto *animacao = new QVariantAnimation(this);
    animacao->setDuration(460);
    animacao->setEasingCurve(QEasingCurve::OutCubic);
    animacao->setStartValue(0.0);
    animacao->setEndValue(1.0);
    connect(animacao,
            &QVariantAnimation::valueChanged,
            this,
            [this, start_positions, target_positions, animation_generation](const QVariant &valor) {
                if (animation_generation != layout_animation_generation_) {
                    return;
                }
                const qreal t = valor.toReal();
                for (auto it = target_positions.constBegin(); it != target_positions.constEnd(); ++it) {
                    NoConceito *node = it.key();
                    if (node == nullptr || !nos_.contains(node)) {
                        continue;
                    }
                    const QPointF start = start_positions.value(node);
                    const QPointF target = it.value();
                    node->setPos(start + (target - start) * t);
                }
            });
    connect(animacao, &QVariantAnimation::finished, this, [this, target_positions, animacao, animation_generation]() {
        if (animation_generation != layout_animation_generation_) {
            animacao->deleteLater();
            return;
        }

        QRectF bounds;
        bool first = true;
        for (auto it = target_positions.constBegin(); it != target_positions.constEnd(); ++it) {
            NoConceito *node = it.key();
            if (node == nullptr || !nos_.contains(node)) {
                continue;
            }
            node->setPos(it.value());
            node->set_layout_animating(false);
            bounds = first ? node->sceneBoundingRect() : bounds.united(node->sceneBoundingRect());
            first = false;
        }

        for (LinhaConexao *linha : conexoes_) {
            if (linha != nullptr) {
                linha->atualizar_posicao();
            }
        }

        if (scene_ != nullptr && !bounds.isNull()) {
            scene_->setSceneRect(bounds.adjusted(-520.0, -420.0, 520.0, 420.0));
        }
        salvar_backup_automatico();
        animacao->deleteLater();
    });
    animacao->start();
}

}  // namespace source::plugin::MapaMental
