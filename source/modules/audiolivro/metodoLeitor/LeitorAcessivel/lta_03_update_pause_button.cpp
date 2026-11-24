#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_03_update_pause_button.hpp"

#include <QPushButton>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::update_pause_button()
{
    if (btn_pause_ != nullptr) {
        btn_pause_->setText(is_paused_ ? tr("▶️ Continuar") : tr("⏸️ Pausar"));
        btn_pause_->setChecked(is_paused_);
    }
}

}  // namespace source::plugin::Audiolivro
