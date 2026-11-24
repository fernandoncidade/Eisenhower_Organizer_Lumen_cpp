#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_11_setup_ui.hpp"

#include <QCoreApplication>
#include <QGraphicsScene>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_07_SmoothGraphicsView.hpp"

namespace source::plugin::MapaMental {

void MapaMental::setup_ui()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    scene_ = new MapaScene(this);
    view_ = new SmoothGraphicsView(scene_, this);
    view_->setFocusPolicy(Qt::StrongFocus);
    view_->setAcceptDrops(true);
    view_->installEventFilter(this);
    if (view_->viewport() != nullptr) {
        view_->viewport()->setAcceptDrops(true);
        view_->viewport()->installEventFilter(this);
    }

    auto *map_area = new QHBoxLayout;
    map_area->setContentsMargins(0, 0, 0, 0);
    map_area->setSpacing(8);

    controles_group_ = new QGroupBox(QCoreApplication::translate("MapaMentalPlugin", "Controles"), this);
    controles_group_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto *side_actions = new QVBoxLayout(controles_group_);
    side_actions->setContentsMargins(8, 14, 8, 8);
    side_actions->setSpacing(6);

    const auto add_action_row = [this, side_actions](const QString &label_name,
                                                     QPushButton *button,
                                                     const QString &button_name) {
        auto *row = new QHBoxLayout;
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(6);

        auto *label = new QLabel(controles_group_);
        label->setObjectName(label_name);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->setMinimumWidth(92);

        button->setObjectName(button_name);

        row->addWidget(label, 0, Qt::AlignLeft);
        row->addWidget(button, 0, Qt::AlignLeft);
        row->addStretch(1);
        side_actions->addLayout(row);
    };

    auto *btn_adicionar = new QPushButton(controles_group_);
    connect(btn_adicionar, &QPushButton::clicked, this, [this]() { adicionar_no(); });
    add_action_row(QStringLiteral("lbl_adicionar"), btn_adicionar, QStringLiteral("btn_adicionar"));

    btn_conectar_ = new QPushButton(controles_group_);
    btn_conectar_->setCheckable(true);
    connect(btn_conectar_, &QPushButton::clicked, this, [this]() { toggle_modo_conexao(); });
    add_action_row(QStringLiteral("lbl_conectar"), btn_conectar_, QStringLiteral("btn_conectar"));

    auto *btn_cor = new QPushButton(controles_group_);
    connect(btn_cor, &QPushButton::clicked, this, [this]() { mudar_cor(); });
    add_action_row(QStringLiteral("lbl_cor"), btn_cor, QStringLiteral("btn_cor"));

    auto *btn_salvar = new QPushButton(controles_group_);
    connect(btn_salvar, &QPushButton::clicked, this, [this]() { salvar_mapa(); });
    add_action_row(QStringLiteral("lbl_salvar"), btn_salvar, QStringLiteral("btn_salvar"));

    auto *btn_carregar = new QPushButton(controles_group_);
    connect(btn_carregar, &QPushButton::clicked, this, [this]() { carregar_mapa(); });
    add_action_row(QStringLiteral("lbl_carregar"), btn_carregar, QStringLiteral("btn_carregar"));

    auto *btn_exportar = new QPushButton(controles_group_);
    connect(btn_exportar, &QPushButton::clicked, this, [this]() { exportar_imagem(); });
    add_action_row(QStringLiteral("lbl_exportar"), btn_exportar, QStringLiteral("btn_exportar"));

    auto *btn_importar = new QPushButton(controles_group_);
    connect(btn_importar, &QPushButton::clicked, this, [this]() { importar_documento_ia(); });
    add_action_row(QStringLiteral("lbl_importar"), btn_importar, QStringLiteral("btn_importar"));

    auto *btn_reorganizar = new QPushButton(controles_group_);
    connect(btn_reorganizar, &QPushButton::clicked, this, [this]() { reorganizar_com_ia(); });
    add_action_row(QStringLiteral("lbl_reorganizar"), btn_reorganizar, QStringLiteral("btn_reorganizar"));

    auto *btn_cinematica = new QPushButton(controles_group_);
    connect(btn_cinematica, &QPushButton::clicked, this, [this]() { iniciar_visao_cinematica(); });
    add_action_row(QStringLiteral("lbl_cinematica"), btn_cinematica, QStringLiteral("btn_cinematica"));

    auto *btn_limpar = new QPushButton(controles_group_);
    connect(btn_limpar, &QPushButton::clicked, this, [this]() { limpar_mapa(); });
    add_action_row(QStringLiteral("lbl_limpar"), btn_limpar, QStringLiteral("btn_limpar"));

    map_area->addWidget(controles_group_, 0, Qt::AlignLeft | Qt::AlignBottom);
    map_area->addWidget(view_, 1);
    root->addLayout(map_area, 1);

    status_label_ = new QLabel(this);
    status_label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    root->addWidget(status_label_);

    connect(scene_, &QGraphicsScene::selectionChanged, this, &MapaMental::refresh_counter);
    atualizar_traducoes();
}

}  // namespace source::plugin::MapaMental
