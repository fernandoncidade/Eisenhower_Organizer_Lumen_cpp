#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_16_mudar_cor.hpp"

#include <QAbstractButton>
#include <QColorDialog>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QMap>
#include <QRegularExpression>
#include <QTranslator>
#include <QDialog>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"

namespace source::plugin::MapaMental {
namespace {

const char *tr_source_basic_colors = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Cores basicas");
const char *tr_source_custom_colors = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Cores personalizadas");
const char *tr_source_add_custom_colors =
    QT_TRANSLATE_NOOP("MapaMentalPlugin", "Adicionar as cores personalizadas");
const char *tr_source_pick_screen_color = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Selecionar cor na tela");
const char *tr_source_hue = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Tom:");
const char *tr_source_sat = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Sat:");
const char *tr_source_val = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Val:");
const char *tr_source_red = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Vermelho:");
const char *tr_source_green = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Verde:");
const char *tr_source_blue = QT_TRANSLATE_NOOP("MapaMentalPlugin", "Azul:");
const char *tr_source_html = QT_TRANSLATE_NOOP("MapaMentalPlugin", "HTML:");
const char *tr_source_cursor_cancel =
    QT_TRANSLATE_NOOP("MapaMentalPlugin", "Cursor at %1, %2\nPress ESC to cancel");
const char *tr_source_cursor_color_cancel =
    QT_TRANSLATE_NOOP("MapaMentalPlugin", "Cursor at %1, %2, color: %3\nPress ESC to cancel");
const char *tr_source_cursor_only =
    QT_TRANSLATE_NOOP("MapaMentalPlugin", "Cursor at %1, %2");
const char *tr_source_cancel_only =
    QT_TRANSLATE_NOOP("MapaMentalPlugin", "Press ESC to cancel");

QString normalized_dialog_text(QString text)
{
    return text.remove(QLatin1Char('&')).replace(QStringLiteral("..."), QString()).trimmed();
}

const char *color_dialog_translation_source(const QString &text)
{
    static const QMap<QString, const char *> sources = {
        {QStringLiteral("Basic colors"), tr_source_basic_colors},
        {QStringLiteral("Cores básicas"), tr_source_basic_colors},
        {QStringLiteral("Cores basicas"), tr_source_basic_colors},
        {QStringLiteral("Custom colors"), tr_source_custom_colors},
        {QStringLiteral("Cores personalizadas"), tr_source_custom_colors},
        {QStringLiteral("Add to Custom Colors"), tr_source_add_custom_colors},
        {QStringLiteral("Adicionar às cores personalizadas"), tr_source_add_custom_colors},
        {QStringLiteral("Adicionar as cores personalizadas"), tr_source_add_custom_colors},
        {QStringLiteral("Pick Screen Color"), tr_source_pick_screen_color},
        {QStringLiteral("Escolher cor da tela"), tr_source_pick_screen_color},
        {QStringLiteral("Selecionar cor na tela"), tr_source_pick_screen_color},
        {QStringLiteral("Hue:"), tr_source_hue},
        {QStringLiteral("Matiz:"), tr_source_hue},
        {QStringLiteral("Tom:"), tr_source_hue},
        {QStringLiteral("Sat:"), tr_source_sat},
        {QStringLiteral("Saturação:"), tr_source_sat},
        {QStringLiteral("Val:"), tr_source_val},
        {QStringLiteral("Valor:"), tr_source_val},
        {QStringLiteral("Red:"), tr_source_red},
        {QStringLiteral("Vermelho:"), tr_source_red},
        {QStringLiteral("Green:"), tr_source_green},
        {QStringLiteral("Verde:"), tr_source_green},
        {QStringLiteral("Blue:"), tr_source_blue},
        {QStringLiteral("Azul:"), tr_source_blue},
        {QStringLiteral("HTML:"), tr_source_html},
        {QStringLiteral("Cursor at %1, %2\nPress ESC to cancel"), tr_source_cursor_cancel},
        {QStringLiteral("Cursor em %1, %2\nPressione ESC para cancelar"), tr_source_cursor_cancel},
        {QStringLiteral("Cursor at %1, %2, color: %3\nPress ESC to cancel"), tr_source_cursor_color_cancel},
        {QStringLiteral("Cursor em %1, %2, cor: %3\nPressione ESC para cancelar"), tr_source_cursor_color_cancel},
        {QStringLiteral("Cursor at %1, %2"), tr_source_cursor_only},
        {QStringLiteral("Cursor em %1, %2"), tr_source_cursor_only},
        {QStringLiteral("Press ESC to cancel"), tr_source_cancel_only},
        {QStringLiteral("Pressione ESC para cancelar"), tr_source_cancel_only},
    };
    return sources.value(normalized_dialog_text(text), nullptr);
}

QString color_dialog_text(const QString &text)
{
    const char *source = color_dialog_translation_source(text);
    if (source != nullptr) {
        return QCoreApplication::translate("MapaMentalPlugin", source);
    }

    static const QRegularExpression rx_cursor_color(
        QStringLiteral(R"(^Cursor (?:at|em) (\d+),\s*(\d+)(?:,\s*(?:color|cor):\s*(\S+))?\s*(?:\n?\s*(Press ESC to cancel|Pressione ESC para cancelar))?$)"),
        QRegularExpression::CaseInsensitiveOption);

    const auto match = rx_cursor_color.match(text.trimmed());
    if (match.hasMatch()) {
        const QString x = match.captured(1);
        const QString y = match.captured(2);
        const QString color = match.captured(3);
        const QString cancel = match.captured(4);

        if (!color.isEmpty()) {
            return QCoreApplication::translate("MapaMentalPlugin", "Cursor at %1, %2, color: %3\nPress ESC to cancel")
                .arg(x, y, color);
        }
        if (!cancel.isEmpty()) {
            return QCoreApplication::translate("MapaMentalPlugin", "Cursor at %1, %2\nPress ESC to cancel")
                .arg(x, y);
        }
        return QCoreApplication::translate("MapaMentalPlugin", "Cursor at %1, %2")
            .arg(x, y);
    }

    if (text.trimmed().compare(QStringLiteral("Press ESC to cancel"), Qt::CaseInsensitive) == 0 ||
        text.trimmed().compare(QStringLiteral("Pressione ESC para cancelar"), Qt::CaseInsensitive) == 0) {
        return QCoreApplication::translate("MapaMentalPlugin", "Press ESC to cancel");
    }

    return QString();
}

class ColorDialogScopedTranslator final : public QTranslator {
public:
    explicit ColorDialogScopedTranslator(QObject *parent = nullptr)
        : QTranslator(parent)
    {
    }

