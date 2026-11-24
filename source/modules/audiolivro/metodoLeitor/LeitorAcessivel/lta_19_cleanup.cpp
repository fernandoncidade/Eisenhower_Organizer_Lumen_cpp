#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_19_cleanup.hpp"

namespace source::plugin::Audiolivro {

void LeitorAcessivel::cleanup()
{
    parar_leitura();
    desativar_regua_foco();
    cleanup_paths_with_retry(generated_files_);
    generated_files_.clear();
}

}  // namespace source::plugin::Audiolivro
