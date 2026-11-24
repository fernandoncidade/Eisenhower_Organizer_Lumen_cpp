#pragma once

#include <QColor>
#include <QFont>
#include <QString>

namespace source::plugin::Audiolivro {

QString audiolivro_persistent_dir();
QString audiolivro_config_path();
QString carregar_configuracao_texto(const QString &key, const QString &fallback = QString());
void salvar_configuracao_texto(const QString &key, const QString &value);
QFont fonte_audiolivro_padrao();
QFont carregar_fonte_configurada();
void salvar_fonte_configurada(const QFont &font);
QColor carregar_cor_configurada();
void salvar_cor_configurada(const QColor &color);
void salvar_pdf_carregado_temp(const QString &pdf_path);
QString carregar_pdf_carregado_temp();
void salvar_texto_carregado_temp(const QString &content);
QString carregar_texto_carregado_temp();
void limpar_texto_carregado_temp();
void salvar_aba_ativa_temp(int index);
int carregar_aba_ativa_temp();

}  // namespace source::plugin::Audiolivro
