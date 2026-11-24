#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoAtividades/Gerenciador_Atividades.hpp"

#include <QApplication>
#include <QColor>
#include <QPalette>

QString GerenciamentoAtividades::cor_texto_do_sistema() const {
    QColor window_color;
    if (textbox) {
        window_color = textbox->palette().color(QPalette::Base);
    }
    if (!window_color.isValid()) {
        window_color = QApplication::palette().color(QPalette::Window);
    }
    const double luminance =
        (0.2126 * window_color.red() + 0.7152 * window_color.green() + 0.0722 * window_color.blue()) / 255.0;
    return luminance >= 0.5 ? QStringLiteral("#000000") : QStringLiteral("#ffffff");
}
