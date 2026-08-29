# B6 — Conferência visual das 1263 imagens de `resources/` e `assets/`

Item `B6` (prioridade Alta, onda 0) da tabela de pendências. QA responsável: `qa-engineer`.

## Método

O método indireto (metadado/EXIF/nome de arquivo) já havia rodado antes e não achou captura de tela — mas
nenhum olho humano tinha passado pelas imagens, e o repositório é público. Enumeração, não busca dirigida:
**toda imagem foi olhada**, não amostrada por hipótese, porque busca dirigida acha só o que já se suspeita.

1. Inventário: `find resources assets -type f \( -iname "*.png" -o -iname "*.jpg" -o -iname "*.jpeg" \)` →
   **1263 arquivos**, todos dentro de `resources/` (`find assets/` devolveu **0** — o diretório `assets/`
   não contém imagem nenhuma nesta árvore, apesar de citado no escopo do item).
2. Lista ordenada alfabeticamente (`sort`) e partida em 13 blocos de até 100 caminhos
   (`split -l 100 -d -a 2`), preservando a ordem para rastreabilidade caminho↔posição-na-folha.
3. Para cada bloco, uma folha de contato com `montage` do ImageMagick 7.1.2 (confirmado instalado antes
   de usar, per L-28): `montage @chunk_NN -label '%f' -background '#202020' -fill white -tile 10x10
   -geometry 160x160+2+2 sheet_NN.png` → 13 folhas de 1640×1640px, cada célula rotulada com o nome do
   arquivo. Um manifesto (`nl -ba` do mesmo chunk) mapeia posição-na-grade → caminho completo.
4. Cada uma das 13 folhas foi **olhada** (ferramenta de leitura visual), célula por célula, procurando:
   janela de SO, barra/abas de navegador, texto de terminal, cursor de desktop, fotografia de tela real
   (moiré, reflexo, borda de monitor), rosto humano, documento pessoal, nome próprio legível.
5. Achado suspeito por imagem → identificado no manifesto pelo nome de arquivo, localizado o caminho
   completo, e conferido em tamanho real (`identify`/leitura direta do arquivo) antes de classificar
   severidade — a imagem renderizada é a prova; metadado e nome de arquivo são só pista.

Artefatos de trabalho (não versionados): `/var/tmp/gw_b6/` — `all_images_sorted.txt` (a lista completa
das 1263), `chunk_00..12`, `manifests/manifest_00..12.txt`, `sheets/sheet_00..12.png`.

## Cobertura

**1263 de 1263 imagens olhadas em folha de contato (13 folhas de até 100 células cada).**

| Folha | Imagens | Faixa alfabética (1º arquivo → último) |
|---|---|---|
| sheet_00 | 100 | `resources/buymecoffe.png` → `resources/sprites/antoneto_chevalier/west.png` |
| sheet_01 | 100 | `resources/sprites/atelaia_chevalier/east.png` → `resources/sprites/bento_requiem/anims/defend/2.png` |
| sheet_02 | 100 | `resources/sprites/bento_requiem/anims/defend/3.png` → `resources/sprites/bento_requiem/walk/walk_north_1.png` |
| sheet_03 | 100 | `resources/sprites/bento_requiem/walk/walk_north_2.png` → `resources/sprites/felicia_tarsila/north.png` |
| sheet_04 | 100 | `resources/sprites/felicia_tarsila/south.png` → `resources/sprites/icons-m5/retratos/retrato_seu_bertoldo_caim.png` |
| sheet_05 | 100 | `resources/sprites/icons-m5/retratos/retrato_sterling.png` → `resources/sprites/lin_torun/south.png` |
| sheet_06 | 100 | `resources/sprites/lin_torun/west.png` → `resources/sprites/personagens_inspirados/gus/anims/attack_melee_east/f5.png` |
| sheet_07 | 100 | `resources/sprites/personagens_inspirados/gus/anims/attack_melee_east/f6.png` → `resources/sprites/personagens_inspirados/gus/anims/run_east/f8.png` |
| sheet_08 | 100 | `resources/sprites/personagens_inspirados/gus/anims/run/f0.png` → `resources/sprites/solange_vix/east.png` |
| sheet_09 | 100 | `resources/sprites/solange_vix/north.png` → `resources/vfx/boot_pixel/frame_16.png` |
| sheet_10 | 100 | `resources/vfx/boot_pixel/frame_17.png` → `resources/vfx/transition_test/boot/boot_00092.png` |
| sheet_11 | 100 | `resources/vfx/transition_test/boot/boot_00093.png` → `resources/vfx/transition_test/boot/boot_00192.png` |
| sheet_12 | 63 | `resources/vfx/transition_test/boot/boot_00193.png` → `resources/vfx/transition_test/concept_b_scanline/variant_3.png` |

**1263 de 1263 imagens olhadas. Zero pulos, zero amostragem por hipótese.**

## Nota de conformidade — ordem de estrangulamento de memória (recebida em 29/08/2026, meio da tarefa)

As 13 folhas acima já tinham sido **geradas e integralmente olhadas** quando a ordem de estrangulamento
chegou (máquina medida em 31 GiB RAM com ~1 GiB livre e 13-17 GiB de swap em uso). Elas foram produzidas
**sem** os limites explícitos de `-limit memory/map/thread` e em lotes de 100 (não 64), porque nessa altura
a pressão de memória ainda não tinha sido reportada. Nenhum comando morreu por sinal 9 e nenhuma folha
precisou ser refeita — não há, portanto, lote estrangulado a reportar. Assim que a ordem chegou:

