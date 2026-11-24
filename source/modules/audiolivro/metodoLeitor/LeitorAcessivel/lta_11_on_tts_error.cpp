#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_11_on_tts_error.hpp"

#include <QMessageBox>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::on_tts_error(const QString &err)
{
    if (!err.trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Audiolivro"), err);
    }
    leitura_finalizada();
}

}  // namespace source::plugin::Audiolivro
