# Licenciamento e Esclarecimento Comercial / Licensing & Commercial Model — EISENHOWER ORGANIZER / LÚMEN

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details open>
<summary>Clique para expandir / recolher o documento de licenciamento em português</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Duplo Licenciamento, Direitos Open Source e Esclarecimento Comercial

**Versão:** `2026.11.22.0`  
**Data:** `22 de novembro de 2026`  
**Licença do Código-Fonte Público:** GNU Affero General Public License v3 (GNU AGPLv3)  
**Licença das Edições Oficiais Microsoft Store:** EULA Oficial + Contrato de Licenciamento Comercial (CLC)  
**Autor e Detentor dos Direitos Autorais:** Fernando Nillsson Cidade  

---

## 1. Visão Geral do Modelo de Duplo Licenciamento

Os projetos **EISENHOWER ORGANIZER** e **LÚMEN** adotam um modelo transparente de **Duplo Licenciamento** que concilia integralmente a liberdade do software livre sob copyleft forte (**GNU AGPLv3**) com a distribuição comercial e assinatura digital oficial do autor na **Microsoft Store**:

| Componente / Distribuição | Regime Aplicável | Condições e Usos Permitidos |
|---|---|---|
| **Código-Fonte Público (Repositório Completo)** | [GNU AGPLv3](../LICENSE) (`AGPL-3.0-only`) | 100% livre para uso, estudo, auditoria, compilação (`MSVC` ou `MinGW`), modificação e criação de forks, preservando a autoria original e a obrigação copyleft (incluindo Seção 13 para uso em rede). |
| **Documentação Institucional em `source/assets/`** | Autoria do Projeto *(Uso Livre para Forks Open Source)* | Elaborada para atender à conformidade da Microsoft Store e às telas `Sobre` internas (`eisenhower/` e `lumen/`). Desenvolvedores têm liberdade total para editar, excluir, acrescentar ou adaptar em seus próprios forks, desde que removam as marcas exclusivas do autor (`EISENHOWER ORGANIZER` e `LÚMEN`). |
| **Binários Compilados por Terceiros (via Código-Fonte)** | [GNU AGPLv3](../LICENSE) | Livres para compilar e redistribuir sob inteira responsabilidade do distribuidor, cumprindo integralmente os termos da GNU AGPLv3 e as atribuições de terceiros, sem usar a marca oficial do autor. |
| **Edições Oficiais Microsoft Store (`9P289X0185C3` / `9N70CLLMVRPN`)** | [EULA Oficial](../source/assets/eisenhower/EULA/EULA_pt_BR.txt) + [CLC](../source/assets/eisenhower/CLC/CLC_pt_BR.txt) | Pacotes compilados e assinados pelo autor (com avaliação de 7 dias via `TrialManager` e aquisição paga na Store ou licenciamento comercial Individual / Empresarial / Corporativo). É proibida a revenda comercial não autorizada por terceiros, o contorno de controles de licença da edição oficial ou a apropriação das marcas oficiais. |
| **Componentes de Terceiros (`Qt 6.12.0`, `FFmpeg n9.0.1`, `llama.cpp`, `Qwen 2.5`, `SQLite`)** | `LGPLv3` / `LGPLv2.1+` / `MIT` / `Apache-2.0` / `Blessing` | Preservados integralmente em `source/assets/LICENSES/` e `NOTICES/`, com oferta correspondente de código-fonte LGPL e direito de relinkagem dinâmica preservado sem restrições. |

---

## 2. Direitos de Código Aberto (`GNU AGPLv3`)

Todo o código-fonte em C++23 do ecossistema — incluindo host (`EisenhowerMatrixApp`), dupla identidade (`AppConfig`), armazenamento atômico (`UnifiedTaskStore`), interface gráfica `Qt 6.12.0`, os 7 módulos integrados (`Pomodoro`, `Feynman`, `Mapa Mental`, `Audiolivro`, `Agenda`, `Gantt & BIM 4D`), serviço de IA Local (`Modelo`), utilitários (`mocks/`) e a suíte de 11 executáveis CTest (`tests/`) — é disponibilizado publicamente sob a **GNU Affero General Public License v3 (GNU AGPLv3)**:

```text
EISENHOWER ORGANIZER / LÚMEN
Copyright (c) 2025-2026 Fernando Nillsson Cidade

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
```

