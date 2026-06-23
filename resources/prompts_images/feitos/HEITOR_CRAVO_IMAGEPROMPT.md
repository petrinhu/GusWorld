# Heitor Cravo — image prompt (nano banana)

## Contexto de lore (para a IA entender o personagem)

**Personagem:** Heitor Cravo, sub-diretor do Distrito 2 Norte da FIR (Forca de Inteligencia Regional), aparato de seguranca distrital sob a Sterling Corp. Antagonista lateral, possivel mini-boss da rota Periferia Norte no ato 2.
**Idade:** 43 anos (CANONICA - homem adulto endurecido, NUNCA jovem, NUNCA velho).
**Perfil:** operacional bruto formado em academia FIR, subiu por desempenho metrico frio (tres promocoes em oito anos). Autoridade militarizada de cartel industrial, presenca intimidadora, sem carisma - so disciplina e ameaca.
**Mundo:** megacidade ciber-gotica, sombria, controle corporativo. Heitor NAO tem aparato magico de personagem-crianca (sem oculos taticos / Matriz Ortodontica / Tavus-Drive - essas amarras de Pillar 3 sao exclusivas dos protagonistas). Heitor e puro hardware de comando: uniforme de oficial, arma lateral, prancheta.

---

## Production target (estilo e pipeline de render)

- Estilo: anime 3D estilizado, **cel-shaded** (bandas duras de toon shading, SEM gradiente PBR), low-poly. Mood cyber-gotico, sombrio, ameacador.
- Proporcao: **EXTREME CHIBI super-deformed (SD)**, ancorado em figura de vinil colecionavel (Funko Pop / nendoroid). Cabeca ENORME e redonda, do tamanho do corpo inteiro; personagem com apenas ~3 cabecas de altura; torso pequeno, bracos e pernas curtinhos e atarracados. NAO e corpo humano realista. (O gerador ignora chibi >50% das vezes - MARTELAR no inicio e no fim.)
- **Fundo branco solido absoluto `#FFFFFF`, shader Unlit, luz chapada uniforme (turnaround), ZERO sombra no plano de fundo.** Personagem isolado, full body de frente, pronto pra recorte e conversao em pixel art / sprite de jogo.
- Vista frontal de corpo inteiro, projecao ortografica, contato visual direto com a camera.

---

## Technical Prompt (corpo do prompt para o gerador)

An EXTREME chibi super-deformed (SD) figurine of a hard, menacing industrial-cartel sub-director man, styled like a small collectible vinyl figure (Funko Pop / nendoroid). CRITICAL PROPORTIONS: the head is enormous, perfectly round, and as large as the entire rest of the body combined; the whole character is only about three heads tall; tiny torso, very short stubby arms, very short stubby legs. This is NOT a realistic adult human body - it is a cute-but-threatening big-headed chibi figure. Anime 3D style, cel-shaded with hard toon bands, low-poly, no PBR.

AGE AND FACE (SIGNATURE - cold adult authority): a forty-three-year-old man with a hard, intimidating face on the giant round head. Heavy square jaw, hard narrowed cold eyes, thin diagonal SCAR across the left cheek, deep stern frown lines. Short slicked-back dark hair, neatly disciplined, going distinctly GREY at the temples. Cold, calculating, humorless expression - the face of a brutal operational officer who measures everything by metric performance, never warm, never amused.

ETHNICITY AND SKIN: light olive-tan weathered skin, matte and slightly hardened, the complexion of a career security operative; cool desaturated shading.

UNIFORM (SIGNATURE - officer-grade FIR, color-brand olive-green + gunmetal): an OFFICER-grade variant of the cartel security uniform, clearly sharper and more decorated than a common soldier's. A long stiff coat in dark OLIVE-GREEN and GUNMETAL-GREY, with rigid raised shoulder boards, a polished metallic cartel insignia on the chest, a fitted dark armored chest plate, tight dark gloves, and a wide command belt. The belt holds a sidearm holster and a short baton. Hard-surface parts (chest plate, insignia, belt buckle) catch crisp specular highlights; the coat is matte synthetic with subtle normal-mapped relief.

SIGNATURE PROP: a metal CLIPBOARD chained to his command belt by a short dark chain - the symbol of his cold metric-driven bureaucracy of control. Keep it small and clearly attached.

SILHOUETTE AND POSE: he stands WIDE and imposing despite the tiny chibi body, feet planted apart, chin slightly raised in cold authority, stubby gloved hands clasped behind his back (or one hand resting on the holster). Domineering officer stance.

COLOR PALETTE: dark olive-green + gunmetal-grey uniform base, black gloves and boots, cold steel insignia, muted desaturated cyber-gothic tones. No bright cheerful colors.

EXPRESSION: cold, hard, intimidating authority - narrowed eyes, set jaw, the look of a man who issues orders and expects metric compliance. Menacing mid-boss energy, never sympathetic, never comic.

Full body head to feet, centered, facing camera, frontal orthographic view, direct eye contact with the camera, plain pure-white `#FFFFFF` background, flat even studio lighting, crisp clean edges, no shadows on the background. REMINDER - EXTREME chibi: gigantic round head equal in size to the whole body, only three heads tall, tiny stubby limbs. Do NOT draw realistic adult proportions.

---

**Lore Notes for AI (reforco - tracos fracos a IA apaga, manter fortes):**
- Idade 43 e CANONICA - homem adulto endurecido, NUNCA jovem nem idoso.
- EXTREME chibi SD: cabeca redonda gigante = corpo inteiro, ~3 cabecas de altura, membros curtinhos atarracados. Ancora Funko Pop / nendoroid. NAO realista.
- Tracos-assinatura OBRIGATORIOS: cicatriz fina na bochecha esquerda, cabelo escuro penteado pra tras grisalho nas tempora, mandibula quadrada, olhar frio e ameacador.
- Uniforme de OFICIAL FIR (nao soldado comum): casaco olive-green + gunmetal-grey, ombreiras rigidas, insignia polida, peitoral blindado, luvas, cinturao de comando com coldre e cassetete.
- Prancheta metalica acorrentada ao cinturao = simbolo da burocracia metrica de controle.
- Silhueta larga e imponente, mãos cruzadas atras das costas, queixo erguido em autoridade fria.
- SEM aparato magico de protagonista (sem oculos taticos / Matriz Ortodontica / Tavus-Drive - isso e Pillar 3, exclusivo dos personagens-crianca jogaveis).
- Cor-marca: olive-green militar + gunmetal-grey, tons sombrios dessaturados. Mood cyber-gotico ameacador.
- Cel-shaded anime 3D low-poly. Normal mapping SO em hard-surface (peitoral/cinturao); specular alto SO em metal (insignia/fivela/corrente); resto flat cel-shaded.
- Fundo `#FFFFFF` unlit, full body frontal, pronto pra recorte + pixel art / sprite.
