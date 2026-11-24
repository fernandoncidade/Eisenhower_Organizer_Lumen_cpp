# Política de Privacidade / Privacy Policy — EISENHOWER ORGANIZER / LÚMEN

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details open>
<summary>Clique para expandir / recolher a Política de Privacidade em português</summary><br>

# POLÍTICA DE PRIVACIDADE — EISENHOWER ORGANIZER & LÚMEN

**Versão:** `2026.11.22.0`  
**Data:** `22 de novembro de 2026`  
**Desenvolvedor / Controlador:** Fernando Nillsson Cidade (Desenvolvedor Individual)  
**E-mail de Contato e Privacidade:** `linceu_lighthouse@outlook.com`  
**Repositório Oficial:** [https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp](https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp)  

---

## 1. Declaração de Independência frente à Política de Privacidade da Microsoft

> [!IMPORTANT]
> **A Microsoft NÃO fornece uma política de privacidade padrão para o EISENHOWER ORGANIZER nem para o LÚMEN, e estes aplicativos NÃO são cobertos por nenhuma política de privacidade da Microsoft.**
> - Esta Política de Privacidade aplica-se exclusiva e integralmente aos softwares **EISENHOWER ORGANIZER** (`9P289X0185C3`) e **LÚMEN** (`9N70CLLMVRPN`) em todas as formas de distribuição (Microsoft Store, instaladores Windows Inno Setup ou compilações independentes do código-fonte).
> - Ao navegar, baixar, atualizar ou avaliar na **Microsoft Store** ou habilitar relatórios de diagnóstico opcionais do **Windows**, os dados tratados pela própria Microsoft seguem a [Política de Privacidade da Microsoft](https://www.microsoft.com/pt-br/privacy/privacystatement).
> - A Microsoft não presta suporte técnico aos usuários destes aplicativos; solicitações sobre privacidade ou funcionamento devem ser encaminhadas diretamente ao autor por e-mail (`linceu_lighthouse@outlook.com`) ou via GitHub.

---

## 2. Arquitetura Local-First e Ausência de Telemetria

O **EISENHOWER ORGANIZER** e o **LÚMEN** foram projetados sob o princípio **Local-First**:

1. **Zero Contas, Zero Telemetria e Zero Analytics:** Nenhum dos aplicativos exige cadastro, login em nuvem, coleta identificadores publicitários, SDKs de rastreamento ou envia métricas de uso ao desenvolvedor.
2. **Soberania Local por Produto (`%LOCALAPPDATA%`):**
   - **Eisenhower Organizer:** `%LOCALAPPDATA%\EisenhowerOrganizer\`
   - **Lúmen:** `%LOCALAPPDATA%\Lumen\`
3. **Arquivos Locais Mantidos Sob Controle Exclusivo do Usuário:**
   - `tarefas.json` e `tarefas.json.bak` (`UnifiedTaskStore` com `QLockFile` + `QSaveFile`);
   - `layout.json`, `language.json`, `recorrencias_config.json`, `recorrencias_estado_original.json`, `lembretes_locais_config.json`;
   - `tarefas_gerais_font_config.json`, `arquivo_vinculado_font_config.json`, além de fontes independentes de módulos;
   - **Feynman:** `conceitos_feynman.json`;
   - **Mapa Mental:** `mapa_mental_autosave.json`, `mapamental_config.json`, mapas `.json` e imagens `.png` salvos pelo usuário;
   - **Audiolivro:** `audiolivro_config.json` e arquivos temporários efêmeros de síntese de áudio/texto removidos após a leitura;
   - **Agenda:** `listas.db` (SQLite), `agenda_config.json` e agendas personalizadas em `agendas/*.json`;
   - **Gantt & BIM 4D:** `gantt_database.db` (SQLite com EAP/WBS, dependências, baselines, diário de obra e trilha local `AuditService`);
   - **Logs Locais:** `logs/file_eisenhower_*.log` ou `logs/file_lumen_*.log`, gravados apenas localmente.

---

## 3. Recursos Opcionais de Rede Acionados Exclusivamente pelo Usuário

Embora o núcleo da plataforma opere offline, determinadas funcionalidades opcionais realizam conexões **somente quando explicitamente acionadas ou configuradas pelo usuário**:

| Funcionalidade Opcional | Módulo | Destino e Finalidade | Controle do Usuário |
|---|---|---|---|
| **Download Sob Demanda do Modelo Qwen 2.5 (`qwen2.5-3b-instruct-q4_k_m.gguf`)** | `Modelo` / `Mapa Mental` | Requisições HTTPS ao repositório oficial `Qwen/Qwen2.5-3B-Instruct-GGUF` no Hugging Face (`ModelDownloadDialog`, com pausa/retomada via HTTP Range). | Opcional; durante a inferência posterior via `llama.cpp`, nenhum dado sai do computador. |
| **Conexão Local com Servidor Ollama** | `Modelo` / `Mapa Mental` | Requisições HTTP estritamente locais em loopback (`http://127.0.0.1:11434`) quando o usuário testa ou seleciona modelos Ollama. | Totalmente local na própria máquina (`OllamaInstrucoesDialog`). |
| **Vozes Online Neurais (Edge TTS)** | `Audiolivro` | WebSocket seguro ao serviço de síntese de voz quando o usuário seleciona explicitamente uma voz online Edge TTS e inicia a leitura. | O texto narrado é enviado ao serviço para gerar o áudio; vozes locais SAPI do Windows permanecem 100% offline. |
| **Servidor HTTP REST e Webhooks do Gantt** | `Gantt & BIM 4D` | Porta local (`HttpServer`) e webhooks configurados explicitamente pelo usuário para automação externa com controle `SecurityService`. | Desativado por padrão; só escuta ou envia eventos se configurado pelo usuário. |
| **Importação / Exportação e Vínculos** | Todos | Leitura/gravação de arquivos `PDF`, `EPUB`, `DOC/DOCX`, `XLSX`, `CSV`, `JSON`, `XML`, `DB` estritamente nos caminhos selecionados pelo usuário. | Sem varredura em segundo plano de pastas não autorizadas. |

---

## 4. Direitos do Titular (LGPD / GDPR) e Remoção Limpa

Em conformidade com a Lei Geral de Proteção de Dados (**Lei nº 13.709/2018**) e requisitos da Microsoft Store (**Política 10.2.7**):
- **Acesso, Portabilidade e Exclusão:** Todos os dados podem ser inspecionados, exportados (`JSON`, `CSV`, `XLSX`, `PDF`, `PNG`, `XML`) ou excluídos diretamente pela interface ou removendo a pasta local do produto (`%LOCALAPPDATA%\EisenhowerOrganizer` ou `%LOCALAPPDATA%\Lumen`).
- **Desinstalação Limpa:** Via **Configurações do Windows → Aplicativos → Aplicativos instalados**, removendo binários sem deixar serviços ocultos residentes.
- **Contato:** `linceu_lighthouse@outlook.com`.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details open>
<summary>Click to expand / collapse Privacy Policy in English</summary><br>

# PRIVACY POLICY — EISENHOWER ORGANIZER & LÚMEN

**Version:** `2026.11.22.0`  
**Date:** `November 22, 2026`  
**Developer / Data Controller:** Fernando Nillsson Cidade (Individual Developer)  
**Privacy & Support Email:** `linceu_lighthouse@outlook.com`  
**Official Repository:** [https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp](https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp)  

---

## 1. Independence from Microsoft Privacy Statement

> [!IMPORTANT]
> **Microsoft does NOT provide a default privacy policy for EISENHOWER ORGANIZER or LÚMEN, and these applications are NOT covered by any Microsoft privacy policy.**
> - This Privacy Policy applies exclusively to **EISENHOWER ORGANIZER** (`9P289X0185C3`) and **LÚMEN** (`9N70CLLMVRPN`) across all distribution formats (Microsoft Store, Windows Inno Setup installers, or independent builds from source code).
> - Data processed by Microsoft when browsing, installing, updating, or reviewing in the **Microsoft Store** or via optional **Windows** diagnostics is governed by the [Microsoft Privacy Statement](https://www.microsoft.com/en-us/privacy/privacystatement).
> - Microsoft does not provide customer support for these apps; privacy or usage inquiries must be sent directly to the developer at `linceu_lighthouse@outlook.com` or GitHub.

---

## 2. Local-First Architecture & Zero Telemetry

**EISENHOWER ORGANIZER** and **LÚMEN** follow a **Local-First** design:

1. **Zero Accounts, Zero Telemetry, Zero Analytics:** Neither application requires registration, cloud login, ad identifiers, analytics SDKs, or background data uploads to the developer.
2. **Per-Product Local Storage (`%LOCALAPPDATA%`):**
   - **Eisenhower Organizer:** `%LOCALAPPDATA%\EisenhowerOrganizer\`
   - **Lúmen:** `%LOCALAPPDATA%\Lumen\`
3. **Local Data Files Under User Control:** `tarefas.json` (+ `.bak`), `layout.json`, `language.json`, recurrence/reminder configs, custom font configs, `conceitos_feynman.json`, `mapa_mental_autosave.json`, `mapamental_config.json`, `audiolivro_config.json`, `listas.db`, `agenda_config.json`, `agendas/*.json`, `gantt_database.db`, and local logs (`logs/file_eisenhower_*.log` / `logs/file_lumen_*.log`).

---

## 3. Optional User-Initiated Network Features

Core functionality works offline. Network communication happens **only when explicitly triggered or enabled by the user**:

| Optional Feature | Module | Endpoint & Purpose | User Control |
|---|---|---|---|
| **On-Demand Qwen 2.5 GGUF Download (`qwen2.5-3b-instruct-q4_k_m.gguf`)** | `Modelo` / `Mind Map` | HTTPS download from official Hugging Face repository `Qwen/Qwen2.5-3B-Instruct-GGUF` via `ModelDownloadDialog` (HTTP Range pause/resume). | Optional; subsequent local `llama.cpp` inference transmits zero data off-device. |
| **Local Ollama Server Connection** | `Modelo` / `Mind Map` | Loopback-only HTTP requests (`http://127.0.0.1:11434`) when probing or invoking locally installed Ollama models. | Runs locally on user's machine (`OllamaInstrucoesDialog`). |
| **Online Neural Voices (Edge TTS)** | `Audiobook` | Secure WebSocket connection to online TTS synthesis only when the user picks an Edge TTS online voice and starts reading. | Spoken text is sent solely for audio synthesis; Windows local SAPI voices stay 100% offline. |
| **Optional Gantt REST HTTP Server & Webhooks** | `4D BIM Gantt` | Local HTTP listener (`HttpServer`) and user-configured outbound webhooks guarded by `SecurityService`. | Disabled unless explicitly enabled/configured by the user. |

---

## 4. Data Subject Rights (LGPD / GDPR) & Clean Uninstallation

In compliance with Brazilian LGPD (**Law No. 13.709/2018**) and Microsoft Store **Policy 10.2.7**, users may inspect, export (`JSON`, `CSV`, `XLSX`, `PDF`, `PNG`, `XML`), or permanently delete all local data at any time, and uninstall cleanly via **Windows Settings → Apps → Installed Apps**. Contact: `linceu_lighthouse@outlook.com`.

</details>
