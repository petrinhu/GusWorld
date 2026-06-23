# Plano de mitigacao de hacks de runtime (G1, single-player offline)

Status: ACEITO (nivel decidido pelo lider 2026-06-22) - politica aplicada conforme o gameplay for construido
Data: 2026-06-22
Autor: security-engineer (defensivo)

> **DECISAO DO LIDER (2026-06-22):** nivel adotado = pacote "FAZER AGORA" + a camada de reacao "marcar run modificada".
> Concretamente, aplicar CONFORME os valores de gameplay forem sendo construidos (a maioria ainda nao existe):
> 1. **Valor protegido em memoria** (XOR por-sessao + shadow/checksum) para os valores sensiveis: HP, creditos, recursos, XP.
> 2. **Sanity-clamp / invariantes** em todo valor sensivel (reusa as invariantes do combate; estender a creditos/recursos/XP).
> 3. **Marcar a run como "modificada"** ao detectar valor impossivel (ex.: desligar conquistas), reusando o selo do save (HMAC) - reacao leve, sem punir.
> 4. Manter HMAC do save (ADR-006) + hash-128 do controls (ADR-007).
> FORA do escopo G1 (NAO fazer): kernel anti-cheat, anti-debug agressivo, deteccao de injection/hook, packing, pointer-scan dedicado.
> Quando os valores de gameplay existirem (combate ja tem HP/AP/mana; economia/XP em progression/save), aplicar 1-3 nesses pontos.
Escopo: GusWorld G1, C++20 + SDL3, engine propria, Linux + Windows, FOSS/GPLv3, SEM code signing.
Conecta com: ADR-006 (HMAC-SHA256 save), ADR-007 (hash-128 controls.json), invariantes de combate.

NAO e codigo. E um plano para o lider escolher o nivel. Descreve os vetores SO o
suficiente para mitigar (postura defensiva).

---

## 1. Modelo de ameaca HONESTO (ler antes de tudo)

O jogo e single-player, offline, sem servidor e sem driver de kernel. O jogador e
DONO da maquina, do processo e da RAM. Consequencias diretas e inegociaveis:

1. **Memory editing nao tem como ser impedido de verdade.** Quem controla a maquina
   le e escreve a RAM do proprio processo. Toda "protecao" em modo-usuario roda
   dentro desse mesmo processo que o atacante controla. E matematicamente perdido
   contra um atacante local determinado. O que funciona de verdade contra cheat
   depende de **servidor autoritativo** (que valida cada acao, o cliente manda input
   e nao resultado) OU **anti-cheat de kernel** (EAC/BattlEye/EAAC) que roda no mesmo
   nivel do SO para bloquear leitura/escrita de memoria. As duas coisas sao de
   **multiplayer/online**. Fontes ao fim.

2. **Anti-cheat de kernel e VETADO neste projeto** e seria contra-producente:
   - Contraria offline (nao ha servidor para apoiar a decisao).
   - Contraria Linux + FOSS/GPLv3 (drivers de kernel anti-cheat sao proprietarios,
     invasivos, e mal vistos no Linux; varios quebram em Proton/anti-cheat ring-0).
   - Contraria privacidade/LGPD (telemetria invasiva, varredura do sistema do jogador).
   - Sem code signing em G1, um driver nem carregaria sem fricao enorme.

3. **Em solo, trapacear NAO prejudica ninguem alem do proprio jogador.** Nao ha
   placar competitivo, nao ha vitima. Skyrim, a serie Souls (offline) e a maioria dos
   indies convivem com trainers e tolerancia a mods. Anti-cheat em single-player ja
   gerou reembolso e revolta (caso Watch Dogs 2). Gastar esforco de G1 para "vencer"
   isso e over-engineering puro.

### Objetivo realista (o que de fato da para fazer)

Nao e "impedir cheat". E:

- **(a) Deter o trapaceiro casual/impulsivo** (o que abre o Cheat Engine, faz uns
  scans, congela HP). Friccao basta: se nao for um `int` cru obvio, a maioria desiste.
