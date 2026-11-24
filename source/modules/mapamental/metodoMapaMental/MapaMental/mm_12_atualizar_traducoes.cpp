#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_12_atualizar_traducoes.hpp"

#include <QCoreApplication>
#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

namespace source::plugin::MapaMental {
namespace {

// ============================================================================
// CONFIGURAÇÃO MANUAL DE DIMENSÕES DOS BOTÕES E ESTAMPAS (ÍCONES/EMOJIS)
// - kControlButtonWidth / kControlButtonHeight: Largura e altura do botão (px)
// - kControlEmojiFontSize: Tamanho da estampa/ícone (em pontos pt)
// ============================================================================
constexpr int kControlButtonWidth = 28;
constexpr int kControlButtonHeight = 28;
constexpr int kControlEmojiFontSize = 10;

QString tr_text(const char *text)
{
    return QCoreApplication::translate("MapaMentalPlugin", text);
}

const char *tr_source_controles = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Controles");
const char *tr_source_adicionar = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Adicionar");
const char *tr_source_conectando = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Conectando");
const char *tr_source_conectar = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Conectar");
const char *tr_source_cor = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Cor");
const char *tr_source_salvar = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Salvar");
const char *tr_source_carregar = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Carregar");
const char *tr_source_exportar = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Exportar");
const char *tr_source_importar = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Importar");
const char *tr_source_reorganizar = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Reorganizar");
const char *tr_source_visao = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Visao");
const char *tr_source_limpar = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Limpar");

}  // namespace

void MapaMental::atualizar_traducoes()
{
    if (controles_group_ != nullptr) {
        controles_group_->setTitle(tr_text(tr_source_controles));
    }

    const auto set_action = [this](const QString &label_name,
                                   const QString &button_name,
                                   const QString &label_text,
                                   const char *emoji) {
        const QString label_with_colon = QStringLiteral("%1:").arg(label_text);
        if (auto *label = findChild<QLabel *>(label_name)) {
            label->setText(label_with_colon);
        }
        if (auto *button = findChild<QPushButton *>(button_name)) {
            button->setText(QString::fromUtf8(emoji));
            button->setToolTip(label_text);
            button->setAccessibleName(label_text);
            button->setFixedSize(kControlButtonWidth, kControlButtonHeight);

            QFont font = button->font();
            font.setPointSize(kControlEmojiFontSize);
            button->setFont(font);
        }
    };

    set_action(QStringLiteral("lbl_adicionar"), QStringLiteral("btn_adicionar"), tr_text(tr_source_adicionar), u8"➕");
    set_action(QStringLiteral("lbl_conectar"),
               QStringLiteral("btn_conectar"),
               modo_conexao_ ? tr_text(tr_source_conectando) : tr_text(tr_source_conectar),
               u8"🔗");
    set_action(QStringLiteral("lbl_cor"), QStringLiteral("btn_cor"), tr_text(tr_source_cor), u8"🎨");
    set_action(QStringLiteral("lbl_salvar"), QStringLiteral("btn_salvar"), tr_text(tr_source_salvar), u8"💾");
    set_action(QStringLiteral("lbl_carregar"), QStringLiteral("btn_carregar"), tr_text(tr_source_carregar), u8"📂");
    set_action(QStringLiteral("lbl_exportar"), QStringLiteral("btn_exportar"), tr_text(tr_source_exportar), u8"📸");
    set_action(QStringLiteral("lbl_importar"), QStringLiteral("btn_importar"), tr_text(tr_source_importar), u8"📖");
    set_action(QStringLiteral("lbl_reorganizar"), QStringLiteral("btn_reorganizar"), tr_text(tr_source_reorganizar), u8"🔄");
    set_action(QStringLiteral("lbl_cinematica"), QStringLiteral("btn_cinematica"), tr_text(tr_source_visao), u8"🔎");
    set_action(QStringLiteral("lbl_limpar"), QStringLiteral("btn_limpar"), tr_text(tr_source_limpar), u8"🧽");
    refresh_counter();
}

}  // namespace source::plugin::MapaMental
