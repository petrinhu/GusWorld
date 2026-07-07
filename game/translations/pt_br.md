# pt_br.md, traduções pt-br canônicas

> Locale primário (dev). **MUST** estar sempre completo. Outras locales fazem fallback aqui.
>
> Formato: `## CHAVE_UPPER_SNAKE` + valor abaixo. Ver engine/foundation/localization/MdTranslationLoader.cs.
>
> Última revisão: 2026-05-19.

---

## §1. Menu principal

## MENU_START_GAME
Iniciar jogo

## MENU_CONTINUE
Continuar

## MENU_NEW_GAME
Novo jogo

## MENU_LOAD_GAME
Carregar

## MENU_SAVE_GAME
Salvar

## MENU_OPTIONS
Opções

## MENU_CREDITS
Créditos

## MENU_QUIT
Sair

## MENU_QUIT_CONFIRM
Sair do jogo?

## MENU_QUIT_CONFIRM_YES
Sim, sair

## MENU_QUIT_CONFIRM_NO
Cancelar

---

## §2. Settings (acessibilidade canon CONTRACT §6)

## SETTINGS_TITLE
Configurações

## SETTINGS_VIDEO
Vídeo

## SETTINGS_AUDIO
Áudio

## SETTINGS_CONTROLS
Controles

## SETTINGS_ACCESSIBILITY
Acessibilidade

## SETTINGS_LANGUAGE
Idioma

## SETTINGS_BACK
Voltar

## SETTINGS_APPLY
Aplicar

## SETTINGS_REVERT
Reverter

## SETTINGS_RESET_DEFAULTS
Restaurar padrões

---

## §3. Acessibilidade (4 gates D1 canon)

## A11Y_REMAP_CONTROLS
Remapear controles

## A11Y_REMAP_KEYBOARD
Teclado

## A11Y_REMAP_GAMEPAD
Controle (gamepad)

## A11Y_REDUCE_MOTION
Reduzir movimento

## A11Y_REDUCE_MOTION_DESC
Desativa screen shake, motion blur e parallax. Reduz desconforto em jogadores sensíveis a movimento.

## A11Y_SUBTITLES
Legendas

## A11Y_CLOSED_CAPTIONS
Closed captions (descrição de áudio)

## A11Y_SUBTITLE_SIZE
Tamanho da legenda

## A11Y_SUBTITLE_SIZE_SMALL
Pequeno

## A11Y_SUBTITLE_SIZE_MEDIUM
Médio

## A11Y_SUBTITLE_SIZE_LARGE
Grande

## A11Y_SUBTITLE_BG
Fundo opaco nas legendas

---

## §4. Save / Load

## SAVE_SLOT_EMPTY
Slot vazio

## SAVE_SLOT_LABEL
Slot {0}

## SAVE_TIMESTAMP_LABEL
Salvo em {0}

## SAVE_LOCATION_LABEL
Local: {0}

## SAVE_PLAYTIME_LABEL
Tempo de jogo: {0}

## SAVE_CONFIRM_OVERWRITE
Sobrescrever este slot?

## SAVE_SUCCESS
Jogo salvo.

## SAVE_FAILED
Falha ao salvar. Verifique espaço em disco.

## LOAD_CONFIRM
Carregar este save? Progresso não salvo será perdido.

## LOAD_SUCCESS
Save carregado.

## LOAD_FAILED
Falha ao carregar save. Arquivo pode estar corrompido.

## LOAD_MIGRATION_REQUIRED
Save em versão anterior. Migrando...

---

## §5. HUD + combate (placeholders)

## HUD_HP_LABEL
HP

## HUD_AP_LABEL
AP

## HUD_MANA_LABEL
MANA

## HUD_ACTION_LABEL
AÇÃO

## HUD_TURN_LABEL
Turno

## COMBAT_PLAYER_TURN
Seu turno

## COMBAT_ENEMY_TURN
Turno do inimigo

## COMBAT_BANNER_BATTLE
BATALHA!

