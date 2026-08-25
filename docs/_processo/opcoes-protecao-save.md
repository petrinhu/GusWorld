# Proteção de save, config, mapa e catálogo de conteúdo: opções para decisão do líder

**Autor:** Narciso (CISO). **Data:** 21/08/2026. **Natureza:** documento de opções. Nada aqui é decisão (L-11). Nenhum código foi escrito, nenhum arquivo do projeto foi tocado.

**Ordens que este documento atende (verbatim, `inicial.md:27-28`):** item 25 *"os saves e mapas do jogo devem ter mecanismo de protecao contra edicao (hash, cripto etc): discutir"*; item 26 *"as configs do jogador no jogo devem ter as mesmas protecoes"*. Mais a decisão do líder de tratar, na mesma rodada, a forma de entrada do catálogo de conteúdo (`GODS_LAWS.md:245`, seção Pendências).

**ORDEM NOVA do líder, 21/08/2026, verbatim, recebida durante a produção deste documento:** *"nada vai ficar em formato de texto. isso facilita fraudes e edicoes. quero maxima protecao a maps, saves, configs, itens, busque na web"*. Consequências incorporadas: (1) formato de texto em distribuição está descartado; todas as opções abaixo são binárias; (2) o alcance é quádruplo: mapas, saves, configs e itens (catálogo de conteúdo incluído); (3) a régua é o teto do tecnicamente alcançável; (4) afirmações externas vêm com URL; (5) onde a ordem tem custo real, o custo está nomeado com mitigação proposta, sem silêncio e sem desobediência (L-11).

**Base de fatos verificados nesta sessão:**

- FATO: o jogo é AGPL-3.0-or-later, single-player offline, sem servidor, sem conta, sem telemetria (`GODS_LAWS.md`, L-08).
- FATO: `docs/design/mecanicas/modos-morte.md` (status "PROPOSTA", pré-reboot) já registra, para o Hardcore: save isolado em slot próprio, 3 camadas offline (âncora monotônica out-of-band, machine-binding, wipe por trechos), e o caveat honesto, verbatim: *"Nenhuma das 3 camadas, nem juntas, é forense-proof offline"* e *"a ameaça real que esta mecânica precisa derrotar é o cheater CASUAL (...) não um adversário forense"*. O líder já rejeitou deliberadamente a rota de servidor por soberania de dado.
- FATO: bus, mensagem `inbox/gusworld/2026-08-20-cripto-DENTRO-decisao-do-lider.md`: o líder decidiu que **criptografia está DENTRO do escopo do GlintFx 1.0** (monocypher a absorver, tabela da mensagem de 19/08 02:10, linha 26). Estado FECHADO. A mesma mensagem registra a ressalva técnica do CTO do GlintFx: implementação própria de cripto falha por canal lateral e comparação que vaza tempo.
- FATO: o GlintFx foi reiniciado do zero em 21/08/2026; hoje o diretório `Projects/GlintFx/include/` contém exatamente um arquivo: `glintfx/core/version.hpp` (verificado por `find` nesta sessão).
- FATO: o editor de mapas externo existe (`Projects/gusworld_mapeditor/`, com `src/`, `tests/`, CI própria; verificado por `ls`).
- FATO: `modos-morte.md` referencia infraestrutura do projeto ANTERIOR (ADR-006, ADR-014, `FsSaveStore`, "Monocypher vendorizado", `title_menu.hpp`). Pela L-01 nada disso existe como código; pela L-13 é canon a re-derivar; pela L-14 eu não declaro o documento morto, apenas aponto: o DESIGN (intenção das 3 camadas) parece vigente, as REFERÊNCIAS de implementação apontam para código que não existe. Cabe ao líder confirmar o que sobrevive.
- INFERÊNCIA: tudo o mais abaixo marcado como tal.
- **ATUALIZAÇÃO, 24/08/2026:** a pergunta 5 do "Resumo para a decisão" (E.5, e os FATOs das linhas 12 e 16 acima) foi respondida pelo líder no item `G3` do `TODO.md`: todo o design de §1 a §5 de `modos-morte.md` sobrevive como canon; só o plano de implementação (§6 de lá, referenciado nas linhas 12 e 16 acima) morreu, e agora está marcado como REVOGADO no próprio documento, não mais como código fantasma sem veredito. Este documento continua valendo como registro do levantamento de 21/08/2026 e não foi reescrito.

---

## A. Modelo de ameaça honesto