- **(b) Preservar a INTEGRIDADE do jogo.** O risco real de um valor adulterado nao e
  "injustica", e **crash / estado corrompido / save quebrado** por valor absurdo
  (HP negativo, recurso gigante que estoura, divisao por zero). Isso a gente tem o
  dever de tratar de qualquer forma, ate sem atacante (bug, overflow, save velho).
- **(c) Opcionalmente detectar e reagir de LEVE**: marcar o save como "modificado"
  (reusa o selo do ADR-007), desligar conquistas/achievements daquela run, ou um
  "modo trapaca" honesto opt-in. Reacao leve, nunca punitiva agressiva.

O dever de contra-argumentar manda dizer com todas as letras: **nao existe pacote de
G1 que impeca memory editing.** Qualquer promessa nesse sentido seria falsa. O que
segue calibra friccao + robustez + deteccao leve, sem cair em anti-debug agressivo
(que dispara falso-positivo de antivirus e irrita jogador honesto e QA).

---

## 2. Vetores de runtime hacking (defensivo) e mitigacao

Colunas: **Vetor** | **O que e** | **Da para impedir em solo offline?** |
**Mitigacao leve (deter casual)** | **Custo** | **Recomendacao G1**

### 2.1 Memory scanning / editing (Cheat Engine e similares)
- **O que e:** o app externo le a RAM do processo; o trapaceiro faz scans sucessivos
  (mudou / nao mudou) ate isolar o endereco de um valor (HP, creditos) e o sobrescreve.
- **Impedir em solo?** NAO. Controle total da RAM pelo dono da maquina.
- **Mitigacao leve:** nao guardar valores sensiveis (HP, creditos, recursos, XP) como
  `int` cru. Guardar **ofuscado** (XOR com mascara por-sessao) + **shadow/checksum**
  (um segundo campo derivado; na leitura, confere). Periodicamente realocar/regerar a
  mascara. Acesso so por getter/setter que cifra/decifra. Isso quebra o scan ingenuo
  por valor exato e o "freeze" do endereco simples.
- **Custo:** baixo de runtime (xor + checksum sao triviais); medio de build (encapsular
  os tipos sensiveis num "valor protegido"); zero antivirus; zero diferenca Linux/Win.
- **Recomendacao G1:** **FAZER AGORA** so para o punhado de valores sensiveis (HP,
  creditos, recursos, XP/progressao). Nao ofuscar a RAM inteira (over-engineering).

### 2.2 Value freezing (congelar valor)
- **O que e:** o app reescreve o endereco a cada frame, mantendo o valor fixo.
- **Impedir em solo?** NAO.
- **Mitigacao leve:** consequencia direta do "valor protegido" 2.1 + da realocacao
  periodica da mascara: o endereco congelado guarda a forma cifrada antiga e o checksum
  passa a divergir -> a sanity-check (2.9) detecta divergencia e clampa/marca.
- **Custo:** zero extra alem de 2.1.
- **Recomendacao G1:** **FAZER AGORA** (vem de graca com 2.1).

### 2.3 Pointer scanning (achar o ponteiro estavel)
- **O que e:** como o endereco direto muda a cada execucao, o trapaceiro acha a
  cadeia de ponteiros estavel (base + offsets) que sempre leva ao valor, criando um
  cheat que sobrevive a reinicios.
- **Impedir em solo?** NAO.
- **Mitigacao leve:** a propria ofuscacao 2.1 (valor nunca esta em claro no endereco
  apontado) ja reduz o ganho; realocacao periodica atrapalha a cadeia. Nao vale
  esforco extra dedicado.
- **Custo:** zero extra.
- **Recomendacao G1:** **NUNCA** medida dedicada. Aceitar; 2.1 ja e o teto sensato.

### 2.4 Speedhack (acelerar o relogio do jogo)
- **O que e:** o cheat intercepta as funcoes de tempo do SO e devolve um tempo
  acelerado, fazendo o jogo todo rodar mais rapido (farm, esquiva, etc.).