- Parei de gerar novas folhas (a enumeração já estava 100% completa, nada ficou pendente).
- **Apaguei as 13 folhas de `/var/tmp/gw_b6/sheets/`** imediatamente (item 6 da ordem), já que cada uma
  tinha sido lida e seu conteúdo registrado neste relatório.
- Uma checagem complementar pequena (decodificar 2 QR codes vistos como prop de jogo na sheet_00) **não
  foi executada**: a ferramenta `zbarimg` não está instalada nesta máquina e, por L-28, não instalo nada
  sem autorização — reportado como não executado, não improvisado. Não bloqueia o veredicto abaixo: o
  achado é um prop declaradamente de jogo (ícone estilizado, não foto de tela), fora do padrão de
  vazamento que este item audita (janela de SO / navegador / terminal / desktop / rosto real / documento).

Comandos de identificação em lote usados **antes** da ordem chegar (`identify -format ... @lista` sobre as
1263 imagens, para mapear dimensões) já haviam terminado com sucesso e não foram repetidos.

## Achados

### Severidade CRÍTICA / ALTA — vazamento de tela pessoal do líder

**Nenhum encontrado.** Nas 1263 imagens não apareceu janela de sistema operacional, barra ou abas de
navegador, texto de terminal real (hostname, prompt de shell, caminho de arquivo do usuário), cursor de
mouse de desktop, fotografia de tela real (moiré, reflexo, borda de monitor), rosto humano fotografado,
documento pessoal ou nome próprio do líder/família legível em qualquer imagem. Todo o conjunto é asset de
jogo: sprites de personagem (paperdoll 2D, 4 direções), ícones de UI/HUD, concept art (retrato/frente com
fundo branco, gerado ou ilustrado — sem indício de fotografia real: sem moiré, sem reflexo, sem borda de
monitor), texturas de mundo (arquitetura, flora, fauna, veículos, props), e uma animação de boot de
terminal **fictício e in-fiction** (240 frames em `resources/vfx/transition_test/boot/`, mais
`resources/vfx/boot_pixel/` e `resources/vfx/boot_pixel_test/`) com texto tipo `LOADING SYSTEM...
MOUNTING DRIVES... OK INITIALIZING CORE... OK` e linhas de memória fabricadas/embaralhadas propositalmente
(ex.: `*PMB:f413264002: 10079) { R5%f41.0B); OF 0.000.0 CED-r-obg; ...`) — flavor text de sci-fi retro para
o próprio jogo, sem hostname real, sem caminho de usuário, sem valor financeiro real, sem nome de pessoa.

### Severidade BAIXA / informativa

1. **`resources/vfx/boot_pixel_test/REJECTED_pro_hallucination_frame001.png`** — frame de teste de boot
   que renderiza o texto `CYBERDYNE SYSTEMS CORP. / MODEL 101A TERMINAL / (C) 1984-2029` — referência
   reconhecível à Cyberdyne Systems e ao "modelo 101" da franquia *Terminator*. Não é vazamento de dado
   pessoal (é conteúdo fictício gerado), mas é risco de IP/marca registrada num repositório público FOSS.
   O próprio nome do arquivo (`REJECTED_..._hallucination_...`) indica que já foi descartado
   criativamente, mas o arquivo permanece versionado. **Não decidi remover** — por L-14, nada é declarado
   morto/descartável por agente; a decisão é do líder. Recomendação: líder avaliar remoção ou substituição
   antes de release/divulgação pública ampla do repositório.
2. **2 QR codes** vistos como prop/ícone de jogo em `sheet_00` (grade estilizada, não foto de tela) — não
   decodificados (ferramenta `zbarimg` ausente; por L-28 não instalo sem autorização). Como são ícones de
   UI declaradamente de jogo, e não capturas de tela, ficam fora do escopo estrito deste item; se o líder
   quiser confirmar o conteúdo codificado, é uma checagem trivial e independente de instalar `zbarimg` (ou
   equivalente) — não bloqueia o veredicto de B6.
3. **`assets/` está vazio de imagem** (`find assets -type f \( -iname "*.png" -o -iname "*.jpg" -o
   -iname "*.jpeg" \)` → 0) — o escopo do item citava `resources/` **e** `assets/`, mas as 1263 imagens
   estão todas em `resources/`. Cobertura de `assets/` é 0 de 0 (diretório sem imagem), não "não
   verificado".

## Veredicto

**APROVADO.**

1263 de 1263 imagens de `resources/` (mais 0 de 0 de `assets/`, que não contém imagem) foram enumeradas em
13 folhas de contato e olhadas integralmente. Nenhuma captura de tela pessoal do líder, nenhuma foto real,
nenhum documento pessoal e nenhum nome próprio legível foi encontrado. Os dois achados de severidade
baixa (referência de marca ao *Terminator* num asset já marcado como rejeitado; QR codes de prop não
decodificados) não configuram vazamento de dado pessoal e ficam registrados para decisão do líder, sem
bloquear a aprovação deste item quanto ao seu escopo (privacidade/vazamento de tela).