## COMBAT_INTRO_ENCARAR
[Enter] Encarar

## COMBAT_INTRO_AUTORESOLVE
[Q] Resolver sem encarar

## COMBAT_BANNER_PLAYER_TURN
Vez de {0}

## COMBAT_BANNER_ENEMY_TURN
Vez de {0}

## COMBAT_BANNER_CHOOSE_ACTOR
Escolha quem age

## COMBAT_SELECT_TARGET
Escolha o alvo:

## COMBAT_VICTORY
Vitória.

## COMBAT_DEFEAT
Derrota.

## COMBAT_DEFEAT_BARK
{0}: Opa. Reboot em 3, 2...

## COMBAT_DEFEAT_BARK_GENERIC
Reboot em 3, 2...

## COMBAT_DEFEAT_CHESS_NOTE
No xadrez, quando o rei cai, a partida acaba.

## COMBAT_FLEE
Fugir

## COMBAT_FLEE_FAILED
Fuga falhou.

## COMBAT_FLEE_SUCCESS
Conseguiu fugir.

## COMBAT_PANEL_ENEMIES
INIMIGOS

## COMBAT_PANEL_ACTIONS
AÇÕES

## COMBAT_PANEL_HAND
MÃO DE CARTAS

## COMBAT_ACTION_ATTACK
Atacar

## COMBAT_ACTION_DEFEND
Defender

## COMBAT_ACTION_SCAN
Scan

## COMBAT_ACTION_GAMBIT_PREDICT
Gambito: Prever

## COMBAT_ACTION_GAMBIT_REORDER
Gambito: Reordenar (+1)

## COMBAT_VERB_GAMBITO
Gambito

## COMBAT_VERB_COMPILAR
Compilar

## COMBAT_ACTION_USE_CARD
Usar Carta

## COMBAT_ACTION_PASS
Passar

## COMBAT_ACTION_RESTART
Reiniciar

## COMBAT_TURN_ROUND
Rodada {0}: {1}

## COMBAT_LOG_ACTION
[{0}] {1} -> {2} ({3})

## COMBAT_LOG_ACTION_NOTARGET
[{0}] {1}

## COMBAT_LOG_STATUS_APPLIED
[{0}] status {1} (x{2}, {3} turno(s))

## COMBAT_LOG_STATUS_EXPIRED
[{0}] status {1} expirou

## COMBAT_LOG_DEFEATED
[{0}] derrotado

## COMBAT_LOG_INCAPACITATED
[{0}] incapacitado

## COMBAT_LOG_COMPILED
COMPILADO: {0}

## COMBAT_LOG_COMPILE_ERROR
ERRO DE COMPILAÇÃO

## COMBAT_ACTOR_RESOURCES
AP {0}/{1} | Mana {2}/{3}

---

## §6. Dialogue (placeholders, expandir conforme arcos)

## DIALOGUE_CONTINUE
Continuar

## DIALOGUE_CHOICE_PROMPT
Escolha:

## DIALOGUE_GUS_INTRO_001
Meus óculos táticos detectaram uma anomalia. Sterling está fazendo algo errado.

## DIALOGUE_GUS_INTRO_002
Onze anos não é jovem demais pra perceber que os adultos mentem.

### NPC Bertoldo (M7-DIALOGO, npc_intro, blueprint F2-N.1 docs/design/narrativa/dialogue-tree-npc-intro.md)
### Prosa final de voz (narrative-writer, polimento pos-playtest ao vivo do lider): velho tecnico de praca,
### seco-afetuoso, meia-frase, respeita a inteligencia do Gus (P4). Grafo/estrutura inalterados (§2 blueprint).

## DIALOGUE_NPC_INTRO_N0_GREET
Cedo pra rua, moço. Essa cidade... nunca soube dormir direito.

## DIALOGUE_NPC_INTRO_N1_HOOK
Aquilo ali, ninguém mais para pra olhar. Você parou. Isso já diz alguma coisa sobre você.

