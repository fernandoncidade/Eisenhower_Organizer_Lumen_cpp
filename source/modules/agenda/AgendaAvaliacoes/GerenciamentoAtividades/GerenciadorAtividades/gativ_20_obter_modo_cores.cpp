#include "source/modules/agenda/AgendaAvaliacoes/GerenciamentoAtividades/Gerenciador_Atividades.hpp"

QString GerenciamentoAtividades::obter_modo_cores(const QString &modulo) const {
    const QString mod = modulo.trimmed().toLower();
    if (mod == QStringLiteral("pomodoro")) {
        return modo_cores_pomodoro.isEmpty() ? QStringLiteral("coloridas") : modo_cores_pomodoro;
    }
    if (mod == QStringLiteral("feynman")) {
        return modo_cores_feynman.isEmpty() ? QStringLiteral("coloridas") : modo_cores_feynman;
    }
    if (mod == QStringLiteral("geral")) {
        if (modo_cores_eisenhower == modo_cores_pomodoro && modo_cores_pomodoro == modo_cores_feynman) {
            return modo_cores_eisenhower.isEmpty() ? QStringLiteral("coloridas") : modo_cores_eisenhower;
        }
        return QString();
    }
    return modo_cores_eisenhower.isEmpty() ? QStringLiteral("coloridas") : modo_cores_eisenhower;
}
