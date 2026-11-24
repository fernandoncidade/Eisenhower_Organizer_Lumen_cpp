#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

#include <optional>

#include "source/utils/LogManager.hpp"

namespace source::plugin::Pomodoro {

class Tarefa {
public:
    Tarefa(const QString &titulo,
           const QString &descricao = QString(),
           const QStringList &etapas = {},
           const QString &prioridade = QStringLiteral("Importante e Urgente"),
           const QString &status = QStringLiteral("Todo"),
           const QString &id = QString(),
           const QString &file_path = QString(),
           const QString &date = QString(),
           const QString &time = QString(),
           int dominio = 0,
           const QString &cor_texto = QString(),
           const QString &integracao_origem = QString());

    QVariantMap to_dict() const;
    static std::optional<Tarefa> from_dict(const QVariantMap &data);
    static QString normalizar_prioridade(const QString &valor);
    static int normalizar_dominio(const QVariant &valor);

    QString titulo;
    QString descricao;
    QStringList etapas;
    QString prioridade;
    QString status;
    QString data_criacao;
    int pomodoros_completados = 0;
    int ciclos_totais = 4;
    bool ciclos_customizados = false;
    QString id;
    QString file_path;
    QString date;
    QString time;
    int dominio = 0;
    double duracao = 1.0;
    QString cor_texto;
    QString integracao_origem;
    bool sincronizada_com_eisenhower = false;
    QString recurrence;
    QString recurrence_until;
    QVariantMap font_config;
    source::utils::LogManager *logger = nullptr;

    double percentual_conclusao() const;
};

}  // namespace source::plugin::Pomodoro