## DIALOGUE_NPC_INTRO_CHOICE_CURIOSO
O desenho ali não fecha. Tem um padrão, eu sei que tem.

## DIALOGUE_NPC_INTRO_CHOICE_PRAGMATICO
É perigoso? Por onde eu sigo?

## DIALOGUE_NPC_INTRO_CHOICE_SECO
(Só aceno e sigo andando.)

## DIALOGUE_NPC_INTRO_N2A_CURIOSO
Se eu contasse, estragava a graça. Olha direito, moço.

## DIALOGUE_NPC_INTRO_N2B_PRAGMATICO
Perigo tem em toda esquina. Segue reto que o resto se mostra.

## DIALOGUE_NPC_INTRO_N2C_SECO
Também não sou de conversa fiada. Vai.

## DIALOGUE_NPC_INTRO_N3_RECONVERGE
Vai andando. Essa praça já viu gente demais sair, e não voltar do jeito que foi.

---

## §7. Errors + mensagens sistema

## ERROR_GENERIC
Algo deu errado.

## ERROR_FILE_NOT_FOUND
Arquivo não encontrado: {0}

## ERROR_INVALID_SAVE
Save inválido ou corrompido.

## ERROR_SAVE_VERSION_NEWER
Save criado em versão mais nova do jogo. Atualize o jogo pra carregar.

## INFO_AUTOSAVE
Salvamento automático em {0} minuto(s).

## INFO_AUTOSAVE_DONE
Salvamento automático concluído.

---

## §8. Nomes de dados de combate (cartas, status, famílias, combos)

> Estes valores são as KEYS que a camada de DADOS de combate carrega no campo DisplayName
> (ComboTable, Card, etc). O runtime de combate carrega a KEY; o DISPLAY/UI (F2-G.5) resolve
> key→pt-br via Localization.TrMd. Nomes pt-br canônicos: combat.md §6 (famílias), §9 (status), §10 (combos).

### Famílias de carta (combat.md §6, FAMILY_<id>_NAME)

## FAMILY_ELETRICO_NAME
Elétrico

## FAMILY_BIOQUIMICO_NAME
Bioquímico

## FAMILY_SONICO_NAME
Sônico

## FAMILY_CINETICO_NAME
Cinético

## FAMILY_CRIPTOGRAFICO_NAME
Criptográfico

### Status (combat.md §9, STATUS_<id>_NAME)

## STATUS_STUN_NAME
Atordoamento

## STATUS_POISON_NAME
Veneno

## STATUS_CORRODE_NAME
Corrosão

## STATUS_DISRUPT_NAME
Interferência

## STATUS_SILENCE_NAME
Silêncio

## STATUS_KNOCKBACK_NAME
Repulsão

## STATUS_BREAK_NAME
Fratura

## STATUS_EXPOSE_NAME
Exposição

## STATUS_DECRYPT_NAME
Decifração

## STATUS_SHIELD_NAME
Escudo

## STATUS_REGEN_NAME
Regeneração

## STATUS_HASTE_NAME
Aceleração

## STATUS_SLOW_NAME
Lentidão

### Combos curados (combat.md §10, COMBO_<comboId>_NAME)

## COMBO_PULSO_STREAM_NAME
Descarga Dupla

## COMBO_RAIZ_NULL_NAME
Antídoto Inverso

### Nomes de atores do slice (ACTOR_<id_upper>; fallback = id lowercase se ausente)

## ACTOR_GUS
Gus

## ACTOR_CAUA
Cauã

## ACTOR_JACI
Jaci

## ACTOR_BERTOLDO
Seu Bertoldo

## ACTOR_SENTINELA_BIT
Sentinela Bit

## ACTOR_DAEMON_GUARD
Daemon Guard

### Cartas placeholder do slice (CARD_<id>_NAME; deck real = CardRepository futuro)

## CARD_PULSO_ELETRICO_NAME
Pulso Elétrico

## CARD_SCAN_BASICO_NAME
Varredura Básica

