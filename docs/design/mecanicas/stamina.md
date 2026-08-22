# Carga do Aparato (mecanica de "stamina" re-enquadrada)

Status: design CANONICO + NUMEROS aprovados pelo lider 2026-06-23. Proposta-base: lead-game-designer; numeros: economy-designer.

## Por que NAO e "stamina fisica"
O `pillars.md` tem anti-pillar literal ("nao e souls-like, sem stamina-management") e o Pillar 4 define o Gus como prodigio ANALITICO (HP baixo, sem forca bruta, nao atleta). Logo, "cansaco fisico gerenciavel" esta VETADO. A mecanica foi re-enquadrada.

## Natureza: ACOPLAMENTO (decisao-mae)
A barra e a **Carga do Tavus-Drive** (energia do aparato de pulso; o poder vem do hardware, Pillar 3). Quando a Carga esgota, quem paga o **overflow** e o **corpo do Gus**: tontura, ofegancia (Pillar 4 = vulnerabilidade do corpo, sem virar atleta). Uma barra, dupla leitura: numero = hardware; feedback corporal = consequencia. A respiracao ofegante ja implementada (idle "cansado") e o feedback canonico desse overflow.

## Escopo: recursos DISTINTOS por camada (mesma fonte diegetica = o aparato)
- **Overworld**: barra de **Carga** (continua). E o que entra no vertical slice.
- **Combate turn-based**: usa o AP/mana de compilacao que JA existe no GDD; NAO usa a barra de Carga (protege o combate do anti-pillar de stamina-management; Pillar 1 = combate por logica, sem pressao de tempo).

## Custo e regeneracao
- **Drena**: sprint (correr); scan ambiental sustentado (modo sonar/Perlin do Pillar 2) [scan = pos-VS ou VS conforme escopo].
- **Regenera**: andar normal e parar; zona-segura (cidade-hub) recupera mais rapido [pos-VS]; Celula de Pulso (consumivel) [pos-VS].
- **REGRA DE OURO (inegociavel)**: a Carga NUNCA trava o deslocamento basico. Esgotada, o Gus so perde o sprint e os modos ativos (scan), JAMAIS a capacidade de andar e progredir. Sem "parede de exaustao" - e isso que mantem o anti-pillar respeitado.

## Modelo numerico (CANONICO, aprovado 2026-06-23; economy-designer)
Barra **continua** [0..max], todos os numeros seguem a **sequencia recorrente canonica** (1, 1, 2, 3, 5, 8, 13, 21...). Recarga FORA da economia de credito (Bio-Ampola continua so HP).

| Parametro | Valor | Deriva |
| :--- | :--- | :--- |
| Carga maxima | **89** | dash util sem virar maratona |
| Drain do sprint | **8 /s** | cheio (89) -> 0 em ~11 s de sprint continuo |
| Regen PARADO | **13 /s** | 0 -> cheio em ~6.8 s (parar recompensa) |
| Regen ANDANDO | **5 /s** | 0 -> cheio em ~17.8 s (explorador nunca trava) |
| Limiar de ofegancia | **34** (unidade, ~38%) | corpo paga overflow; dispara so apos ~7 s de sprint continuo |
| Curva | **linear** | concava (respiro inicial) so se playtest N=3 mostrar abuso andar-parar |
| Celula de Pulso (pos-VS) | **+34** (rara: +89), drop curado, sem credito | espelha a Bio-Ampola; nao farmavel |

Nota: limiar e em UNIDADE de Carga (34), nao percentual - ofegancia le como "Carga acabando" (sobrecarga do aparato), nao "metade" (que soaria cansaco fisico = anti-pillar). Drains curtos quase nunca ofegam.

## Timer de folego (corpo) vs Carga (aparato) - decisao do lider 2026-06-23

Sao DUAS coisas distintas, com ritmos proprios:

- **Carga do aparato** (`core::player::Stamina`): a energia do Tavus-Drive. Drena correndo (8/s) e REGENERA RAPIDO ao parar (13/s). E o numero do hardware.
- **Folego do corpo** (`core::player::WindedTimer`, POCO novo): a respiracao ofegante do Gus. Tem INERCIA: o peito demora a acalmar mesmo com o aparato ja recarregado.

**Por que separar.** Atado so a Carga, o idle ofegante durava ~2-3 s (a Carga sobe a 13/s e cruza o limiar 34 quase imediato ao parar). Sentia curto demais no playtest do M1. O folego do corpo passa a ter timer proprio, independente da Carga.

**Regra do timer de folego (numeros finais, lider 2026-06-23):**

| Parametro | Valor | Deriva |
| :--- | :--- | :--- |
| Folego MINIMO ao parar | **5 s** | piso: parou de correr (acima do limiar) -> ofega ao menos 5 s |
| Folego MAXIMO (teto) | **8 s** | corrida longa nao ofega alem disso |
| Corrida ate o teto | **8 s** | correr 8 s sustentados atinge o teto de folego (escala LINEAR ate aqui) |
| Limiar de corrida | **2 s** | correr menos que isso e parar NAO ofega (corrida curta) |

Formula da duracao ao parar: `folego = 5 + (8 - 5) * clamp(tempo_correndo / 8, 0, 1)` segundos. Saturada no teto. Decai linearmente com o tempo parado ate zerar.

**Integracao (OverworldSim):** o timer e dirigido pelo MESMO `move_state` real. Correndo (`Running`) acumula; qualquer outro estado conta como "parou de correr" (dispara o folego na transicao e decai parado). O idle OFEGANTE e forcado quando `Carga < limiar` OU `folego ativo` (`show_winded_idle()`); voltar a correr limpa o folego e reinicia o acumulo. Tudo ajustavel no `OverworldTuning` (`winded_*`). Numeros em `core::player::WindedConfig`.

## Feedback (multimodal, a11y - Pillar 4 nunca depende de 1 canal)
- Animacao (JA existe): idle calmo (procedural, descansado) vs respiracao ofegante (overflow).
- Aura do aparato: glow ciano firme -> esmaece pra ambar/whine quando a Carga esgota.
- HUD diegetico: anel/barra discreto junto do HUD do triangulo de hardware.
- [pos-VS] Audio (whine de subtensao); [pos-VS] recarga DEGRADANDO gradual = pista da sabotagem do Dante (foreshadow mecanico, pillars.md:258).

## Escopo no vertical slice
- **VS**: SO overworld. Carga (continua) drena no sprint, regenera parado/andando, regra anti-parede, + 2 feedbacks novos (aura esmaecendo + anel HUD) alem da respiracao ja feita.
- **Pos-VS**: scan consumindo Carga, Celula de Pulso (recarga), foreshadow-Dante, e o recurso de combate (se algum dia cruzar camadas).

## Implementacao (estado atual)
Stub funcional ja existe: `core::player::Stamina` (POCO) + `core::anim::BreathOscillator` (idle calmo procedural) + idle ofegante (5 frames) por threshold no `OverworldSim`. Valores atuais sao ad-hoc (max 100, drain 25/s, recover 18/s, threshold 50) e serao substituidos pelos do economy-designer. Renomear "Stamina" -> "Carga"/aparato na fase de implementacao final.
