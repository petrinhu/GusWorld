# Plano de Animacoes - GusWorld

Status: aprovado pelo lider 2026-06-23 (pacote OPCAO 2: completo + signature). Estetica: pixel art nativo PixelLab Pro, 128px, chibi, low top-down. Regra inquebravel: cada geracao reforca os tracos-assinatura canonicos do personagem (ver memoria feedback_reforcar_caracteristicas_canonicas_geracao).

## Matriz por papel

| Animacao | Party (7) | Inimigo combate | NPC cidade | Lore/conceito (~90) |
|---|:--:|:--:|:--:|:--:|
| 8-direcoes (giro 360 do splash) | sim | sim | sim | sim |
| breathing-idle (respira parado) | sim | sim | sim | sim |
| walk (8 dir) | sim | nao | sim | nao |
| run (8 dir) | sim | nao | opc | nao |
| battle-idle (pose de luta) | sim | sim | nao | nao |
| attack-melee (golpe fisico) | sim | sim | nao | nao |
| cast (conjurar script/magia) | sim | sim | nao | nao |
| hurt-fisico (knockback) | sim | sim | nao | nao |
| hurt-magia (shock/glitch "bug") | sim | sim | nao | nao |
| defend/guard | sim | sim | nao | nao |
| KO/down (derrotado) | sim | sim | nao | nao |
| revive/get-up | sim | opc | nao | nao |
| victory | sim | nao | nao | nao |
| signature/ultimate (1 epica/membro) | sim | nao | nao | nao |

## Direcoes por contexto
- **Splash**: 8 direcoes estaticas (turntable 360 em loop). TODOS.
- **Overworld** (walk/run/breathing-idle): 4-8 direcoes. Party + NPCs ativos.
- **Combate** (battle-idle/attack/cast/hurt/defend/KO/revive/victory): 1 direcao de batalha (frente/3-4). Party + inimigos.
- **Signature**: 1 anim epica. Ex: Gus = Dragon Victory (aura cyan->vermelho + Vyrdragon); Caua = electric combo; etc.

## Splash de apresentacao (primeiro encontro)
Moldura tematica por faccao. Esquerda: nome completo + "apelido" + resumo da lore. Direita: personagem girando 360 (8 rotacoes em loop) + respirando.

## Pipeline de geracao
- Novos: `create_character` (mode standard, 8 dir, 128, chibi, high detail, low top-down).
- Ja-feitos em 4 dir: `generate-8-rotations-v3` (REST v2) a partir do south (preserva o sprite, nao recria).
- Gus (image-based, REST v2): `animate-with-text-v3` por animacao (first_frame = rotacao + action descritiva).
- Animacoes MCP: `animate_character` (template) pros characters do MCP.
- Session limit ~70-80 geracoes/janela (reseta 4h America/Recife) -> rodar em ONDAS. Creditos: 2000/mes (folga).

## Ordem de execucao
1. **GUS completo** (piloto) -> integrar na engine SDL (preview/viewer) -> validar com o lider.
2. Resto da party (mesmo pacote completo + signature).
3. Inimigos do slice (quando o encontro for definido).
4. Lore (~90 secundarios): so giro (8 dir) + breathing-idle.
