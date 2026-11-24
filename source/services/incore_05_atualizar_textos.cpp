#include "source/services/incore_05_atualizar_textos.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>

#include "source/services/incore_03_criar_menu_configuracoes.hpp"
#include "source/services/incore_06_atualizar_placeholders.hpp"
#include "source/services/incore_11_atualizar_itens_tarefas.hpp"
#include "source/services/incore_13_prioridade_display.hpp"
#include "source/eisenhower_app.hpp"
#include "source/utils/BarraTitulosUtils.hpp"

namespace source::InterfaceCore::incore_05_atualizar_textos {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

void ensure_placeholder(source::EisenhowerMatrixApp *app, QListWidget *list, const QString &text)
{
    if (app == nullptr || list == nullptr || list->count() != 0) {
        return;
    }
    app->add_placeholder(list, text);
}

}  // namespace

void atualizar_textos(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return;
    }

    app->setWindowTitle(source::utils::titulo_janela_principal());
    if (app->new_task_group != nullptr) {
        app->new_task_group->setTitle(get_text(QStringLiteral("Nova Tarefa")));
    }
    if (app->task_input != nullptr) {
        app->task_input->setPlaceholderText(get_text(QStringLiteral("Adicione uma tarefa ou arraste um arquivo...")));
    }
    if (app->add_button != nullptr) {
        app->add_button->setText(get_text(QStringLiteral("Adicionar Tarefa")));
    }
    if (app->calendar_button != nullptr) {
        app->calendar_button->setText(get_text(QStringLiteral("Calendário")));
    }
    if (app->date_checkbox != nullptr) {
        app->date_checkbox->setText(get_text(QStringLiteral("Vincular data")));
    }
    if (app->time_checkbox != nullptr) {
        app->time_checkbox->setText(get_text(QStringLiteral("Vincular horário")));
    }
    if (app->duration_label != nullptr) {
        app->duration_label->setText(get_text(QStringLiteral("Duração:")));
    }
    if (app->duration_spin != nullptr) {
        app->duration_spin->setSuffix(get_text(QStringLiteral(" dias")));
    }
    if (app->priority_label != nullptr) {
        app->priority_label->setText(get_text(QStringLiteral("Prioridade:")));
    }
    if (app->domain_label != nullptr) {
        app->domain_label->setText(get_text(QStringLiteral("Nível de Domínio:")));
    }

    if (app->quadrant_selector != nullptr) {
        app->quadrant_selector->clear();
        app->quadrant_selector->addItems({
            get_text(QStringLiteral("🔴 Importante e Urgente")),
            get_text(QStringLiteral("🟠 Importante, mas Não Urgente")),
            get_text(QStringLiteral("🟡 Não Importante, mas Urgente")),
            get_text(QStringLiteral("🟢 Não Importante e Não Urgente")),
        });
    }

    if (app->domain_selector != nullptr) {
        const int current_index = qBound(0, app->domain_selector->currentIndex(), 2);
        app->domain_selector->clear();
        app->domain_selector->addItems({
            source::InterfaceCore::dominio_para_texto(0, false),
            source::InterfaceCore::dominio_para_texto(1, false),
            source::InterfaceCore::dominio_para_texto(2, false),
        });
        app->domain_selector->setCurrentIndex(current_index);
    }

    if (app->quadrant1_label != nullptr) {
        app->quadrant1_label->setText(get_text(QStringLiteral("🔴 Importante e Urgente")));
    }
    if (app->quadrant2_label != nullptr) {
        app->quadrant2_label->setText(get_text(QStringLiteral("🟠 Importante, mas Não Urgente")));
    }
    if (app->quadrant3_label != nullptr) {
        app->quadrant3_label->setText(get_text(QStringLiteral("🟡 Não Importante, mas Urgente")));
    }
    if (app->quadrant4_label != nullptr) {
        app->quadrant4_label->setText(get_text(QStringLiteral("🟢 Não Importante e Não Urgente")));
    }

    if (app->quadrant1_in_progress_label != nullptr) {
        app->quadrant1_in_progress_label->setText(get_text(QStringLiteral("🔴 Em Progresso")));
    }
    if (app->quadrant2_in_progress_label != nullptr) {
        app->quadrant2_in_progress_label->setText(get_text(QStringLiteral("🟠 Em Progresso")));
    }
    if (app->quadrant3_in_progress_label != nullptr) {
        app->quadrant3_in_progress_label->setText(get_text(QStringLiteral("🟡 Em Progresso")));
    }
    if (app->quadrant4_in_progress_label != nullptr) {
        app->quadrant4_in_progress_label->setText(get_text(QStringLiteral("🟢 Em Progresso")));
    }

    if (app->quadrant1_completed_label != nullptr) {
        app->quadrant1_completed_label->setText(get_text(QStringLiteral("🔴 Concluídas")));
    }
    if (app->quadrant2_completed_label != nullptr) {
        app->quadrant2_completed_label->setText(get_text(QStringLiteral("🟠 Concluídas")));
    }
    if (app->quadrant3_completed_label != nullptr) {
        app->quadrant3_completed_label->setText(get_text(QStringLiteral("🟡 Concluídas")));
    }
    if (app->quadrant4_completed_label != nullptr) {
        app->quadrant4_completed_label->setText(get_text(QStringLiteral("🟢 Concluídas")));
    }

    if (app->current_task_layout() == source::EisenhowerMatrixApp::TaskLayoutMode::Tabs
        || app->current_task_layout() == source::EisenhowerMatrixApp::TaskLayoutMode::TabsHorizontal) {
        if (app->quadrant1_group != nullptr) {
            app->quadrant1_group->setTitle(get_text(QStringLiteral("🔴 Pendentes")));
        }
        if (app->quadrant2_group != nullptr) {
            app->quadrant2_group->setTitle(get_text(QStringLiteral("🟠 Pendentes")));
        }
        if (app->quadrant3_group != nullptr) {
            app->quadrant3_group->setTitle(get_text(QStringLiteral("🟡 Pendentes")));
        }
        if (app->quadrant4_group != nullptr) {
            app->quadrant4_group->setTitle(get_text(QStringLiteral("🟢 Pendentes")));
        }
    } else {
        if (app->quadrant1_group != nullptr && app->quadrant1_label != nullptr) {
            app->quadrant1_group->setTitle(app->quadrant1_label->text());
        }
        if (app->quadrant2_group != nullptr && app->quadrant2_label != nullptr) {
            app->quadrant2_group->setTitle(app->quadrant2_label->text());
        }
        if (app->quadrant3_group != nullptr && app->quadrant3_label != nullptr) {
            app->quadrant3_group->setTitle(app->quadrant3_label->text());
        }
        if (app->quadrant4_group != nullptr && app->quadrant4_label != nullptr) {
            app->quadrant4_group->setTitle(app->quadrant4_label->text());
        }
    }
    if (app->quadrant1_in_progress_group != nullptr && app->quadrant1_in_progress_label != nullptr) {
        app->quadrant1_in_progress_group->setTitle(app->quadrant1_in_progress_label->text());
    }
    if (app->quadrant2_in_progress_group != nullptr && app->quadrant2_in_progress_label != nullptr) {
        app->quadrant2_in_progress_group->setTitle(app->quadrant2_in_progress_label->text());
    }
    if (app->quadrant3_in_progress_group != nullptr && app->quadrant3_in_progress_label != nullptr) {
        app->quadrant3_in_progress_group->setTitle(app->quadrant3_in_progress_label->text());
    }
    if (app->quadrant4_in_progress_group != nullptr && app->quadrant4_in_progress_label != nullptr) {
        app->quadrant4_in_progress_group->setTitle(app->quadrant4_in_progress_label->text());
    }
    if (app->quadrant1_completed_group != nullptr && app->quadrant1_completed_label != nullptr) {
        app->quadrant1_completed_group->setTitle(app->quadrant1_completed_label->text());
    }
    if (app->quadrant2_completed_group != nullptr && app->quadrant2_completed_label != nullptr) {
        app->quadrant2_completed_group->setTitle(app->quadrant2_completed_label->text());
    }
    if (app->quadrant3_completed_group != nullptr && app->quadrant3_completed_label != nullptr) {
        app->quadrant3_completed_group->setTitle(app->quadrant3_completed_label->text());
    }
    if (app->quadrant4_completed_group != nullptr && app->quadrant4_completed_label != nullptr) {
        app->quadrant4_completed_group->setTitle(app->quadrant4_completed_label->text());
    }

    if (app->quadrant_tabs != nullptr && app->quadrant_tabs->count() >= 4) {
        app->quadrant_tabs->setTabText(0, get_text(QStringLiteral("🔴 Importante e Urgente")));
        app->quadrant_tabs->setTabText(1, get_text(QStringLiteral("🟠 Importante, mas Não Urgente")));
        app->quadrant_tabs->setTabText(2, get_text(QStringLiteral("🟡 Não Importante, mas Urgente")));
        app->quadrant_tabs->setTabText(3, get_text(QStringLiteral("🟢 Não Importante e Não Urgente")));
    }

    for (QTabWidget *tabs : {app->quadrant1_status_tabs,
                             app->quadrant2_status_tabs,
                             app->quadrant3_status_tabs,
                             app->quadrant4_status_tabs}) {
        if (tabs != nullptr && tabs->count() >= 2) {
            tabs->setTabText(0, get_text(QStringLiteral("Pendente")));
            if (tabs->count() >= 3) {
                tabs->setTabText(1, get_text(QStringLiteral("Em Progresso")));
                tabs->setTabText(2, get_text(QStringLiteral("Concluídas")));
            } else {
                tabs->setTabText(1, get_text(QStringLiteral("Concluídas")));
            }
        }
    }

    if (app->current_task_layout() == source::EisenhowerMatrixApp::TaskLayoutMode::Matrix2x2) {
        if (app->quadrant1_in_progress_group != nullptr) {
            app->quadrant1_in_progress_group->setTitle(get_text(QStringLiteral("🔴 Importante e Urgente")));
        }
        if (app->quadrant2_in_progress_group != nullptr) {
            app->quadrant2_in_progress_group->setTitle(get_text(QStringLiteral("🟠 Importante, mas Não Urgente")));
        }
        if (app->quadrant3_in_progress_group != nullptr) {
            app->quadrant3_in_progress_group->setTitle(get_text(QStringLiteral("🟡 Não Importante, mas Urgente")));
        }
        if (app->quadrant4_in_progress_group != nullptr) {
            app->quadrant4_in_progress_group->setTitle(get_text(QStringLiteral("🟢 Não Importante e Não Urgente")));
        }
        if (app->quadrant1_completed_group != nullptr && app->quadrant1_label != nullptr) {
            app->quadrant1_completed_group->setTitle(app->quadrant1_label->text());
        }
        if (app->quadrant2_completed_group != nullptr && app->quadrant2_label != nullptr) {
            app->quadrant2_completed_group->setTitle(app->quadrant2_label->text());
        }
        if (app->quadrant3_completed_group != nullptr && app->quadrant3_label != nullptr) {
            app->quadrant3_completed_group->setTitle(app->quadrant3_label->text());
        }
        if (app->quadrant4_completed_group != nullptr && app->quadrant4_label != nullptr) {
            app->quadrant4_completed_group->setTitle(app->quadrant4_label->text());
        }
    }

    ensure_placeholder(app, app->quadrant1_list, get_text(QStringLiteral("1º Quadrante")));
    ensure_placeholder(app, app->quadrant2_list, get_text(QStringLiteral("2º Quadrante")));
    ensure_placeholder(app, app->quadrant3_list, get_text(QStringLiteral("3º Quadrante")));
    ensure_placeholder(app, app->quadrant4_list, get_text(QStringLiteral("4º Quadrante")));
    ensure_placeholder(app, app->quadrant1_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso")));
    ensure_placeholder(app, app->quadrant2_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso")));
    ensure_placeholder(app, app->quadrant3_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso")));
    ensure_placeholder(app, app->quadrant4_in_progress_list, get_text(QStringLiteral("Nenhuma Tarefa em Progresso")));
    ensure_placeholder(app, app->quadrant1_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
    ensure_placeholder(app, app->quadrant2_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
    ensure_placeholder(app, app->quadrant3_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));
    ensure_placeholder(app, app->quadrant4_completed_list, get_text(QStringLiteral("Nenhuma Tarefa Concluída")));

    source::InterfaceCore::incore_11_atualizar_itens_tarefas::atualizar_itens_tarefas(app);
    source::InterfaceCore::incore_06_atualizar_placeholders::atualizar_placeholders(app);
    source::InterfaceCore::incore_03_criar_menu_configuracoes::criar_menu_configuracoes(app);
}

}  // namespace source::InterfaceCore::incore_05_atualizar_textos