Ponto de partida inegociável: **o jogo é FOSS e roda inteiro na máquina do adversário.** O jogador tem o código-fonte, pode recompilar o binário removendo qualquer verificação, e pode editar a memória do processo em execução (Cheat Engine, gdb, scanmem). Não existe segredo que o binário conheça e o dono da máquina não possa extrair, porque o "segredo" está no fonte ou é derivável pelo fonte. Qualquer desenho que finja o contrário é DRM disfarçado, e DRM em jogo AGPL é teatro por construção. Isto é INFERÊNCIA minha apenas na forma; na substância o líder já assumiu exatamente esta premissa no Hardcore (fato citado acima, `modos-morte.md`).

### Adversários, do mais fraco ao mais forte

| # | Adversário | O que ele faz | Defensável? |
|---|---|---|---|
| A1 | **Corrupção acidental** (crash no meio da gravação, disco, sync de nuvem, bit podre) | Nada de intencional; o arquivo apodrece | **SIM, totalmente.** Hash de integridade + gravação atômica + backups em cadeia. É o ganho real número um de todo este trabalho |
| A2 | **Editor casual** | Abre o save num editor de texto ou hex, muda o ouro, salva | **SIM, com custo baixo.** Formato binário + selo (MAC) detectam. Ele desiste ou vai procurar trainer pronto |
| A3 | **Editor determinado** | Lê o fonte, deriva a chave, recompila sem o check, ou edita a RAM do processo vivo | **NÃO. Inalcançável por construção.** Qualquer camada nova vira corrida armamentista que o dono da máquina sempre vence |
| A4 | **Rollback** (copia a pasta de save antes de arriscar, restaura se der errado) | Cópia de arquivos; no limite, snapshot de disco ou de VM | **PARCIAL.** Âncora out-of-band + machine-binding derrotam o rollback casual de copiar pasta. Snapshot de disco/VM inteiro vence SEMPRE; só servidor autoritativo resolveria, e o líder já rejeitou servidor |
| A5 | **Distribuição de save adulterado a terceiros** ("baixa esse save com tudo destravado") | Publica o arquivo num fórum | **PARCIAL.** Machine-binding faz o save de outra máquina não carregar. Mas quem recompila distribui build sem check junto. E num jogo single-player offline o dano a terceiros é do próprio terceiro que escolheu baixar |

### O que é INALCANÇÁVEL, dito sem rodeio, para o líder não pagar por ilusão

1. **Impedir o dono da máquina de trapacear a si mesmo.** Impossível. O objetivo honesto da proteção de save em jogo offline é o clássico "manter honesto quem quer ser honesto": criar fricção suficiente para que a trapaça seja um ato deliberado (recompilar, instalar trainer), nunca um acidente de curiosidade (abriu o arquivo, viu `gold: 250`, trocou por `999999`). Isso protege a experiência que o próprio jogador declarou querer ao escolher o modo, e é exatamente o valor que o Hardcore precisa.
2. **Permadeath forense.** Snapshot de VM, clone de disco, backup do Btrfs/Timeshift/nuvem restauram tudo, âncora incluída. O líder já sabe e já aceitou este trade-off (fato, `modos-morte.md`). Nada nesta rodada muda isso.
3. **"Wipe seguro" físico em disco moderno.** Em SSD (wear leveling), Btrfs (CoW, snapshots) e pastas sob sync de nuvem, sobrescrever o arquivo NÃO garante destruição do dado antigo em mídia. O alcançável e suficiente é o **wipe lógico**: sobrescrever selo + nonce + cabeçalho torna o save incarregável pelo jogo, e o unlink remove o arquivo. É o que a spec do Hardcore já propõe ("o objetivo é incarregável, não sem resquício"). A copy voltada ao jogador nunca deve prometer "apagado para sempre", e a spec já manda isso (`modos-morte.md`, "nunca vender essa feature como impossível recuperar").
4. **Confidencialidade real do conteúdo do save contra o jogador.** Cifrar o save esconde o formato do editor casual (A2) e mais nada: a chave de decifra está, por necessidade, ao alcance do binário, logo ao alcance do dono do binário. Cifra aqui é fricção e ofuscação de formato, não sigilo. Vale dizer isso com todas as letras porque "cifrado" soa mais forte do que é.

### O que a integridade compra DE VERDADE (o retorno real do investimento)

- **A1 resolvido:** save corrompido detectado no load, com mensagem digna e fallback para backup, em vez de crash ou estado silenciosamente podre. Este é o maior ganho de qualidade de vida e o único que afeta 100% dos jogadores honestos.
- **A2 resolvido:** a experiência escolhida (dificuldade, Hardcore) não se degrada por tentação trivial.
- **Suporte e triagem de bug:** save que chega num bug report com selo inválido é identificável como editado; poupa horas de caça a bug fantasma. Save com selo válido e estado impossível indica bug nosso de verdade.
- **Hardcore com dignidade:** as 3 camadas já desenhadas derrotam o adversário que a spec definiu (o casual), e o jogo pode se comprometer com isso honestamente.

