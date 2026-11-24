#pragma once

#include <QString>

#include "source/services/Arquivo/agregador_01_Arquivo.hpp"

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore {

class Arquivo {
public:
    explicit Arquivo(source::EisenhowerMatrixApp *app);

    static QString get_text(const QString &text);
    source::EisenhowerMatrixApp *app() const;

    void _clear_all_lists();
    void _add_placeholders_after_clear();
    void novo();
    void limpar_tudo();
    void sair();
    source::InterfaceCore::ArquivoModules::ParsedTextDateTime _parse_text_date_time(const QString &raw_text);
    void abrir_arquivo();
    void salvar();
    void salvar_como();

private:
    source::EisenhowerMatrixApp *app_;
};

}  // namespace source::InterfaceCore