- **Impedir em solo?** NAO (o cheat mente sobre o relogio que o jogo consulta).
- **Mitigacao leve:** **timing check** cruzando DUAS fontes de tempo independentes
  (ex.: clock monotonico de alta resolucao vs. um relogio de parede / contagem
  alternativa). Se a razao entre elas sair de uma faixa plausivel por uma janela,
  e sinal de speedhack -> marcar a run (2.10), nunca crashar. Em solo isso e quase
  cosmetico (so prejudica o proprio jogador), entao prioridade baixa.
- **Custo:** baixo runtime; cuidado com falso-positivo em maquina fraca / stutter /
  laptop em throttle (a faixa tem de ser larga e a janela longa para nao acusar
  jogador honesto). Risco de UX/QA se a faixa for apertada.
- **Recomendacao G1:** **DEPOIS** (pos vertical slice), e so se sobrar tempo. Faixa
  larga, janela longa, reacao = marca leve. Nao priorizar.

### 2.5 DLL injection / code injection / function hooking
- **O que e:** carregar codigo externo dentro do processo (Win: LoadLibrary/CreateRemoteThread;
  Linux: LD_PRELOAD/ptrace) e substituir funcoes do jogo (hook) para alterar logica.
- **Impedir em solo?** NAO (o dono do processo injeta o que quiser).
- **Mitigacao leve:** praticamente nada que valha a pena em G1. Listar modulos
  carregados ou detectar hook e jogo de gato-e-rato caro, com alto risco de
  falso-positivo (overlays do Steam/Discord, ferramentas de acessibilidade, RivaTuner)
  e de parecer malware para antivirus.
- **Custo:** alto (build/manutencao), alto risco antivirus + falso-positivo, divergente
  Linux/Win.
- **Recomendacao G1:** **NUNCA** em G1. Vetor de multiplayer/kernel.

### 2.6 Debugger attach (gdb / x64dbg)
- **O que e:** anexar um depurador ao processo para ler/alterar estado e por
  breakpoints na logica.
- **Impedir em solo?** NAO.
- **Mitigacao leve:** anti-debug (IsDebuggerPresent no Win, checar TracerPid/ptrace no
  Linux, timing) e **explicitamente desaconselhado** aqui: dispara falso-positivo de
  antivirus, atrapalha o PROPRIO desenvolvimento/QA (que usa gdb/lldb), e e trivial de
  burlar. Custo-beneficio negativo num indie FOSS.
- **Custo:** alto risco antivirus + atrapalha QA/dev; ganho ~zero.
- **Recomendacao G1:** **NUNCA**. Antipattern para este projeto.

### 2.7 Savefile editing
- **O que e:** editar o arquivo de save fora do jogo para inflar valores.
- **Impedir em solo?** Deteccao, sim (e ja esta resolvido); impedir edicao do arquivo
  no disco do dono, nao.
- **Mitigacao leve:** **JA TRATADO.** ADR-006: envelope binario proprio `GDS2` com
  **HMAC-SHA256** (chave embutida) detecta tamper -> byte-flip e rejeitado. Edicao a
  mao do save quebra o selo. (E integridade casual, nao sigilo: a chave mora no
  binario, por design.)
- **Custo:** ja pago.
- **Recomendacao G1:** **PRONTO** (so referenciar ADR-006). Nao reabrir.

### 2.8 Asset / data-file tampering
- **O que e:** trocar arquivos de dados (tabelas de balanceamento, stats, loot, JSON
  de config) fora do jogo para mudar regras.
- **Impedir em solo?** NAO impedir; detectar, sim, barato.
- **Mitigacao leve:** o `controls.json` ja usa **hash-128 (SHA-256 truncado)** para
  detectar edicao manual e oferecer restauro (ADR-007). Mesmo padrao pode estender a
  outros data-files sensiveis ao gameplay (tabelas de balanceamento) SE virarem
  externos editaveis. Para data embutido no binario, nao precisa nada. Reacao = avisar
  / restaurar default, nunca crashar. Lembrar: GPLv3 da ao jogador o direito de
  modificar; a meta e so distinguir "oficial" de "modificado", nao proibir.
