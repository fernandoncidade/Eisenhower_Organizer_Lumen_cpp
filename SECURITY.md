# Política de Segurança / Security Policy — EISENHOWER ORGANIZER / LÚMEN

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher a política em português</summary><br>

# Política de Segurança

**Versão:** `2026.11.22.0`  
**Data:** `22 de novembro de 2026`  

## 1. Versões com Suporte Oficial

Atualizações e correções de segurança priorizam a série de lançamento mais recente:

| Versão | Suportada |
|:---:|:---:|
| `v2026.11.x` | :white_check_mark: |
| `< v2026.11` | :x: |

---

## 2. Como Relatar uma Vulnerabilidade

Levamos a integridade dos dados locais, a segurança de execução e a estabilidade do **EISENHOWER ORGANIZER** e do **LÚMEN** com seriedade técnica.

Se você identificou uma potencial vulnerabilidade de segurança (por exemplo: corrupção/injeção no parsing de JSON, CSV, XLSX, PDF, EPUB, DOCX, XML ou SQLite; *path traversal* ao importar arquivos ou abrir anexos; escalação de privilégio ou bypass de token/RBAC na API HTTP opcional do módulo Gantt; falha no gerenciamento do subprocesso `llama-cli` ou nas conexões opcionais Hugging Face / Ollama / Edge TTS):

1. **NÃO divulgue publicamente a vulnerabilidade em issues ou fóruns abertos.**
2. Envie o relato de forma privada por meio de **GitHub Security Advisories** (aba *Security* do repositório) ou diretamente por e-mail para o mantenedor: `linceu_lighthouse@outlook.com`.
3. Inclua no relato:
   - Descrição detalhada da vulnerabilidade encontrada;
   - Produto alvo (`EisenhowerOrganizer.exe` ou `Lumen.exe`) e variante de build (`MSVC` ou `MinGW`);
   - Passo a passo reprodutível e arquivo de entrada de demonstração (JSON/XML/CSV/EPUB/PDF sanitizado);
   - Vetor de ataque e impacto estimado;
   - Sugestões de mitigação ou correção, se disponíveis.

### Prazos e Processo de Resposta
- O mantenedor envidará melhores esforços para confirmar o recebimento em tempo hábil, observando a natureza voluntária e independente da manutenção do projeto (sem SLA contratual ou suporte prestado pela Microsoft).
- Após análise e correção, a mitigação será validada nas tarefas `Qt6: Compilar e Executar SUITE (MSVC)` e `Qt6: Compilar e Executar SUITE (MinGW)` e divulgada coordenadamente com os devidos créditos ao pesquisador.

---

## 3. Compromissos Arquiteturais de Segurança

- **Arquitetura Local-First com Rede Estritamente Opcional e Sob Demanda:** Todos os dados principais de tarefas (`tarefas.json`), estudos (`conceitos_feynman.json`, `mapa_mental_autosave.json`), agenda acadêmica (`listas.db`) e cronogramas (`gantt_database.db`) são armazenados localmente em `%LOCALAPPDATA%\EisenhowerOrganizer` ou `%LOCALAPPDATA%\Lumen`. Não há conta obrigatória nem telemetria. As únicas comunicações de rede existentes são opcionais e explicitamente acionadas pelo usuário (download sob demanda de modelo `.gguf` no Hugging Face via HTTPS com retomada Range; conexão local `localhost:11434` com Ollama; síntese opcional de voz online Edge TTS via WebSocket no Audiolivro; e servidor HTTP REST / Webhooks no Gantt somente quando habilitado pelo usuário).
- **Persistência Atômica e Trava Multi-Instância (`UnifiedTaskStore`):** O arquivo principal `tarefas.json` utiliza escrita atômica via `QSaveFile`, trava contra concorrência via `QLockFile` e preservação de backup `tarefas.json.bak`, evitando corrupção em encerramentos abruptos ou gravações paralelas.
- **Banco de Dados SQLite Parametrizado (`Agenda` & `Gantt`):** Consultas de leitura e escrita utilizam *prepared statements* parametrizados (`QSqlQuery`) com validações topológicas em memória (`DependencyValidator`, detecção de ciclos DAG no `CpmEngine`) e trilha de auditoria (`AuditService`).
- **Controle de Acesso na API Local do Gantt (`SecurityService`):** Quando o usuário opta por ativar o servidor HTTP local do módulo Gantt, tokens, permissões RBAC por papel e *rate-limiting* protegem operações sensíveis (edição de atividades, criação de baselines e disparo de webhooks).
- **Extração Segura de Documentos & Abertura de Vínculos:** Os extratores de documentos do Mapa Mental (`Qt6::Pdf`, `Qt6::CorePrivate::QZipReader` para EPUB/DOCX/XLSX, parsers CSV/JSON/XML/SQLite) operam localmente sobre arquivos explicitamente selecionados pelo usuário; caminhos externos vinculados têm existência verificada via `QFileInfo::exists()` antes da delegação ao sistema operacional.
- **Distribuição Limpa com CRT App-Local e Isolamento de Subprocesso IA:** A distribuição MSVC carrega apenas as 5 DLLs app-local do VC++ Runtime (`msvcp140*.dll`, `vcruntime140*.dll`) **sem embutir `vc_redist.x64.exe`**, e os binários auxiliares de inferência local (`llama-cli`) executam apenas em CPU/Vulkan local quando explicitamente invocados.

