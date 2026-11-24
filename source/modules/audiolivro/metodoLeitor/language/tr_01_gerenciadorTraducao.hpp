#pragma once

#include <QMap>
#include <QObject>
#include <QString>

class QTranslator;

namespace source::plugin::Audiolivro::language {

class GerenciadorTraducao : public QObject {
public:
    explicit GerenciadorTraducao(QObject *parent = nullptr);
    ~GerenciadorTraducao() override;

    bool aplicar_traducao(const QString &codigo_idioma);
    QString obter_idioma_atual() const;
    void traduzir_botoes_padrao(QObject *dialogo) const;
    const QMap<QString, QString> &idiomas_disponiveis() const;

private:
    QString normalizar_codigo_idioma(const QString &codigo_idioma) const;
    void remover_tradutor_instalado();
    bool load_qt_translator_for_locale(const QString &codigo_idioma);

    QTranslator *tradutor_ = nullptr;
    QTranslator *qt_translator_ = nullptr;
    QString idioma_atual_ = QStringLiteral("pt_BR");
    QMap<QString, QString> idiomas_disponiveis_;
    QString dir_traducoes_;
};

}  // namespace source::plugin::Audiolivro::language