## CARD_RAIZ_CURA_NAME
Raiz Curativa

---

## §9. Menu de sistema (pausa + config de som, MENU-PAUSA-CONFIG-SOM)

## MENU_SYSTEM_KICKER
Sistema

## MENU_PAUSE_TITLE
Pausado

## MENU_PAUSE_HINT
{0} confirma, {1} volta ao jogo

## SETTINGS_MUSIC_VOLUME
Volume da Música

## SETTINGS_SFX_VOLUME
Volume dos Efeitos (SFX)

## MENU_PLACEHOLDER_TEXT
Em breve.

---

## §10. Controles (tela de remap de teclado, M2)

## CONTROLS_HINT
Selecione uma ação e pressione a nova tecla · Esc cancela a captura

## CONTROLS_CAPTURE_PROMPT
Pressione uma tecla…

## CONTROLS_COL_ACTION
Ação

## CONTROLS_COL_KEYBOARD
Teclado

## CONTROLS_COL_GAMEPAD
Controle

## CONTROLS_NAV_HINT
▲▼ navega · Enter remapeia · Esc volta

## CONTROLS_GROUP_MOVEMENT
Movimento

## CONTROLS_GROUP_WORLD
Mundo

## CONTROLS_GROUP_COMBAT
Combate

## CONTROLS_GROUP_MENU_DIALOGUE
Menu & Diálogo

## CONTROLS_SWAP_NOTICE
(!) trocou de tecla com: {0}

## CONTROLS_RESTORE_CONFIRM_TITLE
Restaurar os controles para o padrão de fábrica?

## CONTROLS_RESTORE_CONFIRM_YES
Sim, restaurar

## CONTROLS_RESTORE_CONFIRM_NO
Cancelar

## CONTROLS_NO_BINDING
—

### Rótulos das 30 ações (ACTION_<nome> = ActionDefinition::label_i18n_key,
### gus/domain/input/action_registry.cpp - NÃO renomear as chaves sem atualizar
### o registry em C++)

## ACTION_MOVE_FORWARD
Andar para frente

## ACTION_MOVE_BACKWARD
Andar para trás

## ACTION_MOVE_LEFT
Andar para a esquerda

## ACTION_MOVE_RIGHT
Andar para a direita

## ACTION_MOVE_RUN
Correr

## ACTION_INTERACT
Interagir / Falar

## ACTION_MENU_OPEN
Abrir menu

## ACTION_MENU_CLOSE
Fechar menu

## ACTION_MENU_CONFIRM
Confirmar

## ACTION_MENU_CANCEL
Cancelar / Voltar

## ACTION_MENU_NAV_UP
Navegar para cima

## ACTION_MENU_NAV_DOWN
Navegar para baixo

## ACTION_MENU_NAV_LEFT
Navegar para a esquerda

## ACTION_MENU_NAV_RIGHT
Navegar para a direita

## ACTION_COMBAT_ATTACK_BASIC
Ataque básico

## ACTION_COMBAT_DEFEND
Defender

## ACTION_COMBAT_CAST
Conjurar (carta)

## ACTION_COMBAT_CARD_1
Carta 1

## ACTION_COMBAT_CARD_2
Carta 2

## ACTION_COMBAT_CARD_3
Carta 3

## ACTION_COMBAT_END_TURN
Encerrar turno

## ACTION_DIALOGUE_CONTINUE
Continuar diálogo

## ACTION_DIALOGUE_SKIP
Pular diálogo

## ACTION_DIALOGUE_CHOICE_1
Escolha 1

## ACTION_DIALOGUE_CHOICE_2
Escolha 2

## ACTION_DIALOGUE_CHOICE_3
Escolha 3

## ACTION_DIALOGUE_CHOICE_4
Escolha 4

## ACTION_INVENTORY_OPEN
Abrir inventário

## ACTION_INVENTORY_CLOSE
Fechar inventário

## ACTION_DIARY_OPEN
Abrir diário