> [!NOTE]
> **Escopo de Segurança vs. Natureza Auxiliar ("AS IS"):** Esta Política de Segurança cobre exclusivamente vulnerabilidades de software (memória, parsing, concorrência de I/O, isolamento e permissões) e **não certifica adequação irrestrita para produção crítica, diagnóstico clínico ou planejamento executivo definitivo de engenharia** — especialmente no cálculo CPM / 4D BIM ou síntese de IA, que exigem revisão independente ([`README.md`](README.md#ptbr), [`docs/security-model.md`](docs/security-model.md#ptbr) e [EULA](source/assets/eisenhower/EULA/EULA_pt_BR.txt)).

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the policy in English</summary><br>

# Security Policy

**Version:** `2026.11.22.0`  
**Date:** `November 22, 2026`  

## 1. Supported Versions

Security updates target the latest active release series:

| Version | Supported |
|:---:|:---:|
| `v2026.11.x` | :white_check_mark: |
| `< v2026.11` | :x: |

---

## 2. Reporting a Vulnerability

We take local data integrity, execution safety, and stability of **EISENHOWER ORGANIZER** and **LÚMEN** seriously.

If you discover a potential security vulnerability (for example: parsing defects or injection in JSON, CSV, XLSX, PDF, EPUB, DOCX, XML, or SQLite; path traversal on document import or linked file launch; privilege/RBAC/rate-limit bypass in the optional Gantt local HTTP server; or subprocess handling issues in `llama-cli` / optional Hugging Face, Ollama, or Edge TTS integrations):

1. **Do NOT disclose the vulnerability publicly via open issues or discussions.**
2. Submit a private report via **GitHub Security Advisories** (repository *Security* tab) or email the maintainer directly at `linceu_lighthouse@outlook.com`.
3. Please include:
   - Detailed description of the vulnerability;
   - Target product (`EisenhowerOrganizer.exe` or `Lumen.exe`) and toolchain variant (`MSVC` or `MinGW`);
   - Reproducible steps and sanitized sample input file;
   - Attack vector and estimated impact;
   - Suggested remediation or patch, if available.

### Response Time & Process
- The maintainer will make best-effort attempts to acknowledge receipt promptly, subject to voluntary independent maintenance availability (no contractual SLA and no customer support from Microsoft).
- Verified fixes are validated across `Qt6: Compilar e Executar SUITE (MSVC)` and `Qt6: Compilar e Executar SUITE (MinGW)` before coordinated disclosure and researcher attribution.

---

## 3. Core Architectural Security Commitments

- **Local-First Architecture with Strictly Optional On-Demand Networking:** Core user data (`tarefas.json`), active study records (`conceitos_feynman.json`, `mapa_mental_autosave.json`), academic schedules (`listas.db`), and project plans (`gantt_database.db`) remain strictly local under `%LOCALAPPDATA%\EisenhowerOrganizer` or `%LOCALAPPDATA%\Lumen`. There are zero mandatory accounts and zero telemetry. Network I/O occurs only when explicitly triggered by the user (on-demand Hugging Face `.gguf` download over HTTPS with HTTP Range resume; local loopback `localhost:11434` Ollama connection; optional Edge TTS online neural voice WebSocket in Audiobook; and optional Gantt local HTTP REST / Webhook endpoints when enabled).
- **Atomic Persistence & Multi-Instance Locking (`UnifiedTaskStore`):** `tarefas.json` enforces atomic writes via `QSaveFile`, concurrent-write protection via `QLockFile`, and `tarefas.json.bak` recovery snapshots.
- **Parameterized SQLite Queries (`Agenda` & `Gantt`):** All database operations use parameterized `QSqlQuery` bindings paired with in-memory graph/cycle verification (`DependencyValidator`, `CpmEngine`) and `AuditService` audit logs.
- **Gantt Local API Access Control (`SecurityService`):** When the user explicitly enables the optional Gantt HTTP server, token validation, RBAC role guards, and rate limiting protect write endpoints and baseline management.
- **Safe Document Extraction & Linked File Handling:** Mind Map document extractors (`Qt6::Pdf`, `Qt6::CorePrivate::QZipReader` for EPUB/DOCX/XLSX, CSV/JSON/XML/SQLite readers) process only user-selected files locally; linked external paths verify existence via `QFileInfo::exists()` prior to OS delegation.
- **Clean App-Local CRT Deployment & Subprocess Isolation:** MSVC distributions bundle only the 5 app-local VC++ Runtime DLLs (`msvcp140*.dll`, `vcruntime140*.dll`) **without privileged `vc_redist.x64.exe` installers**, and local `llama-cli` CPU/Vulkan processes run strictly on demand.

> [!NOTE]
> **Security Scope vs. Auxiliary Scope ("AS IS"):** This policy governs software security defects (memory safety, parsing, I/O concurrency, isolation, and access controls) and **does not certify fitness for safety-critical production, clinical diagnosis, or definitive engineering execution schedules** — notably CPM / 4D BIM schedules and local AI summaries, which require independent human review ([`README.md`](README.md#enus), [`docs/security-model.md`](docs/security-model.md#enus), and [EULA](source/assets/eisenhower/EULA/EULA_en_US.txt)).

</details>
