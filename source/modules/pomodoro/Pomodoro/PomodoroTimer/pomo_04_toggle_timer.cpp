#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_04_toggle_timer.hpp"

#include <QDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVariantList>
#include <QVariantMap>
#include <QVBoxLayout>

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_01_Tarefa.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_03_GerenciadorTarefas.hpp"
#include "source/modules/pomodoro/Pomodoro/tmp_05_GerenciadorTempo.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

void PomodoroTimer::set_gerenciador_tarefas(GerenciadorTarefas *gt)
{
    gerenciador_tarefas_ = gt;
}

GerenciadorTarefas *PomodoroTimer::gerenciador_tarefas() const
{
    if (gerenciador_tarefas_ != nullptr) {
        return gerenciador_tarefas_;
    }
    const QObject *p = parent();
    while (p != nullptr) {
        if (auto *tempo = qobject_cast<const source::plugin::Pomodoro::GerenciadorTempo *>(p)) {
            return tempo->tarefas;
        }
        p = p->parent();
    }
    if (window() != nullptr) {
        return window()->findChild<source::plugin::Pomodoro::GerenciadorTarefas *>();
    }
    return nullptr;
}

bool PomodoroTimer::verificar_e_avisar_discrepancia_ciclos()
{
    if (config_sync_timer_ != nullptr && config_sync_timer_->isActive()) {
        config_sync_timer_->stop();
        salvar_configuracoes_timer_persistidas(ciclos_permitidos, tempo_foco, tempo_descanso_curto);
        if (last_broadcasted_ciclos_ != ciclos_permitidos) {
            last_broadcasted_ciclos_ = ciclos_permitidos;
            emit ciclos_permitidos_alterados(ciclos_permitidos);
            if (compat::host_bridge() != nullptr) {
                compat::host_bridge()->notify_default_cycle_count_changed(ciclos_permitidos);
            }
        }
    }

    if (mismatch_acknowledged_) {
        return true;
    }

    auto *gt = gerenciador_tarefas();
    if (gt == nullptr) {
        return true;
    }

    const int timer_cycles = qMax(1, ciclos_permitidos);

    struct DiscrepanteInfo {
        Tarefa *task = nullptr;
        QString titulo;
        QString origem_id;
        int meta = 0;
    };

    QVector<DiscrepanteInfo> lista_discrepantes;
    for (Tarefa &t : gt->tarefas) {
        if (t.status == QStringLiteral("Doing")) {
            const int meta = t.ciclos_totais > 0 ? t.ciclos_totais : timer_cycles;
            if (meta != timer_cycles) {
                lista_discrepantes.append({&t, t.titulo, t.integracao_origem, meta});
            }
        }
    }

    if (lista_discrepantes.isEmpty()) {
        return true;
    }

    class CycleMismatchDialog : public QDialog {
    public:
        enum class UserChoice { Cancel, ResetAll, ProceedAnyway };

        CycleMismatchDialog(QWidget *parent, int timer_cycles, const QVector<DiscrepanteInfo> &items)
            : QDialog(parent), timer_cycles_(timer_cycles), items_(items)
        {
            setModal(true);
            setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint)
                           | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
            setMinimumSize(660, 360);

            auto *main_layout = new QVBoxLayout(this);
            main_layout->setContentsMargins(16, 16, 16, 16);
            main_layout->setSpacing(12);

            header_label_ = new QLabel(this);
            header_label_->setWordWrap(true);
            main_layout->addWidget(header_label_);

            table_ = new QTableWidget(this);
            table_->setColumnCount(5);
            table_->setRowCount(items_.size());
            table_->setSelectionBehavior(QAbstractItemView::SelectRows);
            table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
            table_->setAlternatingRowColors(true);
            table_->verticalHeader()->setVisible(false);
            table_->horizontalHeader()->setStretchLastSection(true);
            table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
            table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
            table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
            table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
            main_layout->addWidget(table_, 1);

            auto *btn_layout = new QHBoxLayout();
            btn_layout->setSpacing(8);

            btn_redefinir_ = new QPushButton(this);
            btn_iniciar_mesmo_assim_ = new QPushButton(this);
            btn_cancelar_ = new QPushButton(this);

            btn_layout->addWidget(btn_redefinir_);
            btn_layout->addWidget(btn_iniciar_mesmo_assim_);
            btn_layout->addStretch(1);
            btn_layout->addWidget(btn_cancelar_);
            main_layout->addLayout(btn_layout);

            connect(btn_redefinir_, &QPushButton::clicked, this, [this]() {
                choice_ = UserChoice::ResetAll;
                accept();
            });
            connect(btn_iniciar_mesmo_assim_, &QPushButton::clicked, this, [this]() {
                choice_ = UserChoice::ProceedAnyway;
                accept();
            });
            connect(btn_cancelar_, &QPushButton::clicked, this, [this]() {
                choice_ = UserChoice::Cancel;
                reject();
            });

            retranslate_ui();
        }

        UserChoice user_choice() const { return choice_; }

    protected:
        void changeEvent(QEvent *event) override
        {
            if (event != nullptr && event->type() == QEvent::LanguageChange) {
                retranslate_ui();
            }
            QDialog::changeEvent(event);
        }

    private:
        void retranslate_ui()
        {
            setWindowTitle(QCoreApplication::translate("PomodoroPlugin", "⚠️ Aviso de Metas de Ciclos"));
            header_label_->setText(
                QCoreApplication::translate(
                    "PomodoroPlugin",
                    "<h3>⚠️ Discrepância em Tarefas em \"Em Progresso\"</h3>"
                    "<p>O timer do Pomodoro está ajustado para <b>Ciclo 1/%1</b> (%1 ciclos).<br>"
                    "As tarefas abaixo possuem <b>Meta de Ciclos Pomodoro (🎯)</b> diferente de %1 ciclos:</p>")
                    .arg(timer_cycles_));

            table_->setHorizontalHeaderLabels({
                QCoreApplication::translate("PomodoroPlugin", "Tarefa"),
                QCoreApplication::translate("PomodoroPlugin", "Módulo / Origem"),
                QCoreApplication::translate("PomodoroPlugin", "Meta (🎯)"),
                QCoreApplication::translate("PomodoroPlugin", "Ciclo Timer (⚙️)"),
                QCoreApplication::translate("PomodoroPlugin", "Comportamento no Fluxo")
            });

            for (int r = 0; r < items_.size(); ++r) {
                const auto &info = items_[r];
                auto *item_titulo = new QTableWidgetItem(info.titulo);
                QString mod_text = QCoreApplication::translate("PomodoroPlugin", "🍅 Pomodoro");
                const QString origem = info.origem_id.trimmed().toCaseFolded();
                if (origem == QStringLiteral("eisenhower")) {
                    mod_text = QCoreApplication::translate("PomodoroPlugin", "🏛️ Eisenhower");
                } else if (origem == QStringLiteral("feynman")) {
                    mod_text = QCoreApplication::translate("PomodoroPlugin", "🧠 Feynman");
                }
                auto *item_origem = new QTableWidgetItem(mod_text);
                auto *item_meta = new QTableWidgetItem(QString::number(info.meta));
                item_meta->setTextAlignment(Qt::AlignCenter);
                auto *item_timer = new QTableWidgetItem(QString::number(timer_cycles_));
                item_timer->setTextAlignment(Qt::AlignCenter);

                QString comportamento;
                if (info.meta < timer_cycles_) {
                    comportamento = QCoreApplication::translate(
                        "PomodoroPlugin", "🎯 < ⚙️ Concluirá ao atingir %1 ciclos (antes do timer)").arg(info.meta);
                } else {
                    comportamento = QCoreApplication::translate(
                        "PomodoroPlugin", "🎯 > ⚙️ Não concluirá (permanece Em Progresso; timer para em %1)").arg(timer_cycles_);
                }
                auto *item_comp = new QTableWidgetItem(comportamento);

                table_->setItem(r, 0, item_titulo);
                table_->setItem(r, 1, item_origem);
                table_->setItem(r, 2, item_meta);
                table_->setItem(r, 3, item_timer);
                table_->setItem(r, 4, item_comp);
            }

            btn_redefinir_->setText(QCoreApplication::translate("PomodoroPlugin", "🔄 Redefinir todas para %1 ciclos").arg(timer_cycles_));
            btn_redefinir_->setToolTip(QCoreApplication::translate("PomodoroPlugin", "Altera a meta de todas as tarefas acima para o ciclo do timer e inicia."));

            btn_iniciar_mesmo_assim_->setText(QCoreApplication::translate("PomodoroPlugin", "▶️ Iniciar mesmo assim"));
            btn_iniciar_mesmo_assim_->setToolTip(QCoreApplication::translate("PomodoroPlugin", "Mantém as metas individuais de cada tarefa e inicia o timer."));

            btn_cancelar_->setText(QCoreApplication::translate("PomodoroPlugin", "❌ Cancelar"));
        }

        int timer_cycles_ = 4;
        QVector<DiscrepanteInfo> items_;
        QLabel *header_label_ = nullptr;
        QTableWidget *table_ = nullptr;
        QPushButton *btn_redefinir_ = nullptr;
        QPushButton *btn_iniciar_mesmo_assim_ = nullptr;
        QPushButton *btn_cancelar_ = nullptr;
        UserChoice choice_ = UserChoice::Cancel;
    };

    CycleMismatchDialog dialog(this, timer_cycles, lista_discrepantes);
    dialog.exec();

    if (dialog.user_choice() == CycleMismatchDialog::UserChoice::ResetAll) {
        QVariantList affected_tasks;
        for (auto &t : gt->tarefas) {
            if (t.status == QStringLiteral("Doing")) {
                QVariantMap task_info;
                task_info.insert(QStringLiteral("id"), t.id);
                task_info.insert(QStringLiteral("titulo"), t.titulo);
                task_info.insert(QStringLiteral("date"), t.date);
                task_info.insert(QStringLiteral("time"), t.time);
                affected_tasks.append(task_info);

                t.ciclos_totais = timer_cycles;
                t.ciclos_customizados = false;
            }
        }
        gt->salvar_tarefas();
        gt->atualizar_listas();
        if (compat::host_bridge() != nullptr) {
            compat::host_bridge()->reset_in_progress_task_cycles(timer_cycles, affected_tasks);
        }
        mismatch_acknowledged_ = true;
        return true;
    }

    if (dialog.user_choice() == CycleMismatchDialog::UserChoice::ProceedAnyway) {
        mismatch_acknowledged_ = true;
        return true;
    }

    return false;
}

