#include "source/modules/agenda/agenda_plugin.hpp"

#include "source/modules/agenda/AgendaAvaliacoes/BancoDeDados/Banco_Dados.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoInterfaceGrafica/GerenciadorInterfaceGrafica/ginter_13_populate_host_menus.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/CaminhoPersistenteUtils.hpp"

#include <QAction>
#include <QCoreApplication>
#include <QMenu>

namespace source::plugin {

QString AgendaPlugin::plugin_id() const
{
    return QStringLiteral("agenda");
}

QString AgendaPlugin::display_name() const
{
    return QCoreApplication::translate("AgendaPlugin", "Registro de Atividades");
}

QString AgendaPlugin::open_menu_text() const
{
    return QCoreApplication::translate("AgendaPlugin", "Registro de Atividades");
}

void AgendaPlugin::initialize(AgendaHostBridge *host_bridge)
{
    if (host_bridge == nullptr && window_ != nullptr) {
        QObject::disconnect(host_destroyed_connection_);
        host_destroyed_connection_ = {};
        delete window_;
        window_ = nullptr;
    }

    host_bridge_ = host_bridge;
    if (host_bridge_ != nullptr && !host_bridge_->persistent_dir().trimmed().isEmpty()) {
        definir_caminho_persistente_override(host_bridge_->persistent_dir());
    } else {
        limpar_caminho_persistente_override();
    }
    if (host_bridge_ != nullptr) {
        host_bridge_->set_color_resolver_handler([](const QString &title) {
            return obter_cor_por_nome(title);
        });
        GerenciamentoAtividades config_reader(false);
        host_bridge_->notify_color_mode_changed(config_reader.obter_modo_cores());
    }
    apply_host_language();
}

void AgendaPlugin::open_window(QWidget *parent)
{
    InterfaceGerenciadorAtividades *window = ensure_window(parent);
    if (window == nullptr) {
        return;
    }

    window->setAttribute(Qt::WA_DeleteOnClose, false);
    window->show();
    window->raise();
    window->activateWindow();
}

void AgendaPlugin::populate_delete_item_menu(QMenu *menu)
{
    if (menu == nullptr) {
        return;
    }

    popular_menu_excluir_item_agenda(window_, menu);
}

void AgendaPlugin::populate_color_menu(QMenu *menu)
{
    if (menu == nullptr) {
        return;
    }

    if (window_ != nullptr) {
        window_->populate_color_menu(menu);
        return;
    }

    auto tr = [](const char *context, const char *source) { return QCoreApplication::translate(context, source); };
    menu->clear();

    QMenu *sub_preto = menu->addMenu(tr("InterfaceGerenciadorAtividades", "Preto"));
    QMenu *sub_coloridas = menu->addMenu(tr("InterfaceGerenciadorAtividades", "Coloridas"));

    GerenciamentoAtividades config(false);
    const QString modo_eisenhower = config.obter_modo_cores(QStringLiteral("eisenhower"));
    const QString modo_pomodoro = config.obter_modo_cores(QStringLiteral("pomodoro"));
    const QString modo_feynman = config.obter_modo_cores(QStringLiteral("feynman"));

    const auto setup_action = [&](QMenu *parent_menu, const QString &code, const QString &label, bool checked) {
        QAction *action = parent_menu->addAction(tr("InterfaceGerenciadorAtividades", label.toUtf8().constData()));
        action->setCheckable(true);
        action->setData(code);
        action->setChecked(checked);
        connect(action, &QAction::triggered, this, [this, menu, code]() {
            GerenciamentoAtividades cfg(false);
            cfg.definir_modo_cores(code);
            if (host_bridge_ != nullptr) {
                host_bridge_->notify_color_mode_changed(code);
            }
            populate_color_menu(menu);
        });
    };

    setup_action(sub_preto, QStringLiteral("preto_eisenhower"), QStringLiteral("Eisenhower"), modo_eisenhower == QStringLiteral("preto"));
    setup_action(sub_preto, QStringLiteral("preto_pomodoro"), QStringLiteral("Pomodoro"), modo_pomodoro == QStringLiteral("preto"));
    setup_action(sub_preto, QStringLiteral("preto_feynman"), QStringLiteral("Feynman"), modo_feynman == QStringLiteral("preto"));
    setup_action(sub_preto, QStringLiteral("preto_geral"), QStringLiteral("Geral"), modo_eisenhower == QStringLiteral("preto") && modo_pomodoro == QStringLiteral("preto") && modo_feynman == QStringLiteral("preto"));

    setup_action(sub_coloridas, QStringLiteral("coloridas_eisenhower"), QStringLiteral("Eisenhower"), modo_eisenhower == QStringLiteral("coloridas"));
    setup_action(sub_coloridas, QStringLiteral("coloridas_pomodoro"), QStringLiteral("Pomodoro"), modo_pomodoro == QStringLiteral("coloridas"));
    setup_action(sub_coloridas, QStringLiteral("coloridas_feynman"), QStringLiteral("Feynman"), modo_feynman == QStringLiteral("coloridas"));
    setup_action(sub_coloridas, QStringLiteral("coloridas_geral"), QStringLiteral("Geral"), modo_eisenhower == QStringLiteral("coloridas") && modo_pomodoro == QStringLiteral("coloridas") && modo_feynman == QStringLiteral("coloridas"));
}

void AgendaPlugin::retranslate_ui()
{
    apply_host_language();
    if (window_ != nullptr) {
        window_->retranslate_ui();
    }
}

InterfaceGerenciadorAtividades *AgendaPlugin::ensure_window(QWidget *parent)
{
    if (window_ == nullptr) {
        window_ = new InterfaceGerenciadorAtividades(nullptr, nullptr, true, true, false);
        window_->setWindowFlag(Qt::Window, true);
        if (parent != nullptr) {
            QObject::disconnect(host_destroyed_connection_);
            host_destroyed_connection_ = QObject::connect(parent, &QObject::destroyed, this, [this]() {
                if (window_ != nullptr) {
                    delete window_;
                    window_ = nullptr;
                }
                host_destroyed_connection_ = {};
            });
        }
        QObject::connect(window_,
                         &InterfaceGerenciadorAtividades::atividade_registrada,
                         this,
                         [this](const QVariantMap &task) {
                             if (host_bridge_ != nullptr) {
                                 host_bridge_->publish_task_to_eisenhower(task);
                             }
                         });
        QObject::connect(window_,
                         &InterfaceGerenciadorAtividades::modo_cores_alterado,
                         this,
                         [this](const QString &modo) {
                             if (host_bridge_ != nullptr) {
                                 host_bridge_->notify_color_mode_changed(modo);
                             }
                         });
        apply_host_language();
    }

    return window_;
}

void AgendaPlugin::apply_host_language()
{
    if (window_ == nullptr || host_bridge_ == nullptr) {
        return;
    }

    const QString language_code = host_bridge_->language_code();
    if (!language_code.trimmed().isEmpty()) {
        window_->change_language(language_code);
    }
}

}  // namespace source::plugin