**Relevância da Seção 13 da GNU AGPLv3 (Uso em Rede):** Como o módulo **Cronograma de Gantt BIM 4D** incorpora um servidor HTTP REST (`HttpServer`) e despachante de webhooks (`WebhookDispatcher`), qualquer modificação executada para servir terceiros através de rede exige a disponibilização do código-fonte modificado correspondente aos usuários remotos conforme a Seção 13 da GNU AGPLv3.

---

## 3. Documentação em `source/assets/` e Liberdade Total para Desenvolvedores

As subpastas [`source/assets/eisenhower/`](../source/assets/eisenhower/) e [`source/assets/lumen/`](../source/assets/lumen/) (`ABOUT`, `CLC`, `COPYRIGHT`, `EULA`, `NOTICES`, `PRIVACY_POLICY`, `RELEASE` e `icones`) contêm a documentação regulatória exibida na janela `Sobre` ([`OpcoesSobre`](../source/ui/ui_02_OpcoesSobre.cpp)) e nos instaladores Inno Setup ([`library/`](../library/Eisenhower_Organizer.iss)).

1. **Finalidade Original:** Viabilizar a publicação e conformidade regulatória das edições oficiais do **EISENHOWER ORGANIZER** e do **LÚMEN** na Microsoft Store e em instaladores Windows assinados pelo autor.
2. **Autorização Expressa para Desenvolvedores:** O autor **concede liberdade expressa** para que desenvolvedores, mantenedores de forks e empacotadores de código-fonte alterem, excluam, substituam ou adaptem qualquer documento dentro de `source/assets/`.
3. **Resguardo de Marca (Seção 7(e) da GNU AGPLv3):** Ao redistribuir compilações derivadas modificadas, o mantenedor do fork deve remover ou substituir as marcas comerciais exclusivas **`EISENHOWER ORGANIZER`** e **`LÚMEN`** e os ícones oficiais associados (`organizador.ico`, `autismo.ico`), assegurando distinção clara em relação às edições oficiais publicadas pelo autor e observando as leis de sua jurisdição e os textos de terceiros em `source/assets/LICENSES/`.

---

## 4. Edições Oficiais Assinadas na Microsoft Store (`EULA` e `CLC`)

1. **Identidades Oficiais Publicadas:**
   - **EISENHOWER ORGANIZER:** `https://apps.microsoft.com/detail/9P289X0185C3` (`BUILD_TARGET=EISENHOWER`)
   - **LÚMEN:** `https://apps.microsoft.com/detail/9N70CLLMVRPN` (`BUILD_TARGET=LUMEN`)
2. **Apoio ao Projeto e Avaliação (`TrialManager`):** Os binários oficiais distribuídos na Microsoft Store incluem gerenciamento de avaliação gratuita de 7 dias e aquisição oficial que viabiliza financeiramente a manutenção contínua, acessibilidade (TDAH/dislexia/TEA) e hospedagem do projeto.
3. **Modalidades Comerciais do `CLC`:** Para organizações que demandam licenciamento institucional dos pacotes oficiais pré-compilados e assinados pelo autor, o [`CLC`](../source/assets/eisenhower/CLC/CLC_pt_BR.txt) disciplina as modalidades **Individual** (até 2 dispositivos), **Empresarial / Small Business** (até 10 estações) e **Corporativa / Enterprise** (implantações em escala).
4. **Reserva LGPL e Open Source:** Nenhuma disposição do `EULA` ou do `CLC` restringe os direitos garantidos pela `GNU LGPLv3` / `LGPLv2.1+` sobre bibliotecas compartilhadas incluídas no pacote (`Qt 6.12.0` e `FFmpeg n9.0.1`), sendo assegurada a possibilidade de substituição/relinkagem das DLLs LGPL e a consulta à oferta de código-fonte correspondente em [`OPEN_SOURCE_SOURCE_OFFER_pt_BR.txt`](../source/assets/LICENSES/OPEN_SOURCE_SOURCE_OFFER_pt_BR.txt).

---

## 5. Checklist de Conformidade Microsoft Store (Partner Center)

