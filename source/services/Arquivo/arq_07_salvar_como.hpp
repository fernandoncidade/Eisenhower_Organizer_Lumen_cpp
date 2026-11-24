#pragma once

namespace source::InterfaceCore {
class Arquivo;
}

namespace source::InterfaceCore::ArquivoModules {

void salvar(Arquivo *self);
void salvar_como(Arquivo *self);

}  // namespace source::InterfaceCore::ArquivoModules
