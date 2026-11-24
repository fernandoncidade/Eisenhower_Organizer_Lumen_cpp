#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_08_extrair_texto.hpp"

#include <QFileInfo>

#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_03_extrair_texto_pdf.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_04_corrigir_caracteres_extraidos.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_06_extrair_texto_docx.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_07_extrair_texto_txt.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_34_extrair_texto_epub.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_35_extrair_texto_csv.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_36_extrair_texto_xlsx.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_37_extrair_texto_json.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_38_extrair_texto_db.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_39_extrair_texto_xml.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_40_extrair_texto_doc.hpp"

namespace source::plugin::MapaMental {

QString ProcessadorIA::extrair_texto(const QString &caminho) const
{
    const QString extensao = QFileInfo(caminho).suffix().toLower();
    QString texto;

    if (extensao == QStringLiteral("pdf")) {
        texto = ProcessadorIAFns::extrair_texto_pdf(caminho);
    } else if (extensao == QStringLiteral("epub")) {
        texto = ProcessadorIAFns::extrair_texto_epub(caminho);
    } else if (extensao == QStringLiteral("docx")) {
        texto = ProcessadorIAFns::extrair_texto_docx(caminho);
    } else if (extensao == QStringLiteral("doc")) {
        texto = ProcessadorIAFns::extrair_texto_doc(caminho);
    } else if (extensao == QStringLiteral("csv")) {
        texto = ProcessadorIAFns::extrair_texto_csv(caminho);
    } else if (extensao == QStringLiteral("xlsx")) {
        texto = ProcessadorIAFns::extrair_texto_xlsx(caminho);
    } else if (extensao == QStringLiteral("json")) {
        texto = ProcessadorIAFns::extrair_texto_json(caminho);
    } else if (extensao == QStringLiteral("db") || extensao == QStringLiteral("sqlite")
               || extensao == QStringLiteral("sqlite3")) {
        texto = ProcessadorIAFns::extrair_texto_db(caminho);
    } else if (extensao == QStringLiteral("xml")) {
        texto = ProcessadorIAFns::extrair_texto_xml(caminho);
    } else {
        texto = ProcessadorIAFns::extrair_texto_txt(caminho);
    }

    const QString conteudo = ProcessadorIAFns::corrigir_caracteres_extraidos(texto).trimmed();
    if (conteudo.isEmpty()) {
        return {};
    }

    const QString nome_fonte = QFileInfo(caminho).completeBaseName().simplified().trimmed();
    return nome_fonte.isEmpty()
               ? conteudo
               : QStringLiteral("NOME DO ARQUIVO/FONTE: %1\n\n%2").arg(nome_fonte, conteudo);
}

}  // namespace source::plugin::MapaMental
