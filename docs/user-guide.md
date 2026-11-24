# Guia do Usuário / User Guide — EISENHOWER ORGANIZER / LÚMEN

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details open>
<summary>Clique para expandir / recolher o guia do usuário em português</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Guia Rápido e Operacional do Usuário

**Versão:** `2026.11.22.0`  
**Data:** `22 de novembro de 2026`  
**Autor:** Fernando Nillsson Cidade  

> *(Para o manual exaustivo seção por seção de todos os diálogos e configurações históricas, consulte também [`MANUAL.md`](../MANUAL.md).)*

> [!WARNING]
> **Escopo Auxiliar e Responsabilidade do Usuário ("AS IS"):** Este software é fornecido **"NO ESTADO EM QUE SE ENCONTRA" ("AS IS")** como ferramenta auxiliar de produtividade, estudo ativo, acessibilidade e apoio ao planejamento ([`docs/licensing.md`](licensing.md#ptbr) e [EULA Oficial](../source/assets/eisenhower/EULA/EULA_pt_BR.txt)). Resultados gerados por IA local (`Qwen 2.5` / `Ollama`) ou cálculos de cronograma (`Gantt BIM 4D` / CPM) exigem conferência independente pelo usuário e não substituem responsabilidade técnica legalmente habilitada (**ART/RRT**).

---

## 1. Primeiros Passos e Produtos Oficiais

- **EISENHOWER ORGANIZER (`EisenhowerOrganizer.exe`):** Foco em gestão matricial de tarefas, agendas e cronogramas de projetos ([Microsoft Store](https://apps.microsoft.com/detail/9P289X0185C3)).
- **LÚMEN (`Lumen.exe`):** Edição dedicada à acessibilidade cognitiva e plataforma integrada de estudos (TDAH, dislexia, TEA) ([Microsoft Store](https://apps.microsoft.com/detail/9N70CLLMVRPN)).
- **Dados Locais:** Salvos automaticamente em `%LOCALAPPDATA%\EisenhowerOrganizer` ou `%LOCALAPPDATA%\Lumen` (`tarefas.json`, bancos SQLite `listas.db` e `gantt_database.db`, preferências e logs).
- **Idioma Dinâmico:** Alterne instantaneamente entre **Português (Brasil)** e **English (US)** em `Configurações -> Idioma` sem reiniciar o aplicativo.

---

## 2. Operando os 7 Módulos e IA Local

### 2.1. Matriz de Eisenhower (Host Principal)
1. **Adicionar e Editar Tarefas:** Digite o título no campo superior, escolha o quadrante (`Importante e Urgente`, `Importante, mas Não Urgente`, `Não Importante, mas Urgente`, `Não Importante e Não Urgente`) ou dê duplo clique para abrir o diálogo responsivo `EditTaskDialog` (definindo data, hora, ciclos Pomodoro 🍅, metas 🎯, duração em dias e arquivo externo vinculado).
2. **Alternar Layouts:** Em `Configurações -> Layout de Tarefas -> Interface do Usuário -> Matriz de Eisenhower`, escolha `Layout Matriz 2x2` (4 quadrantes simultâneos), `Layout em Abas -> Vertical` ou `Layout em Abas -> Horizontal` (com colunas recolhíveis em ~30 px e rótulo rotacionado em 90°).
3. **Calendário, Recorrências e Exportação:** Pressione `Ctrl+M` para visualizar a `Lista Global de Tarefas` com selos `[Pendente]`, `[Em Progresso]` e `[Concluída]` (duplo clique abre o arquivo vinculado); use `Arquivo -> Salvar Como` para exportar em `JSON`, `CSV`, `XLSX` ou `PDF`, e `Arquivo -> Abrir` para importar planilhas e PDFs.

### 2.2. Pomodoro (`Ctrl+Shift+P`)
- Arraste tarefas entre `A Fazer`, `Em Progresso` e `Concluído` ou sincronize diretamente do Eisenhower.
- Ao iniciar com divergência entre a meta 🎯 da tarefa e o ciclo configurado, o `CycleMismatchDialog` oferece opções seguras (`Resetar Tudo`, `Iniciar Mesmo Assim` ou `Cancelar`).
- Configure toques de alarme WAV e avanço automático em `Configurações -> Configurações Específicas -> Pomodoro`.

### 2.3. Método Feynman (`Ctrl+Shift+F`)
- Estruture conceitos em 4 abas (`Conceito`, `Explique com Simplicidade`, `Lacunas Identificadas`, `Revisão e Simplificação Final`), vincule arquivos de apoio e selecione múltiplas tarefas para exclusão ou coloração pelo menu de contexto.

### 2.4. Mapa Mental & IA Local `Modelo` (`Ctrl+Shift+N`)
- Crie nós, conecte ideias, escolha layouts (`Vertical`, `Horizontal`, `Radial`, `Árvore`) e exporte em `.json` ou imagem `.png`.
- **Importar Documentos com IA:** Importe `PDF`, `EPUB`, `DOC/DOCX`, `TXT`, `MD`, `CSV`, `XLSX`, `JSON`, `DB/SQLite` ou `XML`.
  - **Modelo Qwen 2.5 Sob Demanda:** Acesse `Configurações -> Configurações Específicas -> Mapa Mental -> Como baixar ou gerenciar modelo Qwen (GGUF)...` para baixar `qwen2.5-3b-instruct-q4_k_m.gguf` (~2,17 GB) do Hugging Face com **Pausar**, **Retomar**, **Abrir Pasta** e **Excluir Modelo**.
  - **Ollama Local:** Acesse `Como instalar modelos no Ollama...` para copiar comandos (`llama3.2`, `qwen2.5:3b`) e testar a conexão em `http://localhost:11434`.
  - **Contingência Automática:** Sem modelo instalado, o motor heurístico NLP/TF-IDF gera o mapa automaticamente offline.

### 2.5. Audiolivro — Leitor Acessível (`Ctrl+Shift+L`)
- Digite, cole ou arraste arquivos `.txt` e `.pdf` para alternar entre visão **Texto** e **PDF** (com zoom, ajuste à página, modo mão/seleção e busca).
- Ative a **Régua de Foco**, o realce sincronizado por **palavra, frase ou parágrafo**, escolha vozes offline (Windows SAPI) ou vozes online opcionais (Edge TTS — requer conexão ativa apenas durante a síntese) e ajuste velocidade (PPM) e tipografia acessível.

### 2.6. Agenda / Registro de Atividades (`Ctrl+Shift+R`)
- Alterne entre o modelo `Engenharia Civil` e agendas personalizadas com listas dependentes em formato árvore (`curso -> semestre -> disciplina`).
- Vincule arquivos externos por botão ou arraste e configure o modo visual (`Preto` adaptável claro/escuro ou `Coloridas`) globalmente ou por módulo (`Eisenhower`, `Pomodoro`, `Feynman`).

### 2.7. Cronograma de Gantt & Engenharia 4D BIM (`Ctrl+Alt+G`)
- Crie itens EAP/WBS, defina durações, marcos, dependências (`TI`/`II`/`TT`/`IT`) com validação automática anti-ciclos DAG e visualize o **Caminho Crítico (CPM)**.
- Clique com o botão direito sobre qualquer tarefa do Eisenhower, Pomodoro ou Feynman para criar/vincular atividade no Gantt ou registrar avanço de campo (`0%`–`100%`).
- Utilize `Bim4DSimulationPanel` (reprodução cronológica 4D), importação/exportação **MS Project XML** (`MsProjectXmlConnector`), **CSV** com mapeamento visual (`CsvMappingDialog`), calendários de dias úteis/feriados e auditoria de alterações (`AuditService`).

---

## 3. Atalhos Principais e Diagnóstico

- **Atalhos Rápidos:** `Ctrl+N` (Novo) · `Ctrl+O` (Abrir) · `Ctrl+S` / `Ctrl+Shift+S` (Salvar / Como) · `Ctrl+M` (Calendário) · `Ctrl+Shift+H` (`Hoje / Próximos 7 dias`) · `Ctrl+Shift+G` (Busca global) · `Ctrl+Shift+P` (Pomodoro) · `Ctrl+Shift+F` (Feynman) · `Ctrl+Shift+N` (Mapa Mental) · `Ctrl+Shift+L` (Audiolivro) · `Ctrl+Shift+R` (Agenda) · `Ctrl+Alt+G` (Gantt BIM 4D) · `Ctrl+Shift+M` (Manual) · `Ctrl+Shift+A` (Sobre).
- **Diagnóstico e Suporte:** Use `Ferramentas -> Gerenciador de plugins` e `Ferramentas -> Gerenciador de logs` (`file_eisenhower_*.log` / `file_lumen_*.log`).

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details open>
<summary>Click to expand / collapse user guide in English</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — User & Operations Guide

**Version:** `2026.11.22.0`  
**Date:** `November 22, 2026`  
**Author:** Fernando Nillsson Cidade  

> *(For the exhaustive section-by-section manual covering every dialog option, see [`MANUAL.md`](../MANUAL.md).)*

> [!WARNING]
> **Auxiliary Scope & User Responsibility ("AS IS"):** Supplied **"AS IS"** as an auxiliary productivity, study, accessibility, and scheduling tool ([`docs/licensing.md`](licensing.md#enus) and [Official EULA](../source/assets/eisenhower/EULA/EULA_en_US.txt)). Local AI outputs (`Qwen 2.5`/`Ollama`) and Gantt CPM/4D BIM schedules require independent verification and never replace licensed engineering sign-off (**ART/RRT**).

---

## 1. Getting Started & Official Store Editions

- **EISENHOWER ORGANIZER (`EisenhowerOrganizer.exe`):** Quadrant-based task productivity, schedules, and Gantt timelines ([Microsoft Store](https://apps.microsoft.com/detail/9P289X0185C3)).
- **LÚMEN (`Lumen.exe`):** Cognitive-accessibility and study suite tailored for ADHD, dyslexia, and ASD learners ([Microsoft Store](https://apps.microsoft.com/detail/9N70CLLMVRPN)).
- **Local Persistence & Live Language:** Data lives locally under `%LOCALAPPDATA%\EisenhowerOrganizer` or `%LOCALAPPDATA%\Lumen`. Switch `Portuguese (Brazil)` / `English (US)` immediately under `Settings -> Language` without restarting.

---

## 2. Operating the 7 Modules & On-Demand Local AI

1. **Eisenhower Matrix (Host):** Add tasks, edit detailed fields in the responsive `EditTaskDialog` (Pomodoro targets 🎯/🍅, duration, due dates, linked files), switch between `2x2 Matrix`, `Vertical Tabs`, collapsible 90°-rotated `Horizontal Tabs`, or `Quadrants`, review `[Pending]`/`[In Progress]`/`[Completed]` calendar items (`Ctrl+M`), and export/import `JSON`/`CSV`/`XLSX`/`PDF`.
2. **Pomodoro (`Ctrl+Shift+P`):** Manage `Todo`/`Doing`/`Done` Kanban lists synchronized with Eisenhower, configure WAV focus/break cycles, and resolve cycle mismatches cleanly with `CycleMismatchDialog`.
3. **Feynman (`Ctrl+Shift+F`):** Structure study cards across 4 stages (`Concept`, `Explain Simply`, `Knowledge Gaps`, `Final Review`), link reference files, and edit/delete batches via context menu.
4. **Mind Map & Local AI (`Ctrl+Shift+N`):** Build visual graphs, import `PDF`/`EPUB`/`DOCX`/`TXT`/`MD`/`CSV`/`XLSX`/`JSON`/`SQLite`/`XML`, manage on-demand Qwen 2.5 GGUF downloads (`ModelDownloadDialog` with HTTP Range pause/resume/delete) or local `Ollama` models (`http://127.0.0.1:11434`), with seamless offline TF-IDF heuristic fallback.
5. **Audiobook (`Ctrl+Shift+L`):** Read text and PDF documents with live word/sentence/paragraph speech highlight, line-by-line **Focus Ruler**, offline Windows SAPI voices, or optional online Edge TTS synthesis.
6. **Agenda (`Ctrl+Shift+R`):** Create custom nested tree agendas (`listas.db`), attach external files, and toggle `Black`/`Colored` display modes globally or per module.
7. **4D BIM Gantt Schedule (`Ctrl+Alt+G`):** Build WBS trees, validate cycle-free DAG dependencies (`FS`/`SS`/`FF`/`SF`), inspect the **Critical Path (CPM)**, bind tasks from Eisenhower/Pomodoro/Feynman via right-click context menu, run 4D playback (`Bim4DSimulationPanel`), and exchange **MS Project XML** or mapped **CSV**.

---

## 3. Essential Shortcuts & Diagnostics

`Ctrl+N` (New) · `Ctrl+O` (Open) · `Ctrl+S` / `Ctrl+Shift+S` (Save / Save As) · `Ctrl+M` (Calendar) · `Ctrl+Shift+H` (`Today / Next 7 days`) · `Ctrl+Shift+G` (Global search) · `Ctrl+Shift+P` (Pomodoro) · `Ctrl+Shift+F` (Feynman) · `Ctrl+Shift+N` (Mind Map) · `Ctrl+Shift+L` (Audiobook) · `Ctrl+Shift+R` (Agenda) · `Ctrl+Alt+G` (4D BIM Gantt) · `Ctrl+Shift+M` (Manual) · `Ctrl+Shift+A` (About). Inspect logs under `Tools -> Log Manager` and module status under `Tools -> Plugin Manager`.

</details>
