#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_04_atualizar_traducoes.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTarefas/gtar_internal.hpp"

#include <QEvent>

namespace source::plugin::Pomodoro {
using namespace GerenciadorTarefasInternal;

void GerenciadorTarefas::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event != nullptr && event->type() == QEvent::LanguageChange) {
        atualizar_traducoes();
    }
}

void GerenciadorTarefas::atualizar_traducoes()
{
    try {
        input_tarefa->setPlaceholderText(
            QCoreApplication::translate(
                "PomodoroPlugin",
                "Digite uma nova tarefa ou arraste um arquivo... (Ex: 'Estudar Resistencia dos Materiais - Cap 3')"));

        if (quadro_tarefas != nullptr) {
            quadro_tarefas->setTitle(QCoreApplication::translate("PomodoroPlugin", "📊 Quadro de Tarefas"));
        }

        combo_prioridade->clear();
        combo_prioridade->addItems({
            QCoreApplication::translate("PomodoroPlugin", "🔴 Importante e Urgente"),
            QCoreApplication::translate("PomodoroPlugin", "🟠 Importante, mas Nao Urgente"),
            QCoreApplication::translate("PomodoroPlugin", "🟡 Nao Importante, mas Urgente"),
            QCoreApplication::translate("PomodoroPlugin", "🟢 Nao Importante e Nao Urgente"),
        });
        combo_prioridade->setCurrentText(QCoreApplication::translate("PomodoroPlugin", "🔴 Importante e Urgente"));
        _ajustar_largura_combo(combo_prioridade);

        combo_dominio->clear();
        combo_dominio->addItems({
            GerenciadorTarefasFns::dominio_para_texto(0, false),
            GerenciadorTarefasFns::dominio_para_texto(1, false),
            GerenciadorTarefasFns::dominio_para_texto(2, false),
        });
        combo_dominio->setCurrentIndex(qBound(0, combo_dominio->currentIndex(), 2));
        _ajustar_largura_combo(combo_dominio);

        if (btn_adicionar != nullptr) {
            const int ref_height = btn_adicionar->sizeHint().height();
            if (ref_height > 0) {
                input_tarefa->setFixedHeight(ref_height);
                combo_prioridade->setFixedHeight(ref_height);
                combo_dominio->setFixedHeight(ref_height);
            }
        }

        if (label_prioridade != nullptr) {
            label_prioridade->setText(QCoreApplication::translate("PomodoroPlugin", "Prioridade:"));
        }
        if (label_dominio != nullptr) {
            label_dominio->setText(QCoreApplication::translate("PomodoroPlugin", "Nivel de Dominio:"));
        }
        const QString fluxo_tarefas_text =
            QCoreApplication::translate("PomodoroPlugin", "Fluxo Tarefas Concluídas");
        if (label_fluxo_tarefas_concluidas != nullptr) {
            label_fluxo_tarefas_concluidas->setText(fluxo_tarefas_text);
        }

        gerenciador_botoes_.set_button_text(btn_adicionar, QCoreApplication::translate("PomodoroPlugin", "➕ Adicionar"));
        gerenciador_botoes_.set_button_text(btn_remover, QCoreApplication::translate("PomodoroPlugin", "🗑️ Remover Tarefa(s)"));
        gerenciador_botoes_.set_button_text(btn_mover, QCoreApplication::translate("PomodoroPlugin", "🔀 Mover Tarefa(s)"));
        gerenciador_botoes_.set_button_text(btn_resetar_contagem, QCoreApplication::translate("PomodoroPlugin", "🔄 Resetar Contagem"));

        chk_fluxo_tarefas_concluidas->setText(_layout_robusto() ? QString() : fluxo_tarefas_text);

        col_todo->setTitle(QCoreApplication::translate("PomodoroPlugin", "📋 A Fazer"));
        col_doing->setTitle(QCoreApplication::translate("PomodoroPlugin", "⚙️ Em Progresso"));
        col_done->setTitle(QCoreApplication::translate("PomodoroPlugin", "✅ Concluído"));

        if (status_tabs != nullptr) {
            status_tabs->setTabText(0, QCoreApplication::translate("PomodoroPlugin", "📋 A Fazer"));
            status_tabs->setTabText(1, QCoreApplication::translate("PomodoroPlugin", "⚙️ Em Progresso"));
            status_tabs->setTabText(2, QCoreApplication::translate("PomodoroPlugin", "✅ Concluído"));
        }

        atualizar_listas();
        ajustar_linha_botoes();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao atualizar traduções do GerenciadorTarefas: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao atualizar traduções do GerenciadorTarefas"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTarefasFns { void atualizar_traducoes(GerenciadorTarefas &self) { self.atualizar_traducoes(); } }
