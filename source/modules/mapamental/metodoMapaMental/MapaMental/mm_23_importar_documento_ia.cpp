#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_23_importar_documento_ia.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_39_processar_documento_ia.hpp"

namespace source::plugin::MapaMental {

void MapaMental::importar_documento_ia()
{
    const QString caminho =
        QFileDialog::getOpenFileName(this,
                                     QCoreApplication::translate("MapaMentalPlugin", "Importar documento"),
                                     QDir::homePath(),
                                     QCoreApplication::translate("MapaMentalPlugin", "Documentos (*.pdf *.epub *.doc *.docx *.txt *.md *.csv *.xlsx *.json *.db *.sqlite *.sqlite3 *.xml);;Todos os arquivos (*.*)"));
    if (caminho.isEmpty() || processador_ia_ == nullptr) {
        return;
    }

    processar_documento_com_ia(caminho, QCoreApplication::translate("MapaMentalPlugin", "Importar documento"));
}

}  // namespace source::plugin::MapaMental
