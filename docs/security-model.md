# Modelo de Segurança e Resiliência / Security Model — EISENHOWER ORGANIZER / LÚMEN

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details open>
<summary>Clique para expandir / recolher o modelo de segurança em português</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Modelo de Segurança, Persistência Atômica e Resiliência

**Versão:** `2026.11.22.0`  
**Data:** `22 de novembro de 2026`  
**Autor:** Fernando Nillsson Cidade  

---

## 1. Princípios de Segurança Arquitetural

Os aplicativos **EISENHOWER ORGANIZER** e **LÚMEN** são projetados sobre cinco pilares de segurança e resiliência em **C++23 / Qt 6.12.0**:

1. **Persistência Atômica Protegida Contra Concorrência (`UnifiedTaskStore`):** O armazenamento compartilhado `tarefas.json` emprega `QLockFile` para exclusão mútua entre operações/módulos, `QSaveFile` para substituição atômica (evitando arquivos truncados em quedas de energia) e cópia de segurança `tarefas.json.bak` com recusa em sobrescrever JSON corrompido.
2. **Isolamento por Produto e Minimização de Privilégios:** `%LOCALAPPDATA%\EisenhowerOrganizer` e `%LOCALAPPDATA%\Lumen` são estritamente segregados (`AppConfig`), operando sem privilégios administrativos. As distribuições externas utilizam *app-local CRT deployment* sem invocar instaladores privilegiados como `vc_redist.x64.exe`.
3. **Consultas SQLite Parametrizadas & Validação Topológica (`Agenda` e `Gantt`):** Todos os acessos a `listas.db` e `gantt_database.db` utilizam *prepared statements* via `Qt6::Sql`, evitando injeção SQL; no Gantt, `DependencyValidator` impede ciclos DAG (`TI`/`II`/`TT`/`IT`) antes do cálculo CPM (`CpmEngine`).
4. **Segurança na API HTTP Local do Gantt (`SecurityService`):** Quando habilitado opcionalmente pelo usuário, o servidor REST embutido (`HttpServer`) aplica validação de tokens, controle de acesso baseado em papéis (**RBAC** — ex.: permissão `can_manage_baselines`), *rate-limiting* e registro em trilha `AuditService`.
5. **Execução Local Sob Demanda de IA e Extratores Seguros:** A inferência por `llama.cpp b8987` ocorre em processo local sem exposição externa de dados; os extratores do Mapa Mental (`QPdfDocument`, `QZipReader` para EPUB/DOCX/XLSX, CSV, JSON, XML) leem exclusivamente arquivos escolhidos pelo usuário.

---

## 2. Superfície de Ameaças e Mitigações

| Vetor Potencial | Risco | Mitigação Implementada no Código |
|---|:---:|---|
| **Corrupção de `tarefas.json` por Escrita Simultânea** | Médio | `UnifiedTaskStore` usa `QLockFile`, escrita atômica `QSaveFile` e snapshot `tarefas.json.bak`. |
| **Injeção SQL em Agendas Personalizadas ou WBS Gantt** | Médio | Consultas parametrizadas via `QSqlQuery::bindValue` nos repositórios `AgendaAvaliacoes` e `GanttDatabase`. |
| **Ciclos Infinitos ou Deadlock de Cronograma no CPM** | Baixo | `DependencyValidator` valida grafos acíclicos dirigidos (DAG) antes de liberar o cálculo `ES/EF/LS/LF` em `CpmEngine`. |
| **Uso Indevido na API REST Opcional do Gantt** | Médio | `SecurityService` (autenticação, RBAC granular e *rate-limiting*) + trilha em `AuditService`; serviço desativado sem configuração explícita. |
| **Arquivos Vinculados Inexistentes ou Inválidos** | Baixo | Checagem prévia via `QFileInfo::exists()` e validação de caminho antes de delegar abertura ao shell do Windows. |
| **Download Incompleto ou Corrompido do Modelo GGUF** | Baixo | `ModelDownloadDialog` verifica continuidade via HTTP Range, controla gravação temporária e oferece exclusão segura (`Excluir Modelo`). |
| **Troca de DLLs CRT / Contaminação MinGW vs MSVC** | Baixo | Guardas explícitas de pré-compilação nos scripts `library/*.iss` rejeitam mistura de DLLs MSVC/MinGW ou presença de `vc_redist.x64.exe`. |

