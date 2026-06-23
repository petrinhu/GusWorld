# Style Guide — GusWorld

> **NOTA DE STACK (pós-ADR-008).** As diretrizes de ARTE deste doc (proporção, paleta, silhueta, color script, budget de polígonos, estratégia de texture/shader cel-shaded) são canônicas e independem de engine. Os nomes de API antigos que aparecem nas seções técnicas (5 Lighting, 8 Budget, 10 Shader) foram escritos sobre o stack Godot 4, depois superado pela engine própria C++20 + SDL3 (ver ROADMAP.md e o ADR-008). Leia esses nomes como referência conceitual (luz direcional, ambiente/fog, luzes pontuais, shader custom); serão re-mapeados para a engine atual pelo squad técnico. O conteúdo visual em si não muda.

Solo G1 indie, engine própria C++20 + SDL3, **3D cel-shaded com proporção SD 1:1:1** (chibi-SD com extremidades ectomorfas, NÃO chibi inflado). Câmera 3/4 rotacional + zoom orbital (Chrono Trigger-ish, mas 3D real). Documento vivo. Toda decisão visual valida contra `docs/design/pillars.md`.

**Spec mestre de personagem:** `Resources/gusworld/character-spec-gus.md` (canônica). Sheet de produção: `docs/art/characters/gus.md`.

---

## 1. Vision statement

GusWorld é **low-poly stylized 3D com leitura de pixel-art**: silhuetas chapadas, gradient ramps, cor saturada deliberada, zero PBR. Duas paletas opostas (cidade neon ciano/magenta × floresta biolúmen verde/violeta) que nunca se misturam por preguiça. O protagonista é o único ponto quente em paisagens frias.

## 2. Referências canônicas

| Ref | Por que importa |
|---|---|
| **Sea of Stars** (Sabotage, 2023) | Pipeline solo-ish: 2D pré-renderizado com 3D leve; readability cinematográfica em ângulo fixo-ish |
| **Sable** (Shedworks, 2021) | Toon shader plano + outline ink + paleta limitada; prova que low-poly + flat color = AAA visual com solo team |
| **Death's Door** (Acid Nerve, 2021) | 3/4 view com câmera fixa-rotacional; silhueta forte; mood gótico funcional com low-poly |
| **Hyper Light Drifter** (Heart Machine, 2016) | Paleta restrita + neon accent + ambient fog; combat readability mesmo com cena densa |
| **Tunic** (Andrew Shouldice, 2022) | Isometric-ish low-poly fofo + dungeon sombria; contraste tonal entre overworld e zona perigosa |
| **The Last Night** (mockup) | Ciber-gótico neon noir; referência de city palette, NÃO de execução |

Anti-ref: Genshin (PBR + anime AAA, fora de orçamento), Octopath (HD-2D, escolha 2D que falha em câmera rotacional).

## 3. Pillars visuais

1. **Cel-shading + outline.** Toon shading com bandas duras (não gradient PBR). Outline inverted-hull em chars e props hero. Normal map permitido SÓ em indumentária (jaqueta/botas Kevlar). Especular metálico SÓ em micro-detalhes (braquetes do aparelho).
2. **Proporção SD 1:1:1 com extremidades ectomorfas.** Personagens jogáveis e NPCs principais seguem proporção super-deformed (cabeça/torso/pernas = 1/3 cada), mas braços e pernas mantêm modelagem esguia — NÃO chibi inflado tradicional. NPCs adultos podem ter proporção ligeiramente alongada (1:1:1.2) pra hierarquia visual.
3. **Silhueta primeiro.** Cada personagem/inimigo passa silhouette test em 8 ângulos antes de detalhe (rotação a cada 45° em turntable).
4. **Duas paletas, zero ambiguidade.** Cidade e Selve são personagens opostos. Transição é set-piece.
5. **Protagonista é foco saturado.** Gus carrega o único laranja vibrante (`#FF6B1A`); cenário é tudo frio.
6. **UI rúnica diegética.** HUD/menus são projeções dos óculos táticos — não overlay 2D abstrato.

## 4. Paletas

### 4.1 GusWorld City (megacidade ciber-gótica)

Noite perpétua. Concreto-azulado, neon ciano/magenta, sombras profundas. Saturação **alta nos lights, baixíssima nos midtones**.