    QString translate(const char *context,
                      const char *source_text,
                      const char *disambiguation = nullptr,
                      int n = -1) const override
    {
        Q_UNUSED(disambiguation)
        Q_UNUSED(n)

        if (context == nullptr || source_text == nullptr
            || QString::fromLatin1(context) != QStringLiteral("QColorDialog")) {
            return {};
        }

        const char *source = color_dialog_translation_source(QString::fromUtf8(source_text));
        return source == nullptr ? QString() : QCoreApplication::translate("MapaMentalPlugin", source);
    }
};

void translate_color_dialog(QColorDialog *dialog, MapaMental *owner)
{
    if (dialog == nullptr) {
        return;
    }

    dialog->setWindowTitle(QCoreApplication::translate("MapaMentalPlugin", "Escolher cor"));
    if (owner != nullptr) {
        const auto button_boxes = dialog->findChildren<QDialogButtonBox *>();
        for (QDialogButtonBox *button_box : button_boxes) {
            owner->traduzir_botoes_padrao(button_box);
        }
    }

    const auto labels = dialog->findChildren<QLabel *>();
    for (QLabel *label : labels) {
        const QString translated = color_dialog_text(label->text());
        if (!translated.isEmpty()) {
            label->setText(translated);
        }
    }

    const auto buttons = dialog->findChildren<QAbstractButton *>();
    for (QAbstractButton *button : buttons) {
        const QString translated = color_dialog_text(button->text());
        if (!translated.isEmpty()) {
            button->setText(translated);
        }
    }

    const auto groups = dialog->findChildren<QGroupBox *>();
    for (QGroupBox *group : groups) {
        const QString translated = color_dialog_text(group->title());
        if (!translated.isEmpty()) {
            group->setTitle(translated);
        }
    }
}

}  // namespace

void MapaMental::mudar_cor()
{
    ColorDialogScopedTranslator color_dialog_translator;
    QCoreApplication::installTranslator(&color_dialog_translator);

    QColorDialog dialog(cor_atual_, this);
    dialog.setOption(QColorDialog::DontUseNativeDialog, true);
    dialog.setOption(QColorDialog::ShowAlphaChannel, false);
    dialog.setWindowTitle(QCoreApplication::translate("MapaMentalPlugin", "Escolher cor"));
    traduzir_botoes_padrao(&dialog);
    translate_color_dialog(&dialog, this);

    const int result = dialog.exec();
    QCoreApplication::removeTranslator(&color_dialog_translator);
    if (result != QDialog::Accepted) {
        return;
    }

    const QColor cor = dialog.selectedColor();
    if (!cor.isValid()) {
        return;
    }

    cor_atual_ = cor;
    for (NoConceito *no : selected_nodes()) {
        no->set_cor(cor);
    }
    salvar_backup_automatico();
}

}  // namespace source::plugin::MapaMental
