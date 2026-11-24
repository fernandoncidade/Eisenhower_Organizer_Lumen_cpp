#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_17_salvar_mapa.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"

namespace source::plugin::MapaMental {

void MapaMental::salvar_mapa()
{
    const QString caminho = QFileDialog::getSaveFileName(
        this,
        QCoreApplication::translate("MapaMentalPlugin", "Salvar mapa"),
        QDir::home().filePath(QStringLiteral("mapa_mental.json")),
        QCoreApplication::translate("MapaMentalPlugin", "JSON (*.json)"));
    if (!caminho.isEmpty()) {
        salvar_mapa_em_arquivo(caminho);
    }
}

}  // namespace source::plugin::MapaMental
