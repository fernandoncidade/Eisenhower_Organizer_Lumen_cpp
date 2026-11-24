#pragma once

#include <QString>

namespace source::InterfaceCore {
class Arquivo;
}

namespace source::InterfaceCore::ArquivoModules {

struct ParsedTextDateTime {
    QString text;
    QString date_iso;
    QString time_str;
};

void clear_all_lists(Arquivo *self);
void add_placeholders_after_clear(Arquivo *self);
void novo(Arquivo *self);
void limpar_tudo(Arquivo *self);
ParsedTextDateTime parse_text_date_time(Arquivo *self, const QString &raw_text);
void abrir_arquivo(Arquivo *self);
void salvar(Arquivo *self);
void salvar_como(Arquivo *self);

}  // namespace source::InterfaceCore::ArquivoModules
