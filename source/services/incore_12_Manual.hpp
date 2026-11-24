#pragma once

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

namespace source::InterfaceCore::incore_12_Manual {

struct ManualDetails {
    QString summary;
    QStringList paragraphs;
    QStringList bullets;
};

struct ManualSection {
    QString id;
    QString title;
    QStringList paragraphs;
    QStringList bullets;
    QList<ManualDetails> details;
};

struct ManualBlock {
    QString kind;
    QString text;
    QString section_id;
};

struct ManualTextWithPositions {
    QString text;
    QMap<QString, int> positions;
    QStringList order;
};

QString normalize_language(const QString &lang = QString());
QString get_manual_title(const QString &lang = QString());
QString to_unicode_bold(const QString &text);
// O documento gerado inclui instruções de uso para Configurações -> Layout de Tarefas ->
// Mostrar Abas de Interface, para o Layout em Abas (Vertical / Horizontal com recolhimento, rotação 90° e contorno de checkbox),
// para o Layout Matriz 2x2 do Eisenhower com rótulos de prioridade completos nas abas, para as listas Em Progresso integradas com Drag&Drop bidirecional e sincronização Pomodoro,
// para o design responsivo da janela de edição de tarefas (modos Compacto e Amplo), novos campos de quantidade (🍅) e metas de ciclos Pomodoro (🎯), duração em dias e dicas de contexto com cálculo de conclusão percentual,
// para as ações integradas do Cronograma de Gantt no menu de contexto de tarefas (criação direta, vinculação, abertura com realce, apontamento de avanço físico de obra com diário de anotações, alteração e desvinculação),
// para o aviso inteligente de discrepância em metas de ciclos no Pomodoro (Cycle Mismatch Dialog) com redefinição sincronizada,
// para o módulo de Cronograma de Gantt & Engenharia 4D BIM (EAP/WBS, linha do tempo, caminho crítico, dependências, linha de base, simulação visual 4D com elementos BIM, conectores MS Project XML e CSV com mapeamento visual, calendários de trabalho/feriados, auditoria e API REST/Webhooks),
// para o módulo Modelo Qwen de Inteligência Artificial Local (gerenciador interativo de download sob demanda com pausa, retomada e exclusão para liberação de espaço em disco, detecção dinâmica de modelos Ollama com guia interativo de comandos, inferência quantizada 100% offline via llama.cpp, geração de nós hierárquicos e mapas mentais conceituais automatizados, análise textual semântica e resumos conceituais),
// para as agendas personalizadas com listas aninhadas, vínculo de arquivos e cores dinâmicas do Registro de Atividades,
// para a centralização vertical de botões, para as opções Simples/Robusta do Pomodoro com rótulo '⏱️ Temporizador' / '⏱️ Timer',
// para o encerramento e resposta instantânea da confirmação ao editar tarefas com salvamento e sincronização em segundo plano,
// para a visibilidade do ciclo de vida completo no Calendário (reportando tarefas [Concluída], [Pendente] e [Em Progresso] no quadro Lista Global de Tarefas e tooltips, com abertura de arquivo por duplo clique e tradução dinâmica),
// para ferramentas de produtividade como recorrências e lembretes locais, para a lista global de tarefas,
// para remoção de tarefas pelo menu de contexto no Eisenhower e no Feynman, para colorir tarefas selecionadas,
// para configurações de fonte de tarefas em geral e tarefas com arquivo vinculado (globais e independentes por módulo),
// para confirmação de sucesso ao editar tarefas, para o Gerenciador de módulos e Gerenciador de logs,
// e para a tradução dinâmica em tempo de execução de botões, janelas auxiliares, amostragem de cores e contadores, cobrindo todos os 7 módulos nativos integrados.
QList<ManualSection> get_manual_document(const QString &lang = QString(),
                                         bool include_pomodoro_plugin = false,
                                         bool include_feynman_plugin = false,
                                         bool include_mapamental_plugin = false,
                                         bool include_audiolivro_plugin = false,
                                         bool include_agenda_plugin = false,
                                         bool include_modelo_plugin = false,
                                         bool include_gantt_plugin = false);
QPair<QList<ManualBlock>, QStringList> get_manual_blocks(const QString &lang = QString(),
                                                         bool include_pomodoro_plugin = false,
                                                         bool include_feynman_plugin = false,
                                                         bool include_mapamental_plugin = false,
                                                         bool include_audiolivro_plugin = false,
                                                         bool include_agenda_plugin = false,
                                                         bool include_modelo_plugin = false,
                                                         bool include_gantt_plugin = false);
QString get_manual_text(const QString &lang = QString(),
                        bool include_pomodoro_plugin = false,
                        bool include_feynman_plugin = false,
                        bool include_mapamental_plugin = false,
                        bool include_audiolivro_plugin = false,
                        bool include_agenda_plugin = false,
                        bool include_modelo_plugin = false,
                        bool include_gantt_plugin = false);
ManualTextWithPositions get_manual_text_with_positions(const QString &lang = QString(),
                                                       bool include_pomodoro_plugin = false,
                                                       bool include_feynman_plugin = false,
                                                       bool include_mapamental_plugin = false,
                                                       bool include_audiolivro_plugin = false,
                                                       bool include_agenda_plugin = false,
                                                       bool include_modelo_plugin = false,
                                                       bool include_gantt_plugin = false);

}  // namespace source::InterfaceCore::incore_12_Manual
