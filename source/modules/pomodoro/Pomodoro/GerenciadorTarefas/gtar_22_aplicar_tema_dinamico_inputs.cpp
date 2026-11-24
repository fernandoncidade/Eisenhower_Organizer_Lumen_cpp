#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_22_aplicar_tema_dinamico_inputs.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::_aplicar_tema_dinamico_inputs()
{
    try {
        if (applying_theme_inputs_) {
            return;
        }

        const QScopedValueRollback<bool> guard(applying_theme_inputs_, true);
        const QPalette app_palette = qApp != nullptr ? qApp->palette() : palette();
        const QColor base_color = app_palette.color(QPalette::Window);
        const QColor input_color = QColor(base_color).darker(105);
        const QColor task_report_color = base_color;

        const auto apply_list_bg = [](QListWidget *widget, const QColor &color) {
            if (widget == nullptr) {
                return;
            }

            QPalette pal = widget->palette();
            pal.setColor(QPalette::Base, color);
            pal.setColor(QPalette::AlternateBase, color);
            widget->setPalette(pal);
            widget->setAutoFillBackground(true);
            if (widget->viewport() != nullptr) {
                widget->viewport()->setPalette(pal);
                widget->viewport()->setAutoFillBackground(true);
            }
        };

        const auto apply_input_bg = [](QWidget *widget, const QColor &color) {
            if (widget == nullptr) {
                return;
            }

            QPalette pal = widget->palette();
            pal.setColor(QPalette::Base, color);
            pal.setColor(QPalette::AlternateBase, color);

            if (auto *combo = qobject_cast<QComboBox *>(widget)) {
                pal.setColor(QPalette::Button, color);
                pal.setColor(QPalette::Window, color);
                combo->setPalette(pal);
                if (combo->view() != nullptr) {
                    QPalette view_palette = combo->view()->palette();
                    view_palette.setColor(QPalette::Base, color);
                    view_palette.setColor(QPalette::AlternateBase, color);
                    combo->view()->setPalette(view_palette);
                }
            } else {
                widget->setPalette(pal);
            }
        };

        apply_input_bg(input_tarefa, input_color);
        apply_input_bg(combo_prioridade, input_color);
        apply_input_bg(combo_dominio, input_color);
        atualizar_cor_rotulos_molduras();
        apply_list_bg(col_todo->lista, task_report_color);
        apply_list_bg(col_doing->lista, task_report_color);
        apply_list_bg(col_done->lista, task_report_color);
        if (status_tabs != nullptr) {
            source::utils::AccentTabStyle::refresh(status_tabs);
        }
        update();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao aplicar tema dinâmico (inputs) no GerenciadorTarefas: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao aplicar tema dinâmico (inputs) no GerenciadorTarefas"), true);
    }
}

void GerenciadorTarefas::atualizar_cor_rotulos_molduras()
{
    const QPalette app_palette = qApp != nullptr ? qApp->palette() : palette();
    const QString title_style =
        QStringLiteral("QGroupBox::title { color: %1; }").arg(app_palette.color(QPalette::WindowText).name());

    for (QGroupBox *group : {static_cast<QGroupBox *>(col_todo),
                             static_cast<QGroupBox *>(col_doing),
                             static_cast<QGroupBox *>(col_done)}) {
        if (group == nullptr) {
            continue;
        }
        if (group->styleSheet() != title_style) {
            group->setStyleSheet(title_style);
        }
        group->update();
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void aplicar_tema_dinamico_inputs(GerenciadorTarefas &self) { self._aplicar_tema_dinamico_inputs(); } }