| Slot | Hex | HSL | Uso |
|---|---|---|---|
| BG deep | `#0A0E1A` | 224° 43% 7% | Céu, sombra profunda |
| BG mid | `#1B2238` | 224° 35% 17% | Concreto, fachadas |
| BG light | `#3A4566` | 224° 27% 32% | Midtone, plano médio |
| Neutral warm | `#6B6F7A` | 218° 6% 45% | Pedra, asfalto seco |
| Accent cyan | `#22D3EE` | 187° 85% 53% | Neon principal, holograma |
| Accent magenta | `#E11D74` | 330° 78% 50% | Neon perigo, corporativo hostil |
| Accent violet | `#7C3AED` | 263° 83% 58% | Acento secundário, mistério |
| Hot signal | `#FACC15` | 49° 95% 53% | UI crítica, alerta |

**Contraste declarado:** BG deep × Accent cyan = 9.8:1 (WCAG AAA). Gus laranja `#FF6B1A` contra BG mid = 6.2:1. Daltonismo: cyan/magenta funcionam em protanopia/deuteranopia; testar com filtro Coblis.

### 4.2 Selve Sombria (floresta tecnorgânica)

Biolúmen vegetal, fractais visíveis, gótico-matemático. **Verde-azulado dominante + violeta de bioluminescência**. Saturação média, value baixo.

| Slot | Hex | HSL | Uso |
|---|---|---|---|
| BG deep | `#0D1410` | 138° 22% 6% | Sombra de copa, abismo |
| BG mid | `#1A2E26` | 161° 28% 14% | Folhagem média, tronco |
| BG light | `#2E5447` | 158° 29% 25% | Folhagem iluminada, musgo |
| Neutral cold | `#4A5D58` | 165° 11% 33% | Pedra, casca seca |
| Biolumen green | `#34D399` | 158° 64% 52% | Fungo, seiva, sinal vivo |
| Biolumen violet | `#A78BFA` | 254° 92% 76% | Esporo, fauna noctívaga |
| Anomaly red | `#F43F5E` | 350° 89% 60% | Vírus, bug, corrupção |
| Fractal cyan | `#67E8F9` | 187° 92% 69% | Padrão matemático revelado |

**Contraste declarado:** Anomaly red × BG mid = 7.1:1. Biolumen green é o "neon ciano da Selve" — equivalente funcional, paleta oposta.

### 4.3 Gus (constante cross-bioma)

| Slot | Hex | Uso |
|---|---|---|
| Cabelo ruivo | `#FF6B1A` | Único laranja saturado no jogo. Sempre destaque. |
| Pele alva | `#F5E6D8` | Quase off-white quente |
| Aparelho dental | `#C0D8E8` | Grafeno-tântalo, levemente azulado |
| Óculos lentes | `#22D3EE` | Lentes ativas (mesmo cyan da cidade — link visual ao "modo scan") |
| Roupa principal | `#2B3A55` | Azul-escuro pra contraste com cabelo |
| Tavus-Drive glow | `#A78BFA` | Violeta — link visual à magia/Selve |

**Regra-ouro:** Gus é o único objeto da cena com **laranja vibrante** + **dois neons simultâneos** (cyan óculos + violeta drive). Inimigos podem ter UM neon. Cenário tem zero.

## 5. Lighting bible

Luz direcional + ambiente/fog (nomes de API antigos abaixo são referência conceitual, ver banner de topo).

### 5.1 Cidade

- **Key light:** sem sol. Luz cinza-azulada fraca do céu noturno (`#3A4566`, intensity 0.6). Vinda de cima-leste.
- **Fill:** ambient `#1B2238`, factor 0.4. SDFGI desligado (caro pra solo). AmbientLight constante.
- **Rim:** custom shader (ver §8), cor magenta/cyan dependendo da rua. Aplicado em chars apenas.
- **Practical lights:** neons `OmniLight3D` baratos (range curto, sem shadow). Cyan + magenta alternados a cada quadra.
- **Fog:** depth fog volumétrico `#1B2238`, density 0.015, height fade. Esconde geometria distante = economia de draw call.
- **Hora-do-dia:** **NÃO EXISTE.** Noite perpétua é canônica.

