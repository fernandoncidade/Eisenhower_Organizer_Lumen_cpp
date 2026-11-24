#pragma once

#include <QString>
#include <QVariant>

namespace source::plugin::Pomodoro::GerenciadorTarefasFns {

QString get_text(const QString &text);
QString prioridade_para_texto(const QVariant &prio);
QString dominio_insignia(const QVariant &dominio);
QString dominio_para_texto(const QVariant &dominio, bool short_form = false);

}  // namespace source::plugin::Pomodoro::GerenciadorTarefasFns
