#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QTranslator>

namespace source::gantt::language {

QString obter_diretorio_traducoes();

class GerenciadorTraducao : public QObject {
    Q_OBJECT
public:
    explicit GerenciadorTraducao(QObject *parent = nullptr);
    ~GerenciadorTraducao() override;

    bool aplicar_traducao(const QString &codigo_idioma = QString());
    bool carregar_traducao_direta(const QString &codigo_idioma, const QString &qm_file_path);
    bool definir_idioma(const QString &codigo_idioma);
    QString obter_idioma_atual() const;

    QHash<QString, QString> idiomas_disponiveis;

signals:
    void idioma_alterado(const QString &codigo_idioma);

private:
    void _remover_tradutor_instalado();

    QTranslator *tradutor_ = nullptr;
    QHash<QString, QTranslator *> translators_cache_;
    QString idioma_atual_ = QStringLiteral("pt_BR");
    QString idioma_instalado_;
    QString idioma_padrao_ = QStringLiteral("pt_BR");
    QString dir_traducoes_;
};

} // namespace source::gantt::language