void PomodoroTimer::toggle_timer()
{
    try {
        if (timer_ativo) {
            timer->stop();
            timer_ativo = false;
            gerenciador_botoes_.set_button_text(btn_iniciar, QCoreApplication::translate("PomodoroPlugin", "▶️ Continuar"));
            return;
        }

        const int ciclos_total = qMax(1, ciclos_permitidos);
        if (em_foco && ciclos_completados >= ciclos_total) {
            _executar_alerta(
                QCoreApplication::translate("PomodoroPlugin", "⛔ Limite de ciclos atingido"),
                QCoreApplication::translate(
                    "PomodoroPlugin",
                    "Você atingiu {total}/{total} ciclos de foco.\nAumente o valor em \"Ciclo 1/{total}\" para continuar.")
                    .replace(QStringLiteral("{total}"), QString::number(ciclos_total)));
            atualizar_traducoes();
            return;
        }

        if (!verificar_e_avisar_discrepancia_ciclos()) {
            return;
        }

        timer->start(1000);
        timer_ativo = true;
        gerenciador_botoes_.set_button_text(btn_iniciar, QCoreApplication::translate("PomodoroPlugin", "⏸️ Pausar"));
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao alternar estado do timer: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao alternar estado do timer"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void toggle_timer(PomodoroTimer &self) { self.toggle_timer(); } }
