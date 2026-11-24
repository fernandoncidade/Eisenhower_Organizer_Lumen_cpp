# MANUAL - EISENHOWER ORGANIZER / LÚMEN

Versão documentada: `2026.11.22.0`
Data: `22 de novembro de 2026`

## Português (Brasil)

Este manual descreve o comportamento atual do **EISENHOWER ORGANIZER** e do **LÚMEN** em uma abordagem técnico-operacional. O foco é explicar como usar cada aplicativo, como os módulos integrados se comportam e quais são as diretrizes de licenciamento e privacidade.

> **Documentação Complementar:** Consulte também [`README.md`](README.md#ptbr), [`docs/user-guide.md`](docs/user-guide.md#ptbr), [`docs/architecture.md`](docs/architecture.md#ptbr), [`docs/privacy-policy.md`](docs/privacy-policy.md#ptbr) e [`docs/licensing.md`](docs/licensing.md#ptbr).
>
> **Requisitos Técnicos e Escopo Auxiliar ("AS IS"):** Desenvolvido nativamente em **C++23 com Qt 6.12.0** (Pure MSVC `14.52` e MinGW `15.1.0` x64) sob **Duplo Licenciamento** (**GNU AGPLv3** para o código-fonte público em [`LICENSE`](LICENSE) e **EULA + CLC Oficial** para os pacotes assinados da Microsoft Store — Eisenhower Organizer `9P289X0185C3` e Lúmen `9N70CLLMVRPN`). Aplicativo fornecido **"NO ESTADO EM QUE SE ENCONTRA" ("AS IS")** como ferramenta auxiliar, devendo resultados críticos de prazos, IA local (`Qwen`/`Ollama`) ou cronogramas de engenharia (`Gantt BIM 4D` / CPM — sujeito a **ART/RRT** perante **CREA/CAU**) ser sempre conferidos de forma independente.

## 1. Visão geral

O EISENHOWER ORGANIZER / LÚMEN é um aplicativo desktop para organizar tarefas, estudos, foco, leitura assistida, cronogramas temporais e atividades avaliativas.

Ele reúne 7 áreas de trabalho e módulos nativos integrados de alta produtividade:

- `Eisenhower`: planejamento matricial de tarefas por importância e urgência (Matriz 2x2 e Layout em Abas).
- `Feynman`: método de estudo ativo estruturado por conceitos, explicações e revisões espaçadas.
- `Pomodoro`: execução de tarefas com temporizador de foco e ciclos de pausa (layouts Simples e Robusta).
- `Mapa Mental`: elaboração visual de conceitos conectados, com suporte à geração automatizada via Inteligência Artificial Local.
- `Audiolivro`: leitura assistida multimodal de documentos de texto e PDFs com síntese de voz e destaque visual de termos.
- `Agenda (Registro de Atividades)`: cadastro de atividades acadêmicas, avaliações, provas e compromissos com suporte a agendas personalizadas e listas aninhadas.
- `Cronograma de Gantt BIM 4D`: planejamento profissional de cronogramas de engenharia e projetos com linha do tempo interativa, caminho crítico e simulação 4D construtiva.
- `Modelo de IA Local (Qwen e Ollama)`: motor de inferência privado com suporte a modelos Qwen locais sob demanda e quaisquer modelos instalados via Ollama (Llama 3.2, Qwen 2.5, Gemma 2, Mistral, etc.) com detecção dinâmica, gerenciador de download interativo e fallback offline para síntese e estruturação de conhecimento no Mapa Mental.

A partir da versão 2026.10.7.0, a plataforma adota uma arquitetura de inteligência artificial leve e sob demanda: o instalador não carrega arquivos pesados pré-embutidos, permitindo que o modelo oficial Qwen2.5-3B-Instruct (GGUF) seja baixado pelo Gerenciador de Download com pausa, retomada e opção de exclusão para liberação de espaço em disco, além de integração nativa com modelos instalados no Ollama local.

### 1.1. NOTAS DE LANÇAMENTO: Versão 2026.11.22.0

Data: 22 de novembro de 2026

Esta atualização consolida a modernização da plataforma, o pacote de instalação enxuto, a documentação bilíngue completa e a validação abrangente de todos os módulos:

1. **Base Modernizada (C++23 e Qt 6.12.0) e Suporte Dual MSVC / MinGW**:
   - Código-fonte e todos os 7 módulos nativos atualizados para o padrão C++23 estrito e Qt 6.12.0, compilados nativamente em Pure MSVC (`14.52`) e MinGW-w64 (`15.1.0`).
2. **Distribuição Limpa e Instaladores Compactos**:
   - Pastas externas de distribuição enxutas e instaladores Inno Setup compactos usando implantação local do VC++ Runtime (`app-local CRT`, sem embutir `vc_redist.x64.exe`).
3. **Suíte Unificada de Testes (`tests/` — 11 Executáveis CTest)**:
   - Toda a verificação automatizada foi consolidada em 11 alvos CTest cobrindo utilitários, persistência atômica, interface host e os 7 módulos (`Qt6: Compilar e Executar SUITE (MSVC)` e `Qt6: Compilar e Executar SUITE (MinGW)`, 11/11 aprovados).
4. **Duplo Licenciamento e Janela `Sobre` Atualizada**:
   - Adoção integral do modelo de Duplo Licenciamento (**GNU AGPLv3** para o código-fonte público com liberdade em `source/assets/` + **EULA Oficial + CLC** para as edições da Microsoft Store), com textos informativos atualizados na janela `Sobre` (`Ctrl+Shift+A`) e documentação bilíngue completa em `docs/`.

### 1.2. NOTAS DE LANÇAMENTO: Versão 2026.10.7.0

Data: 7 de outubro de 2026

Esta atualização traz uma evolução substancial nos recursos de Inteligência Artificial Local, com a introdução do Gerenciador de Download do Modelo Qwen sob demanda, integração dinâmica com modelos Ollama, tradução dinâmica em tempo de execução e aprimoramentos gerais:

1. **Gerenciador Interativo de Download do Modelo Local Qwen 2.5 (GGUF)**:
   - Gerenciador com suporte a download sob demanda do modelo oficial `qwen2.5-3b-instruct-q4_k_m.gguf` (~2,17 GB) diretamente do repositório oficial no Hugging Face.
   - Recursos completos de controle: barra de progresso com porcentagem, volume transferido, velocidade em tempo real (MB/s) e tempo restante estimado.
   - Botões para Pausar e Retomar a qualquer momento utilizando requisições de continuação (HTTP Range), sem perda de tráfego de dados.
   - Botão para Excluir Modelo fisicamente do disco com confirmação para liberar imediatamente ~2,17 GB de armazenamento quando não estiver em uso.
   - Botão para Abrir Pasta no Windows Explorer para gerenciamento manual dos arquivos de modelos.

2. **Indicador Dinâmico de Status nos Menus**:
   - O menu `Configurações -> Configurações Específicas -> Mapa Mental` agora reflete em tempo real se o modelo está pronto para uso: exibe `Resumo com modelo qwen2.5-3b [Instalado]` quando presente e validado, ou `Resumo com modelo qwen2.5-3b [Não instalado]` caso ausente.
   - Ação direta `Como baixar ou gerenciar modelo Qwen (GGUF)...` que abre imediatamente a janela interativa do gerenciador de download.

3. **Integração Completa com Modelos Locais via Ollama**:
   - Diálogo com guia interativo passo a passo `Como instalar modelos no Ollama...` com orientações de instalação a partir do site oficial.
   - Botões com um clique para copiar comandos de instalação no terminal para modelos recomendados (`llama3.2`, `qwen2.5:3b`, `gemma2:2b`, `mistral:7b`, etc.).
   - Botão para testar conexão com o servidor local do Ollama (`http://localhost:11434`).
   - Detecção automática de todos os modelos instalados no Ollama, listados dinamicamente no submenu `Modelos Ollama`.

4. **Tradução Dinâmica em Tempo de Execução**:
   - Suporte integral à alternância imediata de idioma entre Português e Inglês para todas as telas, botões, mensagens, diálogos auxiliares e menus do módulo de inteligência artificial sem necessidade de reiniciar o aplicativo.

5. **Distribuição Otimizada e Leve**:
   - O tamanho do pacote de instalação foi drasticamente reduzido, eliminando a dependência de arquivos pesados de modelo pré-embutidos e oferecendo total controle ao usuário sobre seu espaço em disco.

### 1.3. NOTAS DE LANÇAMENTO: Versão 2026.9.27.0

Data: 27 de setembro de 2026

Esta atualização consolida a arquitetura completa da plataforma, integrando em definitivo todos os recursos como módulos nativos integrados de alta performance, expandindo a inteligência artificial local e concluindo a documentação e conformidade legal de todos os módulos:

1. **Consolidação Definitiva dos 7 Módulos Nativos**:
   - Transição integral da arquitetura: os antigos plugins externos passam a operar como 7 módulos nativos integrados (`Eisenhower`, `Feynman`, `Pomodoro`, `Mapa Mental`, `Audiolivro`, `Agenda` e `Cronograma de Gantt BIM 4D`), com o módulo complementar `Modelo` (IA Local).
   - Eliminação de carregamentos dinâmicos frágeis, propiciando inicialização ultrarrápida, maior estabilidade operacional e sincronização bidirecional em tempo real entre todas as visões de trabalho.

2. **Remoção Completa de Tags Delimitadoras e Higienização Visual**:
   - Higienização integral de todas as telas, documentações legais, manuais e notas de lançamento, eliminando em 100% quaisquer tags delimitadoras internas antigas (como chaves duplas e marcadores técnicos de substituição).
   - O texto agora é apresentado de forma limpa, direta, agradável e profissional em todas as caixas de diálogo, telas de informações sobre o aplicativo, avisos e termos de licença.

3. **Documentação e Governança Legal Completa dos Novos Módulos**:
   - Elaboração e inclusão completa de todos os documentos regulatórios (Sobre/About, Contrato de Licença de Usuário Final/EULA, Direitos Autorais/Copyright, Políticas de Privacidade e Avisos de Terceiros/Notice) para os módulos `Agenda` e `Modelo`, tanto em português (pt_BR) quanto em inglês (en_US).
   - Integração com a ferramenta de monitoramento e edição de versão (`find_VersionEditor`), assegurando que revisões de versão atualizem automaticamente todos os 48 arquivos documentais do ecossistema.

4. **Módulo Modelo Qwen de Inteligência Artificial Local Privada**:
   - Integração de primeira classe com o motor de inferência offline da família Qwen2.5, proporcionando análise semântica e geração automatizada de nós conceituais para o módulo Mapa Mental.
   - Operação 100% local e offline: garantia absoluta de privacidade, sem transmissão de documentos, textos ou dados de usuário para servidores em nuvem.
   - Mecanismo inteligente de fallback heurístico transparente para cenários de arquivos extensos ou máquinas com recursos reduzidos de memória RAM.

5. **Localização e Catálogos de Tradução Dinâmicos**:
   - Compilação unificada de catálogos de tradução (`.ts` / `.qm`), propiciando alternância imediata em tempo de execução entre Português e Inglês para todas as novas telas, menus e mensagens informativas sem necessidade de reiniciar a aplicação.

### 1.2. NOTAS DE LANÇAMENTO: Versão 2026.9.21.0

Data: 21 de setembro de 2026

Esta atualização traz otimizações críticas de desempenho e melhorias substanciais no acompanhamento diário de tarefas em toda a aplicação:

1. **Resposta Instantânea ao Aplicar Edições de Tarefas**:
   - Resposta imediata ao clicar em "OK" na janela de edição de tarefas (`✏️ Editar Tarefa`) em todos os módulos (Eisenhower, Pomodoro e Feynman): a confirmação responde e a janela é encerrada no mesmo instante, liberando a interface do usuário sem esperas ou travamentos.
   - A persistência em disco e a sincronização entre módulos ocorrem suavemente em segundo plano, garantindo alta performance e fluidez operacional.
   - Movimentação imediata entre quadrantes pelo menu de contexto com atualização visual prévia ao salvamento em disco.

2. **Visibilidade Abrangente do Ciclo de Vida no Calendário ([Em Progresso])**:
   - O painel do Calendário agora exibe com transparência todas as etapas do ciclo de vida das atividades: o quadro `Lista Global de Tarefas:` e os balões informativos de datas passam a reportar tarefas concluídas (`[Concluída]`), pendentes (`[Pendente]`) e em andamento (`[Em Progresso]`).
   - Visualização limpa e padronizada, sem inclusão indevida de linhas adicionais de pomodoros na listagem do calendário.
   - Interação prática e direta: duplo clique em qualquer tarefa da lista abre imediatamente seu arquivo vinculado (quando existente), além de acesso completo ao menu de contexto pelo clique com botão direito.

3. **Tradução Dinâmica em Tempo de Execução**:
   - Suporte completo à tradução dinâmica entre Português e Inglês para os novos rótulos de status do calendário (`[Em Progresso]` / `[In Progress]`), tooltips informativos e diálogos de confirmação, sem necessidade de reiniciar o programa.

4. **Padronização Visual do Temporizador Pomodoro**:
   - A moldura do quadro do temporizador no módulo Pomodoro agora exibe o rótulo descritivo `⏱️ Temporizador` (com tradução dinâmica para `⏱️ Timer` em inglês).

### 1.3. NOTAS DE LANÇAMENTO: Versão 2026.9.19.0

Data: 19 de setembro de 2026

Esta atualização traz importantes expansões operacionais, sincronização aprofundada de ciclos e prazos, novo diálogo responsivo de tarefas, controle integrado de obras no Cronograma Gantt 4D BIM e melhorias na experiência de usuário em toda a aplicação:

1. **Diálogo Responsivo de Edição de Tarefas**:
   - Redimensionamento e adaptação inteligente do layout de edição de tarefas (`Compacto` em janelas padrão e `Largo` em duas colunas com barra lateral fixada em 410px quando a largura for igual ou superior a 760px ou em tela cheia).
   - Novos campos estruturados: Ciclos Pomodoro realizados (🍅), Meta de Ciclos Pomodoro (🎯) e Duração estimada da atividade (em dias).
   - Tooltip informativo seguro nas listagens de tarefas com cálculo dinâmico do percentual de conclusão baseado em metas de foco e medições.

2. **Detecção e Diálogo de Discrepância de Ciclo no Pomodoro**:
   - Diálogo preventivo ao iniciar tarefas em andamento ("Em Progresso" / Doing) quando houver divergência entre a meta de ciclos da tarefa (🎯) e a meta ativa configurada no temporizador (⚙️).
   - Apresentação de tabela detalhada de divergências e três caminhos de ação: "Resetar Tudo" (com sincronização imediata no aplicativo principal e módulos integrados), "Iniciar Mesmo Assim" ou "Cancelar".

3. **Integração Avançada do Cronograma de Gantt 4D BIM no Menu de Contexto**:
   - Ações diretas pelo menu de contexto (botão direito) no Eisenhower, Pomodoro e Feynman:
     - `📊 Criar Atividade no Gantt`: cria uma nova atividade na EAP/WBS com o título da tarefa, duração em dias e datas de início/término.
     - `📊 Vincular ao Cronograma Gantt...`: vincula a tarefa a uma atividade existente.
     - `📊 Abrir no Cronograma Gantt`: abre diretamente a janela do cronograma posicionada na atividade vinculada, com indicação de código WBS e percentual.
     - `📊 Registrar Avanço de Campo...`: registra percentual físico de conclusão (0% a 100%) e observações do diário de obra em tempo real.
     - `📊 Alterar Vínculo Gantt...` e `📊 Desvincular do Gantt`.

4. **Simulação 4D BIM, Intercâmbio MS Project XML/CSV e Serviços de Engenharia**:
   - Painel de Simulação 4D BIM (`Bim4DSimulationPanel`) com controle de reprodução cronológica e velocidade ajustável para acompanhamento visual da evolução da obra.
   - Conector completo para importação e exportação de arquivos MS Project em formato XML (`MsProjectXmlConnector`).
   - Serviço de intercâmbio CSV com diálogo de mapeamento flexível de colunas (`CsvMappingDialog`) e importador dedicado de tarefas Eisenhower para atividades de cronograma.
   - Gestão de calendários de trabalho com definição de dias úteis semanais e cadastro de feriados e exceções não trabalháveis.
   - Sistema integrado de auditoria de alterações (`AuditService`), medições de campo com diário de obra (`FieldProgressService`), servidor HTTP de API REST e despachante de Webhooks para notificações automáticas.

5. **Exclusão Dinâmica no Registro de Atividades e Sincronização**:
   - Exclusão dinâmica de itens e títulos em listas de agendas personalizadas sincronizada em tempo real com a barra de menus principal do aplicativo.

### 1.4. NOTAS DE LANÇAMENTO: Versão 2026.9.16.0

Data: 16 de setembro de 2026

Esta atualização traz a integração do módulo Cronograma de Gantt & Engenharia 4D BIM, compatibilidade completa de build com compiladores MinGW e MSVC, e melhorias na experiência de gerenciamento de prazos e projetos:

1. **Módulo Cronograma de Gantt & Engenharia 4D BIM**:
   - Planejamento e controle de cronogramas de engenharia e projetos por meio de diagrama de Gantt interativo e metodologia 4D BIM.
   - Estrutura Analítica do Projeto (EAP/WBS) com codificação hierárquica, datas de início e término, duração em dias, marcos (milestones) e percentual de avanço físico.
   - Cálculo automático de dependências entre atividades (FS, SS, FF, SF) e destaque do Caminho Crítico (Critical Path) para mitigação proativa de atrasos.
   - Navegação na linha do tempo com zoom dinâmico em 4 escalas (Dia, Semana, Mês e Ano), identificação de dias úteis, finais de semana e feriados.

2. **Integração Bidirecional com a Matriz Eisenhower**:
   - Vinculação direta de tarefas operacionais da Matriz de Eisenhower a atividades específicas do cronograma via menu de contexto (`📊 Vincular ao Cronograma Gantt...`).
   - Sincronização automática em tempo real: conclusão de tarefas atualiza o progresso da atividade correspondente no cronograma; mudança para "Em Progresso" sincroniza o estado da atividade.
   - Ajuste sincronizado de estimativas de duração em dias e manutenção da integridade dos dados ao remover tarefas.

3. **Atalhos e Acessibilidade**:
   - Acesso rápido ao cronograma através do atalho global `Ctrl+Alt+G` e pelo menu `Ferramentas -> Cronograma de Gantt BIM 4D`.
   - Consulta rápida ao resumo de atividades vinculadas e progresso nas janelas informativas.

### 1.5. NOTAS DE LANÇAMENTO: Versão 2026.8.16.0

Data: 16 de agosto de 2026

Esta atualização traz importantes evoluções nos modos de visualização de tarefas, na gestão de atividades em andamento e no acabamento visual de toda a aplicação:

1. **Novos Modos de Layout em Abas (Vertical e Horizontal)**:
   - O menu `Configurações -> Layout de Tarefas -> Interface do Usuário -> Eisenhower -> Layout em Abas` passa a disponibilizar duas orientações distintas:
     - `Vertical`: exibe as seções de tarefas (Pendentes, Em Progresso e Concluídas) empilhadas verticalmente por quadrante.
     - `Horizontal`: exibe as seções de tarefas organizadas lado a lado em colunas horizontais.
   - **Recolhimento Inteligente com Borda de Contorno e Rotação**: Ao desmarcar a caixa de seleção de uma seção no modo Horizontal, a coluna se recolhe para uma largura ultra-compacta (~30px), seu rótulo é rotacionado em 90° no sentido horário para leitura vertical fluida e a moldura externa nativa desenha um contorno contínuo elegante que começa e termina perfeitamente nas laterais do checkbox no topo, alinhada com as bordas e alturas dos quadrantes vizinhos. Um clique na coluna ou no checkbox a expande de volta à visualização completa.

2. **Integração das Listas "Em Progresso" e Sincronização**:
   - Cada um dos quatro quadrantes da Matriz de Eisenhower agora possui sua própria lista dedicada de tarefas "Em Progresso" (Doing).
   - Suporte completo a arrastar e soltar (Drag & Drop) bidirecional entre as listas `Pendentes`, `Em Progresso` e `Concluídas` em todos os modos de layout.
   - Sincronização automática e em tempo real com o quadro Kanban e temporizador do módulo Pomodoro.

3. **Padronização dos Rótulos no Layout Matriz 2x2**:
   - No `Layout Matriz 2x2`, as molduras dos quadros da aba `Em Progresso` passam a exibir o nome completo e descritivo da prioridade do quadrante (`🔴 Importante e Urgente`, `🟠 Importante, mas Não Urgente`, `🟡 Não Importante, mas Urgente` e `🟢 Não Importante e Não Urgente`), eliminando rótulos genéricos repetidos e padronizando a identificação visual em todos os status.

4. **Centralização Vertical de Conteúdos em Todos os Botões**:
   - Alinhamento vertical aperfeiçoado de textos, emojis, ícones e elementos visuais compostos no centro exato da altura dos botões em toda a aplicação e em todos os módulos/plugins.

### 1.6. NOTAS DE LANÇAMENTO: Versão 2026.8.8.0

Data: 8 de agosto de 2026

Esta atualização traz melhorias na personalização visual do Registro de Atividades e reorganização nos menus de configuração:

1. **Configuração Independente de Cores por Módulo**:
   - Possibilidade de selecionar os modos `Preto` ou `Coloridas` individualmente para cada ambiente (`Eisenhower`, `Pomodoro` ou `Feynman`).
   - Opção `Geral` para aplicar simultaneamente a preferência de cor a todos os três módulos.

2. **Reorganização de Menus de Cores**:
   - Estruturação de submenus `Preto` e `Coloridas` no menu `Configurações -> Registro de Atividades -> Cores` da janela principal e no menu `Configurações -> Cores` da janela `Cadastro de Atividades`.
   - Suporte completo a tradução dinâmica e atualização visual das tarefas em tempo real.

### 1.7. NOTAS DE LANÇAMENTO: Versão 2026.8.7.0

Data: 7 de agosto de 2026

Esta atualização traz refinamentos na sincronização visual das tarefas e melhorias na organização dos filtros, garantindo uma experiência mais estável e organizada:

1. **Sincronização Visual e Cores Consistentes**:
   - Correção na atribuição de cores para atividades: as cores escolhidas ou geradas automaticamente para novas disciplinas agora permanecem consistentes sempre que o aplicativo é reaberto, eliminando mudanças aleatórias nas tarefas.
   - Sincronização inteligente de fontes: ao alterar a fonte de uma tarefa específica pelo menu de contexto (clique com o botão direito), a nova configuração é sincronizada corretamente com o menu principal sem alterar a aparência das demais tarefas visíveis na tela.

2. **Organização e Limpeza de Filtros**:
   - Filtros de data e horário unificados: o sistema agora padroniza a forma como as datas e horários são extraídos em todos os módulos (Eisenhower, Pomodoro e Feynman), evitando que opções duplicadas com formatos diferentes apareçam na lista de filtros.
   - Filtros mais precisos: campos com textos longos (como Descrição, Conceito, Lacunas Identificadas e Revisão) não geram mais opções indesejadas no menu de filtros. A lista de filtros agora se mantém limpa, exibindo apenas as categorias e atributos relevantes para a organização das tarefas.

### 1.8. NOTAS DE LANÇAMENTO: Versão 2026.7.27.0

Data: 27 de julho de 2026

Esta atualização traz uma reorganização completa da estrutura de menus no topo da janela principal, centralizando as opções de fontes, plugins e configurações específicas, além de aperfeiçoar o catálogo de vozes do leitor do Audiolivro:

1. **Estrutura Reorganizada do Menu Configurações**:
   - A barra de menus sob `Configurações` segue agora uma sequência lógica padronizada:
     - `Configurações -> Abrir`: Acesso direto às áreas de trabalho com atalhos de teclado (`Eisenhower Ctrl+Shift+I`, `Pomodoro Ctrl+Shift+P`, `Feynman Ctrl+Shift+F`, `Mapa Mental Ctrl+Shift+N`, `Audiolivro Ctrl+Shift+L`).
     - `Configurações -> Layout de Tarefas`: Seleção do modo de exibição de tarefas (`Interface do Usuário -> Eisenhower` em Abas, Quadrantes ou Matriz 2x2; `Pomodoro` Ao lado ou Embaixo; `Feynman` em Abas, Vertical ou Horizontal; `Mapa Mental` em Vertical, Horizontal, Radial ou Árvore) e ordenação de tarefas.
     - `Configurações -> Configurações de Fonte`: Centralização de fontes globais e por módulo (`Eisenhower`, `Pomodoro`, `Feynman`, `Mapa Mental` para Tarefas em Geral e Arquivo Vinculado, `Audiolivro` para Texto Audiolivro e a ferramenta `Colorir tarefas`).
     - `Configurações -> Configurações Específicas`: Submenu dedicado às opções exclusivas dos complementos (`Pomodoro -> Alarme, Modo automático`; `Mapa Mental -> Resumo método heurístico / Resumo com modelo qwen2.5-3b`; `Audiolivro -> Vozes`).
     - `Configurações -> Registro de Atividades`: Gerenciamento do complemento de atividades (`Excluir item`, `Cores`).
     - `Configurações -> Sincronizar`: Controle da sincronização de tarefas entre módulos (`Eisenhower`, `Pomodoro`, `Feynman`).
     - `Configurações -> Idioma`: Seleção dinâmica do idioma da aplicação em tempo de execução.

2. **Resiliência e Nomes Nativos no Catálogo de Vozes do Audiolivro**:
   - Correção do comportamento em que a lista de vozes do Audiolivro no menu `Configurações -> Configurações Específicas -> Audiolivro -> Vozes` era reduzida às vozes estáticas básicas ao alternar o idioma da interface.
   - O sistema de consulta de vozes agora preserva e re-tentará buscar o catálogo completo online do Edge TTS (300+ vozes em 70+ localidades) caso ocorra timeout pontual durante caixas de mensagem ou alterações de tela.
   - Todos os idiomas e países suportados no catálogo de vozes agora utilizam a nomenclatura nativa e formatada do `QLocale` (ex: `Português (Brasil)`, `English (United States)`, `Español (España)`, `Français (France)`, `Deutsch (Deutschland)`, `日本語 (日本)`, etc.).

3. **Traduções Dinâmicas em Tempo de Execução**:
   - Todas as novas chaves e caminhos de menus possuem suporte a tradução imediata em tempo de execução ao alternar o idioma no aplicativo.

### 1.9. NOTAS DE LANÇAMENTO: Versão 2026.7.25.0

Data: 25 de julho de 2026

Esta atualização traz importantes expansões de personalização de fontes e aprimoramentos na tradução dinâmica em tempo de execução dos plugins Pomodoro e Mapa Mental:

1. **Configuração de Fontes Independente por Plugin**:
   - Foram adicionados os menus de configuração de fonte diretamente nos caminhos dos plugins:
     - `Configurações -> Plugins -> Feynman -> Configurações de Fonte -> Tarefas em Geral, Arquivo Vinculado`
     - `Configurações -> Plugins -> Mapa Mental -> Configurações de Fonte -> Tarefas em Geral, Arquivo Vinculado`
     - `Configurações -> Plugins -> Pomodoro -> Configurações de Fonte -> Tarefas em Geral, Arquivo Vinculado`
   - Quando acionadas por estes caminhos, as configurações de fonte funcionam e são aplicadas **exclusivamente no seu respectivo plugin**, independente de a sincronização entre módulos (`Configurações -> Sincronizar -> Eisenhower, Pomodoro, Feynman`) estar ativa ou não.
   - No **Mapa Mental**, ao aplicar as configurações de fonte selecionadas, a formatação de fonte escolhida é aplicada imediatamente aos títulos de todos os Nós existentes na tela.

2. **Correção Gramatical e Acentuação no Plugin Mapa Mental**:
   - Correção completa dos textos exibidos no carregamento e processamento de documentos no Mapa Mental (janela "Importar documento"), ajustando mensagens de síntese de modelos de IA, extração de texto e contagem de nós para as normas da língua portuguesa brasileira.

3. **Tradução Dinâmica em Tempo de Execução**:
   - **Plugin Mapa Mental**: tradução instantânea ao trocar de idioma para os botões do plugin, abas, janela "Editor conceito", menu de contexto de Nós, janela "Escolher cor" (incluindo texto dinâmico de coordenadas do cursor "Cursor em X, Y" e indicação de cancelamento "Pressione ESC para cancelar") e janelas de Salvar, Carregar, Exportar, Importar e Limpar.
   - **Plugin Pomodoro**: tradução dinâmica em tempo de execução para todos os widgets do Pomodoro (botões, rótulos, janela de diálogo de edição de tarefa, menus de contexto, opções de prioridade e domínio, janelas de configuração de alarme e modo automático e mensagens informativas de dica).
   - **Contador Dinâmico da Interface**: o contador no canto superior direito da tela principal ("Mapa Mental: Selected tasks: 0 | Total tasks: 16" / "Pomodoro: Tarefas selecionadas: 0 | Total de tarefas: 16") passa a atualizar seus rótulos instantaneamente ao trocar o idioma.

### 1.10. NOTAS DE LANÇAMENTO: Versão 2026.7.8.0

Data: 8 de julho de 2026

Esta versão melhora a forma como as tarefas aparecem visualmente nas listas. Agora é possível configurar a fonte das tarefas comuns e também configurar, separadamente, a fonte das tarefas que possuem arquivo externo vinculado.

O novo caminho fica em `Configurações -> Espaço Reservado -> Configurações de Fonte`. Dentro dele existem duas opções:

- `Tarefas em Geral`: altera a aparência das tarefas sem arquivo externo vinculado.
- `Arquivo Vinculado`: altera a aparência das tarefas com arquivo externo vinculado.

Em cada janela, você pode escolher o tipo da fonte, o estilo da fonte, o tamanho, negrito, itálico, sublinhado e tachado. Use `Restaurar` para voltar aos valores padrão daquela categoria, `OK` para salvar ou `Cancelar` para sair sem aplicar mudanças.

As alterações aparecem nas tarefas do Eisenhower e também nas tarefas exibidas pelos plugins Pomodoro e Feynman quando esses módulos estão disponíveis. Tarefas comuns continuam usando a configuração de `Tarefas em Geral`; tarefas com arquivo vinculado usam a configuração de `Arquivo Vinculado`. Assim, uma mudança feita em uma categoria não altera a outra.

O destaque de arquivo vinculado foi movido do menu direto `Configurações -> Espaço Reservado -> Arquivo Vinculado` para o novo submenu `Configurações -> Espaço Reservado -> Configurações de Fonte -> Arquivo Vinculado`, mantendo a função no mesmo grupo visual das demais opções de fonte.

## 2. Tela principal

Na janela principal, você encontra:

- barra de menus no topo;
- abas dos módulos disponíveis;
- contador do módulo ativo no canto superior direito;
- área de tarefas Eisenhower;
- calendário e lista global de tarefas;
- janela opcional `Cadastro de Atividades`, quando o plugin Registro de Atividades está disponível;
- painéis auxiliares de Feynman e Pomodoro quando esses plugins estão disponíveis.

As abas e ferramentas aparecem conforme os plugins carregados. Se um plugin não estiver instalado, compilado ou copiado para o local esperado, a aba ou opção de menu correspondente não aparece.

## 3. Abas da interface

As abas de módulo podem incluir:

- `Eisenhower`;
- `Pomodoro`;
- `Feynman`;
- `Mapa Mental`;
- `Audiolivro`.

Em `Configurações -> Layout de Tarefas -> Abas da Interface`, você pode:

- bloquear o ajuste das abas;
- desbloquear o ajuste das abas;
- restaurar a ordem padrão.

Quando o ajuste está desbloqueado, você pode arrastar as abas para mudar a ordem. A escolha fica salva para os próximos usos.

Em `Configurações -> Layout de Tarefas -> Mostrar Abas de Interface`, você também pode definir quais abas de módulo ficarão visíveis na GUI:

- `Eisenhower`;
- `Pomodoro`, quando o plugin estiver disponível;
- `Feynman`, quando o plugin estiver disponível;
- `Mapa Mental`, quando o plugin estiver disponível;
- `Audiolivro`, quando o plugin estiver disponível.

O aplicativo reconhece automaticamente quais plugins estão disponíveis. As opções de plugins ausentes aparecem desabilitadas ou não ficam disponíveis para seleção. A aba `Eisenhower` pertence ao aplicativo principal e sempre existe; ainda assim, sua visibilidade pode ser desativada quando houver outra aba disponível, para manter a interface focada no módulo desejado. A seleção é salva em `layout.json`, na pasta persistente do usuário, e é reaplicada ao reiniciar o aplicativo.

Para evitar uma interface sem nenhuma área de trabalho visível, o aplicativo impede ocultar a última aba disponível. Se você quiser restaurar rapidamente a visualização principal, reative `Eisenhower` no mesmo menu.

## 4. Contador do módulo ativo

O contador superior muda conforme a aba aberta.

Ele pode indicar:

- quantos itens estão selecionados;
- quantos itens existem no módulo atual;
- total de tarefas, conceitos, itens em progresso ou elementos equivalentes do módulo ativo.

Esse contador ajuda a confirmar se você está olhando para a seleção correta, principalmente em telas com muitos itens.

## 5. Módulo Eisenhower

O Eisenhower organiza tarefas em 4 quadrantes:

- `Importante e Urgente`: fazer primeiro.
- `Importante, mas Não Urgente`: planejar.
- `Não Importante, mas Urgente`: resolver rapidamente ou delegar.
- `Não Importante e Não Urgente`: deixar para depois, reduzir ou eliminar.

Cada tarefa pode conter:

- texto principal;
- descrição;
- quadrante;
- prioridade;
- nível de domínio;
- data;
- horário;
- arquivo vinculado;
- estado pendente ou concluído.

## 6. Criar tarefas

Para criar uma tarefa:

1. Digite o texto da tarefa.
2. Escolha o quadrante.
3. Defina prioridade, se necessário.
4. Defina nível de domínio, se necessário.
5. Vincule data e horário, se a tarefa tiver prazo.
6. Clique em `Adicionar Tarefa` ou pressione Enter.

A tarefa também pode ser criada sem data e sem horário. Esses campos são opcionais.

## 7. Editar tarefas

Clique com o botão direito em uma tarefa e use `Editar Tarefa`.

### 7.1. Interface responsiva e modos de visualização

A janela de edição (`EditTaskDialog`) adapta seu layout dinamicamente de acordo com as dimensões da tela:

- **Modo Compacto**: ativado em janelas de tamanho padrão (< 760px), organizando os campos de texto, prioridade, prazos e metas em uma única coluna vertical fluida e de fácil rolagem.
- **Modo Largo**: ativado automaticamente quando a janela atinge largura igual ou superior a 760px ou ao maximizar. Nesse modo, a área de edição é dividida em duas colunas funcionais:
  - A coluna esquerda expande os campos principais de texto e descrição detalhada da tarefa.
  - A coluna direita fixa um painel lateral especializado com largura estável de 410px, reunindo controles de datas, horários, arquivo vinculado, metas de foco e medições.

### 7.2. Campos editáveis e novas métricas

Na edição, você pode alterar:

- texto principal;
- descrição detalhada;
- prioridade e quadrante de destino;
- nível de domínio;
- arquivo vinculado;
- data e horário de execução;
- **Ciclos Pomodoro realizados (🍅)**: quantidade de blocos de foco já executados na tarefa;
- **Meta de Ciclos Pomodoro (🎯)**: meta estimada de ciclos para conclusão da atividade;
- **Duração estimada (dias)**: duração prevista de execução em dias úteis/corridos, sincronizada automaticamente com o cronograma quando a tarefa está vinculada.

### 7.3. Percentual seguro de conclusão e tooltips informativos

Ao posicionar o cursor do mouse sobre qualquer tarefa nas listas do Eisenhower, Pomodoro ou calendário, um tooltip rico e seguro exibe:
- título e descrição completos;
- prioridade e nível de domínio;
- prazos de início, término e horários;
- tags e arquivos vinculados;
- **Percentual de conclusão calculado**: cálculo dinâmico e seguro do avanço da tarefa (0% a 100%), obtido a partir da relação entre ciclos realizados (🍅) e a meta estipulada (🎯), ou sincronizado com a medição física da atividade vinculada no Cronograma de Gantt.

Depois de concluir a edição e clicar em `OK`, o aplicativo registra as alterações e fecha a janela de diálogo de forma instantânea, liberando imediatamente a tela para que você continue operando o sistema. A janela de confirmação de sucesso é apresentada sem atrasos e a persistência dos dados em disco, assim como a sincronização entre os módulos integrados (Eisenhower, Pomodoro, Feynman e Cronograma Gantt), é processada de maneira suave e desacoplada em segundo plano. Essa confirmação acompanha o idioma ativo em tempo real.

## 8. Concluir, mover, selecionar e remover

Você pode:

- marcar uma tarefa como concluída;
- desmarcar para devolver a tarefa ao grupo ativo;
- mover tarefas entre quadrantes;
- arrastar e soltar uma ou várias tarefas entre quadrantes, listas e abas disponíveis;
- remover uma ou várias tarefas com confirmação;
- integrar e sincronizar tarefas diretamente com o Cronograma de Gantt 4D BIM.

Ao marcar a caixa de seleção de uma tarefa pendente, ela é movida para o grupo de concluídas. Ao desmarcar uma tarefa concluída, ela volta para o grupo pendente. Esse comportamento funciona mesmo quando a tarefa já está selecionada.

Seleção múltipla:

- use `Ctrl` para selecionar itens separados;
- use `Shift` para selecionar um intervalo;
- um clique simples em uma tarefa isolada volta a focar apenas nela.

Remoção pelo menu de contexto:

1. Clique com o botão esquerdo em uma tarefa para selecionar apenas ela, ou use `Ctrl`/`Shift` para selecionar várias tarefas.
2. Clique com o botão direito sobre a tarefa selecionada, ou sobre qualquer tarefa do grupo selecionado.
3. Escolha `Remover Tarefa`.
4. Na confirmação, escolha `Sim` para remover ou `Não` para cancelar.

Quando o idioma do aplicativo é alterado, os botões da confirmação também acompanham o idioma escolhido. Em português, a janela mostra `Sim` e `Não`; em inglês, mostra `Yes` e `No`. A opção `Não` é usada como escolha inicial para reduzir remoções acidentais.

Se você abrir o menu de contexto sobre uma tarefa que não faz parte da seleção atual, a ação considera a tarefa clicada. Se abrir o menu sobre uma das tarefas já selecionadas, a ação preserva o grupo selecionado.

### 8.1. Ações de integração com o Cronograma Gantt no menu de contexto

Ao clicar com o botão direito sobre uma tarefa, o menu de contexto oferece opções dedicadas de engenharia e planejamento de cronograma:

- `📊 Criar Atividade no Gantt`: converte a tarefa em uma nova atividade de cronograma na EAP/WBS do projeto, preenchendo automaticamente título, duração estimada em dias e datas planejadas.
- `📊 Vincular ao Cronograma Gantt...`: abre o diálogo de seleção para associar a tarefa operacional a uma atividade de obra já existente na EAP.
- `📊 Abrir no Cronograma Gantt`: abre imediatamente a janela do Cronograma de Gantt BIM 4D destacando a atividade vinculada, exibindo seu código EAP/WBS e percentual físico de avanço.
- `📊 Registrar Avanço de Campo...`: abre a janela de apontamento de medição para informar o avanço físico real (0% a 100%) e registrar anotações no diário de obra, salvando o progresso e o log de auditoria instantaneamente.
- `📊 Alterar Vínculo Gantt...`: permite reatribuir a tarefa a uma outra atividade de cronograma.
- `📊 Desvincular do Gantt`: desfaz a associação entre a tarefa e a atividade sem apagar os dados do cronograma ou o histórico de medições.

## 9. Layout de tarefas

Em `Configurações -> Layout de Tarefas -> Interface do Usuário -> Eisenhower`, você pode escolher:

- `Layout em Abas`: exibe um quadrante por vez através de abas principais, com subdivisões para seções de tarefas.
  - `Vertical`: organiza as seções (`Pendentes`, `Em Progresso` e `Concluídas`) empilhadas verticalmente de cima para baixo.
  - `Horizontal`: organiza as seções (`Pendentes`, `Em Progresso` e `Concluídas`) dispostas lado a lado em colunas horizontais.
- `Layout em Quadrantes`: mostra os quatro quadrantes ao mesmo tempo na janela principal.
- `Layout Matriz 2x2`: mostra os quatro quadrantes organizados em duas linhas e duas colunas, com abas internas para `Pendente`, `Em Progresso` e `Concluídas`.

### 9.1. Como usar o Layout em Abas (Vertical e Horizontal)

O `Layout em Abas` permite focar em um quadrante específico por vez:

1. **Layout em Abas -> Vertical**:
   - As seções `Pendentes`, `Em Progresso` e `Concluídas` ficam empilhadas uma sobre a outra.
   - Cada seção possui uma caixa de seleção no título. Desmarcar a caixa de seleção recolhe verticalmente a lista de tarefas, deixando visível apenas a barra do cabeçalho. Marcar novamente a caixa de seleção expande a lista de volta.

2. **Layout em Abas -> Horizontal**:
   - As seções `Pendentes`, `Em Progresso` e `Concluídas` ficam distribuídas horizontalmente como colunas lado a lado.
   - **Recolhimento com Borda e Rotação**: Ao desmarcar a caixa de seleção de qualquer uma das três seções, a respectiva coluna se recolhe horizontalmente para uma largura ultra-compacta (~30px).
   - O rótulo da seção é rotacionado em 90° no sentido horário, posicionando o texto na vertical de cima para baixo com perfeita legibilidade.
   - A moldura do quadro desenha um contorno nativo e elegante ao redor de toda a coluna que se conecta exatamente nas laterais do checkbox no topo, perfeitamente alinhada com as bordas e alturas dos quadrantes vizinhos.
   - **Expansão Rápida**: Para restaurar a coluna à sua largura normal, basta clicar em qualquer ponto da coluna recolhida ou diretamente na caixa de seleção.

### 9.2. Como usar o Layout Matriz 2x2

O `Layout Matriz 2x2` organiza os quatro quadrantes em grade (duas linhas e duas colunas):

1. `🔴 Importante e Urgente` (superior esquerdo);
2. `🟠 Importante, mas Não Urgente` (superior direito);
3. `🟡 Não Importante, mas Urgente` (inferior esquerdo);
4. `🟢 Não Importante e Não Urgente` (inferior direito).

Dentro de cada quadrante existem três abas:

- `Pendente`: mostra tarefas abertas ou ainda não iniciadas.
- `Em Progresso`: mostra tarefas em andamento (sincronizadas com o foco ativo e o Kanban do Pomodoro). A moldura do quadro exibe o nome descritivo completo da prioridade do quadrante (ex.: `🔴 Importante e Urgente`).
- `Concluídas`: mostra tarefas marcadas como finalizadas.

Para alternar o estado de uma tarefa:
- Marcar a caixa de seleção de uma tarefa em `Pendente` a move automaticamente para `Concluídas`.
- Desmarcar a caixa de seleção de uma tarefa em `Concluídas` a move automaticamente de volta para `Pendente`.
- Você também pode arrastar e soltar (Drag & Drop) uma ou várias tarefas diretamente entre as abas `Pendente`, `Em Progresso` e `Concluídas` de qualquer quadrante.

As cores auxiliam na identificação: as abas `Pendente` e `Em Progresso` utilizam as cores características de cada prioridade, enquanto as abas `Concluídas` utilizam azul.

Você também pode ordenar tarefas por:

- data;
- horário;
- ordem alfabética.

Cada critério pode ser usado em ordem crescente ou decrescente. Quando mais de um critério está ativo, o menu mostra a prioridade de ordenação.

## 10. Painéis visíveis

Em `Configurações -> Layout de Tarefas -> Mostrar Painel` ou `Mostrar Painéis`, você pode controlar:

- `Calendário`;
- `Minhas Tarefas`, quando Feynman está disponível;
- `Em Progresso`, quando Pomodoro está disponível.

Atalhos:

- `Ctrl+M`: mostra ou oculta o calendário.
- `Ctrl+Shift+T`: mostra ou oculta `Minhas Tarefas`, quando Feynman existe.
- `Ctrl+Shift+E`: mostra ou oculta `Em Progresso`, quando Pomodoro existe.

## 11. Calendário e lista global

O calendário mostra tarefas vinculadas a datas.

Filtros disponíveis:

- dia;
- semana;
- mês;
- ano.

As listas de filtros (acessíveis nos cabeçalhos das colunas das tabelas e listas de tarefas) foram otimizadas. Elas agora ignoram campos de texto longo (como descrições e conceitos) e unificam formatos de datas e horários. Isso mantém as opções de filtro precisas e limpas em todos os módulos (Eisenhower, Pomodoro e Feynman).

A `Lista Global de Tarefas` acompanha o filtro atual e permite revisar todas as atividades do período com visibilidade completa do seu ciclo de vida. Ela reporta tarefas em três estados fundamentais:
- **`[Concluída]`**: atividades finalizadas;
- **`[Pendente]`**: atividades aguardando execução;
- **`[Em Progresso]`**: atividades ativas em andamento (Doing).

A listagem mantém um visual limpo e padronizado, exibindo a data, o horário e a prioridade da tarefa, sem adicionar linhas intermediárias de contagem de pomodoros. Os balões informativos de datas no calendário (tooltips) também exibem todas as tarefas do dia categorizadas com seus respectivos estados, incluindo `[Em Progresso]`.

Todas as etiquetas de status do calendário contam com tradução dinâmica em tempo de execução: ao alternar entre Português e Inglês, os rótulos mudam instantaneamente entre `[Em Progresso]` e `[In Progress]`, `[Concluída]` e `[Completed]`, `[Pendente]` e `[Pending]`, sem necessidade de reiniciar o aplicativo.

Interação rápida:
- **Duplo clique**: ao clicar duas vezes sobre qualquer tarefa da lista que possua arquivo externo vinculado, o aplicativo abre o arquivo diretamente no programa padrão do sistema operacional.
- **Menu de contexto**: clicar com o botão direito sobre qualquer tarefa da lista global abre o menu de contexto completo, permitindo editar, concluir, remover, colorir ou acionar as opções de integração com o Cronograma de Gantt.

Tarefas criadas pelo Registro de Atividades também aparecem nessa lista quando possuem data vinculada. Se o modo `Cores -> Coloridas` da Agenda estiver ativo, a lista global preserva a cor aplicada à tarefa; se houver arquivo vinculado, o título também recebe destaque visual de vínculo.

Você também pode remover uma ou várias tarefas diretamente pela `Lista Global de Tarefas`. Ao usar o menu de contexto nessa lista, o aplicativo localiza a tarefa original no quadrante correspondente e remove o item real, mantendo calendário, armazenamento e módulos sincronizados consistentes.

No filtro mensal, o calendário pode alternar entre visão de meses e visão anual para facilitar a navegação.

## 12. Espaço reservado e marca d'água

Em `Configurações -> Espaço Reservado`, você pode ativar ou desativar recursos visuais por módulo.

As opções podem incluir:

- `Título`;
- `Marca d'Água`;
- `Configurações de Fonte`;
- `Colorir tarefas`.

Esses recursos ajudam a separar visualmente grupos, datas, horários e áreas do aplicativo. As opções exibidas dependem dos módulos carregados.

Para colorir tarefas:

1. Selecione uma ou várias tarefas no Eisenhower, no Pomodoro ou no Feynman.
2. Abra `Configurações -> Espaço Reservado -> Colorir tarefas`.
3. Escolha a cor desejada na janela de cores.
4. Confirme a escolha.

A cor é aplicada às tarefas selecionadas e permanece visível nas listas, no calendário e nos módulos sincronizados quando a tarefa é exibida novamente. Se a tarefa também tiver arquivo vinculado, o título continua recebendo o destaque próprio de arquivo vinculado, preservando a cor da fonte sempre que possível.

Para configurar a fonte das tarefas:

1. Abra `Configurações -> Espaço Reservado -> Configurações de Fonte`.
2. Escolha `Tarefas em Geral` para tarefas sem arquivo externo vinculado.
3. Escolha `Arquivo Vinculado` para tarefas com arquivo externo vinculado.
4. Na janela aberta, selecione tipo da fonte, estilo, tamanho e as opções negrito, itálico, sublinhado ou tachado.
5. Use `Restaurar` para retornar ao padrão daquela categoria, `OK` para salvar ou `Cancelar` para descartar.

As configurações são independentes. Se você alterar `Tarefas em Geral`, somente tarefas sem arquivo vinculado mudam. Se você alterar `Arquivo Vinculado`, somente tarefas com arquivo externo vinculado mudam. A atualização vale para as listas do Eisenhower, para a lista global do calendário e para as tarefas mostradas no Pomodoro e no Feynman quando esses plugins estiverem disponíveis.

Além disso, você pode alterar a fonte de uma tarefa específica diretamente pelo menu de contexto (clique com o botão direito). Essa configuração é sincronizada corretamente com os demais módulos sem afetar a aparência das demais tarefas na tela.

## 13. Sincronização entre módulos

Em `Configurações -> Sincronizar`, você controla a atualização entre módulos.

O compartilhamento principal ocorre entre:

- Eisenhower;
- Feynman;
- Pomodoro.

Fluxos comuns:

- planejar uma tarefa no Eisenhower e executar no Pomodoro;
- transformar uma tarefa de estudo em conceito no Feynman;
- retornar um item de foco para o Eisenhower;
- revisar conceitos e enviar tarefas para execução.
- registrar uma atividade avaliativa na Agenda e transformá-la em tarefa para Eisenhower, Pomodoro e Feynman, quando esses métodos estiverem sincronizados.

Mapa Mental e Audiolivro possuem dados próprios, mas se integram ao host por aba, menu, idioma e contador.

## 14. Ferramentas de produtividade

O menu `Ferramentas` concentra recursos operacionais que usam a lista unificada de tarefas e, quando disponíveis, dados dos plugins. Antes de abrir essas ferramentas, o aplicativo salva o estado atual para trabalhar com os registros mais recentes.

### 14.1 Busca global

Use `Ferramentas -> Busca global` ou `Ctrl+Shift+G`.

A busca global procura em:

- tarefas do Eisenhower;
- conceitos do Feynman;
- itens do Pomodoro;
- nós e registros do Mapa Mental;
- texto aberto no Audiolivro, quando o plugin fornece conteúdo ao host.

A pesquisa considera título, descrição, módulo, data, horário, prioridade, nível de domínio, arquivo vinculado e campos de integração. Use esse recurso quando você lembra de uma palavra, arquivo, tema ou data, mas não sabe em qual aba o item está.

### 14.2 Hoje / Próximos 7 dias

Use `Ferramentas -> Hoje / Próximos 7 dias` ou `Ctrl+Shift+H`.

Esse painel mostra itens com data válida entre a data atual e os próximos sete dias. Ele é útil como lista curta de execução, revisão de prazos e checagem de compromissos próximos. Itens sem data não aparecem nesse painel; para encontrá-los, use a busca global ou abra o módulo correspondente.

### 14.3 Recorrência por item

Clique com o botão direito em uma tarefa e escolha `Recorrência`.

Você pode definir:

- `Sem recorrência`;
- `Diária`;
- `Semanal`;
- `Mensal`;
- data final opcional.

A recorrência individual não cria cópias imediatamente. Ela registra a regra no item para que `Gerar próximas recorrências` possa criar as próximas ocorrências em lote.

### 14.4 Gerar próximas recorrências

Use `Ferramentas -> Gerar próximas recorrências`.

O diálogo permite escolher:

- métodos de origem e destino: Eisenhower, Pomodoro, Feynman e Mapa Mental;
- uso de todos os itens filtrados ou somente dos itens selecionados;
- pesquisa de itens específicos, com ações para selecionar os visíveis ou limpar a seleção;
- alcance em dias;
- máximo de ocorrências por item;
- inclusão ou não de itens concluídos;
- prevenção de duplicadas existentes;
- frequência padrão para itens sem recorrência definida;
- data e hora inicial/final do período de aplicação;
- data e hora de referência para itens sem data ou sem horário, quando `Usar hora inicial quando o item não tiver horário` estiver ativo.

A área de prévia mostra quais itens serão considerados, onde as novas ocorrências serão criadas, qual frequência será aplicada e quais próximas datas serão usadas.

Botões importantes:

- `OK`: salva as definições do diálogo sem gerar ocorrências.
- `Aplicar`: salva as definições e cria as ocorrências.
- `Restaurar padrões`: volta as opções do diálogo aos valores iniciais.
- `Restaurar original`: quando disponível, desfaz a última geração de recorrências e recupera o estado anterior.

Quando um item está compartilhado entre módulos, a nova ocorrência é criada nos módulos marcados em que o item já existe. O aplicativo tenta evitar cópias repetidas quando `Evitar duplicadas existentes` está ativo.

### 14.5 Lembretes locais

Use `Ferramentas -> Ativar lembretes locais`.

Os lembretes são temporizadores locais do aplicativo. Eles funcionam enquanto o aplicativo permanece aberto; não substituem um agendador do sistema operacional.

No diálogo, você pode:

- selecionar os métodos de origem;
- usar todos os itens filtrados ou somente itens selecionados;
- pesquisar e selecionar itens específicos;
- incluir ou ignorar itens concluídos;
- definir `Máximo`, ou seja, quantas repetições serão disparadas para cada horário agendado;
- usar a data/hora do item ou uma data/hora escolhida manualmente;
- definir período rápido em dias, semanas ou meses;
- usar horário padrão quando uma tarefa tem data, mas não tem hora;
- exibir aviso dentro do app;
- exibir aviso pela bandeja do sistema quando ela estiver disponível.

Quando vários itens caem na mesma data/hora, o aplicativo agrupa tudo em um único aviso com a lista de itens. Se `Máximo` for maior que 1, esse aviso é repetido em sequência para o mesmo horário.

Botões importantes:

- `Testar`: dispara um lembrete de teste.
- `Restaurar padrões`: volta as opções do diálogo aos valores iniciais.
- `Remover ativos`: cancela lembretes já agendados na sessão atual.
- `OK`: salva preferências sem agendar.
- `Aplicar`: salva preferências e agenda os lembretes possíveis.

Se a bandeja do sistema não estiver disponível, use `Alertar no app`.

### 14.6 Exportar calendário .ics

Use `Ferramentas -> Exportar calendário .ics`.

O aplicativo gera um arquivo iCalendar com tarefas que possuem data válida. Quando a tarefa também possui horário, o evento recebe horário de início e duração padrão de 30 minutos. Tarefas sem horário são exportadas como eventos de dia inteiro. Tarefas sem data ficam fora do arquivo.

Use esse recurso para importar prazos em aplicativos compatíveis com `.ics`, como calendários pessoais, clientes de e-mail e ferramentas de agenda.

### 14.7 Histórico de produtividade

Use `Ferramentas -> Histórico de produtividade`.

O histórico resume:

- tarefas concluídas por semana;
- quantidade de pomodoros registrada nos dados compartilhados;
- conceitos Feynman com revisão preenchida;
- quantidade de nós no Mapa Mental;
- estimativa de minutos de leitura do texto aberto no Audiolivro.

Esse relatório é informativo. Ele ajuda a revisar progresso, mas depende dos campos salvos pelos módulos disponíveis.

### 14.8 Fluxo integrado de estudo

Use `Ferramentas -> Fluxo integrado de estudo`.

Quando existe texto aberto no Audiolivro, esse texto é usado como base. Se não houver texto disponível, o aplicativo pede um trecho manualmente.

Ao confirmar, o fluxo cria:

- um conceito no Feynman com o trecho como explicação;
- uma tarefa no Eisenhower para revisar o tema dois dias depois;
- um nó no Mapa Mental com o título e notas do trecho.

Use esse fluxo para transformar leitura em revisão prática sem recriar o mesmo conteúdo em três módulos.

### 14.9 Templates

Use `Ferramentas -> Templates`.

Os modelos disponíveis criam tarefas iniciais para:

- estudo;
- trabalho;
- revisão;
- leitura;
- planejamento semanal.

Depois de inserir um template, edite datas, prioridades, domínio e descrições conforme sua rotina. Os templates são pontos de partida, não planos rígidos.

### 14.10 Revisões espaçadas

Use `Ferramentas -> Criar revisões espaçadas`.

A ferramenta usa a aba ativa como fonte. Ela lê os itens encontrados, calcula intervalos com base no nível de domínio e prepara revisões futuras.

Intervalos padrão:

- iniciante: revisão em 1 dia;
- intermediário: revisão em 3 dias;
- avançado: revisão em 7 dias.

No diálogo, você pode:

- ajustar os intervalos por domínio;
- definir domínio padrão para itens sem domínio;
- manter a prioridade original ou forçar uma prioridade de revisão;
- definir prioridade padrão para itens sem prioridade;
- criar ou não datas de revisão para calendário e painel;
- incluir ou ignorar itens concluídos.

A janela de revisão espaçada permite revisar a lista antes de transferir:

- edite uma revisão com duplo clique ou `Editar na janela...`;
- remova itens que não devem entrar na rodada;
- escolha destino em Eisenhower, Pomodoro, Feynman e Mapa Mental;
- use `Transferir para abas selecionadas` para criar os registros.

Quando disponível, `Desfazer` recupera o estado anterior da última transferência.

### 14.11 Gerenciador de plugins

Use `Ferramentas -> Gerenciador de plugins`.

Esse painel mostra:

- nome do plugin;
- status de carregamento;
- versão declarada;
- caminho encontrado;
- erro de carregamento, quando houver.

Use `Abrir pasta` para conferir a localização do plugin selecionado. A janela acompanha a troca de idioma durante a execução e pode ficar atrás da janela principal quando você voltar a trabalhar no aplicativo.

O Gerenciador de plugins serve para responder perguntas como:

- quais plugins foram encontrados;
- quais plugins carregaram corretamente;
- qual versão de cada plugin está disponível;
- onde o plugin foi localizado;
- qual erro ocorreu quando algum plugin não carregou.

Para abrir ou acompanhar arquivos de log, use `Ferramentas -> Gerenciador de logs`.

### 14.12 Gerenciador de logs

Use `Ferramentas -> Gerenciador de logs`.

Esse painel lista os arquivos `.log` encontrados na pasta persistente de logs, permite abrir o log selecionado ou sua pasta, e mantém um monitor em tempo real. Novos registros aparecem em ordem temporal, do mais antigo para o mais recente, usando o timestamp de cada linha.

Na prática, use essa janela assim:

1. Abra `Ferramentas -> Gerenciador de logs`.
2. Selecione um arquivo de log na lista.
3. Use `Abrir log` para abrir o arquivo selecionado no visualizador padrão do sistema.
4. Use `Abrir pasta` para abrir o diretório onde os logs estão salvos.
5. Deixe a janela aberta enquanto usa o aplicativo para acompanhar novos registros no monitor.

O monitor atualiza a leitura durante a execução. Isso ajuda a confirmar quando uma ação foi concluída, quando um plugin foi carregado ou quando uma falha foi registrada.

Os arquivos principais seguem o produto em uso:

- Eisenhower Organizer: logs iniciados com `file_eisenhower`.
- Lúmen: logs iniciados com `file_lumen`.

O Gerenciador de logs também acompanha a troca de idioma em tempo de execução e pode ficar atrás da janela principal. Assim, você pode manter o diagnóstico aberto sem bloquear o uso normal do aplicativo.

## 15. Módulo Feynman

O Feynman é voltado para estudo e revisão.

Ele permite:

- criar conceitos;
- escrever uma explicação simples;
- registrar lacunas de entendimento;
- criar revisão final;
- definir prioridade;
- definir nível de domínio;
- vincular arquivo;
- selecionar vários conceitos;
- editar tarefas do painel `Minhas Tarefas` com confirmação de sucesso;
- compartilhar itens com Eisenhower e Pomodoro.

O painel `Minhas Tarefas` pode ser aberto ou fechado pelo menu de layout quando o plugin está disponível.

### 15.1. Excluir tarefas no Feynman

O menu de contexto da lista `Minhas Tarefas` também permite remover tarefas diretamente.

Para excluir uma tarefa:

1. Clique com o botão direito diretamente sobre a tarefa.
2. Escolha `Excluir Tarefa`.
3. Confirme a remoção na janela exibida.

Para excluir várias tarefas:

1. Selecione as tarefas desejadas com `Ctrl`, `Shift` ou seleção normal da lista.
2. Clique com o botão direito sobre qualquer uma das tarefas selecionadas.
3. Escolha `Excluir Tarefas Selecionadas`.
4. Confira a lista de itens que será removida e confirme.

Se você clicar com o botão direito em uma tarefa que ainda não estava selecionada, o Feynman seleciona aquela tarefa antes de abrir o menu. Se você clicar com o botão direito em uma tarefa que já faz parte de uma seleção múltipla, o grupo selecionado é preservado. Assim, a remoção individual e a remoção em lote ficam mais previsíveis.

As mensagens de confirmação, sucesso e erro usam a palavra `tarefas` e acompanham o idioma ativo do aplicativo.

### 15.2. Editar tarefas no Feynman

Para editar uma tarefa exibida no painel `Minhas Tarefas`:

1. Clique com o botão direito na tarefa.
2. Escolha `Editar Tarefa`.
3. Altere as informações necessárias: texto, descrição, prioridade, nível de domínio, data, horário, arquivo vinculado, ciclos pomodoro concluídos (🍅), meta de ciclos (🎯) e duração estimada em dias.
4. Clique em `OK`.
5. Aguarde a mensagem de sucesso e clique em `OK` para fechar o aviso.

A confirmação indica que as alterações foram aplicadas e acompanha o idioma ativo do aplicativo. As métricas de prazo e ciclos são propagadas automaticamente para as tarefas sincronizadas no Eisenhower e Pomodoro.

### 15.3. Integração com o Cronograma Gantt no Feynman

O menu de contexto da lista `Minhas Tarefas` e dos conceitos também disponibiliza atalhos de integração com o cronograma:
- `📊 Criar Atividade no Gantt`: cria uma nova atividade no cronograma vinculada ao conceito ou tarefa de estudo.
- `📊 Vincular ao Cronograma Gantt...`: associa o item de estudo a uma atividade existente na EAP/WBS.
- `📊 Abrir no Cronograma Gantt`: abre a tela do cronograma na linha correspondente da atividade.
- `📊 Registrar Avanço de Campo...`: permite lançar medições de progresso e notas no diário de obra.
- `📊 Alterar Vínculo Gantt...` e `📊 Desvincular do Gantt`.

## 16. Módulo Pomodoro

O Pomodoro é voltado para execução com foco. Ele combina temporizador, lista de tarefas e controles de acompanhamento para ajudar você a trabalhar em ciclos de concentração e pausa.

Ele permite:

- organizar tarefas em progresso;
- executar ciclos de foco;
- usar pausas;
- acompanhar tarefas durante a sessão;
- configurar alarme;
- escolher entre uma interface simples e uma interface robusta;
- alternar o layout a qualquer momento sem alterar tarefas já salvas;
- editar tarefas com confirmação de sucesso, ciclos de foco e metas;
- detectar discrepâncias entre ciclos de tarefas e configurações do timer;
- sincronizar durações e apontar avanço no cronograma Gantt;
- compartilhar itens com Eisenhower e Feynman.

O painel `Em Progresso` pode ser aberto ou fechado pelo menu de layout quando o plugin está disponível.

### 16.1. Como abrir o Pomodoro

Você pode abrir o módulo em:

- `Configurações -> Abrir -> Pomodoro Ctrl+Shift+P`;
- atalho `Ctrl+Shift+P`;
- aba principal `Pomodoro`, quando ela estiver visível.

### 16.2. Escolher o layout da interface

O Pomodoro possui dois modos de visualização:

- `Simples`: mantém a organização mais compacta, com o quadro do temporizador (`⏱️ Temporizador`), configurações e lista de tarefas em uma disposição direta.
- `Robusta`: usa uma organização mais completa, com o temporizador (`⏱️ Temporizador`), quadro de tarefas, abas de status e painel lateral `Em Progresso`.

Para alternar:

1. Abra `Configurações -> Layout de Tarefas -> Interface do Usuário -> Pomodoro`.
2. Escolha `Simples` ou `Robusta`.
3. O aplicativo aplica o layout escolhido, abre o Pomodoro e mostra uma mensagem de aviso confirmando qual layout foi aplicado.
4. O layout pode ser trocado sempre que necessário e não altera suas tarefas, apenas a forma de exibição.

A escolha fica salva e é reaplicada nos próximos usos do aplicativo.

### 16.3. Layout Robusta

No layout `Robusta`, a moldura do temporizador é identificada de forma padronizada pelo rótulo `⏱️ Temporizador` (com tradução dinâmica para `⏱️ Timer`), e o quadro `Quadro de Tarefas` foi reorganizado para deixar as ações mais fáceis de encontrar.

Abaixo do campo de texto da tarefa, a primeira linha reúne:

- rótulo `Prioridade:`;
- lista de prioridade;
- rótulo `Nível de Domínio:`;
- lista de nível de domínio;
- checkbox de `Fluxo Tarefas Concluídas`;
- rótulo `Fluxo Tarefas Concluídas`.

A segunda linha reúne os botões:

- `Adicionar`;
- `Mover Tarefa(s)`;
- `Remover Tarefa(s)`;
- `Resetar Contagem`.

Use esse layout quando quiser acompanhar mais informações ao mesmo tempo, principalmente em sessões de estudo ou trabalho com várias tarefas.

As abas de status ajudam a separar o andamento das tarefas, enquanto o painel lateral mantém o acompanhamento do que está em execução.

### 16.4. Layout Simples

No layout `Simples`, o Pomodoro mantém uma visualização mais direta com o quadro `⏱️ Temporizador` e lista direta. Ele é indicado quando você quer menos elementos na tela ou quando está usando uma janela menor.

Você pode voltar ao layout `Robusta` a qualquer momento pelo mesmo caminho no menu de configurações.

### 16.5. Painel Em Progresso

No layout `Robusta`, o painel `Em Progresso` aparece ao lado da área principal e mostra as tarefas que estão em execução. Você pode recolher ou expandir esse painel para ganhar espaço durante o foco. O aplicativo lembra a última escolha de visibilidade.

Você também pode mostrar ou ocultar o painel em `Configurações -> Layout de Tarefas -> Mostrar Painel/Painéis -> Em Progresso` ou pelo atalho `Ctrl+Shift+E` quando o plugin estiver disponível.

Configuração de alarme:

- acesse `Configurações -> Configurações Específicas -> Pomodoro -> Alarme`.
- escolha o som do aviso;
- teste a prévia antes de salvar;
- ajuste o intervalo de repetição do alarme;
- salve para aplicar a configuração.

### 16.6. Editar tarefas no Pomodoro

Para editar uma tarefa no Pomodoro:

1. Clique com o botão direito na tarefa.
2. Escolha `Editar Tarefa`.
3. Ajuste texto, descrição, prioridade, nível de domínio, data, horário, arquivo vinculado, ciclos pomodoro concluídos (🍅), meta de ciclos (🎯) e duração estimada em dias.
4. Clique em `OK`.
5. A confirmação responde e fecha a janela de imediato sem travamento, enquanto os dados são salvos e sincronizados com os demais módulos em segundo plano.

O aviso confirma que a tarefa foi atualizada e acompanha o idioma ativo do aplicativo em tempo real.

### 16.7. Detecção e diálogo de discrepância de ciclo (CycleMismatchDialog)

Ao clicar no botão `Iniciar` para começar um bloco de foco, o temporizador analisa todas as tarefas presentes na coluna ou aba `Em Progresso` (Doing):

- **Verificação Automática**: se alguma tarefa em execução possuir uma meta de ciclos (🎯) diferente da configuração ativa de ciclos do temporizador (⚙️), o sistema abre automaticamente a janela preventiva `CycleMismatchDialog`.
- **Tabela Comparativa de Divergências**: a janela apresenta uma tabela discriminando o título de cada tarefa em andamento, sua meta de ciclos cadastrada e a meta de ciclos selecionada no temporizador.
- **Opções de Resolução**:
  1. `Resetar Tudo`: redefine a contagem de ciclos de todas as tarefas em andamento para o padrão, sincronizando imediatamente a atualização com a Matriz Eisenhower e o armazenamento persistente por meio da ponte de integração do host (`reset_in_progress_task_cycles`), iniciando o temporizador logo em seguida.
  2. `Iniciar Mesmo Assim`: mantém as metas individuais de cada tarefa sem alterações e inicia imediatamente a contagem regressiva do timer.
  3. `Cancelar`: interrompe o início do temporizador sem alterar tarefas ou cronômetros, permitindo ao usuário revisar as definições antes de focar.

### 16.8. Ações do Cronograma Gantt no Pomodoro

As listas de tarefas do Pomodoro (`gtar_14`) também possuem integração com o Cronograma de Gantt BIM 4D no menu de contexto (clique com o botão direito):
- `📊 Criar Atividade no Gantt`;
- `📊 Vincular ao Cronograma Gantt...`;
- `📊 Abrir no Cronograma Gantt`;
- `📊 Registrar Avanço de Campo...`;
- `📊 Alterar Vínculo Gantt...`;
- `📊 Desvincular do Gantt`.

## 17. Módulo Mapa Mental

O Mapa Mental aparece quando o plugin correspondente está carregado.

Ele permite:

- criar conceitos em forma de nós visuais;
- conectar conceitos;
- mudar cores;
- editar título, notas e arquivo vinculado;
- abrir arquivo vinculado;
- remover conceitos;
- limpar o mapa;
- salvar e carregar mapas em JSON;
- exportar a visualização para PNG;
- importar documentos para gerar estrutura inicial;
- reorganizar visualmente o mapa por layout.

Layouts disponíveis em `Configurações -> Layout de Tarefas -> Interface do Usuário -> Mapa Mental`:

- vertical;
- horizontal;
- radial;
- árvore.

A importação assistida aceita documentos como PDF, EPUB, DOC, DOCX, TXT, MD, CSV, XLSX, JSON, SQLite e XML.

Quando o módulo `Modelo` está disponível, acesse `Configurações -> Configurações Específicas -> Mapa Mental` para definir o método de síntese da próxima importação de documento:

- `Resumo método heurístico`: análise local rápida e determinística, sem executar modelos de linguagem. Detecta títulos, capítulos, conceitos e relações e produz resumos extrativos. É indicada para documentos bem estruturados, computadores com menos memória ou quando a velocidade é a prioridade.
- `Resumo com modelo qwen2.5-3b [Instalado]`: executa localmente o modelo oficial Qwen2.5-3B-Instruct (GGUF via llama.cpp) para criar sínteses contextuais profundas e hierarquias conceituais semânticas.
- `Resumo com modelo qwen2.5-3b [Não instalado]`: indica que o arquivo de modelo ainda não foi baixado para o computador. Para baixá-lo sob demanda de forma assistida, clique em `Como baixar ou gerenciar modelo Qwen (GGUF)...`.
- `Como baixar ou gerenciar modelo Qwen (GGUF)...`: abre a janela interativa do Gerenciador do Modelo Local Qwen 2.5 (GGUF), permitindo iniciar o download oficial (~2,17 GB), pausar, retomar via HTTP Range, excluir para liberar espaço em disco ou abrir a pasta no Explorer.
- `Modelos Ollama`: submenu que detecta e lista automaticamente quaisquer modelos de linguagem instalados localmente no Ollama (Llama 3.2, Qwen 2.5, Gemma 2, Mistral, etc.).
- `Como instalar modelos no Ollama...`: abre a janela com o guia passo a passo, modelos recomendados, botões para copiar comandos do terminal e teste de conexão com o servidor local (`localhost:11434`).

Depois de escolher o método, abra o Mapa Mental, clique em `Importar documento`, selecione o arquivo e acompanhe as etapas de extração, análise e criação dos nós na janela de progresso. Ao terminar, revise o nó central, títulos, resumos e conexões; edite ou remova itens pouco relevantes e salve o mapa em JSON ou exporte-o em PNG. Os modelos funcionam de forma estritamente local e, caso o modelo selecionado encontre arquivos ilegíveis, erro de sintaxe ou restrição de memória, o aplicativo registra a ocorrência e aplica automaticamente o método heurístico transparente para concluir a importação com sucesso.

Se o módulo modelo não estiver disponível na instalação, o menu `Mapa Mental` oculta as opções de IA e a importação utiliza o método heurístico de forma padrão.

O Mapa Mental também possui backup automático. Se o aplicativo for fechado, o último estado pode ser restaurado no próximo carregamento do plugin.

## 18. Módulo Audiolivro

O Audiolivro aparece quando o plugin correspondente está carregado.

Ele permite:

- abrir e editar texto;
- salvar texto;
- abrir PDF;
- arrastar arquivos para a área de texto ou PDF;
- ler texto em voz;
- ler PDF em voz;
- pausar, continuar e parar a leitura;
- ajustar velocidade;
- ajustar volume;
- escolher voz;
- configurar fonte;
- buscar termos no texto;
- buscar termos no PDF;
- selecionar e copiar texto do PDF;
- usar régua de foco;
- acompanhar destaque por palavra, frase ou parágrafo durante a leitura.

Configurações do Audiolivro:

- `Configurações -> Configurações de Fonte -> Audiolivro -> Texto Audiolivro`;
- `Configurações -> Configurações Específicas -> Audiolivro -> Vozes`.

Observação: a leitura em voz pode depender da voz selecionada, do serviço de síntese disponível e da conexão do ambiente.

## 19. Módulo Agenda e Registro de Atividades

O Registro de Atividades aparece quando o plugin correspondente está carregado. Ele não substitui o Eisenhower; ele funciona como uma janela de cadastro para transformar atividades acadêmicas em tarefas organizadas.

Você pode abrir a Agenda por:

- `Ferramentas -> Registro de Atividades`;
- `Configurações -> Plugins -> Registro de Atividades`.

Quando o plugin não está disponível, esses caminhos não aparecem no aplicativo e o manual interno também não mostra as instruções da Agenda.

Dentro da janela da Agenda existe um menu de configurações próprio. Ele permite alternar entre o exemplo `Engenharia Civil`, criar novas agendas e gerenciar agendas personalizadas sem alterar a disponibilidade do plugin no aplicativo principal.

### 19.1. Janela Cadastro de Atividades

A janela de cadastro reúne as informações da atividade em dois lados.

No lado esquerdo ficam as listas suspensas:

- `Curso:`;
- `Ementa:`;
- `Semestre:`;
- `Disciplina:`;
- `Turma da Disciplina:`;
- `Tipo de Atividade Avaliativa:`;
- `Sequência da Atividade:`;
- `Prioridade:`;
- `Nível de Domínio:`.

No lado direito ficam:

- `Defina a Data da Atividade:`;
- calendário;
- `Vincular data`;
- `Vincular horário`;
- seleção de horário;
- `Arquivo vinculado (arraste ou escolha):`;
- botões `Escolher...`, `Abrir` e `Remover` para o arquivo vinculado;
- botão `Registrar Definições`;
- botão `Limpar`;
- botão `Fechar`.

A janela foi mantida compacta e não usa botão de maximizar. Quando a janela principal do aplicativo é fechada, a janela da Agenda também é encerrada. A hierarquia entre a janela principal e a Agenda segue o comportamento das demais janelas do produto: a janela ativa vem para frente e a outra fica atrás.

Arquivo vinculado:

- use `Escolher...` para selecionar qualquer arquivo externo que ajude na atividade;
- arraste um arquivo do Windows diretamente para o campo de arquivo vinculado;
- use `Abrir` para abrir o arquivo selecionado no aplicativo padrão do sistema;
- use `Remover` para retirar apenas o vínculo, sem apagar o arquivo do computador.

O Registro de Atividades guarda o caminho do arquivo na tarefa criada. O arquivo continua no local original; se ele for movido, renomeado ou apagado, será necessário editar a tarefa e vincular o arquivo novamente.

### 19.2. Exemplo Engenharia Civil e agendas personalizadas

O menu `Configurações -> Exemplo de Agenda -> Engenharia Civil` abre o modelo acadêmico padrão da Agenda, com as listas de curso, ementa, semestre, disciplina, turma, tipo de atividade e sequência.

Para criar uma agenda própria:

1. Abra `Configurações -> Crie sua agenda -> Novo título para sua agenda`.
2. Digite o nome desejado, por exemplo `Agenda Universal`.
3. Pressione Enter.
4. A nova agenda passa a aparecer no menu `Configurações`.

Para abrir uma agenda personalizada, use `Configurações -> Nome da Agenda -> Abrir agenda`. Apenas passar o mouse sobre o menu não abre a agenda.

Cada agenda personalizada permite criar listas suspensas próprias. Para isso:

1. Marque o controle circular da próxima linha disponível.
2. Digite o título da nova lista no campo `Insira um título para a nova lista`.
3. Pressione Enter para transformar o campo em rótulo.
4. Use a lista suspensa criada para inserir ou selecionar os valores daquela categoria.

A próxima lista só aparece depois que a anterior é criada. Isso evita que a janela fique cheia de listas vazias.

### 19.2.1. Listas aninhadas em agendas personalizadas

As agendas personalizadas também podem funcionar como uma árvore de listas. Esse recurso é útil quando uma escolha deve filtrar a próxima, por exemplo:

- `Curso` -> `Semestre`;
- `Semestre` -> `Disciplina`;
- `Área` -> `Projeto`;
- `Projeto` -> `Etapa`;
- `Cliente` -> `Contrato`.

Para criar uma lista aninhada:

1. Crie a primeira lista e insira os itens principais.
2. Crie a próxima lista.
3. Quando a Agenda perguntar se deseja aninhar os itens desta lista aos itens da lista anterior, escolha `Sim`.
4. Digite um item na nova lista.
5. Ao confirmar o item, escolha a qual item da lista anterior ele pertence.
6. Repita o processo para cadastrar os demais itens.

Depois disso, a lista filha passa a acompanhar a seleção da lista anterior. Se você escolher um item pai, a lista seguinte mostra somente os itens ligados a ele. Se mudar o item pai, a lista filha é atualizada para mostrar o conjunto correspondente.

Exemplo simples:

1. Na lista `Curso`, cadastre `Engenharia Civil` e `Arquitetura`.
2. Crie a lista `Semestre` como lista aninhada.
3. Cadastre `1º Semestre` ligado a `Engenharia Civil`.
4. Cadastre `Projeto Integrado` ligado a `Arquitetura`.
5. Ao selecionar `Engenharia Civil`, a lista `Semestre` mostra os itens ligados a esse curso.

Você pode criar mais de uma camada. Uma lista filha também pode ter outra lista dependente logo depois dela, formando uma sequência organizada de escolhas.

Para gerenciar uma agenda personalizada:

- `Abrir agenda`: ativa a agenda selecionada.
- `Editar agenda`: renomeia a agenda.
- `Remover agenda`: remove a agenda personalizada.
- `Editar título`: altera o rótulo de uma lista da agenda ativa.
- `Remover título`: remove apenas o rótulo/lista escolhida, mantendo as demais listas.

A última agenda aberta é lembrada. Se você trabalhou por último com `Engenharia Civil`, a Agenda volta nesse modelo ao reabrir. Se a última agenda usada foi uma agenda personalizada, ela é carregada na próxima abertura da janela.

### 19.3. Como cadastrar uma atividade

Para registrar uma atividade avaliativa:

1. Escolha o curso.
2. Escolha ou cadastre a ementa.
3. Escolha o semestre.
4. Escolha a disciplina.
5. Escolha a turma.
6. Escolha o tipo de atividade avaliativa.
7. Escolha a sequência da atividade.
8. Defina a prioridade.
9. Defina o nível de domínio.
10. Marque `Vincular data` se a atividade deve aparecer no calendário.
11. Marque `Vincular horário` se a tarefa deve ter horário.
12. Clique em `Registrar Definições`.

Opcionalmente, antes de registrar, vincule um arquivo externo pelo botão `Escolher...` ou arraste o arquivo para o campo `Arquivo vinculado`. O vínculo acompanha a tarefa criada e pode ser aberto depois pelas telas que exibem a tarefa.

A prioridade padrão é `Importante e Urgente`. O checkbox `Vincular horário` abre habilitado por padrão. O botão `Limpar` limpa as escolhas do cadastro e restaura as opções padronizadas de `Prioridade` e `Nível de Domínio`, sem apagar as listas disponíveis.

Nas agendas personalizadas, preencha as listas criadas por você e mantenha os campos padrão de prioridade, domínio, data, horário e botões de ação. Os valores digitados nas listas personalizadas ficam salvos na agenda correspondente.

Quando a agenda personalizada possui listas aninhadas, preencha as listas na ordem em que aparecem. Primeiro escolha o item da lista anterior; depois escolha ou cadastre o item da lista seguinte. Se um item digitado ainda não estiver ligado a nenhum item pai, a Agenda solicitará essa ligação antes de concluir o registro.

### 19.4. Como a Agenda cria tarefas

Ao registrar uma atividade, a Agenda cria uma tarefa no armazenamento unificado do produto.

A descrição da tarefa recebe somente os dados acadêmicos do cadastro:

- curso;
- ementa;
- semestre;
- disciplina;
- turma;
- tipo de atividade avaliativa;
- sequência da atividade.

Prioridade, nível de domínio, data e horário não são repetidos dentro da descrição, porque já fazem parte dos campos próprios da tarefa e da apresentação visual na interface.

Quando a sincronização está ativa, a tarefa pode aparecer em:

- Eisenhower;
- Pomodoro;
- Feynman.

No Eisenhower, a prioridade define o quadrante. No Pomodoro e no Feynman, a tarefa é exibida conforme a sincronização e as regras de cada plugin.

### 19.5. Cores da Agenda

Em `Configurações -> Registro de Atividades -> Cores` (na barra de menus principal) e em `Configurações -> Cores` (na barra de menus da janela `Cadastro de Atividades`), você pode escolher o comportamento de cor para cada módulo individualmente ou para todos simultaneamente.

A estrutura é organizada em submenus:

- `Preto`:
  - `Eisenhower`: define fonte preta (no tema claro) ou branca (no tema escuro) apenas para as tarefas da Matriz Eisenhower e Calendário.
  - `Pomodoro`: define fonte preta (no tema claro) ou branca (no tema escuro) apenas para as tarefas no quadro do Pomodoro.
  - `Feynman`: define fonte preta (no tema claro) ou branca (no tema escuro) apenas para os conceitos na lista do método Feynman.
  - `Geral`: aplica o modo Preto simultaneamente para Eisenhower, Pomodoro e Feynman.
- `Coloridas`:
  - `Eisenhower`: aplica a cor de destaque por disciplina/categoria apenas para as tarefas da Matriz Eisenhower e Calendário.
  - `Pomodoro`: aplica a cor de destaque por disciplina/categoria apenas para as tarefas no quadro do Pomodoro.
  - `Feynman`: aplica a cor de destaque por disciplina/categoria apenas para os conceitos na lista do método Feynman.
  - `Geral`: aplica o modo Coloridas simultaneamente para Eisenhower, Pomodoro e Feynman.

Quando o modo `Coloridas` está ativo em um módulo, as tarefas mantêm sua cor própria no ambiente correspondente. Se a tarefa tiver arquivo vinculado, o título também recebe destaque visual de vínculo sem descartar a cor da fonte sempre que possível.

Em agendas personalizadas, quando `Coloridas` está ativo, a Agenda escolhe cores de destaque de forma alternada e evita repetir imediatamente a última cor usada em uma tarefa recém-criada.

A alteração dos modos de cor é aplicada em tempo de execução e atualiza instantaneamente as tarefas exibidas na tela, sem necessidade de reiniciar o aplicativo.

Tarefas antigas criadas em versões anteriores também passam a seguir essas regras de cor configuradas por módulo.

### 19.6. Excluir itens das listas

Use `Configurações -> Plugins -> Registro de Atividades -> Excluir item` para remover entradas cadastradas nas listas suspensas.

O menu `Excluir item` acompanha a agenda ativa em tempo real. Se `Engenharia Civil` estiver aberta, ele mostra as listas do modelo acadêmico. Se uma agenda personalizada estiver aberta, a barra de menus do aplicativo principal reconstrói dinamicamente os submenus para refletir exatamente os títulos e listas criadas para essa agenda.

Você também pode excluir um título/lista inteiro da agenda personalizada através de `Configurações -> Plugins -> Registro de Atividades -> [Nome da Agenda] -> Remover título`.

Esse recurso serve para corrigir cadastros criados por engano ou limpar opções que não serão mais usadas. Ele não deve ser usado para apagar tarefas já registradas; tarefas devem ser removidas pelo fluxo normal do Eisenhower, Pomodoro ou Feynman.

Em agendas personalizadas com listas aninhadas, a exclusão respeita a relação entre os itens. Ao remover um item de uma lista anterior, os vínculos dos itens dependentes também são atualizados para evitar opções sem ligação válida.

### 19.7. Persistência da Agenda

A Agenda usa a pasta persistente do produto aberto:

- Eisenhower Organizer: `%LOCALAPPDATA%/EisenhowerOrganizer`;
- Lúmen: `%LOCALAPPDATA%/Lumen`.

Arquivos usados:

- `listas.db`: listas personalizadas de curso, ementa, semestre, disciplina, turma, tipo e sequência.
- `agenda_config.json`: preferência de cores da Agenda e última agenda aberta.
- `agendas/*.json`: agendas personalizadas criadas pelo usuário, incluindo listas próprias e relações entre listas aninhadas.
- `tarefas.json`: tarefas criadas e compartilhadas com Eisenhower, Pomodoro e Feynman.

O diretório legado `AgendaAvaliacoesAcademicas` não deve ser usado pelo plugin dentro do produto atual.

### 19.8. Boas práticas

- Cadastre disciplinas com nomes consistentes para aproveitar melhor as cores.
- Use sequência quando houver mais de uma atividade do mesmo tipo.
- Vincule data para atividades com prazo real.
- Vincule horário quando houver entrega, prova ou apresentação em horário definido.
- Revise a tarefa no Eisenhower depois do cadastro para conferir quadrante, descrição e sincronização.

## 20. Módulo Cronograma de Gantt & Engenharia 4D BIM

O módulo Cronograma de Gantt & Engenharia 4D BIM (disponível através do plugin Gantt) expande a plataforma com recursos profissionais de planejamento temporal, gestão de obras e controle físico de empreendimentos de engenharia, integrando o diagrama de Gantt tradicional aos conceitos da modelagem e planejamento 4D BIM.

### 20.1. Como acessar

Você pode abrir o Cronograma de Gantt das seguintes formas:

- **Menu Ferramentas**: selecione `Ferramentas -> Cronograma de Gantt BIM 4D`.
- **Atalho de Teclado**: pressione `Ctrl+Alt+G` em qualquer momento enquanto o aplicativo principal estiver focado.
- **Menu de Contexto de Tarefas**: na Matriz Eisenhower, no Pomodoro ou no Feynman, clique com o botão direito sobre qualquer tarefa selecionada e escolha `📊 Vincular ao Cronograma Gantt...` ou `📊 Criar Atividade no Gantt`.

### 20.2. Estrutura da interface

A janela do Cronograma de Gantt é dividida em duas áreas principais integradas:

1. **Tabela de Atividades (EAP/WBS)**: localizada à esquerda, lista as etapas e atividades da obra em formato de árvore hierárquica. Exibe código da EAP, nome da atividade, data de início, data de término, duração em dias, percentual de conclusão e status.
2. **Gráfico de Gantt Interativo**: localizado à direita, projeta as barras temporais de cada atividade sobre um calendário contínuo, ilustrando datas de início, prazos de entrega, marcos contratuais e dependências entre etapas.

### 20.3. Gestão de atividades e marcos

- **Criação de Atividades**: defina o nome, código hierárquico EAP, datas planejadas de início e término e duração estimada em dias.
- **Marcos (Milestones)**: atividades com duração zero funcionam como marcos de projeto, representando eventos decisivos como aprovações contratuais, vistorias técnicas ou conclusão de etapas-chave.
- **Percentual de Progresso**: informe o avanço físico de 0% a 100%. O status da atividade é atualizado automaticamente para `Não Iniciada` (0%), `Em Progresso` (1% a 99%) ou `Concluída` (100%), preenchendo a barra do gráfico proporcionalmente.

### 20.4. Dependências e Caminho Crítico

- **Tipos de Vínculos**: estabeleça relações de precedência entre atividades:
  - *Término-para-Início (FS)*: a atividade seguinte só inicia após o término da predecessora.
  - *Início-para-Início (SS)*: ambas as atividades iniciam simultaneamente.
  - *Término-para-Término (FF)*: ambas concluem simultaneamente.
  - *Início-para-Término (SF)*: a predecessora determina a conclusão da sucessora.
- **Latência e Folgas (Lag/Lead)**: defina intervalos de espera ou sobreposição entre atividades.
- **Caminho Crítico (Critical Path)**: o motor de cálculo identifica automaticamente o caminho crítico do projeto — a sequência de atividades sem folga que define a data final de entrega da obra. Atividades críticas recebem destaque visual no gráfico, permitindo ação preventiva contra atrasos.

### 20.5. Escalas da Linha do Tempo e Navegação

- **Níveis de Zoom**: alterne dinamicamente a resolução do calendário entre os modos `Dia`, `Semana`, `Mês` e `Ano`.
- **Navegação Visual**: deslize pela linha do tempo horizontalmente usando a barra de rolagem ou a roda do mouse.
- **Calendário Operacional**: a linha do tempo demarca visualmente a data atual ('Hoje'), fins de semana e feriados não trabalháveis cadastrados no calendário do projeto.

### 20.6. Sincronização Bidirecional Multi-Módulos

A integração une a estratégia do cronograma de longo prazo à rotina tática diária do Eisenhower, Pomodoro e Feynman:

- **Vinculação Direta**: conecte tarefas operacionais a atividades de obra.
- **Atualização Automática de Status**: ao concluir uma tarefa vinculada no Eisenhower, o progresso da atividade correspondente no Gantt avança automaticamente. Ao mover a tarefa para `Em Progresso` (ou executá-la no Pomodoro), a atividade no cronograma reflete o estado de execução em andamento.
- **Duração e Prazos**: mudanças na duração estimada das tarefas refletem na previsão de conclusão da atividade no cronograma.
- **Segurança de Dados**: excluir uma tarefa da Matriz desfaz o vínculo com a atividade sem apagar as medições ou o cronograma original da obra.

### 20.7. Registro de Avanço de Campo e Diário de Obra (FieldProgressService)

- Registre medições físicas de obra diretamente nas atividades com avanço de 0% a 100%.
- Anexe notas técnicas de inspeção, justificativas de variações, fotos e relatórios do diário de obra.
- Rastreabilidade com registro do profissional responsável pela medição e data do apontamento.
- Comparação visual entre o cronograma previsto (linha de base) e o cronograma realizado.

### 20.8. Painel de Simulação 4D BIM (Bim4DSimulationPanel)

- **Simulação Construtiva Passo a Passo**: reproduz visualmente a evolução temporal da obra através do painel `Bim4DSimulationPanel`.
- **Controles de Reprodução**: botões de Iniciar, Pausar, Reiniciar e controle deslizante de linha do tempo com ajuste dinâmico de velocidade da simulação.
- **Vínculos BIM**: visualização da sincronização entre os elementos do modelo de engenharia e as fases planejadas do cronograma físico.

### 20.9. Intercâmbio de Dados: MS Project XML e Planilhas CSV

- **Conector MS Project XML (`MsProjectXmlConnector`)**:
  - Importação de projetos completos criados no Microsoft Project com estrutura EAP, durações, datas e predecessoras.
  - Exportação fiel do cronograma local para formato XML compatível com MS Project para compartilhamento com clientes e parceiros.
- **Serviço de Intercâmbio CSV (`CsvExchangeService`, `CsvMappingDialog`)**:
  - Importação de cronogramas tabulares em CSV com mapeamento interativo de colunas (Nome, Início, Término, Duração, Progresso, EAP).
  - Exportação de dados do cronograma para planilhas eletrônicas.
- **Importador de Tarefas do Eisenhower (`EisenhowerTaskImportDialog`)**:
  - Janela dedicada para selecionar tarefas pendentes e em progresso da Matriz Eisenhower e transformá-las em atividades de cronograma com um clique.

### 20.10. Calendários de Trabalho, Dias Úteis e Feriados

- **Dias Úteis Semanais (`GanttWorkingDaysDialog`)**: configuração customizada dos dias trabalháveis da semana (segunda a sexta, sábados parciais/integrais, turnos).
- **Feriados e Exceções (`GanttHolidaysDialog`)**: cadastro de feriados nacionais, municipais ou paralisações programadas da obra.
- **Cálculo Realista de Prazos**: o motor de agendamento desconta automaticamente fins de semana e feriados ao calcular datas de término a partir da duração em dias úteis.

### 20.11. Auditoria, Servidor REST API e Webhooks

- **Log de Auditoria (`AuditService`, `AuditLog`)**: rastreamento completo de todas as alterações feitas no cronograma, gravando autor, horário, campo modificado, valor anterior e novo valor.
- **Servidor HTTP Embutido (`HttpServer`, `RestApiController`)**: disponibiliza uma API REST local para integração do cronograma com sistemas ERP, ferramentas de Business Intelligence ou painéis web.
- **Despachante de Webhooks (`WebhookDispatcher`)**: disparo automático de notificações HTTP para endpoints externos em eventos de criação, alteração ou conclusão de atividades do cronograma.

### 20.12. Boas práticas no planejamento de cronogramas

- Estruture a EAP completa com todas as fases e entregas antes de vincular tarefas operacionais diárias.
- Evite dependências circulares onde atividades dependam direta ou indiretamente de si mesmas.
- Monitore atentamente as atividades no caminho crítico para evitar atrasos no prazo final.
- Registre o avanço físico periodicamente para manter o cronograma calibrado com a realidade da obra.
- Combine a Matriz Eisenhower para o foco diário com o Gantt para a visão estratégica do projeto.

## 21. Módulo Modelo Qwen de Inteligência Artificial Local

O Módulo Modelo adiciona inteligência artificial generativa e semântica de alto desempenho ao EISENHOWER ORGANIZER / LÚMEN, permitindo processar textos, resumir documentos e estruturar conceitos complexos de forma 100% offline e privada. Ele combina a execução local de modelos da família Qwen2.5 (quantizados em formato GGUF via llama.cpp) com a compatibilidade nativa com modelos instalados através do Ollama.

### 21.1. O que é e como funciona
- **Execução Offline e Local**: o motor utiliza quantizações otimizadas da família Qwen2.5 (modelo `qwen2.5-3b-instruct-q4_k_m.gguf`) executadas diretamente pela biblioteca llama.cpp na CPU da sua máquina.
- **Privacidade Absoluta**: nenhuma palavra digitada, documento importado ou resultado gerado trafega pela internet ou por servidores em nuvem. A sua produção intelectual e os seus dados permanecem estritamente no seu computador.
- **Arquitetura Leve sob Demanda**: para manter o instalador do aplicativo leve e rápido de baixar, o arquivo de modelo não vem pré-embutido pesando desnecessariamente no instalador. O usuário tem total autonomia para baixá-lo quando desejar ou utilizar modelos que já possua no Ollama.
- **Independência de Conectividade**: após o download do modelo, a inferência opera perfeitamente sem conexão com a internet, sem necessidade de contas de usuário e sem assinaturas de serviços de terceiros.

### 21.2. Gerenciador do Modelo Local Qwen 2.5 (GGUF) sob Demanda
O aplicativo conta com uma janela interativa dedicada para gerenciar a instalação e o ciclo de vida do modelo Qwen:

1. **Como abrir o Gerenciador**:
   - No menu do Mapa Mental, acesse `Configurações -> Configurações Específicas -> Mapa Mental -> Como baixar ou gerenciar modelo Qwen (GGUF)...`.
2. **Informações exibidas na janela**:
   - **Status atual**: indica se o modelo está *Instalado*, *Não instalado*, *Baixando*, *Pausado* ou *Cancelado*.
   - **Tamanho esperado**: detalha o espaço aproximado em disco (~2,17 GB).
   - **Pasta de destino**: exibe o caminho exato onde o arquivo é armazenado (`%LOCALAPPDATA%\EisenhowerOrganizer\models` ou pasta correspondente do Lúmen).
3. **Controles e Recursos Interativos**:
   - **Botão Baixar Modelo**: conecta-se diretamente ao repositório oficial no Hugging Face e inicia a transferência do arquivo `qwen2.5-3b-instruct-q4_k_m.gguf`.
   - **Barra de Progresso com Métricas**: exibe visualmente o percentual de conclusão, o volume baixado (ex.: 650 MB / 2,17 GB), a taxa de download em tempo real (MB/s) e a estimativa de tempo restante.
   - **Botão Pausar**: permite interromper o download a qualquer momento sem perder os dados já baixados.
   - **Botão Retomar**: retoma a transferência exatamente do último byte baixado, graças ao suporte a requisições de continuação (HTTP Range).
   - **Botão Cancelar**: cancela a operação de download de forma limpa e segura.
   - **Botão Excluir Modelo**: quando você precisar liberar espaço de armazenamento em seu computador, clique neste botão para remover fisicamente o arquivo GGUF do disco após confirmação rápida, recuperando imediatamente ~2,17 GB de espaço.
   - **Botão Abrir Pasta**: abre diretamente a pasta de modelos no Windows Explorer para visualização ou cópia manual de arquivos.
4. **Indicador de Status Dinâmico nos Menus**:
   - O menu `Configurações -> Configurações Específicas -> Mapa Mental` verifica a presença real do arquivo e atualiza seus rótulos em tempo real:
     - `Resumo com modelo qwen2.5-3b [Instalado]`: quando o arquivo está presente e validado para uso imediato.
     - `Resumo com modelo qwen2.5-3b [Não instalado]`: quando o arquivo ainda não foi baixado, orientando o usuário a abrir o gerenciador.
5. **Tradução Dinâmica em Tempo de Execução**:
   - Todos os botões, mensagens, perguntas de confirmação e dicas do Gerenciador traduzem-se imediatamente ao alternar entre Português e Inglês, sem necessidade de reiniciar o aplicativo.

### 21.3. Integração com Modelos Locais via Ollama
Para usuários que já utilizam ou preferem outros modelos de inteligência artificial de código aberto:

1. **Guia Interativo de Instalação**:
   - Acesse `Configurações -> Configurações Específicas -> Mapa Mental -> Como instalar modelos no Ollama...`.
   - Uma janela interativa apresenta instruções passo a passo para baixar e instalar o Ollama no Windows (`https://ollama.com`).
2. **Modelos Recomendados com Cópia Rápida de Comandos**:
   - A janela lista os modelos mais indicados para criação de mapas mentais:
     - `llama3.2` (modelo leve e de alta velocidade da Meta);
     - `qwen2.5:3b` (versão Qwen via Ollama com excelente capacidade de raciocínio);
     - `gemma2:2b` (modelo compacto do Google);
     - `mistral:7b` (modelo robusto para textos mais densos);
     - `phi3:mini` (modelo compacto de alto desempenho da Microsoft).
   - Ao lado de cada modelo, botões dedicados de copiar permitem enviar o comando correspondente (ex.: `ollama run llama3.2`) diretamente para a área de transferência com um único clique.
3. **Teste de Conexão com o Servidor Local**:
   - O botão de teste verifica se o serviço do Ollama está rodando localmente na porta padrão `http://localhost:11434`, informando o status de conectividade em tempo real.
4. **Detecção Dinâmica no Submenu**:
   - Todos os modelos baixados e disponíveis no Ollama são detectados automaticamente e listados no submenu `Configurações -> Configurações Específicas -> Mapa Mental -> Modelos Ollama`. Ao adicionar novos modelos via terminal, o submenu é atualizado dinamicamente.

### 21.4. Como utilizar a IA Local na geração de Mapas Mentais
A integração entre inteligência artificial e mapas mentais permite transformar longos textos em estruturas conceituais claras:

1. Abra a aba ou janela do **Mapa Mental**.
2. No menu superior da ferramenta, acesse `Configurações -> Configurações Específicas -> Mapa Mental` e escolha seu método de preferência:
   - `Resumo método heurístico` (instantâneo e determinístico);
   - `Resumo com modelo qwen2.5-3b [Instalado]` (análise semântica profunda via llama.cpp);
   - Um dos modelos listados no submenu `Modelos Ollama`.
3. Clique no botão **Importar documento** na barra de ferramentas do Mapa Mental.
4. Selecione o arquivo de texto ou documento desejado (formatos suportados: PDF, EPUB, DOCX, TXT, Markdown, CSV, XLSX, etc.).
5. Uma janela com barra de progresso indicará as etapas de leitura, segmentação textual, chamada do modelo e montagem dos nós.
6. Ao término, a tela do Mapa Mental apresentará o conceito central rodeado por nós principais de tópicos e nós secundários de detalhes conceituais, todos sintetizados e estruturados de forma coerente.
7. **Fallback Heurístico Transparente**: se o modelo selecionado não estiver disponível, encontrar limites de memória RAM ou se o documento contiver padrões não sintetizáveis pelo modelo local, o sistema ativa automaticamente o algoritmo heurístico integrado de extração de tópicos, garantindo que o mapa mental seja gerado sem interrupções ou erros.

### 21.5. Desempenho, Requisitos e Boas Práticas
- **Multithreading**: o motor de inferência adapta-se automaticamente à quantidade de núcleos lógicos da CPU da sua máquina para proporcionar a resposta mais rápida possível.
- **Requisitos de Memória**: recomenda-se dispor de pelo menos 4 GB de memória RAM livre no sistema durante a inferência com modelos locais de 3 bilhões de parâmetros.
- **Boas Práticas de Uso**: para documentos muito volumosos (como livros inteiros com centenas de páginas), recomenda-se a importação por capítulos específicos ou blocos temáticos para enriquecer a precisão dos nós conceituais e acelerar a resposta.
- **Flexibilidade**: você pode alternar livremente entre o método heurístico (ultra-rápido) e os modelos generativos conforme a necessidade da sua atividade intelectual.

## 22. Arquivo, importação e exportação

No menu `Arquivo`, você encontra:

- `Novo`: inicia uma sessão limpa.
- `Abrir`: importa ou abre arquivos suportados.
- `Salvar`: salva o estado atual e gera exportações padrão.
- `Salvar Como`: permite escolher destino e formato explicitamente.
- `Limpar`: remove dados da sessão com confirmação.
- `Sair`: fecha o aplicativo.

Formatos disponíveis para `Salvar Como`:

- JSON;
- CSV;
- XLSX;
- PDF.

O comando `Salvar` gera o conjunto padrão `JSON`, `CSV`, `XLSX` e `PDF` usando o nome e o destino configurados. Se ainda não houver destino anterior, o aplicativo usa a pasta de documentos do usuário como ponto de partida.

Use cada formato assim:

- `JSON`: melhor opção para preservar os dados estruturados e transportar registros entre instalações.
- `CSV`: opção simples para abrir em planilhas e revisar campos em texto.
- `XLSX`: opção de planilha com os registros em uma aba `records`.
- `PDF`: relatório legível para consulta, impressão ou compartilhamento.

O arquivo de dados principal do aplicativo fica na pasta persistente do usuário como `tarefas.json`. O salvamento bem-sucedido mantém também `tarefas.json.bak`, usado como cópia de segurança. Se um JSON inválido for detectado, o aplicativo evita sobrescrever o arquivo corrompido e registra o problema em log.

Formatos adicionais:

- Mapa Mental: JSON e PNG.
- Audiolivro: texto e PDF.

## 23. Idiomas

Em `Configurações -> Idioma`, você pode alternar entre:

- Português (Brasil);
- English (United States).

A troca de idioma atualiza a interface principal e os plugins que foram carregados. A escolha fica salva para a próxima abertura.

Na janela `Cadastro de Atividades`, os menus, rótulos, listas e botões de diálogo também acompanham a troca de idioma. Isso inclui botões como `Sim`, `Não`, `OK` e `Cancelar` nas confirmações e janelas de escolha.

As janelas `Gerenciador de plugins` e `Gerenciador de logs` também atualizam seus textos quando o idioma é alterado. Se elas já estiverem abertas, os títulos, botões, colunas e mensagens são atualizados sem precisar fechar e abrir novamente.

## 24. Sobre e notas internas

Em `Opções -> Sobre`, você encontra:

- detalhes do aplicativo;
- histórico;
- notas de lançamento;
- licenças;
- avisos;
- política de privacidade.

As seções sobre Feynman, Pomodoro, Mapa Mental, Audiolivro, Registro de Atividades e Cronograma de Gantt BIM 4D aparecem nas notas internas apenas quando os plugins correspondentes estão disponíveis.

## 25. Atalhos úteis

- `Ctrl+N`: novo.
- `Ctrl+O`: abrir.
- `Ctrl+S`: salvar.
- `Ctrl+Shift+S`: salvar como.
- `Ctrl+Q`: sair.
- `Ctrl+M`: calendário.
- `Ctrl+Shift+T`: painel `Minhas Tarefas`, quando Feynman existe.
- `Ctrl+Shift+E`: painel `Em Progresso`, quando Pomodoro existe.
- `Ctrl+Shift+F`: abrir Feynman.
- `Ctrl+Shift+P`: abrir Pomodoro.
- `Ctrl+Shift+N`: abrir Mapa Mental.
- `Ctrl+Shift+L`: abrir Audiolivro, quando o plugin existe.
- `Ctrl+Shift+R`: abrir Registro de Atividades, quando a Agenda existe.
- `Ctrl+Alt+G`: abrir Cronograma de Gantt BIM 4D, quando o plugin existe.
- `Ctrl+Shift+Del`: limpar.
- `Ctrl+Shift+G`: busca global.
- `Ctrl+Shift+H`: painel `Hoje / Próximos 7 dias`.
- `Ctrl+Shift+A`: sobre.
- `Ctrl+Shift+M`: manual.
- `Alt+A`: menu Arquivo.
- `Alt+I`: menu Idioma.
- `Alt+O`: menu Opções.

## 26. Problemas comuns

### Um plugin não aparece

Confirme se o plugin foi compilado, se o deploy foi executado e se a pasta `plugins` está junto da aplicação. Se o plugin não for encontrado, a aba e o menu dele não aparecem.

Abra `Ferramentas -> Gerenciador de plugins` para conferir status, caminho e erro de carregamento. Use `Ferramentas -> Gerenciador de logs` para localizar e acompanhar os logs da sessão.

No Gerenciador de logs, procure o log mais recente do produto em uso. Para Eisenhower Organizer, procure `file_eisenhower`. Para Lúmen, procure `file_lumen`. Mensagens `INFO` normalmente indicam ações concluídas; `WARNING`, `ERROR` e `CRITICAL` indicam situações que precisam de atenção.

### O Cronograma de Gantt não abre ou não aparece no menu

Verifique se a biblioteca `plugins/gantt/GanttPlugin.dll` foi compilada e implantada na pasta de plugins do aplicativo. O item `Cronograma de Gantt BIM 4D` no menu `Ferramentas` e o atalho `Ctrl+Alt+G` só são ativados quando o plugin é carregado com sucesso.

### O Registro de Atividades não aparece

Confirme se o plugin Registro de Atividades foi compilado e copiado para `plugins/agenda`. A funcionalidade só aparece em `Ferramentas` e `Configurações -> Plugins` quando o host encontra e carrega o plugin.

Se o Registro de Atividades aparece, mas uma tarefa registrada não fica colorida, confirme se `Configurações -> Plugins -> Registro de Atividades -> Cores -> Coloridas` está ativo. Tarefas com arquivo vinculado também recebem destaque visual de vínculo, além da cor de fonte quando ela estiver disponível.

### Uma recorrência não gerou novas tarefas

Confira se o item está incluído pelos métodos, filtro e seleção atual. Ele precisa ter recorrência definida ou entrar pela `Frequência padrão`; se não tiver data ou horário, habilite `Usar hora inicial quando o item não tiver horário` e informe a data/hora de referência. Verifique também se o período de aplicação inclui as próximas datas e se `Máximo por item` permite novas ocorrências. Se `Evitar duplicadas existentes` estiver ativo, o aplicativo pode ignorar ocorrências que já existem.

### Um lembrete local não apareceu

Mantenha o aplicativo aberto até o horário do aviso. Confira se os métodos, filtro e seleção produzem lembretes possíveis, se há pelo menos uma forma de aviso ativa (`Alertar no app` ou `Bandeja do sistema`) e se a data/hora não está no passado. Se a tarefa tem data sem horário, ative `Horário padrão se faltar`; se a bandeja não estiver disponível, use `Alertar no app`.

### O Mapa Mental não importa documento com IA

Verifique se os arquivos necessários do runtime local de IA e o modelo estão disponíveis. Sem isso, a importação assistida pode não gerar estrutura automática.

### O Audiolivro não fala

Verifique a voz selecionada, o volume, a permissão de áudio, a conexão e a disponibilidade do serviço de síntese usado pelo ambiente.

### Minhas tarefas não aparecem no calendário

Confira se a tarefa possui data vinculada e se o filtro do calendário está no período correto.

### O arquivo vinculado não abre

O caminho pode ter sido movido, renomeado ou apagado. Edite a tarefa ou o conceito e vincule o arquivo novamente.

### Tarefas ou preferências não ficam salvas

Verifique a permissão de escrita na pasta do usuário. O aplicativo salva dados e preferências em uma pasta persistente local.

## 27. Fluxo recomendado de uso

1. Planeje no Eisenhower.
2. Use o calendário para revisar prazos.
3. Use o Registro de Atividades para cadastrar provas, trabalhos, seminários e entregas acadêmicas quando o plugin estiver disponível.
4. Use `Hoje / Próximos 7 dias` para escolher a próxima execução.
5. Defina recorrências e lembretes para compromissos que se repetem ou têm horário.
6. Envie tarefas de estudo para o Feynman.
7. Execute atividades no Pomodoro.
8. Estruture ideias complexas no Mapa Mental.
9. Use o Audiolivro para revisar textos e PDFs com leitura assistida.
10. Transforme leituras importantes em `Fluxo integrado de estudo` e revisões espaçadas.
11. Estruture cronogramas e marcos de projeto no `Cronograma de Gantt BIM 4D` (`Ctrl+Alt+G`) somente como ferramenta auxiliar sujeita à revisão independente.

---

## English (US)

Documented Version: `2026.11.22.0`
Date: `November 22, 2026`

This manual describes the current behavior of **EISENHOWER ORGANIZER** and **LÚMEN** with a technical-operational approach. It explains how to use each application, how integrated modules behave, and the applicable licensing and privacy rules.

> **Additional Documentation:** See also [`README.md`](README.md#enus), [`docs/user-guide.md`](docs/user-guide.md#enus), [`docs/architecture.md`](docs/architecture.md#enus), [`docs/privacy-policy.md`](docs/privacy-policy.md#enus), and [`docs/licensing.md`](docs/licensing.md#enus).
>
> **Technical Stack & Auxiliary Scope ("AS IS"):** Built natively in **C++23 with Qt 6.12.0** (Pure MSVC `14.52` and MinGW `15.1.0` x64) under **Dual Licensing** (**GNU AGPLv3** for public source code in [`LICENSE`](LICENSE) and **Official EULA + CLC** for signed Microsoft Store editions — Eisenhower Organizer `9P289X0185C3` and Lúmen `9N70CLLMVRPN`). Supplied **"AS IS"** as an auxiliary tool; critical deadlines, Local AI outputs (`Qwen`/`Ollama`), and engineering schedules (`4D BIM Gantt` / CPM — requiring licensed **ART/RRT** oversight before **CREA/CAU** where applicable) must always be independently verified.

## 1. Overview

EISENHOWER ORGANIZER / LÚMEN is a desktop application for managing tasks, studies, focus, assisted reading, and assessment activities.

It brings together 7 high-productivity integrated native modules and tools:

- `Eisenhower`: task planning by importance and urgency (2x2 Matrix and Tabbed Layout).
- `Feynman`: active study structured around concepts, plain explanations, and spaced reviews.
- `Pomodoro`: task execution with focus timer and break intervals (Simple and Robust layouts).
- `Mind Map`: visual creation of connected concepts, featuring automated AI-assisted generation.
- `Audiobook`: multimodal assisted reading of text files and PDFs with speech synthesis and synchronized word highlighting.
- `Agenda (Activity Registration)`: assessment activities, exams, homework, and commitments with custom agendas and nested lists.
- `4D BIM Gantt Schedule`: professional project and construction scheduling with interactive timeline, critical path, and 4D simulation.
- `Local AI Model (Qwen and Ollama)`: private offline generative AI inference engine with on-demand local Qwen model support and any models installed via Ollama (Llama 3.2, Qwen 2.5, Gemma 2, Mistral, etc.) with dynamic discovery, interactive download manager, and offline fallback for automated synthesis and knowledge structuring in the Mind Map.

Starting with version 2026.10.7.0, the platform introduces a lightweight on-demand AI architecture: the application installer no longer bundles heavy model binaries, allowing the official Qwen2.5-3B-Instruct (GGUF) model to be downloaded via the interactive Download Manager with pause, resume, and disk space reclamation features, alongside native support for local Ollama models.

### 1.1. RELEASE NOTES: Version 2026.11.22.0

Date: November 22, 2026

This update consolidates platform modernization, compact external distribution, complete bilingual documentation, and unified end-to-end verification across all modules:

1. **Modernized Foundation (C++23 & Qt 6.12.0) with Dual MSVC / MinGW Support**:
   - Codebase and all 7 integrated modules standardized on strict C++23 and Qt 6.12.0, natively built with Pure MSVC (`14.52`) and MinGW-w64 (`15.1.0`).
2. **Clean External Distribution & Compact Installers**:
   - Lean external distribution folders and compact Inno Setup installers using app-local VC++ Runtime DLLs (without embedding `vc_redist.x64.exe`).
3. **Unified Test Suite (`tests/` — 11 CTest Executables)**:
   - Consolidated 11-target CTest verification covering core utilities, atomic storage, host UI, and all 7 modules (`Qt6: Compilar e Executar SUITE (MSVC)` and `Qt6: Compilar e Executar SUITE (MinGW)`, 11/11 passing).
4. **Dual Licensing & Updated `About` Dialog**:
   - Full adoption of the Dual-Licensing model (**GNU AGPLv3** for public source code with developer freedom over `source/assets/` + **Official EULA + CLC** for Microsoft Store editions), updated texts inside the `About` dialog (`Ctrl+Shift+A`), and complete bilingual documentation under `docs/`.

### 1.2. RELEASE NOTES: Version 2026.10.7.0

Date: October 7, 2026

This release brings a major evolution to local artificial intelligence capabilities, introducing the on-demand Qwen Model Download Manager, dynamic local Ollama model integration, dynamic runtime translation, and general refinements:

1. **Interactive On-Demand Local Qwen 2.5 (GGUF) Model Manager**:
   - Download manager supporting on-demand retrieval of the official `qwen2.5-3b-instruct-q4_k_m.gguf` model (~2.17 GB) directly from the official Hugging Face repository.
   - Comprehensive interactive controls: progress bar showing percentage, transferred bytes, real-time speed (MB/s), and estimated remaining time.
   - Pause and Resume buttons at any moment using HTTP Range requests without wasting bandwidth.
   - Delete Model button to permanently remove the GGUF file from disk upon quick confirmation, immediately freeing up ~2.17 GB of local storage when not in use.
   - Open Folder button to open the models directory directly in Windows Explorer for manual management.

2. **Dynamic Menu Status Indicators**:
   - The menu `Settings -> Specific Settings -> Mind Map` dynamically reflects live model readiness: displays `Summary with qwen2.5-3b model [Installed]` when verified and ready, or `Summary with qwen2.5-3b model [Not installed]` when missing.
   - Direct action `How to download or manage Qwen model (GGUF)...` instantly opens the interactive download manager window.

3. **Complete Local Ollama Model Integration**:
   - Interactive step-by-step guide dialog `How to install Ollama models...` with instructions for installing Ollama on Windows from the official site.
   - One-click copy buttons for terminal installation commands for recommended models (`llama3.2`, `qwen2.5:3b`, `gemma2:2b`, `mistral:7b`, etc.).
   - Connection test button to probe local server connectivity (`http://localhost:11434`).
   - Automatic detection of all locally pulled Ollama models, listed dynamically in the `Ollama Models` submenu.

4. **Dynamic Runtime Translation**:
   - Full support for seamless language toggling between English and Portuguese across all screens, buttons, dialogs, messages, and menus in the AI module without restarting the application.

5. **Optimized and Lightweight Distribution**:
   - Installer size has been drastically reduced, eliminating pre-bundled heavy model files and giving users complete control over their disk space.

### 1.3. RELEASE NOTES: Version 2026.9.27.0

Date: September 27, 2026

This release fully consolidates the platform's architecture, definitively integrating all features as high-performance native modules, expanding private local artificial intelligence, and completing documentation and legal compliance for all modules:

1. **Definitive Consolidation of 7 Native Modules**:
   - Complete architectural transition: legacy external plugins now operate as 7 integrated native modules (`Eisenhower`, `Feynman`, `Pomodoro`, `Mind Map`, `Audiobook`, `Agenda`, and `4D BIM Gantt Schedule`), alongside the auxiliary `Model` (Local AI) module.
   - Elimination of fragile external dynamic loading, delivering near-instant application startup, enhanced operational stability, and fluid real-time bidirectional synchronization across all views.

2. **Complete Removal of Delimiter Tags and Text Sanitization**:
   - Comprehensive sanitization of all user-facing dialogs, legal documentation, manuals, and release notes, removing 100% of legacy internal delimiter tokens (such as double brackets and placeholder tags).
   - Information flows naturally and cleanly across dialogs, About screens, EULA agreements, and legal notices.

3. **Complete Legal Documentation and Governance for All Modules**:
   - Full implementation of standardized regulatory documents (About, EULA, Copyright, Privacy Policy, and Third-Party Notices) for the `Agenda` and `Model` modules, in both Portuguese (pt_BR) and English (en_US).
   - Direct integration with the documentation version editor (`find_VersionEditor`), ensuring version upgrades propagate automatically across all 48 documentation files in the ecosystem.

4. **Private Local AI Qwen Model Module**:
   - First-class offline inference support powered by the Qwen2.5 family via llama.cpp, delivering automated semantic summarization and concept hierarchy generation for Mind Maps.
   - 100% offline execution: absolute privacy with zero telemetry and zero user data transmitted to cloud servers.
   - Transparent heuristic fallback ensures seamless mind map generation even with very large documents or constrained system memory.

5. **Unified Localization and Translation Catalogs**:
   - Unified compilation of translation catalogs (`.ts` / `.qm`), allowing instant runtime language toggling between English and Portuguese for all new screens, menus, and status messages without restarting the application.

### 1.2. RELEASE NOTES: Version 2026.9.21.0

Date: September 21, 2026

This update brings critical performance optimizations and comprehensive improvements to daily activity tracking across the entire application:

1. **Instantaneous Response When Applying Task Edits**:
   - Immediate response when clicking "OK" in the task editing window (`✏️ Edit Task`) across all modules (Eisenhower, Pomodoro, and Feynman): the confirmation responds and closes on the spot, freeing the interface immediately without any freezing, delays, or UI lockup.
   - Persistence to disk and cross-module synchronization proceed smoothly and asynchronously in the background, ensuring maximum agility and workflow fluidity.
   - Immediate visual quadrant moves via the context menu prior to background disk saving.

2. **Comprehensive Lifecycle Visibility in Calendar ([In Progress])**:
   - The Calendar panel now offers complete transparency across every stage of the activity lifecycle: the `Global Task List:` panel and date tooltips now report completed tasks (`[Completed]`), pending tasks (`[Pending]`), and active tasks currently underway (`[In Progress]`).
   - Clean and standardized presentation, without any unwanted extra pomodoro count sub-lines in the calendar task list.
   - Fast and direct interaction: double-click any task in the list to immediately open its linked external file (if present), with full access to the right-click context menu.

3. **Runtime Dynamic Translation**:
   - Full support for real-time dynamic language switching between English and Portuguese for all calendar status labels (`[In Progress]` / `[Em Progresso]`, `[Completed]` / `[Concluída]`, `[Pending]` / `[Pendente]`), informative tooltips, and confirmation dialogs without requiring an application restart.

4. **Standardized Visual Appearance for the Pomodoro Timer**:
   - The timer panel frame in the Pomodoro module now consistently displays the descriptive label `⏱️ Timer` (with dynamic runtime translation to `⏱️ Temporizador` in Portuguese).

### 1.3. RELEASE NOTES: Version 2026.9.19.0

Date: September 19, 2026

This update brings key operational enhancements, comprehensive cycle and timeline synchronization, a responsive task dialog, integrated construction management in the 4D BIM Gantt Schedule, and user experience polish across the platform:

1. **Responsive Task Editing Dialog**:
   - Dynamic layout adaptation (`Compact` in standard windows and `Wide` two-column layout with a 410px frozen sidebar when window width is 760px or greater, or maximized).
   - New structured metrics: Pomodoro cycles completed (🍅), Pomodoro cycle goal (🎯), and estimated task duration in days.
   - Safe information tooltip across task listings with real-time completion percentage calculation based on focus goals and site progress.

2. **Pomodoro Cycle Mismatch Detection & Resolution Dialog**:
   - Preventative dialog when starting in-progress tasks ("Doing") whose target cycle (🎯) differs from the timer's active cycle setting (⚙️).
   - Detailed discrepancy comparison table and three action choices: "Reset All" (with instant host bridge synchronization across all modules), "Start Anyway", or "Cancel".

3. **Advanced 4D BIM Gantt Schedule Context Menu Integration**:
   - Direct context menu (right-click) actions in Eisenhower, Pomodoro, and Feynman:
     - `📊 Create Activity in Gantt`: creates a new WBS schedule activity using task title, duration in days, and start/finish dates.
     - `📊 Link to Gantt Schedule...`: binds task to an existing schedule activity.
     - `📊 Open in Gantt Schedule`: opens the Gantt window focused on the linked activity, showing WBS code and completion percentage.
     - `📊 Record Field Progress...`: logs physical completion percentage (0% to 100%) and site diary observations in real time.
     - `📊 Change Gantt Link...` and `📊 Unlink from Gantt`.

4. **4D BIM Simulation, MS Project XML/CSV Exchange, and Engineering Services**:
   - 4D BIM Simulation panel (`Bim4DSimulationPanel`) with chronological step-by-step playback controls and adjustable simulation speed.
   - Comprehensive Microsoft Project XML import and export connector (`MsProjectXmlConnector`).
   - CSV exchange service with interactive column mapping dialog (`CsvMappingDialog`) and dedicated Eisenhower task import dialog.
   - Work calendar management with custom weekly working days configuration and non-working holidays/exceptions tracking.
   - Built-in audit trail service (`AuditService`), field progress measurements with construction diary (`FieldProgressService`), embedded HTTP REST API server, and Webhook dispatcher for event notifications.

5. **Dynamic Exclusion in Activity Registration & Host Menu Sync**:
   - Real-time dynamic exclusion of items and entire titles in custom agendas, synchronized instantly with host application menus.

### 1.4. RELEASE NOTES: Version 2026.9.16.0

Date: September 16, 2026

This update brings the integration of the 4D BIM Gantt Schedule & Engineering module, full build compatibility with MinGW and MSVC compilers, and enhanced project timeline management:

1. **4D BIM Gantt Schedule & Engineering Module**:
   - Comprehensive planning and control of engineering projects and construction sites through interactive Gantt charts and 4D BIM methodology.
   - Work Breakdown Structure (WBS) with hierarchical codes, start and finish dates, duration in days, contractual milestones, and physical progress percentages.
   - Automatic activity dependency calculation (FS, SS, FF, SF) and dynamic Critical Path highlighting for proactive delay mitigation.
   - Interactive timeline navigation with 4 zoom scales (Day, Week, Month, and Year), identifying working days, weekends, and project calendar holidays.

2. **Bidirectional Integration with Eisenhower Matrix**:
   - Direct linking of operational Eisenhower tasks to designated schedule activities via context menu (`📊 Link to Gantt Schedule...`).
   - Real-time automatic synchronization: completing tasks recalculates activity progress; transitioning tasks to "In Progress" updates the corresponding schedule activity.
   - Synchronized duration adjustments and clean detachment when tasks are removed without affecting the underlying engineering schedule.

3. **Shortcuts and Accessibility**:
   - Quick schedule access using global keyboard shortcut `Ctrl+Alt+G` and the `Tools -> 4D BIM Gantt Schedule` menu item.
   - Instant summary of linked activities and progress in info dialogs.

### 1.5. RELEASE NOTES: Version 2026.8.16.0

Date: August 16, 2026

This update brings key enhancements to task visualization modes, in-progress activity management, and visual polish across the entire application:

1. **New Tabbed Layout Modes (Vertical and Horizontal)**:
   - The `Settings -> Task Layout -> User Interface -> Eisenhower -> Tabbed Layout` menu now offers two distinct orientations:
     - `Vertical`: displays task sections (Pending, In Progress, and Completed) stacked vertically per quadrant.
     - `Horizontal`: displays task sections arranged side-by-side in horizontal columns.
   - **Smart Collapsing with Contour Border and Text Rotation**: Unchecking a section's checkbox in Horizontal mode collapses the column into an ultra-compact width (~30px), rotates the section label 90° clockwise for fluid vertical reading, and draws a native, elegant frame contour that begins and ends precisely at the checkbox sides on top, aligned with neighboring quadrant borders. Clicking anywhere on the collapsed column or its checkbox expands it back to full width.

2. **"In Progress" Lists Integration and Synchronization**:
   - Each of the four Eisenhower quadrants now features a dedicated "In Progress" (Doing) task list.
   - Full bidirectional drag-and-drop support between `Pending`, `In Progress`, and `Completed` lists across all layout modes.
   - Real-time automatic synchronization with the Pomodoro module's Kanban board and timer.

3. **Standardized 2x2 Matrix Labels**:
   - In `2x2 Matrix Layout`, the group box headers for `In Progress` tabs now display the quadrant's full descriptive priority title (`🔴 Important and Urgent`, `🟠 Important, but Not Urgent`, `🟡 Not Important, but Urgent`, and `🟢 Not Important and Not Urgent`), replacing generic repeated titles and ensuring consistent visual identification.

4. **Vertical Centering of Button Content**:
   - Enhanced vertical centering for text, emojis, icons, and composite buttons throughout the host application and all plugins.

### 1.6. RELEASE NOTES: Version 2026.8.8.0

Date: August 8, 2026

This update brings improvements to Activity Registration visual customization and menu reorganization:

1. **Independent Color Configuration per Module**:
   - Option to select `Black` or `Colorful` modes individually for each environment (`Eisenhower`, `Pomodoro`, or `Feynman`).
   - `General` option to apply color preferences to all three modules simultaneously.

2. **Colors Menu Reorganization**:
   - Structured `Black` and `Colorful` submenus under `Settings -> Activity Registration -> Colors` in the main menu and `Settings -> Colors` in the `Activity Registration` window.
   - Complete support for dynamic runtime translation and real-time task visual updates.

### 1.7. RELEASE NOTES: Version 2026.7.8.0

Date: July 8, 2026

This version improves how tasks appear visually in lists. You can now configure the font for regular tasks and separately configure the font for tasks that have a linked external file.

The new path is `Settings -> Reserved Space -> Font Settings`. It contains two options:

- `General Tasks`: changes the appearance of tasks without a linked external file.
- `Linked File`: changes the appearance of tasks with a linked external file.

In each window, you can choose font family, font style, size, bold, italic, underline, and strikeout. Use `Restore` to return that category to its default values, `OK` to save, or `Cancel` to leave without applying changes.

The changes appear in Eisenhower tasks and also in tasks shown by the Pomodoro and Feynman plugins when those modules are available. Regular tasks continue using the `General Tasks` setting; tasks with a linked file use the `Linked File` setting. This keeps one category from changing the other.

The linked-file emphasis was moved from the direct menu `Settings -> Reserved Space -> Linked File` to the new submenu `Settings -> Reserved Space -> Font Settings -> Linked File`, keeping font-related options together.

## 2. Main screen

On the main window, you will find:

- top menu bar;
- tabs for available modules;
- active-module counter on the top right;
- Eisenhower task area;
- calendar and global task list;
- optional `Activity Registration` window, when the Activity Registration plugin is available;
- Feynman and Pomodoro side panels when those plugins are available.

Tabs and tools appear according to loaded plugins. If a plugin is not installed, built, or copied to the expected location, its tab or menu option does not appear.

## 3. Interface tabs

Module tabs may include:

- `Eisenhower`;
- `Pomodoro`;
- `Feynman`;
- `Mind Map`;
- `Audiobook`.

Under `Settings -> Task Layout -> Interface Tabs`, you can:

- lock tab adjustment;
- unlock tab adjustment;
- restore the default order.

When adjustment is unlocked, you can drag tabs to change their order. The choice is saved for future launches.

Under `Settings -> Task Layout -> Show Interface Tabs`, you can also choose which module tabs remain visible in the GUI:

- `Eisenhower`;
- `Pomodoro`, when the plugin is available;
- `Feynman`, when the plugin is available;
- `Mind Map`, when the plugin is available;
- `Audiobook`, when the plugin is available.

The application automatically detects which plugins are available. Options for missing plugins are disabled or unavailable for selection. The `Eisenhower` tab belongs to the main application and always exists; however, its visibility can be disabled when another tab is available, allowing the interface to stay focused on the desired module. The selection is saved in `layout.json` inside the user's persistent folder and is restored when the application starts again.

To avoid an interface with no visible work area, the application prevents hiding the last available tab. If you want to restore the main view quickly, enable `Eisenhower` again from the same menu.

## 4. Active-module counter

The top counter changes according to the open tab.

It may show:

- how many items are selected;
- how many items exist in the current module;
- total tasks, concepts, in-progress items, or equivalent items in the active module.

This counter helps confirm that you are working with the intended selection, especially on screens with many items.

## 5. Eisenhower module

Eisenhower organizes tasks into 4 quadrants:

- `Important and Urgent`: do first.
- `Important, but Not Urgent`: plan.
- `Not Important, but Urgent`: solve quickly or delegate.
- `Not Important and Not Urgent`: postpone, reduce, or eliminate.

Each task can contain:

- main text;
- description;
- quadrant;
- priority;
- mastery level;
- date;
- time;
- linked file;
- pending or completed state.

## 6. Create tasks

To create a task:

1. Type the task text.
2. Choose the quadrant.
3. Set priority if needed.
4. Set mastery level if needed.
5. Link date and time if the task has a deadline.
6. Click `Add Task` or press Enter.

A task can also be created without date or time. Those fields are optional.

## 7. Edit tasks

Right-click a task and use `Edit Task`.

### 7.1. Responsive interface and layout modes

The editing dialog (`EditTaskDialog`) dynamically adapts its layout according to screen dimensions:

- **Compact Mode**: activated in standard window sizes (< 760px), organizing title, description, priority, dates, and goals into a single fluid, easily scrollable vertical column.
- **Wide Mode**: automatically activated when window width reaches 760px or greater, or upon maximizing. In this mode, the editing area is organized into two functional columns:
  - The left column expands main task text and detailed description.
  - The right column docks a specialized sidebar with a stable width of 410px, consolidating dates, times, linked files, focus cycle goals, and progress metrics.

### 7.2. Editable fields and new metrics

When editing, you can change:

- main text;
- detailed description;
- priority and target quadrant;
- mastery level;
- linked file;
- date and time;
- **Pomodoro cycles completed (🍅)**: number of focus sessions already completed on the task;
- **Pomodoro cycle goal (🎯)**: target focus sessions estimated to conclude the task;
- **Estimated duration (days)**: expected execution duration in working or calendar days, synchronized automatically with the schedule when the task is linked.

### 7.3. Safe completion percentage and informative tooltips

Hovering the mouse cursor over any task in Eisenhower lists, Pomodoro boards, or the calendar displays a rich, safe tooltip with:
- full title and description;
- priority and mastery level;
- start/finish dates and scheduled times;
- tags and linked files;
- **Calculated completion percentage**: safe, dynamic calculation of task progress (0% to 100%), calculated from completed cycles (🍅) versus target cycles (🎯), or synchronized directly with the physical progress of the linked Gantt activity.

After finishing the edit and clicking `OK`, the application registers the changes and closes the dialog instantaneously, immediately freeing the user interface so you can continue operating the system. The success confirmation dialog responds promptly, and disk persistence as well as synchronization across integrated modules (Eisenhower, Pomodoro, Feynman, and Gantt Schedule) proceed smoothly and asynchronously in the background. This confirmation follows the active language in real time.

## 8. Complete, move, select, and remove

You can:

- mark a task as completed;
- unmark it to return it to the active group;
- move tasks between quadrants;
- drag and drop one or multiple tasks between quadrants, lists, and available tabs;
- remove one or more tasks with confirmation;
- link and synchronize tasks directly with the 4D BIM Gantt Schedule.

When you check a pending task, it moves to the completed group. When you uncheck a completed task, it returns to the pending group. This also works when the task is already selected.

Multiple selection:

- use `Ctrl` to select separate items;
- use `Shift` to select a range;
- a single click on one task focuses only that task again.

Removal from the context menu:

1. Left-click a task to select only it, or use `Ctrl`/`Shift` to select multiple tasks.
2. Right-click the selected task, or any task in the selected group.
3. Choose `Remove Task`.
4. In the confirmation, choose `Yes` to remove or `No` to cancel.

When the application language changes, the confirmation buttons follow the selected language. In Portuguese, the window shows `Sim` and `Não`; in English, it shows `Yes` and `No`. `No` is the initial choice to reduce accidental removals.

If you open the context menu on a task that is not part of the current selection, the action uses the clicked task. If you open the menu on one of the already selected tasks, the action keeps the selected group.

### 8.1. Gantt Schedule integration actions in context menu

Right-clicking any task provides dedicated engineering schedule integration actions:

- `📊 Create Activity in Gantt`: converts the task into a new schedule activity in the project WBS, automatically populating title, estimated duration in days, and planned dates.
- `📊 Link to Gantt Schedule...`: opens the selection dialog to link the task to an existing WBS schedule activity.
- `📊 Open in Gantt Schedule`: instantly opens the 4D BIM Gantt Schedule window focused on the linked activity, displaying its WBS code and physical completion percentage.
- `📊 Record Field Progress...`: opens the progress measurement dialog to log actual completion (0% to 100%) and record site diary notes, updating the schedule and audit log immediately.
- `📊 Change Gantt Link...`: reassigns the task to another schedule activity.
- `📊 Unlink from Gantt`: detaches the task without deleting schedule data or measurement history.

## 9. Task layout

Under `Settings -> Task Layout -> User Interface -> Eisenhower`, you can choose:

- `Tabbed Layout`: shows one quadrant at a time through main tabs, with internal task section partitions.
  - `Vertical`: arranges sections (`Pending`, `In Progress`, and `Completed`) stacked vertically from top to bottom.
  - `Horizontal`: arranges sections (`Pending`, `In Progress`, and `Completed`) side-by-side in horizontal columns.
- `Quadrant Layout`: shows all four quadrants at once in the main window.
- `2x2 Matrix Layout`: shows the four quadrants arranged in two rows and two columns, with internal tabs for `Pending`, `In Progress`, and `Completed`.

### 9.1. How to use the Tabbed Layout (Vertical and Horizontal)

The `Tabbed Layout` allows focusing on a single quadrant at a time:

1. **Tabbed Layout -> Vertical**:
   - The `Pending`, `In Progress`, and `Completed` sections are stacked on top of one another.
   - Each section has a checkbox in its header. Unchecking the checkbox vertically collapses the task list, leaving only the header bar visible. Checking it again expands the list.

2. **Tabbed Layout -> Horizontal**:
   - The `Pending`, `In Progress`, and `Completed` sections are arranged horizontally as side-by-side columns.
   - **Collapsing with Border and Rotation**: Unchecking the checkbox of any section collapses that column horizontally to an ultra-compact width (~30px).
   - The section label is rotated 90° clockwise, positioning the text vertically from top to bottom with perfect legibility.
   - The outer frame draws an elegant native contour around the entire column that connects directly to the checkbox sides on top, aligned with neighboring quadrant borders and heights.
   - **Quick Expansion**: To restore the column to its normal width, simply click anywhere on the collapsed column or directly on its checkbox.

### 9.2. How to use the 2x2 Matrix Layout

The `2x2 Matrix Layout` organizes the four quadrants in a grid (two rows and two columns):

1. `🔴 Important and Urgent` (top-left);
2. `🟠 Important, but Not Urgent` (top-right);
3. `🟡 Not Important, but Urgent` (bottom-left);
4. `🟢 Not Important and Not Urgent` (bottom-right).

Each quadrant contains three tabs:

- `Pending`: shows open or not-yet-started tasks.
- `In Progress`: shows tasks currently being worked on (synchronized with active focus and the Pomodoro Kanban). The group box frame displays the quadrant's full descriptive priority title (e.g. `🔴 Important and Urgent`).
- `Completed`: shows tasks marked as finished.

To change task state:
- Checking a task in `Pending` moves it automatically to `Completed`.
- Unchecking a task in `Completed` moves it automatically back to `Pending`.
- You can also drag and drop one or multiple tasks directly between `Pending`, `In Progress`, and `Completed` tabs in any quadrant.

Colors assist identification: `Pending` and `In Progress` tabs use priority-specific colors, while `Completed` tabs use blue.

You can also sort tasks by:

- date;
- time;
- alphabetical order.

Each criterion can be ascending or descending. When more than one criterion is active, the menu shows the sorting priority.

## 10. Visible panels

Under `Settings -> Task Layout -> Show Panel` or `Show Panels`, you can control:

- `Calendar`;
- `My Tasks`, when Feynman is available;
- `In Progress`, when Pomodoro is available.

Shortcuts:

- `Ctrl+M`: show or hide calendar.
- `Ctrl+Shift+T`: show or hide `My Tasks`, when Feynman exists.
- `Ctrl+Shift+E`: show or hide `In Progress`, when Pomodoro exists.

## 11. Calendar and global list

The calendar shows tasks linked to dates.

Available filters:

- day;
- week;
- month;
- year.

The `Global Task List` follows the current filter and lets you review all tasks for the selected period with complete visibility across their lifecycle. It reports tasks across three fundamental states:
- **`[Completed]`**: finished tasks;
- **`[Pending]`**: tasks awaiting execution;
- **`[In Progress]`**: active tasks currently underway (Doing).

The listing maintains a clean, standardized layout showing the scheduled date, time, and priority, without any extraneous pomodoro cycle sub-lines. Calendar date tooltips also display all scheduled tasks for the day categorized by their corresponding status, including `[In Progress]`.

All calendar status labels support runtime dynamic translation: when switching the application language between English and Portuguese, labels update immediately between `[In Progress]` and `[Em Progresso]`, `[Completed]` and `[Concluída]`, `[Pending]` and `[Pendente]`, without requiring an application restart.

Fast interaction:
- **Double-click**: double-clicking any task with a linked external file opens that file directly in the operating system's default application.
- **Context menu**: right-clicking any task in the global list opens the full context menu, allowing you to edit, complete, delete, color, or access 4D BIM Gantt Schedule integration actions.

Tasks created by Activity Registration also appear in this list when they have a linked date. If `Colors -> Colored` is active in Activity Registration, the global list preserves the task color; if there is a linked file, the title also receives linked-file visual emphasis.

You can also remove one or more tasks directly from the `Global Task List`. When you use the context menu in that list, the application locates the original task in its source quadrant and removes the real item, keeping the calendar, storage, and synchronized modules consistent.

In the monthly filter, the calendar can switch between month view and year view to make navigation easier.

## 12. Reserved space and watermark

Under `Settings -> Reserved Space`, you can enable or disable visual resources by module.

Options may include:

- `Title`;
- `Watermark`;
- `Font Settings`;
- `Color Tasks`.

These resources help visually separate groups, dates, times, and application areas. The displayed options depend on the loaded modules.

To color tasks:

1. Select one or more tasks in Eisenhower, Pomodoro, or Feynman.
2. Open `Settings -> Reserved Space -> Color Tasks`.
3. Choose the desired color in the color window.
4. Confirm the choice.

The color is applied to the selected tasks and remains visible in lists, calendar views, and synchronized modules when the task is shown again. If the task also has a linked file, the title still receives its linked-file emphasis while preserving the font color whenever possible.

To configure task fonts:

1. Open `Settings -> Reserved Space -> Font Settings`.
2. Choose `General Tasks` for tasks without a linked external file.
3. Choose `Linked File` for tasks with a linked external file.
4. In the window, choose font family, style, size, and the bold, italic, underline, or strikeout options.
5. Use `Restore` to return that category to its default, `OK` to save, or `Cancel` to discard.

The settings are independent. If you change `General Tasks`, only tasks without linked files change. If you change `Linked File`, only tasks with linked external files change. The update applies to Eisenhower lists, the global calendar list, and tasks shown in Pomodoro and Feynman when those plugins are available.

## 13. Synchronization between modules

Under `Settings -> Sync`, you control updates between modules.

The main sharing flow happens between:

- Eisenhower;
- Feynman;
- Pomodoro.

Common flows:

- plan a task in Eisenhower and execute it in Pomodoro;
- turn a study task into a Feynman concept;
- return a focus item to Eisenhower;
- review concepts and send tasks for execution.
- register an assessment activity in Activity Registration and turn it into a task for Eisenhower, Pomodoro, and Feynman when those methods are synchronized.

Mind Map and Audiobook have their own data, but integrate with the host through tab, menu, language, and counter.

## 14. Productivity tools

The `Tools` menu gathers operational resources that use the unified task list and, when available, plugin data. Before opening these tools, the application saves the current state so it can work with the latest records.

### 14.1 Global search

Use `Tools -> Global Search` or `Ctrl+Shift+G`.

Global search looks through:

- Eisenhower tasks;
- Feynman concepts;
- Pomodoro items;
- Mind Map nodes and records;
- text currently open in Audiobook, when the plugin exposes content to the host.

The search considers title, description, module, date, time, priority, mastery level, linked file, and integration fields. Use it when you remember a word, file, subject, or date but do not know which tab contains the item.

### 14.2 Today / Next 7 days

Use `Tools -> Today / Next 7 days` or `Ctrl+Shift+H`.

This panel shows items with a valid date between the current date and the next seven days. It is useful as a short execution list, deadline review, and near-term check. Undated items do not appear here; to find them, use global search or open the corresponding module.

### 14.3 Per-item recurrence

Right-click a task and choose `Recurrence`.

You can set:

- `No recurrence`;
- `Daily`;
- `Weekly`;
- `Monthly`;
- optional end date.

Per-item recurrence does not create copies immediately. It stores the rule on the item so `Generate Next Recurrences` can create upcoming occurrences in batch.

### 14.4 Generate next recurrences

Use `Tools -> Generate Next Recurrences`.

The dialog lets you choose:

- source and destination methods: Eisenhower, Pomodoro, Feynman, and Mind Map;
- whether to use all filtered items or only selected items;
- search for specific items, with actions to select visible items or clear the selection;
- range in days;
- maximum occurrences per item;
- whether completed items are included;
- duplicate prevention;
- default frequency for items without a defined recurrence;
- start/end date and time for the application period;
- reference date and time for items without a date or time when the missing-time option is enabled.

The preview area shows which items will be considered, where new occurrences will be created, which frequency will be used, and which next dates will be applied.

Important buttons:

- `OK`: saves dialog settings without generating occurrences.
- `Apply`: saves settings and creates occurrences.
- `Restore defaults`: resets the dialog options to their initial values.
- `Restore Original`: when available, undoes the latest recurrence generation and restores the previous state.

When an item is shared between modules, the new occurrence is created in the selected modules where the item already exists. The application tries to avoid repeated copies when `Avoid existing duplicates` is enabled.

### 14.5 Local reminders

Use `Tools -> Enable Local Reminders`.

Reminders are local application timers. They work while the application remains open; they do not replace an operating-system scheduler.

In the dialog, you can:

- select source methods;
- use all filtered items or only selected items;
- search and select specific items;
- include or ignore completed items;
- set `Maximum`, meaning how many repeated alerts fire for each scheduled time;
- use the item date/time or a manually chosen date/time;
- define a quick period in days, weeks, or months;
- use a default time when a task has a date but no time;
- show an in-app alert;
- show a system-tray alert when the tray is available.

When multiple items fall on the same date/time, the application groups them into one alert with the item list. If `Maximum` is greater than 1, that alert is repeated in sequence for the same time.

Important buttons:

- `Test`: fires a test reminder.
- `Restore defaults`: resets the dialog options to their initial values.
- `Remove active`: cancels reminders already scheduled in the current session.
- `OK`: saves preferences without scheduling.
- `Apply`: saves preferences and schedules the possible reminders.

If the system tray is not available, use `Alert in app`.

### 14.6 Export calendar .ics

Use `Tools -> Export Calendar .ics`.

The application generates an iCalendar file with tasks that have a valid date. When the task also has a time, the event gets a start time and a default duration of 30 minutes. Tasks without time are exported as all-day events. Tasks without a date are left out.

Use this feature to import deadlines into `.ics`-compatible applications, such as personal calendars, email clients, and scheduling tools.

### 14.7 Productivity history

Use `Tools -> Productivity History`.

The history summarizes:

- completed tasks by week;
- pomodoro count recorded in shared data;
- Feynman concepts with review text;
- number of Mind Map nodes;
- estimated reading minutes from the text currently open in Audiobook.

This report is informational. It helps review progress, but depends on fields saved by the available modules.

### 14.8 Integrated study flow

Use `Tools -> Integrated Study Flow`.

When text is open in Audiobook, that text is used as the source. If no text is available, the application asks for a passage manually.

After confirmation, the flow creates:

- a Feynman concept with the passage as its explanation;
- an Eisenhower task to review the subject two days later;
- a Mind Map node with the title and passage notes.

Use this flow to turn reading into practical review without recreating the same content in three modules.

### 14.9 Templates

Use `Tools -> Templates`.

The available templates create starter tasks for:

- study;
- work;
- review;
- reading;
- weekly planning.

After inserting a template, edit dates, priorities, mastery levels, and descriptions according to your routine. Templates are starting points, not rigid plans.

### 14.10 Spaced reviews

Use `Tools -> Create Spaced Reviews`.

The tool uses the active tab as its source. It reads the found items, calculates intervals from mastery level, and prepares future reviews.

Default intervals:

- beginner: review in 1 day;
- intermediate: review in 3 days;
- advanced: review in 7 days.

In the dialog, you can:

- adjust intervals by mastery level;
- set a default mastery level for items without one;
- keep the original priority or force a review priority;
- set a default priority for items without one;
- choose whether review dates should appear in calendar and panels;
- include or ignore completed items.

The spaced-review window lets you review the list before transfer:

- edit a review by double-clicking or using `Edit in window...`;
- remove items that should not enter the round;
- choose targets in Eisenhower, Pomodoro, Feynman, and Mind Map;
- use `Transfer to selected tabs` to create the records.

When available, `Undo` restores the state before the latest transfer.

### 14.11 Plugin manager

Use `Tools -> Plugin Manager`.

This panel shows:

- plugin name;
- load status;
- declared version;
- found path;
- load error, when present.

Use `Open folder` to inspect the selected plugin location. The window follows runtime language changes and can move behind the main window when you return to regular application work.

Use Plugin Manager to answer questions such as:

- which plugins were found;
- which plugins loaded correctly;
- which version of each plugin is available;
- where each plugin was found;
- which error occurred when a plugin could not load.

To open or monitor log files, use `Tools -> Log Manager`.

### 14.12 Log manager

Use `Tools -> Log Manager`.

This panel lists `.log` files found in the persistent logs folder, can open the selected log or its folder, and keeps a real-time monitor. New records appear in chronological order, oldest first and newest last, using each line timestamp.

In daily use:

1. Open `Tools -> Log Manager`.
2. Select a log file in the list.
3. Use `Open log` to open the selected file in the system's default viewer.
4. Use `Open folder` to open the directory where logs are stored.
5. Leave the window open while using the application to follow new records in the monitor.

The monitor updates while the application is running. This helps confirm when an action finished, when a plugin loaded, or when a problem was recorded.

The main log files follow the active product:

- Eisenhower Organizer: logs starting with `file_eisenhower`.
- Lúmen: logs starting with `file_lumen`.

Log Manager also follows runtime language changes and can move behind the main window. This lets you keep diagnostics open without blocking normal use.

## 15. Feynman module

Feynman is focused on study and review.

It allows you to:

- create concepts;
- write a simple explanation;
- register knowledge gaps;
- create a final review;
- set priority;
- set mastery level;
- link a file;
- select multiple concepts;
- edit tasks from the `My Tasks` panel with a success confirmation;
- share items with Eisenhower and Pomodoro.

The `My Tasks` panel can be opened or closed from the layout menu when the plugin is available.

### 15.1. Delete tasks in Feynman

The `My Tasks` list context menu also lets you remove tasks directly.

To delete one task:

1. Right-click directly on the task.
2. Choose `Delete Task`.
3. Confirm the removal in the window that appears.

To delete multiple tasks:

1. Select the desired tasks with `Ctrl`, `Shift`, or normal list selection.
2. Right-click any task in the selected group.
3. Choose `Delete Selected Tasks`.
4. Review the list of items that will be removed and confirm.

If you right-click a task that was not selected yet, Feynman selects that task before opening the menu. If you right-click a task that is already part of a multiple selection, the selected group is preserved. This makes both individual and batch removal more predictable.

Confirmation, success, and error messages use task wording and follow the active application language.

### 15.2. Edit tasks in Feynman

To edit a task shown in the `My Tasks` panel:

1. Right-click the task.
2. Choose `Edit Task`.
3. Change the needed information: title, description, priority, mastery level, date, time, linked file, Pomodoro cycles completed (🍅), cycle goal (🎯), and estimated duration in days.
4. Click `OK`.
5. Wait for the success message and click `OK` to close it.

The confirmation says that the changes were applied and follows the active application language. Timeline and cycle metrics propagate automatically to synchronized tasks in Eisenhower and Pomodoro.

### 15.3. Gantt Schedule Integration in Feynman

The context menu for the `My Tasks` list and study concepts also provides schedule integration shortcuts:
- `📊 Create Activity in Gantt`: creates a new schedule activity linked to the study concept or task.
- `📊 Link to Gantt Schedule...`: binds the study item to an existing WBS schedule activity.
- `📊 Open in Gantt Schedule`: opens the schedule view highlighting the corresponding activity.
- `📊 Record Field Progress...`: logs progress measurements and site notes.
- `📊 Change Gantt Link...` and `📊 Unlink from Gantt`.

## 16. Pomodoro module

Pomodoro is focused on execution. It combines a timer, task list, and tracking controls to help you work in focus and break cycles.

It allows you to:

- organize in-progress tasks;
- run focus cycles;
- use breaks;
- track tasks during a session;
- configure alarm;
- choose between a simple interface and a robust interface;
- switch layouts at any time without changing saved tasks;
- edit tasks with success confirmation, focus cycles, and goals;
- detect discrepancies between task cycle goals and timer settings;
- synchronize durations and record progress in the Gantt schedule;
- share items with Eisenhower and Feynman.

The `In Progress` panel can be opened or closed from the layout menu when the plugin is available.

### 16.1. How to open Pomodoro

You can open the module through:

- `Settings -> Plugins -> Pomodoro -> Open Pomodoro`;
- shortcut `Ctrl+Shift+P`;
- main `Pomodoro` tab, when it is visible.

### 16.2. Choose the interface layout

Pomodoro has two display modes:

- `Simple`: keeps a more compact organization, with the timer (`⏱️ Timer`), settings, and task list in a direct layout.
- `Robust`: uses a more complete organization, with the timer (`⏱️ Timer`), task board, status tabs, and the side `In Progress` panel.

To switch:

1. Open `Settings -> Task Layout -> User Interface -> Pomodoro`.
2. Choose `Simple` or `Robust`.
3. The application applies the selected layout, opens Pomodoro, and shows a warning message confirming which layout was applied.
4. You can change the layout whenever you need; it only changes the visual arrangement.

Your choice is saved and reapplied the next time you use the application.

### 16.3. Robust layout

In the `Robust` layout, the timer frame is standardized with the label `⏱️ Timer` (with dynamic runtime translation to `⏱️ Temporizador` in Portuguese), and the `Task Board` area was reorganized to make actions easier to find.

Below the task text field, the first row contains:

- `Priority:` label;
- priority drop-down list;
- `Mastery Level:` label;
- mastery level drop-down list;
- `Completed Tasks Flow` checkbox;
- `Completed Tasks Flow` label.

The second row contains the buttons:

- `Add`;
- `Move Task(s)`;
- `Remove Task(s)`;
- `Reset Count`.

Use this layout when you want to track more information at the same time, especially during study or work sessions with several tasks.

Status tabs help separate task progress, while the side panel keeps track of what is currently running.

### 16.4. Simple layout

In the `Simple` layout, Pomodoro keeps a more direct view with the `⏱️ Timer` frame and task list. It is useful when you want fewer elements on screen or when using a smaller window.

You can switch back to the `Robust` layout at any time using the same settings menu path.

### 16.5. In Progress panel

In the `Robust` layout, the `In Progress` panel appears beside the main area and shows tasks currently being executed. You can collapse or expand this panel to gain space during focus. The application remembers your latest visibility choice.

You can also show or hide this panel under `Settings -> Task Layout -> Show Panel/Show Panels -> In Progress` or with the `Ctrl+Shift+E` shortcut when the plugin is available.

Alarm settings:

- open `Settings -> Plugins -> Pomodoro -> Alarm`.
- choose the alert sound;
- test the preview before saving;
- adjust the alarm repeat interval;
- save to apply the setting.

### 16.6. Edit tasks in Pomodoro

To edit a task in Pomodoro:

1. Right-click the task.
2. Choose `Edit Task`.
3. Adjust text, description, priority, mastery level, date, time, linked file, Pomodoro cycles completed (🍅), cycle goal (🎯), and estimated duration in days.
4. Click `OK`.
5. The confirmation responds and closes the dialog on the spot without freezing, while data saving and cross-module synchronization proceed asynchronously in the background.

The notice confirms that the task was updated and follows the active application language in real time.

### 16.7. Pomodoro Cycle Mismatch Detection (CycleMismatchDialog)

When clicking the `Start` button to begin a focus session, the timer verifies all tasks currently in the `In Progress` (Doing) list or column:

- **Automated Check**: if any task has a cycle target (🎯) differing from the timer's current active cycle setting (⚙️), the system immediately presents the `CycleMismatchDialog`.
- **Comparison Table**: displays an informative discrepancy table showing task titles, each task's configured cycle target, and the timer's active cycle setting.
- **Resolution Choices**:
  1. `Reset All`: resets cycle counts across all in-progress tasks to default, synchronizes immediately with the Eisenhower Matrix and persistent store via the host integration bridge (`reset_in_progress_task_cycles`), and starts the timer.
  2. `Start Anyway`: starts the timer immediately without altering individual task cycle goals.
  3. `Cancel`: aborts timer start without modifying tasks, allowing the user to review settings before focusing.

### 16.8. Gantt Schedule Actions in Pomodoro

Pomodoro task lists (`gtar_14`) also feature direct 4D BIM Gantt Schedule integration in their context menus (right-click):
- `📊 Create Activity in Gantt`;
- `📊 Link to Gantt Schedule...`;
- `📊 Open in Gantt Schedule`;
- `📊 Record Field Progress...`;
- `📊 Change Gantt Link...`;
- `📊 Unlink from Gantt`.

## 17. Mind Map module

Mind Map appears when its plugin is loaded.

It allows you to:

- create visual concept nodes;
- connect concepts;
- change colors;
- edit title, notes, and linked file;
- open linked files;
- remove concepts;
- clear the map;
- save and load maps as JSON;
- export the view as PNG;
- import documents to generate an initial structure;
- reorganize the map by layout.

Layouts available under `Settings -> Plugins -> Mind Map`:

- vertical;
- horizontal;
- radial;
- tree.

Assisted import accepts documents such as PDF, EPUB, DOC, DOCX, TXT, MD, CSV, XLSX, JSON, SQLite, and XML.

When the `Model` module is available, navigate to `Settings -> Specific Settings -> Mind Map` to configure the node summarization method:

- `Heuristic method summary`: fast, deterministic local analysis without running language models. Detects headings, chapters, concepts, and relations and creates extractive summaries; recommended for well-structured documents, computers with limited memory, or when speed is the priority.
- `Summary with qwen2.5-3b model [Installed]`: executes the official local Qwen2.5-3B-Instruct model (GGUF via llama.cpp) to produce deep contextual summaries and semantic concept hierarchies.
- `Summary with qwen2.5-3b model [Not installed]`: indicates that the model file has not yet been downloaded to the computer. To download it on-demand with interactive assistance, click `How to download or manage Qwen model (GGUF)...`.
- `How to download or manage Qwen model (GGUF)...`: opens the interactive Local Qwen 2.5 (GGUF) Model Manager dialog, allowing you to start the official download (~2.17 GB), pause, resume via HTTP Range, delete to reclaim disk space, or open the models folder in Windows Explorer.
- `Ollama Models`: submenu that automatically detects and lists all language models installed locally in Ollama (Llama 3.2, Qwen 2.5, Gemma 2, Mistral, etc.).
- `How to install Ollama models...`: opens the interactive guide with recommended models, copyable terminal installation commands, and local server connection status (`localhost:11434`).

After selecting a method, open Mind Map, click `Import document`, select the file, and follow progress across text extraction, analysis, and node assembly in the status dialog. When finished, review the central node, titles, summaries, and relationships; edit or remove irrelevant items, then save as JSON or export as PNG. Models run strictly locally; if the selected model encounters unreadable content, syntax issues, or memory limits, the application records the event and seamlessly engages the heuristic engine to ensure import succeeds.

Mind Map also has automatic backup. If the application closes, the last state can be restored when the plugin loads again.

## 18. Audiobook module

Audiobook appears when its plugin is loaded.

It allows you to:

- open and edit text;
- save text;
- open PDF;
- drag files to the text or PDF area;
- read text aloud;
- read PDF aloud;
- pause, resume, and stop reading;
- adjust speed;
- adjust volume;
- choose voice;
- configure font;
- search in text;
- search in PDF;
- highlight found terms;
- select and copy PDF text;
- use a focus ruler;
- follow word, sentence, or paragraph highlight during reading.

Audiobook settings:

- `Settings -> Plugins -> Audiobook -> Font Settings`;
- `Settings -> Plugins -> Audiobook -> Voices`.

Note: speech reading may depend on the selected voice, the available speech service, and the environment connection.

## 19. Agenda and Activity Registration Module

Activity Registration appears when its plugin is loaded. It does not replace Eisenhower; it works as a registration window that turns academic assessment activities into organized tasks.

You can open Activity Registration through:

- `Tools -> Activity Registration`;
- `Settings -> Plugins -> Activity Registration`.

When the plugin is not available, these paths do not appear in the application and the internal manual does not show Activity Registration instructions.

Inside the Activity Registration window, a dedicated settings menu lets you switch to the `Civil Engineering` example, create new agendas, and manage custom agendas without changing plugin availability in the main application.

### 19.1. Activity Registration window

The registration window organizes the activity information into two sides.

On the left side, the drop-down lists are:

- `Course:`;
- `Syllabus:`;
- `Semester:`;
- `Subject:`;
- `Subject Class:`;
- `Assessment Activity Type:`;
- `Activity Sequence:`;
- `Priority:`;
- `Mastery Level:`.

On the right side, you will find:

- `Set the Activity Date:`;
- calendar;
- `Link date`;
- `Link time`;
- time selection;
- `Linked file (drag or choose):`;
- `Choose...`, `Open`, and `Remove` buttons for the linked file;
- `Register Definitions` button;
- `Clear` button;
- `Close` button.

The window is kept compact and does not use a maximize button. When the main application window closes, the Activity Registration window is also closed. The relationship between the main window and Activity Registration follows the behavior of the other product windows: the active window moves to the front and the other one stays behind.

Linked file:

- use `Choose...` to select any external file that helps with the activity;
- drag a file from Windows directly onto the linked-file field;
- use `Open` to open the selected file in the system's default application;
- use `Remove` to remove only the link, without deleting the file from the computer.

Activity Registration stores the file path in the created task. The file stays in its original location; if it is moved, renamed, or deleted, edit the task and link the file again.

### 19.2. Civil Engineering example and custom agendas

The `Settings -> Agenda Example -> Civil Engineering` menu opens the standard academic agenda model, with course, syllabus, semester, subject, class, activity type, and sequence lists.

To create your own agenda:

1. Open `Settings -> Create your agenda -> New title for your agenda`.
2. Type the desired name, for example `Universal Schedule`.
3. Press Enter.
4. The new agenda appears in the `Settings` menu.

To open a custom agenda, use `Settings -> Agenda Name -> Open agenda`. Hovering over the menu does not open the agenda.

Each custom agenda lets you create its own drop-down lists. To do that:

1. Enable the circular control on the next available row.
2. Type the new list title in the `Enter a title for the new list` field.
3. Press Enter to turn the field into a label.
4. Use the created drop-down list to enter or select values for that category.

The next list appears only after the previous one is created. This keeps the window from filling with unused empty lists.

### 19.2.1. Nested lists in custom agendas

Custom agendas can also work like a tree of lists. This is useful when one choice should filter the next one, for example:

- `Course` -> `Semester`;
- `Semester` -> `Subject`;
- `Area` -> `Project`;
- `Project` -> `Stage`;
- `Client` -> `Contract`.

To create a nested list:

1. Create the first list and add its main items.
2. Create the next list.
3. When Activity Registration asks whether to nest the items in this list under the previous list, choose `Yes`.
4. Type an item in the new list.
5. When confirming the item, choose which item from the previous list it belongs to.
6. Repeat the process for the remaining items.

After that, the child list follows the previous list selection. If you choose a parent item, the next list shows only the items linked to it. If you change the parent item, the child list updates to show the matching set.

Simple example:

1. In the `Course` list, add `Civil Engineering` and `Architecture`.
2. Create the `Semester` list as a nested list.
3. Add `1st Semester` linked to `Civil Engineering`.
4. Add `Integrated Project` linked to `Architecture`.
5. When you select `Civil Engineering`, the `Semester` list shows the items linked to that course.

You can create more than one layer. A child list can also have another dependent list immediately after it, forming an organized sequence of choices.

To manage a custom agenda:

- `Open agenda`: activates the selected agenda.
- `Edit agenda`: renames the agenda.
- `Remove agenda`: removes the custom agenda.
- `Edit title`: changes a list label in the active agenda.
- `Remove title`: removes only the selected label/list while keeping the other lists.

The last opened agenda is remembered. If you last worked with `Civil Engineering`, the Schedule returns to that model when reopened. If the latest agenda was a custom one, it is loaded the next time the window opens.

### 19.3. How to register an activity

To register an assessment activity:

1. Choose the course.
2. Choose or register the syllabus.
3. Choose the semester.
4. Choose the subject.
5. Choose the class.
6. Choose the assessment activity type.
7. Choose the activity sequence.
8. Set the priority.
9. Set the mastery level.
10. Enable `Link date` if the activity should appear in the calendar.
11. Enable `Link time` if the task should have a time.
12. Click `Register Definitions`.

Optionally, before registering, link an external file with `Choose...` or drag the file onto the `Linked file` field. The link follows the created task and can be opened later from the screens that display the task.

The default priority is `Important and Urgent`. The `Link time` checkbox opens enabled by default. The `Clear` button clears the current selections and restores the standard `Priority` and `Mastery Level` options without deleting the available lists.

In custom agendas, fill the lists you created and keep the standard priority, mastery, date, time, and action buttons. Values typed into custom lists are saved in the corresponding agenda.

When a custom agenda uses nested lists, fill the lists in the order they appear. First choose the item in the previous list; then choose or register the item in the next list. If a typed item is not linked to a parent item yet, Activity Registration asks for that link before completing the registration.

### 19.4. How Activity Registration creates tasks

When an activity is registered, Activity Registration creates a task in the product's unified storage.

The task description receives only the academic registration data:

- course;
- syllabus;
- semester;
- subject;
- class;
- assessment activity type;
- activity sequence.

Priority, mastery level, date, and time are not repeated inside the description because they already have their own task fields and visual presentation in the interface.

When synchronization is active, the task may appear in:

- Eisenhower;
- Pomodoro;
- Feynman.

In Eisenhower, the priority defines the quadrant. In Pomodoro and Feynman, the task is displayed according to synchronization and each plugin's rules.

### 19.5. Activity Registration colors

Under `Settings -> Activity Registration -> Colors` (on the main menu bar) and under `Settings -> Colors` (on the registration window's menu bar), you can choose the color behavior per module or for all modules simultaneously.

The menu structure is organized into submenus:

- `Black`:
  - `Eisenhower`: sets black font (in light theme) or white font (in dark theme) only for tasks in the Eisenhower Matrix and Calendar.
  - `Pomodoro`: sets black font (in light theme) or white font (in dark theme) only for tasks in the Pomodoro board.
  - `Feynman`: sets black font (in light theme) or white font (in dark theme) only for concepts in the Feynman method list.
  - `General`: applies Black mode to Eisenhower, Pomodoro, and Feynman simultaneously.
- `Colorful`:
  - `Eisenhower`: applies subject/category highlight color only to tasks in the Eisenhower Matrix and Calendar.
  - `Pomodoro`: applies subject/category highlight color only to tasks in the Pomodoro board.
  - `Feynman`: applies subject/category highlight color only to concepts in the Feynman method list.
  - `General`: applies Colorful mode to Eisenhower, Pomodoro, and Feynman simultaneously.

When `Colorful` mode is active for a module, tasks keep their subject color in that environment. If the task has a linked file, the title also receives linked-file visual emphasis without discarding font color whenever possible.

In custom agendas, when `Colorful` is active, Activity Registration alternates highlight colors and avoids immediately repeating the last color used for a newly created task.

Color mode changes are applied at runtime and immediately update visible tasks without restarting the application.

Older tasks created in previous versions also follow these per-module color configuration rules.

### 19.6. Delete items from lists

Use `Settings -> Plugins -> Activity Registration -> Delete item` to remove entries registered in the drop-down lists.

The `Delete item` menu follows the active agenda in real time. If `Civil Engineering` is open, it shows the academic model lists. If a custom agenda is open, the main application's menu bar dynamically reconstructs the submenus to reflect exactly the titles and lists created for that agenda.

You can also delete an entire title/list from a custom agenda via `Settings -> Plugins -> Activity Registration -> [Agenda Name] -> Remove title`.

This feature is meant to correct entries created by mistake or clear options that will no longer be used. It should not be used to delete already registered tasks; tasks should be removed through the normal Eisenhower, Pomodoro, or Feynman workflow.

In custom agendas with nested lists, deletion respects the relationship between items. When an item from a previous list is removed, dependent links are also updated to avoid options without a valid connection.

### 19.7. Activity Registration persistence

Activity Registration uses the persistent folder of the product currently open:

- Eisenhower Organizer: `%LOCALAPPDATA%/EisenhowerOrganizer`;
- Lúmen: `%LOCALAPPDATA%/Lumen`.

Files used:

- `listas.db`: custom lists for course, syllabus, semester, subject, class, type, and sequence.
- `agenda_config.json`: Activity Registration color preference and last opened agenda.
- `agendas/*.json`: custom agendas created by the user, including custom lists and relationships between nested lists.
- `tarefas.json`: tasks created and shared with Eisenhower, Pomodoro, and Feynman.

The legacy `AgendaAvaliacoesAcademicas` directory should not be used by the plugin inside the current product.

### 19.8. Good practices

- Register subjects with consistent names to make better use of colors.
- Use sequence when there is more than one activity of the same type.
- Link a date for activities with a real deadline.
- Link a time when there is a delivery, exam, or presentation at a defined time.
- Review the task in Eisenhower after registration to confirm quadrant, description, and synchronization.

## 20. Gantt Schedule & 4D BIM Engineering Module

The Gantt Schedule & 4D BIM Engineering module (available via the Gantt plugin) equips the platform with professional capabilities for project scheduling, construction management, and physical milestone tracking by integrating traditional Gantt charts with 4D BIM planning principles.

### 20.1. How to access

You can access the Gantt Schedule in the following ways:

- **Tools Menu**: select `Tools -> 4D BIM Gantt Schedule`.
- **Keyboard Shortcut**: press `Ctrl+Alt+G` at any time while the main window is focused.
- **Task Context Menu**: in the Eisenhower Matrix, Pomodoro, or Feynman, right-click any selected task and select `📊 Link to Gantt Schedule...` or `📊 Create Activity in Gantt`.

### 20.2. Interface structure

The Gantt Schedule window is split into two synchronized primary views:

1. **Activity Table (WBS)**: located on the left, displays project stages and activities in a hierarchical tree view. Shows WBS code, activity name, start date, finish date, duration in days, progress percentage, and status.
2. **Interactive Gantt Chart**: located on the right, projects graphical activity timeline bars across a continuous calendar grid, highlighting start dates, finish deadlines, contractual milestones, and dependency linkages.

### 20.3. Activity and milestone management

- **Creating Activities**: define activity name, WBS hierarchical code, planned start and finish dates, and estimated duration in days.
- **Milestones**: activities configured with zero duration represent project milestones, denoting decisive events such as permits, contractual handovers, or inspections.
- **Progress Tracking**: enter completion from 0% to 100%. The activity status updates automatically to `Not Started` (0%), `In Progress` (1% to 99%), or `Completed` (100%), filling the chart bar proportionately.

### 20.4. Dependencies and Critical Path

- **Dependency Link Types**: establish precedence relationships between activities:
  - *Finish-to-Start (FS)*: successor activity can only begin once the predecessor finishes.
  - *Start-to-Start (SS)*: both activities start concurrently.
  - *Finish-to-Finish (FF)*: both activities finish concurrently.
  - *Start-to-Finish (SF)*: predecessor start controls successor finish.
- **Lags and Leads**: specify buffer delays or lead overlaps between dependent activities.
- **Critical Path Engine**: automatically identifies the sequence of zero-float activities that directly governs total project duration. Critical path items receive distinct visual highlighting, enabling proactive management against schedule slip.

### 20.5. Timeline zoom and navigation

- **Zoom Levels**: switch between `Day`, `Week`, `Month`, and `Year` timescales.
- **Navigation Controls**: scroll smoothly across the timeline horizontally using scrollbars or the mouse wheel.
- **Working Calendar**: visually demarcates working days from non-working weekends and scheduled project holidays.

### 20.6. Multi-Module Bidirectional Synchronization

The integration bridges strategic project planning with the tactical daily workflows of Eisenhower, Pomodoro, and Feynman:

- **Direct Linking**: connect operational tasks to schedule activities.
- **Automated Status Sync**: completing a linked task in Eisenhower or Feynman recalculates progress on the associated Gantt activity. Moving a task to `In Progress` (or executing it in Pomodoro) marks the schedule activity in progress in real time.
- **Duration Updates**: adjusting task duration estimates updates projected activity completion dates.
- **Data Safety**: removing an operational task unlinks it without deleting the underlying engineering schedule or field measurement records.

### 20.7. Field Progress Tracking and Construction Site Diary (FieldProgressService)

- Log physical completion measurements directly against schedule activities (0% to 100%).
- Record technical site notes, deviation rationales, inspection reports, and photos in the construction diary.
- Full auditability capturing the inspecting engineer's name and measurement timestamp.
- Visually evaluate the original baseline schedule against actual field performance.

### 20.8. 4D BIM Simulation Panel (Bim4DSimulationPanel)

- **Step-by-Step Construction Simulation**: visually simulates the timeline progress of the project through `Bim4DSimulationPanel`.
- **Playback Controls**: Play, Pause, Reset buttons and a continuous timeline slider with dynamic simulation speed adjustments.
- **BIM Linkage**: visual synchronization between engineering model entities and scheduled project phases.

### 20.9. Data Exchange: MS Project XML and CSV Spreadsheets

- **MS Project XML Connector (`MsProjectXmlConnector`)**:
  - Full import of projects created in Microsoft Project, preserving WBS hierarchy, durations, dates, and predecessors.
  - Faithful export of local schedules to MS Project-compatible XML format for external collaboration.
- **CSV Exchange Service (`CsvExchangeService`, `CsvMappingDialog`)**:
  - Tabular CSV schedule import with interactive column mapping (Name, Start, Finish, Duration, Progress, WBS).
  - Clean CSV export for spreadsheet analysis.
- **Eisenhower Task Importer (`EisenhowerTaskImportDialog`)**:
  - Dedicated dialog to select pending and in-progress tasks from the Eisenhower Matrix and convert them into schedule activities with a single click.

### 20.10. Work Calendars, Working Days, and Holidays

- **Weekly Working Days (`GanttWorkingDaysDialog`)**: custom configuration of workable days of the week (Monday through Friday, full/half Saturdays, shift settings).
- **Holidays and Exceptions (`GanttHolidaysDialog`)**: registry of national, municipal, and site-specific non-working holidays.
- **Realistic Deadline Calculation**: the CPM planning engine automatically factors in weekends and holidays when calculating finish dates from working-day durations.

### 20.11. Audit Trail, Embedded REST API Server, and Webhooks

- **Audit Service (`AuditService`, `AuditLog`)**: comprehensive tracking of all schedule modifications, logging user, timestamp, modified field, old value, and new value.
- **Embedded HTTP REST Server (`HttpServer`, `RestApiController`)**: local REST API for seamless integration with corporate ERPs, BI dashboards, and web services.
- **Webhook Dispatcher (`WebhookDispatcher`)**: automatic asynchronous HTTP POST notifications to external endpoints upon activity creation, update, or completion events.

### 20.12. Schedule planning best practices

- Establish the full WBS hierarchy before binding daily operational tasks.
- Prevent circular dependencies where tasks directly or indirectly depend on their own output.
- Monitor critical path activities closely to safeguard the overall delivery deadline.
- Log field progress regularly to keep completion forecasts aligned with job-site reality.
- Use the Eisenhower Matrix for daily triage and the Gantt Schedule for long-term project control.

## 21. Local AI Qwen Model Module

The Model Module brings state-of-the-art semantic and generative AI capabilities to EISENHOWER ORGANIZER / LÚMEN, enabling document analysis, automated summarization, and concept extraction while remaining 100% offline and private. It combines local inference for the Qwen2.5 model family (quantized in GGUF format via llama.cpp) with native compatibility for models installed through Ollama.

### 21.1. What it is and How it Works
- **Offline Local Execution**: the inference engine uses optimized quantized models from the Qwen2.5 family (`qwen2.5-3b-instruct-q4_k_m.gguf`) running directly on your CPU via the high-performance llama.cpp engine.
- **Absolute Privacy**: no text, imported documents, or generated concepts are ever uploaded to cloud servers or internet endpoints. Your intellectual property stays entirely on your local machine.
- **Lightweight On-Demand Architecture**: to keep the initial application download fast and compact, heavy model binaries are not pre-bundled. You have complete autonomy to download the official Qwen model on demand or use models already installed in Ollama.
- **Zero Subscriptions or Accounts**: once downloaded, local inference operates entirely offline without requiring API keys, cloud accounts, or third-party service fees.

### 21.2. On-Demand Local Qwen 2.5 (GGUF) Model Manager
The application includes a dedicated interactive dialog for managing the lifecycle of the local Qwen model:

1. **How to open the Manager**:
   - In the Mind Map menu, choose `Settings -> Specific Settings -> Mind Map -> How to download or manage Qwen model (GGUF)...`.
2. **Displayed Information**:
   - **Current Status**: indicates whether the model is *Installed*, *Not installed*, *Downloading*, *Paused*, or *Canceled*.
   - **Expected Size**: details the expected disk footprint (~2.17 GB).
   - **Storage Directory**: shows the exact local target path (`%LOCALAPPDATA%\EisenhowerOrganizer\models` or the Lumen equivalent).
3. **Interactive Controls and Features**:
   - **Download Model button**: connects directly to the official Hugging Face repository to download `qwen2.5-3b-instruct-q4_k_m.gguf`.
   - **Progress Bar with Metrics**: visual progress bar reporting percentage complete, transferred volume (e.g. 650 MB / 2.17 GB), real-time transfer speed (MB/s), and estimated time remaining.
   - **Pause button**: pauses the active download at any moment without losing previously downloaded data.
   - **Resume button**: seamlessly continues the download from the exact last byte using HTTP Range requests.
   - **Cancel button**: cancels the active download cleanly and safely.
   - **Delete Model button**: when you want to free up disk storage, click this button to permanently delete the GGUF file after quick confirmation, immediately reclaiming ~2.17 GB of disk space.
   - **Open Folder button**: opens the models folder directly in Windows Explorer for manual inspection or file management.
4. **Dynamic Menu Status Indicators**:
   - The menu `Settings -> Specific Settings -> Mind Map` checks actual file presence on disk and updates menu item labels in real time:
     - `Summary with qwen2.5-3b model [Installed]`: when the file is present and validated for immediate inference.
     - `Summary with qwen2.5-3b model [Not installed]`: when the file is not yet downloaded, guiding the user to the download manager.
5. **Full Dynamic Runtime Translation**:
   - All dialog titles, buttons, prompts, confirmation messages, and tooltips instantly adapt when switching languages between English and Portuguese without restarting the application.

### 21.3. Local Model Integration via Ollama
For users who prefer other open-source models:

1. **Interactive Installation Guide**:
   - Open `Settings -> Specific Settings -> Mind Map -> How to install Ollama models...`.
   - An interactive dialog provides step-by-step instructions for installing Ollama on Windows (`https://ollama.com`).
2. **Recommended Models with One-Click Command Copy**:
   - The guide lists the most popular and efficient models for conceptual mapping:
     - `llama3.2` (lightweight and fast model by Meta);
     - `qwen2.5:3b` (Qwen version via Ollama with excellent reasoning);
     - `gemma2:2b` (compact model by Google);
     - `mistral:7b` (robust model for dense texts);
     - `phi3:mini` (compact high-performing model by Microsoft).
   - Beside each model, dedicated copy buttons place the terminal installation command (e.g. `ollama run llama3.2`) into your clipboard with a single click.
3. **Local Server Connection Test**:
   - A connection test button verifies whether the Ollama background daemon is running and responding on `http://localhost:11434`, reporting live connection status.
4. **Dynamic Discovery Submenu**:
   - All models pulled and running in your local Ollama environment are automatically discovered and listed under `Settings -> Specific Settings -> Mind Map -> Ollama Models`. When new models are pulled in terminal, the menu updates dynamically.

### 21.4. Using Local AI in Mind Maps
Combining local AI with mind mapping transforms unstructured reading materials into clear conceptual structures:

1. Open the **Mind Map** tab or window.
2. From the menu, navigate to `Settings -> Specific Settings -> Mind Map` and choose your preferred method:
   - `Heuristic method summary` (instant and deterministic);
   - `Summary with qwen2.5-3b model [Installed]` (deep semantic analysis via llama.cpp);
   - Any model in the `Ollama Models` submenu.
3. Click **Import document** on the Mind Map toolbar.
4. Select your target document (supported formats: PDF, EPUB, DOCX, TXT, Markdown, CSV, XLSX, etc.).
5. A progress window will show document reading, text chunking, model analysis, and concept node assembly.
6. Once complete, the Mind Map renders a structured conceptual tree with a central concept node, primary topic branches, and organized subtopic nodes.
7. **Smart Heuristic Fallback**: if the selected model is unavailable, encounters memory limits, or the document contains irregular syntax, the system seamlessly falls back to the embedded heuristic extraction engine, guaranteeing uninterrupted mind map creation.

### 21.5. Performance, System Requirements, and Best Practices
- **CPU Multithreading**: the inference engine dynamically allocates worker threads according to available CPU cores for optimal throughput via llama.cpp.
- **Memory Requirements**: at least 4 GB of free system RAM is recommended during 3B parameter model inference for smooth operation.
- **Best Practices**: for extensive books or documents spanning hundreds of pages, importing individual chapters or dedicated sections yields faster turnaround and more focused concept nodes.
- **Workflow Flexibility**: you can freely switch between the heuristic method (instantaneous) and generative AI models based on the nature of your reading material.

## 22. File, import, and export

In the `File` menu, you will find:

- `New`: starts a clean session.
- `Open`: imports or opens supported files.
- `Save`: saves the current state and generates default exports.
- `Save As`: lets you explicitly choose destination and format.
- `Clear`: removes session data with confirmation.
- `Exit`: closes the application.

Formats available for `Save As`:

- JSON;
- CSV;
- XLSX;
- PDF.

The `Save` command generates the default `JSON`, `CSV`, `XLSX`, and `PDF` set using the configured name and destination. If there is no previous destination, the application starts from the user's Documents folder.

Use each format this way:

- `JSON`: best option for preserving structured data and moving records between installations.
- `CSV`: simple option for opening in spreadsheets and reviewing fields as text.
- `XLSX`: spreadsheet option with records in a `records` sheet.
- `PDF`: readable report for consultation, printing, or sharing.

The application's main data file is stored in the user's persistent folder as `tarefas.json`. A successful save also keeps `tarefas.json.bak` as a backup copy. If invalid JSON is detected, the application avoids overwriting the corrupted file and records the issue in the log.

Additional formats:

- Mind Map: JSON and PNG.
- Audiobook: text and PDF.

## 23. Languages

Under `Settings -> Language`, you can switch between:

- Portuguese (Brazil);
- English (United States).

The language change updates the main interface and loaded plugins. The choice is saved for the next launch.

In the `Activity Registration` window, menus, labels, lists, and dialog buttons also follow language changes. This includes buttons such as `Yes`, `No`, `OK`, and `Cancel` in confirmation and selection windows.

The `Plugin Manager` and `Log Manager` windows also update their texts when the language changes. If they are already open, titles, buttons, columns, and messages are refreshed without closing and reopening the windows.

## 24. About and internal notes

Under `Options -> About`, you will find:

- application details;
- history;
- release notes;
- licenses;
- notices;
- privacy policy.

Sections about Feynman, Pomodoro, Mind Map, Audiobook, Activity Registration, and 4D BIM Gantt Schedule appear in the internal notes only when the corresponding plugins are available.

## 25. Useful shortcuts

- `Ctrl+N`: new.
- `Ctrl+O`: open.
- `Ctrl+S`: save.
- `Ctrl+Shift+S`: save as.
- `Ctrl+Q`: exit.
- `Ctrl+M`: calendar.
- `Ctrl+Shift+T`: `My Tasks` panel, when Feynman exists.
- `Ctrl+Shift+E`: `In Progress` panel, when Pomodoro exists.
- `Ctrl+Shift+F`: open Feynman.
- `Ctrl+Shift+P`: open Pomodoro.
- `Ctrl+Shift+N`: open Mind Map.
- `Ctrl+Shift+L`: open Audiobook, when the plugin exists.
- `Ctrl+Shift+R`: open Activity Registration, when Agenda exists.
- `Ctrl+Alt+G`: open 4D BIM Gantt Schedule, when the plugin exists.
- `Ctrl+Shift+Del`: clear.
- `Ctrl+Shift+G`: global search.
- `Ctrl+Shift+H`: `Today / Next 7 days` panel.
- `Ctrl+Shift+A`: about.
- `Ctrl+Shift+M`: manual.
- `Alt+A`: File menu.
- `Alt+I`: Language menu.
- `Alt+O`: Options menu.

## 26. Common issues

### A plugin does not appear

Confirm that the plugin was built, deployment was executed, and the `plugins` folder is next to the application. If the plugin is not found, its tab and menu do not appear.

Open `Tools -> Plugin Manager` to check status, path, and load error. Use `Tools -> Log Manager` to locate and follow session logs.

In Log Manager, look for the most recent log for the active product. For Eisenhower Organizer, look for `file_eisenhower`. For Lúmen, look for `file_lumen`. `INFO` messages usually indicate completed actions; `WARNING`, `ERROR`, and `CRITICAL` messages indicate situations that need attention.

### 4D BIM Gantt Schedule does not appear or open

Verify that the plugin library `plugins/gantt/GanttPlugin.dll` was compiled and deployed to the application plugins directory. The `4D BIM Gantt Schedule` menu item under `Tools` and the `Ctrl+Alt+G` shortcut are activated only when the plugin is successfully loaded.

### Activity Registration does not appear

Confirm that the Activity Registration plugin was built and copied to `plugins/agenda`. The feature appears under `Tools` and `Settings -> Plugins` only when the host finds and loads the plugin.

If Activity Registration appears but a registered task is not colored, confirm that `Settings -> Plugins -> Activity Registration -> Colors -> Colored` is active. Tasks with linked files also receive linked-file visual emphasis, in addition to the font color when available.

### A recurrence did not generate new tasks

Check whether the item is included by the current methods, filter, and selection. It needs a recurrence rule or must be covered by `Default frequency`; if it has no date or time, enable the missing-time option and set the reference date/time. Also confirm that the application period includes the next dates and that `Maximum per item` allows new occurrences. If `Avoid existing duplicates` is enabled, the application may skip occurrences that already exist.

### A local reminder did not appear

Keep the application open until the alert time. Confirm that the methods, filter, and selection produce possible reminders, that at least one alert method is active (`Alert in app` or `System tray`), and that the date/time is not in the past. If the task has a date without a time, enable `Default time if missing`; if the tray is unavailable, use `Alert a in app`.

### Mind Map does not import a document with AI

Check whether the required local AI runtime files and model are available. Without them, assisted import may not generate an automatic structure.

### Audiobook does not speak

Check the selected voice, volume, audio permission, connectivity, and availability of the speech service used by the environment.

### My tasks do not appear in the calendar

Check whether the task has a linked date and whether the calendar filter is set to the correct period.

### The linked file does not open

The path may have been moved, renamed, or deleted. Edit the task or concept and link the file again.

### Tasks or preferences are not saved

Check write permission in the user folder. The application saves data and preferences in a local persistent folder.

## 27. Recommended workflow

1. Plan in Eisenhower.
2. Use the calendar to review deadlines.
3. Use Activity Registration to register exams, assignments, seminars, and academic deadlines when the plugin is available.
4. Utilize the 4D BIM Gantt Schedule (`Ctrl+Alt+G`) to structure project timelines and bind operational tasks to schedule activities.
5. Use `Today / Next 7 days` to choose the next execution.
6. Define recurrences and reminders for repeating or time-sensitive commitments.
7. Send study tasks to Feynman.
8. Execute activities in Pomodoro.
9. Structure complex ideas in Mind Map.
10. Use Audiobook to review text and PDF with assisted reading.
11. Turn important readings into `Integrated Study Flow` and spaced reviews.
12. Structure project schedules in `4D BIM Gantt Schedule` (`Ctrl+Alt+G`) strictly as an auxiliary planning aid subject to independent human verification.