### 5.2 Selve Sombria

- **Key light:** filtrada pela copa, verde-acinzentada `#2E5447`, intensity 0.4. Sem shadow real (custo); fake shadow via vertex AO.
- **Fill:** ambient `#0D1410`, factor 0.3 — mais escuro que cidade.
- **Rim:** verde biolúmen `#34D399` em vegetação, violeta em fauna.
- **Practical lights:** emissive materials nas plantas/fungos (sem cast light, só glow no shader). Onde precisar de cast: 1-2 `OmniLight3D` por bioma.
- **Fog:** volumétrico denso `#1A2E26`, density 0.03. Esconde fim da floresta + cria mood.
- **Hora-do-dia:** uniforme (sempre crepúsculo gótico). Variação por bioma interno, não por relógio.

### 5.3 Anomalia/vírus (overlay)

Cena infectada sobrepõe pulso vermelho `#F43F5E` no rim de tudo + glitch shader leve. Sinaliza puzzle de debug.

## 6. Color script (3 atos)

| Ato | Setting dominante | Paleta dominante | Accent emocional | Arco |
|---|---|---|---|---|
| **Ato 1 — Distritos Inferiores** | Cidade (90%) | City BG mid + neon cyan | Magenta cresce no fim do ato | Isolamento → descoberta. Saturação cresce devagar. |
| **Ato 2 — Selve Sombria** | Selve (80%) + flashbacks cidade (20%) | Selve BG + biolumen green | Violeta dominante, anomaly red pontual | Curiosidade → vertigem matemática. Maior contraste de value do jogo. |
| **Ato 3 — Catedrais de Silício** | Convergência (50/50) | Mix deliberado: city + selve em mesma cena (corrupção) | Anomaly red + hot signal yellow | Crise → resolução. Primeira (e única) cena onde as duas paletas coexistem — é o ponto temático do jogo. |

**Regra dura:** atos 1 e 2 nunca misturam paletas. Mistura no ato 3 = recompensa narrativa.

## 7. Silhouette rules

- **Silhouette test obrigatório:** screenshot da cena em preto puro contra BG branco. Protagonista, inimigo e objetivo identificáveis em 3s.
- **Character read em 3/4 rotacional:** silhueta tem que funcionar em 8 ângulos (rotação a cada 45°). Testar em turntable Blender antes de aprovar.
- **Gus:** SD 1:1:1 com triângulo invertido (cabelo asimétrico dominando **quadrante superior DIREITO** + ombros do sobretudo + base estreita das pernas finas). Aparelho ortodôntico visível mesmo em silhueta lateral (mandíbula com "linha" extra). Tavus-Drive saliente no pulso ESQUERDO. Sheet completo: `docs/art/characters/gus.md`.
- **Inimigos cidade:** ângulos agudos, simetria quebrada (corporativo = uncanny). Triangular shape language.
- **Inimigos Selve:** fractal/orgânico, ramificação em razão recorrente visível. Curvas com pontas (cipó + espinho).
- **NPCs amigáveis:** silhueta arredondada, simetria estável. Quadrado/círculo dominante.
- **Figure/ground:** char sempre 2+ steps de value acima ou abaixo do BG imediato. Sem char ocupando o midtone do bioma.

## 8. Target poly budget

Solo G1, 12 meses. PC mid-range (GTX 1060+). Alvo: **60fps @ 1080p**.

| Asset | Tris | Texture | Material slots | Notas |
|---|---|---|---|---|
| Gus (hero) | 5.000–7.000 | 1024² atlas + 512² normal (jaqueta/botas) + 256² emission | 3 | SD 1:1:1, cabelo hi-density pra cel-shading. Spec: `docs/art/characters/gus.md` |
| NPC importante | 1.500–2.500 | 256² atlas | 1 | |
| Inimigo comum | 800–1.500 | 256² atlas compartilhado por família | 1 | Famílias de 3-5 inimigos dividem 1 atlas |
| Boss | 4.000–6.000 | 512² | 1-2 | Raro, vale o custo |
| Prop pequeno | 100–400 | nenhuma (vertex color) ou compartilhada | 1 | |
| Prop hero (interativo) | 500–1.000 | 256² | 1 | |
| Tile de cenário modular | 200–600 | atlas 512² compartilhado por bioma | 1 | Bioma inteiro num atlas |
| Vegetação Selve | 50–200 (billboard cross) | atlas folhagem 512² | 1 | Imposter onde possível |
| VFX mesh (carta rúnica, fractal) | 50–300 | gradient ramp + alpha | 1 unshaded | |

