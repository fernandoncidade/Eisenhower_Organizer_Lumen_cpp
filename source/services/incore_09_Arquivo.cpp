#include "source/services/incore_09_Arquivo.hpp"

#include <QApplication>
#include <QCoreApplication>

#include "source/services/Arquivo/agregador_01_Arquivo.hpp"
#include "source/eisenhower_app.hpp"

namespace source::InterfaceCore {

Arquivo::Arquivo(source::EisenhowerMatrixApp *app) : app_(app) {}

QString Arquivo::get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

source::EisenhowerMatrixApp *Arquivo::app() const
{
    return app_;
}

void Arquivo::_clear_all_lists()
{
    ArquivoModules::clear_all_lists(this);
}

void Arquivo::_add_placeholders_after_clear()
{
    ArquivoModules::add_placeholders_after_clear(this);
}

void Arquivo::novo()
{
    ArquivoModules::novo(this);
}

void Arquivo::limpar_tudo()
{
    ArquivoModules::limpar_tudo(this);
}

void Arquivo::sair()
{
    QApplication::quit();
}

source::InterfaceCore::ArquivoModules::ParsedTextDateTime Arquivo::_parse_text_date_time(const QString &raw_text)
{
    return ArquivoModules::parse_text_date_time(this, raw_text);
}

void Arquivo::abrir_arquivo()
{
    ArquivoModules::abrir_arquivo(this);
}

void Arquivo::salvar()
{
    ArquivoModules::salvar(this);
}

void Arquivo::salvar_como()
{
    ArquivoModules::salvar_como(this);
}

}  // namespace source::InterfaceCore