---

## 3. Distinção Entre Segurança de Software e Escopo Auxiliar ("AS IS")

Este modelo trata exclusivamente da segurança da aplicação (integridade de armazenamento, isolamento, parsing e controle de acesso local) e **não certifica adequação definitiva para sistemas críticos ou planejamento executivo sem supervisão humana**. Os módulos operam no estado em que se encontram (**"AS IS"**) como ferramentas auxiliares ([`docs/licensing.md`](licensing.md#ptbr) e [EULA](../source/assets/eisenhower/EULA/EULA_pt_BR.txt)).

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details open>
<summary>Click to expand / collapse security model in English</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Security Model, Atomic Persistence & Resilience

**Version:** `2026.11.22.0`  
**Date:** `November 22, 2026`  
**Author:** Fernando Nillsson Cidade  

---

## 1. Architectural Security Pillars

**EISENHOWER ORGANIZER** and **LÚMEN** rely on five core engineering safeguards in **C++23 / Qt 6.12.0**:

1. **Concurrency-Safe Atomic Persistence (`UnifiedTaskStore`):** Shared `tarefas.json` persistence enforces `QLockFile` mutual exclusion, `QSaveFile` atomic swap-writes, and `.bak` recovery snapshots refusing to overwrite corrupted JSON.
2. **Product Isolation & Least Privilege:** `%LOCALAPPDATA%\EisenhowerOrganizer` and `%LOCALAPPDATA%\Lumen` remain isolated (`AppConfig`) and run without administrator elevation. MSVC external packages use app-local CRT DLLs without bundling privileged `vc_redist.x64.exe` installers.
3. **Parameterized SQLite Queries & DAG Guards (`Agenda` & `Gantt`):** All SQLite operations (`listas.db`, `gantt_database.db`) use parameterized `QSqlQuery` bindings, while `DependencyValidator` blocks DAG cycles (`FS`/`SS`/`FF`/`SF`) prior to `CpmEngine` schedule passes.
4. **Gantt Local HTTP API Guardrails (`SecurityService`):** When explicitly enabled by the user, the embedded REST `HttpServer` applies token validation, role-based access control (**RBAC**, e.g., `can_manage_baselines`), rate limiting, and `AuditService` event logging.
5. **On-Demand Local AI & Controlled Document Parsing:** `llama.cpp b8987` runs locally on demand without uploading prompt payloads; Mind Map extractors (`QPdfDocument`, `QZipReader`, CSV/JSON/XML) inspect only user-selected files.

---

## 2. Threat Surface & Mitigations

| Vector | Risk | Implemented Safeguard |
|---|:---:|---|
| **Concurrent `tarefas.json` Write Corruption** | Medium | `UnifiedTaskStore` (`QLockFile` + `QSaveFile` + `tarefas.json.bak` preservation). |
| **SQL Injection in Custom Agendas or Gantt WBS** | Medium | Parameterized `QSqlQuery::bindValue` queries across SQLite repositories. |
| **Cyclic Graph / Infinite CPM Pass** | Low | `DependencyValidator` DAG verification before `CpmEngine` `ES/EF/LS/LF` evaluation. |
| **Unauthorized Action on Optional Gantt REST API** | Medium | `SecurityService` RBAC + rate limiting + `AuditService`; inactive unless enabled. |
| **Broken / Missing Linked External Paths** | Low | `QFileInfo::exists()` pre-checks before OS delegation. |
| **Corrupted GGUF Weight Downloads** | Low | `ModelDownloadDialog` HTTP Range resume integrity and explicit file deletion. |
| **MSVC / MinGW Runtime Cross-Contamination** | Low | Pre-build guards inside `library/*.iss` reject mixed toolchain DLLs or `vc_redist.x64.exe`. |

---

## 3. Software Security vs. Auxiliary Scope ("AS IS")

This document addresses software execution security (storage atomicity, SQLite parameterization, parsing bounds, local access control) and **does not certify fitness for safety-critical use or unsupervised engineering sign-off**. All software is provided **"AS IS"** as an auxiliary tool ([`docs/licensing.md`](licensing.md#enus) and [Official EULA](../source/assets/eisenhower/EULA/EULA_en_US.txt)).

</details>
