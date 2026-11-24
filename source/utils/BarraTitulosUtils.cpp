#include "source/utils/BarraTitulosUtils.hpp"

#include <QCoreApplication>

#include "source/utils/AppConfig.hpp"

namespace source::utils {
namespace {

QString get_text(const QString &text)
{
    const QString translated = QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
    return translated.isEmpty() ? text : translated;
}

QString title_with_product(const QString &title_key)
{
    return QStringLiteral("%1 — %2").arg(get_text(title_key), AppConfig::app_manual_name());
}

}  // namespace

QString nome_aplicacao_marca()
{
    return AppConfig::app_brand_name();
}

QString titulo_janela_principal()
{
    return get_text(AppConfig::main_window_title());
}

QString titulo_janela_sobre()
{
    return title_with_product(QStringLiteral("Sobre"));
}

QString titulo_janela_manual()
{
    return title_with_product(QStringLiteral("Manual de Utilização"));
}

QString titulo_janela_compilador_traducoes()
{
    return title_with_product(QStringLiteral("Compilador de Traduções"));
}

}  // namespace source::utils