- **Custo:** baixo (reusa crypto do core); zero antivirus; igual Linux/Win.
- **Recomendacao G1:** **DEPOIS / conforme surgir** data-file externo sensivel. Reusar
  o padrao do ADR-007. Sem urgencia no vertical slice.

### 2.9 Sanity-clamp / invariantes de valor (DEFESA DE INTEGRIDADE)
- **O que e:** nao e vetor de ataque; e a defesa que mais importa em solo. Manter todo
  valor de gameplay dentro de limites plausiveis no momento de uso: dano, HP, creditos,
  recursos clampados a `[min, max]`; HP nunca negativo nem acima do max; recurso nunca
  estoura o tipo.
- **Impedir cheat?** Nao impede ler/escrever; **impede o jogo de quebrar** com valor
  absurdo e neutraliza o ganho mais comum do cheat ingenuo (HP=999999 vira HP=max).
- **Mitigacao leve:** o combate **JA TEM invariantes** (HP/dano em faixas). Estender a
  disciplina de invariante/clamp a HP, creditos, recursos e XP em todo ponto de leitura
  e mutacao. Se o valor lido violar o invariante (sinal de edicao externa), clampar +
  marcar a run (2.10). Isto e secure-by-default / fail-safe: o valor impossivel nunca
  propaga.
- **Custo:** baixo; e boa engenharia de qualquer forma (robustez contra bug e save
  velho, nao so contra cheat). Zero antivirus; igual Linux/Win.
- **Recomendacao G1:** **FAZER AGORA.** Maior retorno do plano. Reusa os invariantes de
  combate que ja existem; estende aos demais valores sensiveis.

### 2.10 Deteccao de valor impossivel -> marcar a run / save "modificado"
- **O que e:** reacao leve. Quando 2.9 detecta valor fora do invariante, ou 2.1/2.2 o
  checksum diverge, ou 2.4 o timing acusa, o jogo nao briga: registra "estado modificado".
- **Mitigacao leve:** setar uma flag no save (o selo do ADR-007 / envelope do ADR-006 ja
  da onde gravar com integridade) e, opcionalmente, desligar achievements daquela run
  ou exibir um "modo trapaca" honesto. Sem ban, sem fechar o jogo, sem acusar em
  popup agressivo.
- **Custo:** baixo; reusa selo existente.
- **Recomendacao G1:** **DEPOIS** (quando houver achievements / razao de jogo para
  marcar). A infra de marcar ja existe nos selos; ligar quando houver gameplay que use.

---

## 3. Recomendacao de PACOTE para G1

### Pacote recomendado (FAZER AGORA, custo baixo, alto retorno)
1. **Valor protegido para o punhado de valores sensiveis** (HP, creditos, recursos,
   XP/progressao): XOR por-sessao + shadow/checksum, getter/setter que cifra. (2.1, 2.2)
2. **Sanity-clamp / invariantes em TODO valor de gameplay sensivel** estendendo os
   invariantes de combate ja existentes. Esta e a peca principal. (2.9)
3. **Manter o que ja temos**: HMAC do save (ADR-006) e hash-128 do controls.json
   (ADR-007). Nada a fazer alem de referenciar. (2.7, 2.8)

### Fazer DEPOIS (pos vertical slice, se houver razao de gameplay)
4. **Marcar run/save como "modificado"** ligado aos selos existentes, quando houver
   achievements. (2.10)
5. **Estender hash-128 a data-files externos** sensiveis ao balanceamento, se algum
   virar editavel externo. (2.8)
6. **Timing check de speedhack** com faixa larga e reacao leve, baixa prioridade. (2.4)

### NUNCA em G1 (over-engineering / custo-beneficio negativo / contra os pilares)
- **Anti-cheat de kernel (EAC/BattlEye/EAAC-like):** de multiplayer/online; contra
  offline + Linux + FOSS + privacidade/LGPD + sem-signing. (secao 1)
- **Anti-debug agressivo (IsDebuggerPresent / ptrace / breakpoint traps):** falso-
  positivo de antivirus, atrapalha o proprio QA/dev, trivial de burlar. (2.6)
