#include "source/modules/pomodoro/Pomodoro/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"

namespace source::plugin::Pomodoro::utils {

QString obter_caminho_persistente()
{
    return source::utils::obter_caminho_persistente();
}

}  // namespace source::plugin::Pomodoro::utils
