#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_38_extrair_texto_db.hpp"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>
#include <QVariant>

namespace source::plugin::MapaMental::ProcessadorIAFns {
namespace {

QString quote_identifier(QString name)
{
    name.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QStringLiteral("\"%1\"").arg(name);
}

}  // namespace

QString extrair_texto_db(const QString &caminho)
{
    const QString conexao = QStringLiteral("mapamental_db_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    QString texto;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), conexao);
        db.setDatabaseName(caminho);
        if (!db.open()) {
            db = QSqlDatabase();
            QSqlDatabase::removeDatabase(conexao);
            return {};
        }

        QStringList tabelas;
        QSqlQuery query_tabelas(db);
        if (query_tabelas.exec(QStringLiteral("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name"))) {
            while (query_tabelas.next()) {
                tabelas.append(query_tabelas.value(0).toString());
            }
        }

        for (const QString &tabela : tabelas) {
            texto += QStringLiteral("\n[%1]\n").arg(tabela);
            QSqlQuery query(db);
            if (!query.exec(QStringLiteral("SELECT * FROM %1 LIMIT 1000").arg(quote_identifier(tabela)))) {
                continue;
            }

            const QSqlRecord record = query.record();
            QStringList colunas;
            for (int i = 0; i < record.count(); ++i) {
                colunas.append(record.fieldName(i));
            }
            texto += colunas.join(QLatin1Char('\t'));
            texto += QLatin1Char('\n');

            while (query.next()) {
                QStringList valores;
                for (int i = 0; i < record.count(); ++i) {
                    valores.append(query.value(i).toString().simplified());
                }
                texto += valores.join(QLatin1Char('\t'));
                texto += QLatin1Char('\n');
            }
        }

        db.close();
    }

    QSqlDatabase::removeDatabase(conexao);
    return texto.trimmed();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