**Cena inteira:** alvo ≤ 250k tris visíveis, ≤ 150 draw calls. LOD só pra hero e bosses; resto usa culling + fog.

## 9. Texture strategy

**Escolha:** **gradient atlas + vertex color**, sem texture painting AAA.

Justificativa solo G1:
1. **Gradient atlas (256² ou 512²):** UV de cada poly aponta pra um pixel/zona do atlas. Atlas é pintado uma vez em Krita/Aseprite. Reuso massivo entre assets. Tempo de criar texture por asset: minutos, não horas.
2. **Vertex color:** detalhe extra (sujeira, gradient de cor, AO fake) pintado direto em Blender vertex paint. Zero custo de memória, zero tempo de UV.
3. **Normal map: exceção controlada.** Permitido em indumentária hard-surface (Kevlar/couro sintético/metal) — Gus jaqueta+botas, alguns NPCs corporativos. PROIBIDO em pele, cabelo, vegetação, terreno, props orgânicos.
4. **Metallic/specular: exceção micro.** Permitido em micro-detalhes metálicos (braquetes do aparelho do Gus, ornamentos de armadura, neon practical). PROIBIDO em superfícies grandes.
5. **Sem roughness map dedicado** (uniformizar via material). Sem AO map (vertex AO).
4. **Emissive:** canal R do vertex color = máscara de emissive. Bioluminescência grátis.

**Exceções permitidas:**
- Gus (hero): atlas 1024² + normal 512² (jaqueta+botas) + emission 256² (lentes+Tavus-Drive). 3 material slots.
- Bosses: 1 texture de 1024² própria + emission 256² se aplicável.
- NPCs importantes: 512² atlas próprio se design exige.
- UI rúnica: textures alpha pra glyphs (vetor exportado pra PNG).

**Matcap:** considerado e **rejeitado**. Quebra com câmera rotacional real (matcap pressupõe view fixa). Manter para hipotético menu/portrait, não in-game.

## 10. Shader strategy

Mínimo de shader custom; maximizar o que a engine oferece de fábrica.

| Necessidade | Solução | Custo |
|---|---|---|
| Toon shading | `render_mode diffuse_toon, specular_toon` (built-in) | Zero. Usar `StandardMaterial3D` direto. |
| Outline | **Custom shader** — método "inverted hull" (mesh duplicado, normais invertidas, vertex push pelo normal) | 1 shader reutilizável; +1 draw call por asset com outline. Outline só em chars + props hero. |
| Rim light | `RIM` built-in no spatial shader, modulado por cor do bioma | Zero custo extra. |
| Unshaded (UI rúnica, VFX) | `render_mode unshaded` | Zero. |
| Vertex color | `COLOR` built-in, multiplicado no albedo | Zero. |
| Flat shading (faces planas, sem smooth) | Normais "flat" no Blender export OU custom shader com `dFdx/dFdy` | Preferir export flat (zero shader custo). |
| Fog volumétrico | `WorldEnvironment` fog (built-in) | Médio. Aceitável. |
| Glitch/anomalia | Custom shader screen-space leve (UV distortion + chromatic aberration mínima) | Aplicado só em zonas infectadas. |
| Holograma (cartas, óculos UI) | Custom shader: unshaded + fresnel + scanline + transparency | 1 shader reutilizável. |
| Água/lúmen Selve | Custom shader: vertex displacement (seno) + emissive vertex color | 1 shader. |

**Total de custom shaders:** ~5 reutilizáveis. Tudo o mais é `StandardMaterial3D` ajustado.

## 11. VFX language

Três famílias visuais, **nunca misturadas**:

### 11.1 Rúnico / Holográfico (combat, UI)
- Cor: cyan `#22D3EE` + violeta `#A78BFA`.
- Forma: glyphs geométricos (hexágono, octógono, linhas retas), scanline horizontal, fresnel borda.
- Tipografia VFX: monospace fictícia (estilo `C-Arcane`), caracteres glitching.
- Movimento: snap a grid (steps discretos), nunca interpolado suave.
- Uso: cartas, alvos do Vetor do Gambito, projeções dos óculos, menus.