- **Deteccao de DLL injection / hook / varredura de modulos:** caro, falso-positivo com
  overlay legitimo (Steam/Discord) e acessibilidade, cheira a malware. (2.5)
- **Packing / virtualizacao de codigo (VMProtect-like):** peso de build enorme, quebra
  reprodutibilidade, conflita com GPLv3 (codigo aberto de qualquer forma), dispara
  antivirus. Inutil em solo.
- **Pointer-scan defense dedicada:** sem ganho alem do que 2.1 ja da. (2.3)

---

## 4. Conexao com o que ja existe no projeto

| Peca existente | Como o plano reusa |
|---|---|
| **Invariantes de combate** (HP/dano em faixas) | Base do sanity-clamp 2.9; estender o mesmo rigor a creditos/recursos/XP. |
| **HMAC-SHA256 do save, envelope GDS2 (ADR-006)** | Cobre savefile tampering 2.7 (PRONTO); da onde gravar a flag "modificado" 2.10 com integridade. |
| **Hash-128 do controls.json (ADR-007)** | Cobre data-file tampering 2.8; padrao reusavel para outros data-files externos. |
| **`core/crypto/` (SHA-256 + HMAC dep-free)** | Reaproveitado para checksum do valor protegido 2.1 e hash de data-files 2.8, sem nova dependencia. |
| **Filosofia "integridade casual, nao sigilo"** (ADR-006/007) | E exatamente a postura deste plano: deter o casual, nao o atacante determinado. Coerencia total. |
| **Invariante de camadas (core/domain ZERO Qt, ZERO dep externa)** | Valor protegido + clamp + checksum sao POCO puro: cabem em domain/ sem violar a invariante; nada exige Qt nem dep externa. |

---

## 5. Fontes (URLs)

- Cheat Engine Wiki, Memory Scanning: https://wiki.cheatengine.org/index.php?title=Cheat_Engine%3AMemory_Scanning
- Cheat Engine, Scan settings (fast/aligned scan): https://www.cheatengine.org/help/scan-settings.htm
- Irdeto, Anti-cheat in video games A to Z (servidor autoritativo, memoria, anti-hook/anti-debug, single-player como base de reverse-engineering): https://irdeto.com/blog/cheating-in-games-everything-you-always-wanted-to-know-about-it
- EA, A Deep Dive on EA anticheat (EAAC) for PC (kernel-level): https://www.ea.com/security/news/eaac-deep-dive
- i3D.net, Countering the scourge of cheating (servidor autoritativo, cliente nao confiavel): https://www.i3d.net/countering-scourge-of-cheating-in-games/
- arXiv 2512.21377, Systematic Review of Technical Defenses Against Software-Based Cheating in Online Multiplayer Games: https://arxiv.org/pdf/2512.21377
- USPTO 11273380, Preventing cheating by providing obfuscated game variables (value relocation, XOR, offset randomization, shadow/checksum): https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/11273380
- Sharp Coder Blog, Unity Obfuscation Methods and Anti-Hack Protection (encrypted value, shadow var, sanity check): https://www.sharpcoderblog.com/blog/unity-3d-obfuscation-methods-and-anti-cheat-protection
- The Gistre Blog (EPITA), History of Anti-Cheat (servidor autoritativo + kernel-level): https://blog.gistre.epita.fr/posts/lucas.demenais-2025-06-24-history_of_anticheat/
- DLL Injection and Function Hooking (defensivo, entender o vetor): https://everthessel.nl/blog/dll-injection-function-hooking/
- arXiv 2210.11047, Anti-debugging (timing checks, custo): https://arxiv.org/pdf/2210.11047
- Watch Dogs 2, discussao anti-cheat em single-player offline (reacao negativa de jogadores): https://steamcommunity.com/app/447040/discussions/0/152390014801453165/
- Quora, anti-cheat em single-player offline (consenso "desnecessario"): https://www.quora.com/What-do-you-think-of-developers-putting-anti-cheats-into-single-player-offline-games
