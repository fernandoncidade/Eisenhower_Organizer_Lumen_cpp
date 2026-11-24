#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_21_ajustar_largura_combo.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::_ajustar_largura_combo(QComboBox *combo, int padding_extra, int max_visiveis)
{
    try {
        if (combo == nullptr) {
            return;
        }

        int max_width = 0;
        for (int i = 0; i < combo->count(); ++i) {
            max_width = qMax(max_width, combo->fontMetrics().horizontalAdvance(combo->itemText(i)));
        }

        const int width = max_width + padding_extra;
        combo->setFixedWidth(width);
        combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        combo->setMaxVisibleItems(max_visiveis);

        if (auto *view = combo->view()) {
            view->setMinimumWidth(width);
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao ajustar largura do combo: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao ajustar largura do combo"), true);
    }
}

void GerenciadorTarefas::ajustar_linha_botoes()
{
    try {
        if (ajustando_linha_botoes_) {
            return;
        }
        QScopedValueRollback<bool> guard(ajustando_linha_botoes_, true);

        const bool robust_layout = _layout_robusto();
        constexpr int row_spacing = 6;
        auto widget_width = [](QWidget *widget) {
            return widget != nullptr ? qMax(widget->minimumSizeHint().width(), widget->sizeHint().width()) : 0;
        };

        int buttons_width = 0;
        int buttons_row_width = 0;
        int visible_buttons = 0;
        for (const QPointer<QPushButton> &button : botoes_linha_principal_) {
            if (button != nullptr) {
                const int width = widget_width(button);
                button->setMinimumWidth(width);
                buttons_width = qMax(buttons_width, width);
                buttons_row_width += width;
                ++visible_buttons;
            }
        }
        if (visible_buttons > 1) {
            buttons_row_width += (visible_buttons - 1) * row_spacing;
        }

        int seletores_width = 0;
        int seletores_row_width = 0;
        int visible_selectors = 0;
        const QList<QWidget *> selector_widgets = {
            label_prioridade,
            combo_prioridade,
            label_dominio,
            combo_dominio,
            chk_fluxo_tarefas_concluidas,
            label_fluxo_tarefas_concluidas,
        };
        for (QWidget *widget : selector_widgets) {
            if (widget == nullptr) {
                continue;
            }
            const int width = widget_width(widget);
            seletores_width = qMax(seletores_width, width);
            seletores_row_width += width;
            ++visible_selectors;
        }
        if (visible_selectors > 1) {
            seletores_row_width += (visible_selectors - 1) * row_spacing;
        }

        if (chk_fluxo_tarefas_concluidas != nullptr) {
            const int fluxo_width = widget_width(chk_fluxo_tarefas_concluidas);
            chk_fluxo_tarefas_concluidas->setMinimumWidth(fluxo_width);
            chk_fluxo_tarefas_concluidas->setMaximumWidth(fluxo_width);
            seletores_width = qMax(seletores_width, fluxo_width);
        }

        QWidget *target = quadro_tarefas != nullptr ? static_cast<QWidget *>(quadro_tarefas) : static_cast<QWidget *>(this);
        const QMargins target_margins = quadro_tarefas != nullptr && quadro_tarefas->layout() != nullptr
            ? quadro_tarefas->layout()->contentsMargins()
            : QMargins();
        int input_width = 0;
        if (input_tarefa != nullptr) {
            input_width = qMax(input_tarefa->minimumSizeHint().width(), input_tarefa->sizeHint().width());
        }

        const int controls_width = robust_layout ? qMax(buttons_row_width, seletores_row_width)
                                                 : buttons_width + seletores_width + 12;
        const int required_width =
            qMax(input_width, controls_width) + target_margins.left() + target_margins.right();
        const int new_min_width = qMax(320, required_width);
        if (target->minimumWidth() != new_min_width) {
            target->setMinimumWidth(new_min_width);
        }
        target->updateGeometry();
    } catch (...) {
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void ajustar_largura_combo(GerenciadorTarefas &self, QComboBox *combo, int padding_extra, int max_visiveis) { self._ajustar_largura_combo(combo, padding_extra, max_visiveis); } }