| Campo no Partner Center | Referência no Projeto | Observação Regulatória |
|---|---|---|
| **Privacy Policy URL (Política 10.5)** | [`docs/privacy-policy.md`](privacy-policy.md#ptbr) e `PRIVACY_POLICY/Privacy_Policy_pt_BR.txt` | Obrigatória para aplicativos desktop Win32 (`EisenhowerOrganizer.exe` e `Lumen.exe`). **A Microsoft não fornece política padrão nem cobre estes aplicativos sob a Privacy Statement da Microsoft.** |
| **Website do Desenvolvedor** | `https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp` | Aponta para o repositório/site do autor, nunca para a página da Store. |
| **Customer Support** | `linceu_lighthouse@outlook.com` e GitHub Issues | **A Microsoft não presta suporte técnico ao cliente.** Atendimento voluntário *best-effort* pelo autor, sem SLA. |
| **Clean Uninstall (Política 10.2.7)** | Configurado no instalador Inno Setup (`library/*.iss`) e MSIX | Remoção limpa via Configurações do Windows / Menu Iniciar, sem serviços residentes ocultos. |
| **Direitos Autorais (<= 200 chars)** | — | `© 2025-2026 Fernando Nillsson Cidade. Código-fonte sob GNU AGPLv3; edições oficiais da Store sob EULA/CLC do autor. Qt (LGPLv3), FFmpeg (LGPL), llama.cpp (MIT).` *(161 caracteres)* |

---

## 6. Natureza Auxiliar, Limitações de Maturidade e Isenção Legal ("AS IS")

Tanto o código-fonte público sob **GNU AGPLv3** quanto as edições binárias oficiais são fornecidos **"NO ESTADO EM QUE SE ENCONTRAM" ("AS IS")**, sem garantias expressas ou implícitas de infalibilidade, adequação comercial absoluta ou infalibilidade algorítmica (em harmonia com a Lei nº 9.609/1998, Lei nº 9.610/1998, Código Civil, CDC e Seções 15–16 da [GNU AGPLv3](../LICENSE)):

- **Uso Auxiliar e Responsabilidade do Usuário:** As ferramentas de priorização (Eisenhower), cronometragem (Pomodoro), estudo ativo (Feynman), agendas avaliativas (Agenda), geração conceitual por IA local/heurística (Mapa Mental / Modelo Qwen / Ollama), leitura assistida/voz (Audiolivro) e planejamento de projetos (Gantt & BIM 4D) são **recursos auxiliares** e não substituem conferência manual, critérios médicos/clínicos (Audiolivro/Lúmen) ou validação profissional formal.
- **Engenharia e Planejamento 4D BIM (Módulo Gantt):** Resultados de Caminho Crítico (CPM), dependências lógicas, folgas, baselines, medições de campo e simulações 4D **jamais substituem** conferência técnica independente nem a responsabilidade legal exclusiva de profissional habilitado com emissão de **ART perante o CREA** ou **RRT perante o CAU** (**Leis nº 5.194/1966, nº 6.496/1977 e nº 12.378/2010**).

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details open>
<summary>Click to expand / collapse licensing in English</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Dual Licensing, Open Source Rights & Commercial Clarification

**Version:** `2026.11.22.0`  
**Date:** `November 22, 2026`  
**Public Source Code License:** GNU Affero General Public License v3 (GNU AGPLv3)  
**Official Microsoft Store Binary License:** Author's Official EULA + Commercial Licensing Agreement (CLC)  
**Author & Copyright Holder:** Fernando Nillsson Cidade  

---

## 1. Dual-Licensing Architecture Overview

**EISENHOWER ORGANIZER** and **LÚMEN** follow a transparent **Dual-Licensing** model that reconciles strong-copyleft open-source freedom (**GNU AGPLv3**) with the author's digitally signed commercial Microsoft Store editions:

| Component / Distribution | Applicable Regime | Permitted Uses & Conditions |
|---|---|---|
| **Public Source Code (Full Repository)** | [GNU AGPLv3](../LICENSE) (`AGPL-3.0-only`) | 100% Free to inspect, study, compile (`MSVC` or `MinGW`), modify, and fork, preserving original attribution and copyleft obligations (including Section 13 network provisions). |
| **Institutional Documentation in `source/assets/`** | Project Authorship *(Free for Open-Source Forks)* | Authored for Microsoft Store compliance and in-app About dialogs (`eisenhower/` and `lumen/`). Fork developers have full freedom to alter, remove, add, or adapt these files provided the author's exclusive trademarks (`EISENHOWER ORGANIZER` and `LÚMEN`) are removed. |
| **Third-Party Builds from Source Code** | [GNU AGPLv3](../LICENSE) | Free to compile and redistribute under the builder's sole responsibility in full compliance with AGPLv3 and third-party notices, without using the author's exclusive branding. |
| **Official Microsoft Store Editions (`9P289X0185C3` / `9N70CLLMVRPN`)** | [Official EULA](../source/assets/eisenhower/EULA/EULA_en_US.txt) + [CLC](../source/assets/eisenhower/CLC/CLC_en_US.txt) | Official compiled and signed Store packages (with 7-day trial via `TrialManager` and Store purchase or Individual / Small Business / Enterprise tiers under `CLC`). Unauthorized third-party commercial resale, license bypass on official binaries, or brand misappropriation is strictly prohibited. |
| **Third-Party Libraries (`Qt 6.12.0`, `FFmpeg n9.0.1`, `llama.cpp`, `Qwen 2.5`, `SQLite`)** | `LGPLv3` / `LGPLv2.1+` / `MIT` / `Apache-2.0` / `Blessing` | Fully documented under `source/assets/LICENSES/` and `NOTICES/`, preserving corresponding source offers and dynamic relinking rights for LGPL components. |

---

## 2. Open Source Rights (`GNU AGPLv3`)

All public C++23 source code — including host (`EisenhowerMatrixApp`), dual build target config (`AppConfig`), atomic store (`UnifiedTaskStore`), `Qt 6.12.0` UI, the 7 integrated modules (`Pomodoro`, `Feynman`, `Mind Map`, `Audiobook`, `Agenda`, `4D BIM Gantt`), local AI service (`Modelo`), tools (`mocks/`), and the 11-executable CTest suite (`tests/`) — is licensed under the **GNU Affero General Public License v3 (GNU AGPLv3)**.

**Network Provision Relevance (AGPLv3 Section 13):** Because the **4D BIM Gantt Schedule** module includes an optional local REST HTTP server (`HttpServer`) and webhook dispatcher (`WebhookDispatcher`), operating a modified network service for remote users triggers AGPLv3 Section 13 corresponding source availability obligations.

---

## 3. Freedom for Developers on `source/assets/`

Developers creating independent forks from source under **GNU AGPLv3** are **expressly authorized** to customize, replace, or delete any file in [`source/assets/eisenhower/`](../source/assets/eisenhower/) and [`source/assets/lumen/`](../source/assets/lumen/), provided they remove the author's exclusive brand names (`EISENHOWER ORGANIZER` and `LÚMEN`) and official icons (`organizador.ico`, `autismo.ico`) under Section 7(e) and abide by local law and third-party licenses in `source/assets/LICENSES/`.

---

## 4. Official Microsoft Store Editions (`EULA` & `CLC`)

- **EISENHOWER ORGANIZER:** `https://apps.microsoft.com/detail/9P289X0185C3` (`BUILD_TARGET=EISENHOWER`)
- **LÚMEN:** `https://apps.microsoft.com/detail/9N70CLLMVRPN` (`BUILD_TARGET=LUMEN`)

Store purchases and commercial `CLC` tiers (Individual up to 2 devices, Small Business up to 10 workstations, Enterprise site licensing) fund ongoing maintenance, cognitive accessibility work, and project evolution. Nothing in the Official EULA or CLC restricts downstream rights granted under `GNU LGPLv3` / `LGPLv2.1+` to replace or relink shared LGPL libraries (`Qt 6.12.0` and `FFmpeg n9.0.1`), as confirmed in [`OPEN_SOURCE_SOURCE_OFFER_en_US.txt`](../source/assets/LICENSES/OPEN_SOURCE_SOURCE_OFFER_en_US.txt).

---

## 5. Auxiliary Scope, Maturity Disclaimer & Legal Grounding ("AS IS")

Both public source builds under **GNU AGPLv3** and official binary editions are provided **"AS IS"**, without express or implied warranties (Brazilian Law No. 9.609/1998, Law No. 9.610/1998, Civil Code, Consumer Defense Code, and [GNU AGPLv3](../LICENSE) Sections 15–16). Local AI syntheses (`Qwen 2.5`/`Ollama`), Audiobook speech reading, academic deadline reminders, and 4D BIM Gantt Critical Path (CPM) calculations are auxiliary tools requiring independent human verification and **never replace** licensed engineering oversight (**ART/RRT** before **CREA/CAU** or equivalent PE/CEng registration).

</details>