### A.1 O que a web diz sobre a eficácia real (fatos com fonte, pedidos pela ordem nova)

- FATO (web): HMAC anexado ao save detecta alteração, e "só o jogo consegue criar HMACs autênticos"; mas a mesma literatura de prática diz que converter para binário e cifrar **não para o jogador determinado**: ele faz engenharia reversa do executável e acha a chave. Fontes: [GameMaker, "Protect Your Savefiles"](https://gamemaker.io/en/blog/protect-your-savefiles); [Salivity, "How to Secure Game Save Files"](https://salivity.github.io/game-development/article/how-to-secure-game-save-files-and-skill-trees).
- FATO (web): casos documentados de quebra por engenharia reversa existem em série: saves de Pokémon NDS tiveram cifra e CRC contornados e o checksum recalculado pelo editor ([LostMyPlaintext](https://www.lostmyplaintext.com/project/2021/06/20/Reverse-Engineering-Pokemon-Save-Files.html)); Zelda Ocarina of Time 3D usa CRC-16 nos últimos bytes, conhecido e recalculável ([GBAtemp](https://gbatemp.net/threads/help-understanding-save-file-checksums-and-encryption-on-raw-saves-exported-with-savedatafiler.392878/)); a comunidade de save editors trata "reverter e editar save" como tutorial de entrada ([GuidedHacking](https://guidedhacking.com/threads/how-to-reverse-edit-save-game-files-save-game-editor.13753/)); há relato de decifra por clean-room ([Medium, "Protecting Game Saves and the case of Unworthy"](https://medium.com/@pantelis/protecting-game-saves-and-the-case-of-unworthy-e24c8fd68e16)).
- FATO (web): em jogo de código aberto a limitação é estrutural: com o fonte disponível, qualquer função de proteção pode ser removida por patch e recompilação; a única autoridade que resiste é um servidor, que este projeto rejeitou de propósito. Fontes: [GameDev.net, "Open Source and Anti Cheat"](https://www.gamedev.net/forums/topic/368086-open-source-and-anti-cheat/3428460/?page=3); [cboard, "Anti-Cheating Techniques Viable for Open Source Games"](https://cboard.cprogramming.com/game-programming/136810-guarding-anti-cheating-techniques-viable-open-source-games.html).

INFERÊNCIA a partir disso, e é a frase mais importante do documento: **contra o dono da máquina com o fonte na mão, PREVENÇÃO é inalcançável; DETECÇÃO confiável é alcançável e vale o investimento.** O teto real de "máxima proteção" num jogo FOSS offline é: (1) tornar a edição casual impossível sem ferramenta dedicada; (2) detectar com certeza criptográfica, no nosso build, qualquer byte alterado; (3) validar semanticamente que o estado é alcançável pelas regras (a única checagem que nenhuma chave vazada quebra); (4) no Hardcore, derrotar o rollback casual. Acima disso é ilusão comprada, não proteção.

---

## B. Opções completas (todas binárias, por ordem nova do líder)

### Espinha comum às três opções (o que eu proporia em qualquer cenário)

INFERÊNCIA (desenho meu, nomes ilustrativos meus, nada é decisão):

- **Envelope binário único do projeto** (chamo de "envelope selado" e o nome real é decisão futura): `magic | versão do formato | tipo do conteúdo | comprimento | nonce | payload | tag de autenticação`. Um formato só para save, config, mapa e pack de conteúdo; muda o payload, não o envelope. Coerente com a exigência do líder de que os dois temas compartilhem formato, parser e integridade.
- **Payload = serialização binária versionada dos POCOs de domínio** (L-17: um estado canônico só torna o save completo por construção). Escrita em casa, determinística, campo a campo com ordem fixa: é código puro de `domain/`, não precisa de GlintFx nem de terceiro, e evita depender de lib de schema externa (FlatBuffers e afins seriam um terceiro elo, o que a LEI ZERO não dá de graça).
- **Gravação atômica** (escreve arquivo temporário, fsync, rename) + **cadeia de backups** (`.backup1..3`): responde ao adversário A1, que é o que atinge todo jogador honesto.
- **Verificação em dois níveis no load:** nível 1, o selo criptográfico (byte a byte); nível 2, o **validador semântico** (invariantes de domínio: valores dentro de faixa, IDs existentes no catálogo, deck sem carta impossível). O nível 2 roda mesmo se um dia o selo for quebrado por recompilação, e roda também no CI contra fixtures.

O que muda entre as opções é a força do selo, o lugar do catálogo e o formato do save.

### Opção 1: "Selo autenticado uniforme" (custo médio, proteção alta)

| Alvo | Mecanismo |
|---|---|
| Save normal | Envelope com **AEAD XChaCha20-Poly1305** (cifra + autenticação num passo; construção recomendada pela referência libsodium para uso local, nonce de 192 bits seguro para nonce aleatório: [doc oficial](https://libsodium.gitbook.io/doc/secret-key_cryptography/aead/chacha20-poly1305/xchacha20-poly1305_construction)). Chave derivada de constante do build + tipo do arquivo |
| Config | Mesmo envelope, mesma AEAD (item 26: mesmas proteções). Com rota de recuperação: config ilegível = reset para default com aviso, nunca recusa de boot |
| Mapa | Mesmo envelope; o editor de mapas sela com a MESMA biblioteca de selo (dependência entre projetos, via bus) |
| Catálogo (itens, cartas, inimigos, receitas, diálogos) | **Pack binário único** com índice, entrada por átomo, hash BLAKE2b por entrada + tag AEAD do pack inteiro, gerado no build; validador roda no CI e no boot |
| Hardcore | Tudo acima MAIS: chave adicional derivada da máquina (machine-binding), âncora anti-rollback out-of-band com contador monotônico, wipe lógico por trechos + unlink na morte. Exatamente as 3 camadas que `modos-morte.md` já desenha |

- **Onde a chave vive e por que isso não é seguro:** derivada no código, portanto visível no fonte AGPL. Serve para derrotar A1 e A2 e para carimbar autenticidade no nosso build; não resiste a A3, e nenhuma opção resiste (seção A.1, fontes citadas).
- **O que quebra / não quebra:** edição casual morre; corrupção é detectada; save de fórum de outra máquina não carrega no Hardcore; quem recompila passa por tudo.
- **Custo de implementação:** médio. Um envelope, uma derivação de chave, um pack de conteúdo, o validador. **Manutenção:** baixa depois de estável; o formato versionado absorve migração.
- **Fluxo de trabalho:** balancear número em playtest exige regenerar o pack (um passo de build); editar mapa exige o editor selar; consertar save quebrado de jogador exige ferramenta de dump/re-selo dev-only (ver E.1).

### Opção 2: "Teto técnico" (custo alto, a máxima proteção alcançável de fato) <- RECOMENDADA

Tudo da Opção 1, com três elevações:

1. **Catálogo COMPILADO DENTRO do executável.** O gerador do repositório transforma a fonte canônica de conteúdo em tabelas C++ (`constexpr`/`std::array` de POCOs) na hora do build. Na distribuição **não existe arquivo de conteúdo em disco**: cartas, itens, inimigos e receitas são bytes do executável. É a proteção máxima possível para "itens": não há arquivo para editar, e adulterar exige recompilar ou patchear o binário, que é exatamente o ataque que NENHUM esquema detém (A3). Bônus: zero parser em runtime, zero I/O de conteúdo, o validador roda no build e vira erro de compilação. A prática de empacotar/pré-processar dado para formato de runtime é padrão da indústria ([Metric Panda, PAK files](https://www.metricpanda.com/rival-fortress-update-26-more-about-game-pak-files/); [GameDev.net, pack formats](https://gamedev.net/forums/topic/658896-how-to-pack-your-assests-into-one-binary-file-custom-file-format-etc/5170765/)); embutir no executável é o degrau acima disso.
2. **Save híbrido: snapshot + log de comandos com hash encadeado.** O save guarda o estado (snapshot AEAD, como na Opção 1) MAIS a lista de comandos desde o último ponto de verificação, com hash encadeado (cada entrada autentica a anterior). No load, e no CI, o jogo pode **re-executar o log sobre o snapshot anterior e conferir que o estado final bate byte a byte** (L-17 entrega isso de graça: replay é teste permanente do projeto). Isto é a única defesa que sobrevive ao adversário com fonte: para forjar um save que passe no replay do NOSSO build, o trapaceiro precisa produzir uma história de comandos que legitimamente alcance o estado desejado, ou seja, precisa jogar. Perdas honestas: load mais caro (replay), save maior, e toda mudança de regra exige versionar o motor de replay ou invalidar logs antigos (custo de manutenção real, ver E).
3. **Âncora anti-rollback com teto por hardware quando existir.** Além da âncora em arquivo out-of-band (o desenho já aprovado no Hardcore), onde a máquina tiver TPM 2.0 o contador monotônico pode viver no TPM, que é exatamente o mecanismo usado para detecção de rollback em firmware e boot ([linuxboot/heads, doc do TPM](https://github.com/linuxboot/heads/blob/master/doc/tpm.md)). Fallback para a âncora em arquivo quando não houver TPM. Custo: implementação por plataforma (TBS no Windows, `/dev/tpmrm0` no Linux) e mais casos de erro; é a única peça deste documento que eu marcaria como "teto opcional, avaliar preço na hora", porque só endurece o Hardcore contra um ataque (snapshot de pasta) que a âncora em arquivo já pega em parte, e continua perdendo para snapshot de disco inteiro.

- **Custo de implementação:** alto (gerador de tabelas, motor de replay versionado, ferramenta de selo compartilhada com o editor, TPM opcional). **Manutenção:** a mais alta das três, concentrada no replay versionado.
- **Fluxo de trabalho:** balancear número = editar fonte canônica e rebuildar (rápido com build incremental; o dado vira erro de compilação se inválido, o que playtest agradece); consertar save de jogador = ferramenta dev que abre, valida por replay, corrige e re-sela.
- **Hardcore:** é onde esta opção brilha: slot isolado, AEAD machine-bound, âncora (arquivo, TPM quando houver), log com hash encadeado, wipe lógico. Tudo que `modos-morte.md` pede, no teto do alcançável offline.
- **Por que recomendo esta:** a ordem do líder pede o teto ("quero maxima protecao"), e esta é a única opção cujo componente central (validação por replay) não evapora quando o adversário lê o fonte. O que dela é caro pode ser faseado: Opção 1 primeiro como fundação, itens 1 e 2 na sequência, item 3 só se o líder quiser pagar.

### Opção 3: "Binário com MAC, sem cifra fora do Hardcore" (custo baixo, piso honesto)

Envelope binário com **BLAKE2b com chave (keyed hash), sem cifrar** o payload fora do Hardcore; catálogo em pack binário com hash; Hardcore igual ao desenho já aprovado. É o mínimo que ainda cumpre "nada em texto": detecta A1 e A2, custa pouco, e é transparente para depuração (o payload binário é legível com a ferramenta de dump). Registro por dever de completude e para dar régua de preço; **não a recomendo** frente à ordem de máxima proteção, porque deixa na mesa a cifra (fricção real contra A2) e a validação por replay (a única peça à prova de fonte aberto).

### Comparação em uma linha

| | Detecta corrupção (A1) | Barra edição casual (A2) | Resiste a recompilação (A3) | Rollback casual (A4) | Save de terceiro (A5) | Custo |
|---|---|---|---|---|---|---|
| Opção 1 | Sim | Sim | Não | Hardcore: sim | Hardcore: sim | Médio |
| Opção 2 | Sim | Sim | **Parcial (replay semântico obriga a forjar história jogável)** | Hardcore: sim, com teto TPM | Hardcore: sim | Alto |
| Opção 3 | Sim | Razoável | Não | Hardcore: sim | Hardcore: sim | Baixo |

Nenhuma das três resiste ao jogador que recompila sem os checks: isso é matemática do FOSS, não falha de desenho (fontes na A.1).

---

## C. O catálogo de conteúdo (itens, cartas, inimigos, receitas, diálogos), sob a ordem "nada em texto"

A alternativa (a) do levantamento original (arquivo de dado em texto lido em runtime) está **descartada pela ordem do líder** e não volta aqui. As opções vivas são todas binárias:

### C.1 As formas binárias possíveis

1. **Tabelas compiladas no executável** (gerador do repositório emite C++ no build; o conteúdo vira `constexpr` dentro do binário). Máxima proteção possível para itens: não existe arquivo de conteúdo para editar na máquina do jogador. Validação vira erro de compilação. Custo: qualquer ajuste de número exige rebuild (segundos com build incremental); mod de conteúdo exige recompilar o jogo (que é FOSS, então é possível e legítimo, só não é conveniente).
2. **Pack binário externo selado** (arquivo único com índice, hash por entrada, tag do pack; o padrão PAK da indústria: [Metric Panda](https://www.metricpanda.com/rival-fortress-update-26-more-about-game-pak-files/), [GameDev.net](https://gamedev.net/forums/topic/658896-how-to-pack-your-assests-into-one-binary-file-custom-file-format-etc/5170765/); há packs comerciais com AES e assinatura ECC: [GameDev.net, packaging](https://gamedev.net/forums/topic/700709-how-to-package-the-assets-of-a-game-and-allow-only-the-engine-to-be-able-to-read/5398781/)). Proteção alta mas menor que a 1 (o arquivo existe e pode ser substituído junto com um patch no binário); em troca, atualizar conteúdo não exige rebuild do executável e o formato serve também ao mapa do editor.
3. **Híbrido, que é o que recomendo:** catálogo OFICIAL compilado no executável (forma 1); **mapas** (que nascem fora, no editor) e qualquer conteúdo pós-lançamento em pack binário selado (forma 2), com o MESMO envelope do save. Os dois temas do líder ficam com um formato e um selo comuns, como ele pediu.

### C.2 O que a integridade do catálogo protege DE VERDADE (honestidade exigida pelo brief)

INFERÊNCIA: o jogo é FOSS; a fonte do conteúdo estará legível no repositório público de qualquer forma (os números das cartas são visíveis para quem quiser ler o repo). Então a integridade do catálogo NÃO protege sigilo, e sim três coisas reais: (1) **correção de instalação**: o binário sabe com certeza criptográfica que o dado que carregou é o dado com que foi testado (instalação corrompida ou incompleta é detectada no boot, não vira bug fantasma); (2) **fricção de edição local**: mudar o ataque de uma carta deixa de ser "abrir arquivo e trocar número" e passa a ser "recompilar o jogo", o que converte trapaça acidental em ato deliberado; (3) **triagem de suporte**: relatório de bug com catálogo adulterado é identificável na hora. O que ela NÃO faz: impedir mod (impossível e indesejável em FOSS) ou esconder balanceamento (o repo é público).

### C.3 A fonte canônica do conteúdo no REPOSITÓRIO: a única contra-argumentação que devo ao líder nesta seção

Dever de contra-argumentar (L-11), exercido: **a ordem "nada em formato de texto" tem custo zero na distribuição e custo alto se aplicada também ao repositório.** Se as centenas de cartas forem mantidas como literais dentro de C++ escrito a mão, ou como binário opaco no repo, perde-se: revisão de balanceamento em pull request (diff de binário é ilegível), histórico de mudança número a número no git, e a possibilidade de o validador apontar linha do erro. E NÃO se ganha proteção nenhuma, porque o repositório é público por licença: o "texto" que o fraudador precisaria já está no fonte, em qualquer forma que ele esteja. **Mitigação que proponho, para decisão do líder:** fonte canônica de conteúdo em texto estruturado DENTRO do repositório (lida só pelo gerador, no build), e **nenhum texto na distribuição**: o jogador recebe apenas o executável com as tabelas compiladas e os packs binários selados. Isso cumpre a letra e a intenção da ordem (o que chega à máquina do jogador não tem nada em texto) e preserva o fluxo de trabalho de balanceamento. Se o líder quiser "nada em texto" incluindo o repo, obedece-se, com o custo nomeado acima; a decisão é dele.

---

## D. A pergunta da LEI ZERO: de onde vêm as primitivas criptográficas

O jogo pode ligar em exatamente duas coisas: GlintFx e SO (`GODS_LAWS.md`, LEI ZERO). Todo o desenho acima precisa de: hash criptográfico, MAC/hash com chave, AEAD (cifrar + autenticar), derivação de chave, bytes aleatórios e comparação em tempo constante. Três caminhos:

### Caminho (i): o GlintFx fornece <- o único sem exceção de lei

- FATO: o líder já decidiu, no bus, que criptografia está DENTRO do escopo do GlintFx 1.0, com monocypher como biblioteca a absorver (`inbox/gusworld/2026-08-20-cripto-DENTRO-decisao-do-lider.md`, estado FECHADO). FATO: o GlintFx foi reiniciado do zero em 21/08/2026 e hoje tem um único header. INFERÊNCIA: a decisão de escopo é do líder e presumo que sobrevive ao reboot, mas isso é confirmação que cabe a ele, não a mim (L-14).
- Consequência: o GusWorld registra a necessidade no bus e **espera parado** essa peça (LEI ZERO, verbatim). O que NÃO fica parado: envelope, serialização binária dos POCOs, validador semântico, gravação atômica, gerador de tabelas e todo o núcleo de regra são código puro sem criptografia, escrevíveis já (L-06). A criptografia é a última milha do selo, não a fundação.
- **Texto da necessidade concreta a registrar no bus, se o líder aprovar este caminho** (formulado como manda a L-07: o quê e para quê, sem prioridade, sem prazo): *"O GusWorld precisa, para selar e verificar arquivos locais (save, configuração, mapa e pack de conteúdo), de: (1) hash criptográfico (BLAKE2b ou equivalente); (2) MAC ou hash com chave; (3) AEAD para cifrar e autenticar um buffer com nonce e dados associados (XChaCha20-Poly1305 ou equivalente); (4) derivação de chave a partir de material de entrada; (5) geração de bytes aleatórios criptográficos; (6) comparação em tempo constante. Uso real: envelope binário de integridade dos arquivos do jogador e do conteúdo, e o slot Hardcore cifrado com chave ligada à máquina."*
- Prós: zero exceção de lei; uma implementação para as cinco plataformas do CI (L-09); o custo de "fazer cripto direito" fica onde o líder já o alocou. Contras: dependência de cronograma de outro projeto; a ressalva técnica registrada no bus (canal lateral, comparação que vaza tempo) passa a ser risco do GlintFx, e o GusWorld deve tratá-la como risco herdado conhecido.

### Caminho (ii): API do sistema operacional

- Nominalmente permitido pela LEI ZERO ("liga em GlintFx e no SO"). Na prática: no Windows existe API de primeira classe (CNG/BCrypt, e DPAPI com `CRYPTPROTECT_LOCAL_MACHINE` para atar dado à máquina: [Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata)); no Linux **não existe equivalente de userspace no SO em si**: a prática corrente é usar biblioteca (libsecret, OpenSSL, libsodium), que NÃO é o sistema operacional ([comparativo](https://codingtechroom.com/question/linux-data-protection-api-equivalent); [DZone, storing secrets in Linux](https://dzone.com/articles/storing-secrets-in-linux)).
- INFERÊNCIA dura: este caminho, dito honestamente, é "BCrypt no Windows e uma biblioteca de terceiro nos outros quatro alvos", ou seja, ele **degenera no caminho (iii) fora do Windows**, com o agravante de manter duas implementações divergentes na matriz de cinco plataformas (L-09). Não o recomendo; registro porque o brief manda tratá-lo.

### Caminho (iii): exceção nominal à LEI ZERO para uma biblioteca no GusWorld

- Sub-caminho (iii-a), **escrever primitivas em casa: desaconselho formalmente.** É a posição unânime da literatura e foi a ressalva do próprio CTO do GlintFx registrada no bus (falha por canal lateral e comparação que vaza tempo, modos que os testes não pegam). Não trago como opção viável; trago como alerta.
- Sub-caminho (iii-b), **vendorizar uma biblioteca auditada pequena** (monocypher, que é a mesma que o GlintFx pretende absorver, ou libsodium, cuja documentação é a referência das construções citadas: [doc](https://libsodium.gitbook.io/doc/secret-key_cryptography/aead/chacha20-poly1305/xchacha20-poly1305_construction)). Tecnicamente são as mesmas primitivas do caminho (i), disponíveis imediatamente. MAS: é um terceiro elo além de GlintFx e SO, portanto **só existe se o líder conceder exceção nominal, caso a caso, como a própria LEI ZERO prevê**. Nota de contexto: o projeto anterior fez exatamente isso (monocypher vendorizado, ADR-014), e o bus registra que aquela exceção era "do líder deles" e independente; pelo L-01 aquilo não é base, e a exceção não se herda sozinha, se re-decide.
- Prós: destrava o selo sem esperar cronograma alheio; primitiva auditada, não caseira. Contras: fura a pureza "só GlintFx + SO"; quando o GlintFx entregar cripto, ou se migra (retrabalho) ou o jogo carrega duas cripto (feio e confuso).

### Como os caminhos se combinam com as opções da seção B

INFERÊNCIA: qualquer opção da seção B funciona com qualquer caminho daqui; são eixos independentes. A combinação mais alinhada às leis é **Opção 2 (ou 1) + caminho (i)**, construindo agora tudo que não é cripto e registrando a necessidade no bus. Se o líder considerar inaceitável o selo esperar o GlintFx, a alternativa coerente é **exceção nominal temporária (iii-b) com cláusula de migração** para o GlintFx quando a cripto dele existir.

---

## E. Riscos e armadilhas (inclui a contra-argumentação devida, L-11)

1. **E.1, o maior custo operacional da ordem "tudo binário": a ferramenta de inspeção deixa de ser opcional.** Com texto, depurar save era abrir o arquivo; com binário selado, TODO fluxo de diagnóstico (playtest, bug report de jogador, QA, consertar save quebrado) passa por uma **ferramenta de dump e re-selo dev-only** que precisa existir desde o primeiro dia do formato, não depois. Mitigação: tratá-la como parte do entregável do envelope (mesmo PR), e o CI usá-la para validar fixtures. Sem ela, o time fica cego para o próprio formato.
2. **Config selada nunca pode brickar o jogo.** Config com selo inválido (ou corrompida) tem de cair em "reset para default com aviso", jamais em recusa de boot. Caso clássico: jogador força resolução errada e o jogo abre fora da tela; se a config é inviolável e o jogo não abre, a única saída dele vira apagar arquivo escondido. A proteção existe contra adulteração silenciosa, não contra o dono legítimo se socorrer.
3. **Machine-binding fora do Hardcore pune o jogador honesto.** Backup legítimo, PC novo, reinstalação de SO: tudo isso é direito do dono do dado (soberania de dado é princípio da casa). Recomendo machine-binding SÓ no slot Hardcore, onde o jogador aceitou a regra com aviso explícito (a copy aprovada em `modos-morte.md` já diz "reinstalar/trocar PC perde"). Save normal viaja com o dono.
4. **A dependência do editor de mapas é real e passa pelo bus.** Se o mapa é selado, o editor precisa selar com a MESMA biblioteca e versão de formato; divergência de versão entre selador e verificador é a nova classe de bug. Mitigação: a biblioteca do envelope vive num único lugar e o editor a consome; o contrato (formato + vetores de teste com hashes conhecidos) publicado no bus. Isso cria acoplamento de release entre dois projetos: nomeio o custo, a mitigação é vetores de teste compartilhados no CI dos dois.
5. **Canon desatualizado (L-13/L-14):** `modos-morte.md` referencia ADR-006/ADR-014, `FsSaveStore` e "Monocypher vendorizado" do projeto anterior, que não existem (L-01). Não declaro o documento morto; aponto que a decisão desta rodada deve dizer explicitamente o que daquele desenho sobrevive como intenção e o que se re-deriva, senão o próximo agente vai implementar contra referência fantasma.
6. **Replay como validador tem manutenção própria.** Cada mudança de regra muda o resultado do replay; logs antigos precisam de versionamento de regras ou de invalidação declarada na migração de save. Quem já quebrou determinismo em silêncio foi ponto flutuante e ordem de iteração de contêiner (a própria L-17 lista as armadilhas). O teste de replay do CI é o detector permanente; sem ele, a Opção 2 apodrece.
7. **Teatro de segurança na comunicação.** Nenhuma copy, changelog ou material público deve prometer "impossível editar", "impossível recuperar" ou "à prova de trapaça". O que se promete honestamente: "o jogo detecta arquivos alterados ou corrompidos" e, no Hardcore, "a morte apaga o save; não há recuperação PELO JOGO". A spec do Hardcore já manda isso; estendo a regra a todos os quatro alvos.
8. **Wipe físico não existe em disco moderno** (SSD com wear leveling, Btrfs CoW, pasta sob sync). O entregável é wipe lógico + unlink. Prometer mais é mentira técnica.
9. **Chave "escondida" no binário é fricção, não segredo.** Vale escrever no ADR para ninguém, no futuro, "melhorar" o esquema com ofuscação cara acreditando que compra segurança. Compra nada contra A3 (fontes na A.1) e custa manutenção.
10. **Licenças: catálogo compilado no executável mistura regimes.** O executável é AGPL (L-08); parte do conteúdo (Zona 2, pós 01/08/2026) é "direitos reservados" (`ASSETS-LICENSE.md` via L-08). Embutir dado de conteúdo dentro do binário AGPL pode arrastar esse dado para a obra combinada, ou pelo menos criar ambiguidade que um terceiro explorará. INFERÊNCIA minha de risco, não parecer jurídico: **antes de fechar a forma 1 do catálogo (tabelas compiladas), a questão vai ao Cláudio (CLO)**: statline de carta é código AGPL ou asset reservado? A resposta muda se o catálogo oficial pode ir dentro do executável ou deve ficar em pack externo com licença própria.
11. **TPM como âncora é teto opcional, não fundação.** Só existe em parte das máquinas, exige código por plataforma, e continua perdendo para snapshot de disco inteiro. Se entrar, entra como camada extra com fallback, nunca como requisito de jogar Hardcore.

---

## Resumo para a decisão (as perguntas que voltam ao líder)

1. **Qual opção da seção B?** Recomendo a Opção 2 (teto técnico), faseada: fundação = Opção 1; depois catálogo compilado + replay; TPM só se quiser pagar. Alternativas: Opção 1 (médio) ou 3 (piso).
2. **De onde vêm as primitivas (seção D)?** Recomendo caminho (i): registrar no bus a necessidade formulada acima e construir, sem esperar, tudo que não é cripto. Alternativa: exceção nominal temporária (iii-b) com cláusula de migração.
3. **A fonte canônica do conteúdo pode ser texto DENTRO do repositório, com distribuição 100% binária (C.3)?** Recomendo que sim (custo zero de proteção, ganho alto de fluxo); a alternativa literal (nada de texto nem no repo) obedece com o custo nomeado.
4. **Machine-binding só no Hardcore (E.3)?** Recomendo que sim.
5. **O que de `modos-morte.md` sobrevive como intenção (E.5)?** Confirmação do líder, exigida pela L-13/L-14. **RESPONDIDO em 24/08/2026, item `G3` do `TODO.md`: sobrevive todo o design (§1 a §5); só o plano de implementação (§6) morreu.**
6. **Rota ao CLO sobre conteúdo embutido no binário AGPL (E.10)?** Recomendo despachar antes de fechar a forma do catálogo.