### 11.2 Biolúmen (Selve, magia natural)
- Cor: verde `#34D399` + violeta `#A78BFA`.
- Forma: partículas orgânicas, trilhas curvas, fractal recorrente.
- Movimento: senoidal, lento, breathing.
- Uso: plantas reagindo a Gus, fauna passiva, magia "natural" (cartas de Raiz, Esporo).

### 11.3 Neon urbano (cidade, ambient)
- Cor: cyan + magenta + ocasional yellow `#FACC15`.
- Forma: barras retas, letreiros, faíscas elétricas.
- Movimento: flicker (oscilação rápida randomizada), buzz constante.
- Uso: ambiente da cidade, NPC corporativo, equipamento hostil.

### 11.4 Anomalia (vírus, bug)
- Cor: red `#F43F5E` + hot yellow `#FACC15`.
- Forma: glitch quadriculado, RGB split, scanline corrompida.
- Movimento: erratico, pulse irregular.
- Uso: inimigos especiais (vírus), zonas de debug-puzzle, momentos narrativos de crise.

## 12. Anti-objetivos (o que GusWorld NÃO é)

- **Não é PBR.** Zero metallic/roughness workflow. Quem propor texture realista é redirecionado.
- **Não é cel-shaded anime.** Toon sim, mas com paleta restrita e silhueta low-poly — não Genshin/BoTW.
- **Não é pixel art.** A leitura é pixel-art-like (gradient atlas, paleta limitada), mas geometria é 3D real pra suportar câmera rotacional.
- **Não é fotorrealista cyberpunk.** A cidade é estilizada gótica neon, não Cyberpunk 2077.
- **É chibi-SD canônico, NÃO realista.** Proporção 1:1:1 (cabeça 1/3 do corpo). Mas com extremidades ectomorfas delgadas — anti-chibi-inflado. Tom analítico/gótico, não fofo/kawaii. Expressão default: séria/analítica, não sorrisinho.
- **Não é gore.** Sangue, vísceras, dismemberment proibidos. Inimigos derrotados "compilam erro" + dissolvem em partículas.
- **Não é zona-híbrida-genérica.** Cidade e Selve nunca se misturam visualmente até o ato 3 deliberado.
- **Não é HD-2D (Octopath).** Decisão consciente: 3D real estilizado, não sprites 2D em ambiente 3D.
- **Não é open-world.** Áreas curadas com câmera orbital limitada por região.
- **Não é marketing visualmente diferente do jogo.** Key art final renderiza no engine + retoque mínimo. Sem bait.

## 13. Don'ts visuais (checklist de rejeição em review)

- Asset com 4+ cores fora da paleta declarada → reprovado.
- Char com silhueta indistinguível em preto → reprovado.
- Material com `metallic > 0` fora das exceções listadas (micro-detalhes hard-surface) → reprovado.
- Normal map fora das exceções listadas (indumentária hard-surface) → reprovado.
- Char SEM proporção SD 1:1:1 (exceto NPCs adultos em 1:1:1.2) → reprovado.
- Char com cabeça redonda inflada + bracinhos curtos gordos (chibi tradicional) → reprovado. SD ectomorfo é a regra.
- Texture com lighting baked (sombra desenhada) → reprovado.
- Mistura cidade+Selve fora do ato 3 → reprovado.
- Outline em todos os assets (deve ser só chars + props hero) → reprovado.
- Polycount acima do budget sem aprovação explícita → reprovado.
- VFX usando família errada (ex: rúnico cyan dentro da Selve sem motivo) → reprovado.
- Gus com cabelo em qualquer hex ≠ `#FF6B1A` → reprovado.
- Gus com Tavus-Drive em pulso direito → reprovado (canônico: pulso ESQUERDO).
- Gus com mechas dominando lado esquerdo → reprovado (canônico: quadrante superior DIREITO).

---

**Revisão prevista:** após primeiro vertical slice (cidade + uma zona Selve + combate funcional). Atualizar paletas/budgets com dados reais de perf.
