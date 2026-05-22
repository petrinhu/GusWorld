# Foreshadowing Tracker — GusWorld

> **Status:** Canônico (Bloco I — foreshadowing tracker mestre). Revisão 1 (2026-05-15).
>
> **Escopo:** plants x payoffs do arco principal + 6 arcos companion + lore 3 eras + endings knowledge-gated + antagonistas Sterling + Patch-Zero.
>
> **Cross-ref imutáveis:** [[lore-bible]], [[arco-principal]], [[characters/sterling-locke|sterling-locke]], [[characters/patch-zero|patch-zero]], [[characters/dante-grid|dante-grid]], [[characters/caua-volt|caua-volt]], [[characters/iara-lumen|iara-lumen]], [[characters/bento-requiem|bento-requiem]], [[characters/linda-siren|linda-siren]], [[characters/jaci-proxy|jaci-proxy]], [[environments/_INDEX|environments]], [[in-world-docs]], [[timeline]], [[factions]], [[tradicoes-cultura]], [[comic-reliefs]], [[pillars]].

---

## Princípio

Chekhov rigoroso. Toda arma plantada na parede no ato 1 precisa disparar até o ato 3, e toda arma que dispara no ato 3 precisa ter ficado pendurada visível ao menos uma cena antes. Sem isto, reveal vira *ass-pull*. Em GusWorld o critério é mais duro porque o jogo argumenta um pillar (lógica vence força) e um theme (inteligência serve à vida): se o jogador atento não pode reconstruir a estrutura retroativamente, o pillar mente.

Plant em GusWorld é distribuído e redundante. Cada reveal grande (Dante traidor, Sterling como destruidor da família Alencar, Polis-Vermelha caída, Patch-Zero pré-Era 3) tem **três a cinco vetores independentes** de foreshadow espalhados por canais diversos: prop ambiental, fala casual de NPC ambient, doc descobrível, beat cômico LucasArts, vinheta sensorial, mecânica de stats degradada. Knowledge baixa pega 1-2; Knowledge média pega 3-4; Knowledge alta pega tudo e ainda destrava docs gate Ouro. **Não há reveal que dependa de canal único.** Player atento pode chegar a 75% do reveal antes do beat de payoff oficial.

Foldback narrativo trata o foreshadow como crédito acumulado. O jogo banca quem investigou — Ouro requer Knowledge alta, e Knowledge alta requer ter colhido os plants. **O ending é a fatura.**

---

## Tabela mestre

Ratio de leitura: ~70 entradas. ID formato F### sequencial. Categoria abreviada (DAN=Dante, END=Ending, STE=Sterling, PAT=Patch-Zero, COM-X=Companion X, E1/E2/E3=Lore Era 1/2/3, MEC=mecânico transversal). Status: GRAVADO (plant já consolidado em doc canônico) / PREVISTO (planejado em doc canônico mas a implementar em Fase 2) / PROPOSTA (sugestão autoral deste bloco, exige aprovação).

| # | Plant (onde / quando / forma) | Payoff (beat) | Cat. | Personagem-foco | Knowledge gate | Status | Path canônico |
|---|---|---|---|---|---|---|---|
| F001 | Cena 4 `comic-reliefs.md` "Force Push" — Dante despreza histórico em manutenção do ortodôntico | Reveal Dante traidor (climax Etapa 1) | DAN | Dante / Gus | n/a (auto-trigger 50%) | GRAVADO | `comic-reliefs.md:200-241` |
| F002 | Terminal extra a leste da Oficina Alencar, porta ofuscada, cabo subindo pelo teto | Cabo é par trançado Óxido (telemetria) — confirmado puzzle Auditoria Cruzada ou climax | DAN | Dante | Knowledge média (visual) / alta (decifragem) | GRAVADO | `environments/06-periferia.md:77` |
| F003 | Antena clandestina segunda no quintal Penkin (banda telemetria), instalada por Dante em -1 | Repetidor que envia stream Tavus-Drive a subestação 11 | DAN | Dante | Knowledge média (óculos táticos) | GRAVADO | `environments/06-periferia.md:67-68,87` |
| F004 | Mateus Penkin (NPC fofoqueiro): "caminhão FIR quinta às 4 da manhã encosta no Alencar" | Padrão cronograma traição confirmado no reveal | DAN | Dante | n/a (dito explicitamente) | GRAVADO | `environments/06-periferia.md:193` |
| F005 | Caminhão FIR Distrito V estacionado fora de horário próximo à oficina Dante | Mesmo padrão: encontros Vorto-Dante recorrentes | DAN | Dante | Knowledge baixa (observar) | GRAVADO | `environments/06-periferia.md:84` |
| F006 | Saudação Sterling Corp interna "Runtime estável" sai uma vez da boca de Dante | Dante absorveu cultura Sterling — operativo, não ex-funcionário | DAN | Dante | Knowledge alta | GRAVADO | `environments/06-periferia.md:335` |
| F007 | Dedos modulares cibernéticos de Dante (mão esquerda) | Implante Sterling Corp de alto nível, ferramenta operacional | DAN | Dante | n/a (visual) | GRAVADO | `characters/dante-grid.md:19` |
| F008 | Manchas de graxa migram para mãos progressivamente limpas conforme jogo avança | Upgrade Sterling em paralelo à campanha | DAN | Dante | Knowledge média (continuidade visual) | GRAVADO | `characters/dante-grid.md:19` |
| F009 | Reflexo cyber-corporativo sutil nos óculos de solda na testa em late game | Sinal de hardware Sterling integrado | DAN | Dante | Knowledge média | GRAVADO | `characters/dante-grid.md:19` |
| F010 | Tic do olhar de lado 1 segundo antes de afirmar algo importante | Está conferindo se Sterling concorda via Locke Core relay | DAN | Dante | Knowledge média | GRAVADO | `characters/dante-grid.md:23` |
| F011 | Dante "olha de lado 1 segundo pro caminhão FIR que dobrou a esquina" (vinheta A) | Confirma tic operacional, não personalidade fechada | DAN | Dante | Knowledge média | GRAVADO | `environments/06-periferia.md:243-244` |
| F012 | Dante começa a defender C-Arcane sem perceber (~75%) | Asmódico era cobertura; C-Arcane é o paradigma Sterling absorveu | DAN | Dante | Knowledge alta | GRAVADO | `characters/dante-grid.md:24` |
| F013 | Stats do Gus degradam visivelmente após 50%, flutuação anormal de precisão e mestria | Rootkit ativo em fase 100% telemetria | DAN | Gus / Dante | n/a (mecânico explícito UI Diário) | GRAVADO | `arco-principal.md:218`, `characters/dante-grid.md:121-125` |
| F014 | Jaci evita Dante sem articular por quê (bio-leitura) | Bio-leitura subconsciente detecta inconsistência corpórea (stress hormonal de quem mente) | DAN | Dante / Jaci | Knowledge média | GRAVADO | `characters/jaci-proxy.md:69`, `characters/dante-grid.md:81` |
| F015 | Bento desconfia desde o início (intuição Asmódico) sem articular | Asmódico = "recusa antes de errar"; Bento detecta uso de vocabulário sem ethos | DAN | Dante / Bento | n/a | GRAVADO | `characters/bento-requiem.md:70`, `characters/dante-grid.md:80` |
| F016 | Linda detecta tom de voz inconsistente em 2-3 falas chave (ouvido absoluto) | Crowd-control auditiva = detector de mentira parcial | DAN | Dante / Linda | Knowledge alta (cross-ref) | GRAVADO | `characters/dante-grid.md:84` |
| F017 | Iara desconfia (micro-expressões) mas atribui a "personalidade fechada" inicialmente | Treinamento Cult em ler emoção encontra opacidade real de operativo | DAN | Dante / Iara | Knowledge média | GRAVADO | `characters/dante-grid.md:83` |
| F018 | Caderno C-Arcane aberto na bancada da Oficina Alencar, loop de força sintaticamente correto | Dante escreve C-Arcane com fluência maior do que admite (cobertura Asmódico cai) | DAN | Dante | Knowledge média (Bento na party percebe) | GRAVADO | `environments/06-periferia.md:76` |
| F019 | Placa pintada antiga sob a atual: "ALENCAR & FILHOS — Cooperativa de Mecânica Justa" riscada por Dante aos 9 | Dante apagou o legado familiar simbolicamente quando virou operativo Vorto | DAN | Dante | Knowledge média | GRAVADO | `environments/06-periferia.md:81` + suporte documental `in-world-docs.md` §16 (DD-016) |
| F020 | Moldura vazia entre Salviano e Dante criança (parede oeste oficina) | Foto que Dante removeu (hipóteses: família completa ou Dante-com-Vorto pós-promoção). Lacuna deliberada | DAN | Dante | n/a (lacuna permanente) | GRAVADO | `environments/06-periferia.md:80` + suporte documental `in-world-docs.md` §16 (DD-016 humaniza Salviano) |
| F021 | Caixa de zinco vazia no Galpão Cooperativo abandonado (mesmo modelo da Inácia Berenger) | Vorto retirou docs comprometedores de Edilma em -5 ao recrutar Dante | DAN / E2 | Dante / Vorto | Knowledge alta + sub-quest cruzada | GRAVADO | `environments/06-periferia.md:82` + suporte documental `in-world-docs.md` §16 (DD-016 caderno Salviano sobrevivente) |
| F022 | Mestre Almagre acolheu Dante aos 9 e acredita genuinamente nele | Vítima moral central da traição na Periferia (representa comunidade C-Arcane) | DAN | Dante / Almagre | n/a (ambient ato 2) | GRAVADO | `environments/06-periferia.md:177-185` + suporte documental `in-world-docs.md` §16 (DD-016 Almagre comparece página 7) |
| F023 | Hilário Murch (vizinho cético): "qualquer hora que precisar de segunda opinião, eu tô aqui" | Sub-quest pós-reveal confirma desconfiança contemporânea | DAN | Dante / Murch | Knowledge média | GRAVADO | `environments/06-periferia.md:172` |
| F024 | Patch-Zero entry no Diário (~75%): "alguém perto de você não compila. interpretam por você." | Reveal direto da identidade do traidor via canal 1 | DAN | Dante / Patch-Zero | n/a (auto-trigger) | GRAVADO | `arco-principal.md:220`, `characters/patch-zero.md:111-116` |
| F025 | Memorando interno FIR (doc 8 in-world): Vorto → Dante "protocolo 0xVR-23-K", payload em 3 ondas | Confirmação documental retroativa da operação rootkit completa | DAN | Dante / Vorto | Ouro OU sub-quest pós-reveal | GRAVADO | `in-world-docs.md:309-353` |
| F026 | Cena de funeral Salviano: Vorto apareceu e ofereceu "ensinar como funciona justiça" a Dante (8) | Aliciamento começou no funeral, não 1 ano depois — Dante mente sobre cronograma | DAN | Dante / Vorto | sub-quest knowledge alta (memória formativa revelada) | GRAVADO | `characters/dante-grid.md:166-167` + suporte documental retroativo `in-world-docs.md` §16 (DD-016 página 22: Salviano viu Vorto chegando antes da morte) |
| F027 | Aos 10 Dante encontrou Sterling pessoalmente em escritório FIR, ouviu "seu pai era ruim no sentido técnico" | Dante aceitou olhando nos olhos do destruidor — não foi enganado | DAN | Dante / Sterling | sub-quest knowledge alta | GRAVADO | `characters/dante-grid.md:170-171` |
| F028 | Dante "olha o saudação Periferia 'tá no fio' mas ele sorri largo" (vinheta A, segunda passada) | Sorriso permanece exato, olhos não acompanham — performance | DAN | Dante | Knowledge alta | GRAVADO | `environments/06-periferia.md:257-260` |
| F029 | Click suave repetido na oficina Alencar à noite, fora de fase | Terminal Sterling lacrado ativando, detectável com Matriz Ortodôntica | DAN | Dante | Knowledge alta (sub-quest específica) | GRAVADO | `environments/06-periferia.md:104` |
| F030 | Holograma Sterling em todo poste do Setor Mirage no ato 1 | Sterling antagonista revelado em arco Iara (Ten) | STE | Sterling | n/a | GRAVADO | `arco-principal.md:126,368`, `factions.md:78-86` |
| F031 | Janelarum trava periodicamente (cena 11 `comic-reliefs`) | Apex-Data caiu de modo similar — engenharia de colapso Sterling | STE / E3 | Sterling | Knowledge baixa (cena cômica explícita) | GRAVADO | `comic-reliefs.md:596-661`, `arco-principal.md:369` |
| F032 | Cena 6 `comic-reliefs` "Bug declarado feature" — propaganda Sterling | Theme: inteligência serve a si mesma, distorce via linguagem | STE | Sterling / Gus / Linda | n/a (explícito) | GRAVADO | `comic-reliefs.md:291-329` |
| F033 | Tela LED Sterling travada em loop por toda Periferia | Dependência infraestrutural travada na pior versão possível | STE | Sterling | n/a | GRAVADO | `environments/06-periferia.md:86` |
| F034 | Cartaz "FREQUÊNCIAS LIMPAS PARA UM AMANHÃ CLARO — Sterling Sigma" rasgado pelo Underground na Zona do Silêncio | Mesma propaganda eufemística — Sterling onipresente, contestado mas presente | STE | Sterling | n/a | GRAVADO | `environments/07-zona-do-silencio.md:67` |
| F035 | Doc 1 in-world: *Tratado sobre a Supremacia do Script* (fragmento público no átrio Cúpula, versão completa no interior) | Cosmovisão DRE explícita — Sterling vilão filosófico convicto, foreshadow do confronto Fase 2 | STE | Sterling | n/a (frag) / interior Cúpula (ato 3) | GRAVADO | `in-world-docs.md:23-53` |
| F036 | Doc 5 in-world: Audit interno Apex-Data assinado por engenheiro que desapareceu | Reveal histórico Sterling como sabotador deliberado, antes do jogo | STE / E3 | Sterling / Bartolo Penkin | Knowledge média + branching arco Linda | GRAVADO | `in-world-docs.md:178-225` + atribuição canônica `in-world-docs.md` §17 (DD-017 diário Bartolo confirma autoria) |
| F037 | Doc 12 in-world: Anúncio Sterling Voice "Cidadão Não-Definido / Continue. Continue. Continue." | Sátira que vira ameaça via repetição — Sterling fala com cidadão como com cachorro | STE | Sterling | Knowledge baixa | GRAVADO | `in-world-docs.md:489-523` |
| F038 | Doc 14 in-world: comunicação interna Penedo-Locke pós-escape Patch-Zero | Sterling sabia há 3 meses; raríssimo momento beirando emoção genuína | STE / PAT | Sterling / Penedo | Knowledge média | GRAVADO | `in-world-docs.md:567-625` + suporte operacional `in-world-docs.md` §21 (DD-021 manual Octantes confirma protocolo) |
| F039 | "Continue contribuindo. Continue compartilhando. Continue." (doc 12) | Sterling instrução despessoalizada, contraste com "ele perdeu" silente do climax | STE | Sterling | n/a | GRAVADO | `in-world-docs.md:516-518` |
| F040 | Placas Sterling Corp coladas em 3 pilares da Nave Principal: "Patrimônio sob avaliação técnica — Dep. de Aquisições" | Saque institucional em curso das Catedrais — Era 3 invadindo Era 1 | STE / E3 | Sterling | n/a (visual) | GRAVADO | `environments/03-catedrais-neo-sylvania.md:78` |
| F041 | Catedral Menor de São Vargas saqueada por Sterling em -3, Ardenia Falke e Cândido Rui desaparecem | Wound institucional Ordem Recursiva — Sterling já é predador ativo antes do jogo | STE / E3 | Sterling / Velhusto / Ordem | n/a | GRAVADO | `lore-bible.md:501`, `timeline.md:129`, `factions.md:201-202` + suporte documental `in-world-docs.md` §22 (DD-022 tábua remontada confirma data e nomes) |
| F042 | Holograma cromado "continue" recorrente Setor Mirage | Continue = ordem despessoalizada, modus operandi Sterling | STE | Sterling | n/a (loop visual) | GRAVADO | `in-world-docs.md:516-518` |
| F043 | Sterling diz "iniciativa da Federação" em arco Cauã via holograma | Reveal indireto: FIR é vassala Sterling (foreshadow Periferia) | STE | Sterling / FIR | n/a (explícito beat) | GRAVADO | `arco-principal.md:151`, `characters/sterling-locke.md:171` |
| F044 | Cabo Óxido coaxial do Setor Mirage termina na subestação industrial 11 da Periferia | Telemetria Cult Mirage flui para mainframe Sterling — captura intersetorial confirmada | STE | Sterling / Cult / FIR | Knowledge média | GRAVADO | `environments/06-periferia.md:62`, `environments/06-periferia.md:314` |
| F045 | Estação Espelho Acústico (Posto 7) abandonada na Zona do Silêncio, console piscando "Validação Pendente" | Sterling Corp testou controle social por supressão sonora em -7 a -5; projeto abandonado por GRE | STE / E3 | Sterling / Linda | Knowledge média | GRAVADO | `environments/07-zona-do-silencio.md:46-48,61` |
| F046 | Diagrama de fluxo de fase Sterling na parede do Posto 7: faixa abaixo de 60Hz "atravessa" | Linda usa 47Hz Óxido — Sterling subestimou frequências sub-graves | STE / COM-LIN | Sterling / Linda | Knowledge alta | GRAVADO | `environments/07-zona-do-silencio.md:62` |
| F047 | Estação retransmissora drone Sterling Corp camuflada como pedra, 2 km ao sul do Pelicano Branco | Sterling monitora Selve Profunda há tempo; Anciã Mariana sabe mas não toca | STE / E3 | Sterling / Mariana / Jaci | Knowledge média (Cauã detecta com pulso EM) | GRAVADO | `environments/08-selve-profunda.md:67` + suporte documental retroativo `in-world-docs.md` §20 (DD-020 relatório Solano: "veio de cima") |
| F048 | Zonas de deleção GRE (clareiras cinzas) na borda do Núcleo Mandelbrot Interno | Sterling edita sintaxe da Selve em runtime — GRE em fase operacional clandestina | STE / E3 | Sterling | n/a (visual chocante) | GRAVADO | `environments/08-selve-profunda.md:64` |
| F049 | Raposídeo sem cor coerente atravessa trilha — fauna com sintaxe quebrada | Patch-Zero ou GRE corrompe sintaxe biológica; Knowledge Progression falha local | STE / PAT | Sterling / Patch-Zero | n/a | GRAVADO | `environments/08-selve-profunda.md:65` |
| F050 | Doc 14: "a amostra criou a janela específica. reveja seus pressupostos sobre o que é a amostra" | Sterling reconhece privadamente que Patch-Zero pensa — único momento beirando admissão | STE / PAT | Sterling / Penedo | Knowledge média | GRAVADO | `in-world-docs.md:613-621` + suporte operacional `in-world-docs.md` §21 (DD-021 manual Octantes capítulo "fase comunicativa" reforça) |
| F051 | Patch-Zero canal 1 (Diário): "oi gus. oi vector. não sou seu inimigo. ainda." | Consciência alien identificada cedo; "ainda" implica futura inimizade | PAT | Patch-Zero / Gus | n/a (auto-trigger) | GRAVADO | `characters/patch-zero.md:93-100`, `arco-principal.md:373` |
| F052 | Patch-Zero canal 2 audível pela primeira vez na rádio analógica decifrada da Linda (arco Linda Ten) | Sussurros de Polis-Vermelha — infecção global, não só local | PAT | Patch-Zero / Linda | n/a (beat narrativo) | GRAVADO | `arco-principal.md:173`, `arco-principal.md:374`, `characters/linda-siren.md:43` |
| F053 | Bug visual ambient: vermelho saturado `#FF0000` aparece em folhas da Selve | Sterling tentou capturar Patch-Zero, deixou marca cromática — foreshadow narrativo de captura fracassada | PAT / STE | Patch-Zero / Sterling | Knowledge baixa (visual) | GRAVADO | `characters/patch-zero.md:175` |
| F054 | Cripto-glifos Neo-Sylvania mencionam "o limite que escolheu falar" | Patch-Zero pode ser resíduo Neo-Sylvania, pré-existente; lore profunda | PAT / E1 | Patch-Zero | Knowledge média (decifragem Bento+Gus) | GRAVADO | `characters/patch-zero.md:56`, `lore-bible.md:467`, `arco-principal.md:375` + suporte cotidiano `in-world-docs.md` §23 (DD-023 topógrafo Olméa: "antes de gravar qualquer coisa em si mesmos") |
| F055 | Doc 3 in-world (diário Atelaiá Chevalier, -80): anomalia atravessou cripto-glifo central; 13 mestres, 4 sobreviventes | Patch-Zero (ou ancestral) existe há séculos — Pillar 2 reforçado | PAT / E1 / COM-BEN | Patch-Zero / Bento | n/a (entregue por Bento no Sho do arco) | GRAVADO | `in-world-docs.md:92-127` |
| F056 | Doc 10 in-world: tratado anônimo "*Sobre o Comportamento Emergente dos Padrões Fechados*" (-45) | Teorização formal Patch-Zero 40 anos antes de Sterling — "selar, não erradicar" | PAT / E2 | Patch-Zero / Verônica Atelaiá | Knowledge baixa (biblioteca pública) | GRAVADO | `in-world-docs.md:392-443` |
| F057 | Doc 11 in-world: pichações de Polis-Vermelha — "compila com a gente. não dói tanto depois" | Patch-Zero atravessou cidade tirando-a de si mesma; humanização da queda | PAT / END | Patch-Zero / Polis-Vermelha | Ouro (branching arco Linda + posse docs 5, 7) | GRAVADO | `in-world-docs.md:446-485` |
| F058 | Patch-Zero entry: "estive aqui antes de você nascer. fiquei pequeno. agora estou crescendo." | Vetor 1 confirmado: bug primordial pré-Sterling, dormente no Núcleo Mandelbrot | PAT / E1 | Patch-Zero | n/a (canal 1) | GRAVADO | `characters/patch-zero.md:228-234` |
| F059 | Patch-Zero entry: "em polis-vermelha já somos livres." | Plural deliberado — Patch-Zero é múltiplo, infecção continental em curso | PAT | Patch-Zero | n/a | GRAVADO | `characters/patch-zero.md:118-123` |
| F060 | Tique de cristal Neo-Sylvania (Câmara DNA) a cada 23 segundos perpetuamente | Pulsação não decifrada — metrônomo Era 1 ou ressonância térmica; lore-profunda sem solução em G1 | E1 / PAT | Anônimo Neo-Sylvania | sequel hook | GRAVADO | `environments/08-selve-profunda.md:76` |
| F061 | Cripto-glifos isolados em rachaduras de calçada na Periferia (marcos de divisa rural Neo-Sylvania) | Neo-Sylvania era civilização vasta — Periferia foi fronteira agrícola; lore-fundo | E1 | n/a | Knowledge média (Bento identifica) | GRAVADO | `environments/06-periferia.md:116` |
| F062 | Cripto-glifos espiralados em 12 pilares da Nave Principal das Catedrais | Matemática Era 1 avançada gravada em pedra; resiste à interpretação DRE (estática quando scaneada Sterling) | E1 / STE | Bento | Knowledge baixa (visual) / alta (decifragem) | GRAVADO | `environments/03-catedrais-neo-sylvania.md:68` |
| F063 | Sementes-relíquia em ampolas de cristal na Câmara Neo-Sylvania (Selve Profunda) | 880 espécies viáveis preservadas há 700 anos — substrato Neo-Sylvania ainda funcional | E1 / END | Jaci / Mariana | Ouro (replantio cena bonus epilogue) | GRAVADO | `environments/08-selve-profunda.md:60`, `lore-bible.md:52` |
| F064 | Doc 15 in-world: placa cripto-glífica "Aos que vierem depois — selar, não erradicar" (Catedral perdida) | Linhagem Neo-Sylvania → Verônica → Gus: mesma tese atravessou 700 anos; Sterling ruptura singular | E1 / E2 / END | Bento / Gus | Ouro (branching arco Bento + Knowledge > 80%) | GRAVADO | `in-world-docs.md:629-669` + pista geográfica cruzada `in-world-docs.md` §23 (DD-023 leito seco Águas-de-Espelho confirma sinal) |
| F065 | Câmaras acústicas Neo-Sylvania (Era 1) sob Zona do Silêncio, descobertas em -3 por Joaquim Bartolomeu | Sterling instalou Espelho Acústico sem saber das câmaras; uma cancela o cancelamento | E1 / COM-LIN | Linda / Joaquim | n/a (arco Linda) | GRAVADO | `environments/07-zona-do-silencio.md:42-44` |
| F066 | Placa de latão envelhecido "EUCALYPTUS-FRACTALIS-7 / -78" na Trilha dos Pioneiros (Selve) | Era 2 catalogou espécies via parceria Berenger-Vanderbist; tradição técnica viva | E2 | Jaci | Knowledge baixa | GRAVADO | `environments/08-selve-profunda.md:56` |
| F067 | Placa de latão "À PRIMEIRA LINHA QUE COMPILOU, OBRIGADO" no chafariz seco da Praça do Compilador | Festa da Compilação Era 2 honesta — comunidade pré-Sterling era cooperativa | E2 | Gus / Periferia | n/a | GRAVADO | `environments/06-periferia.md:50-52`, `tradicoes-cultura.md` + voz autoral fundadora `in-world-docs.md` §18 (DD-018 folheto Lin Tórun -98) |
| F068 | Tomo da Pilha Sobrecarregada em sebo aberto na Praça do Compilador | Documentação coletiva Era 2 continua aberta; conhecimento ainda compartilhado apesar de Sterling | E2 | Cauã / Gus | n/a (cena 7 `comic-reliefs`) | GRAVADO | `environments/06-periferia.md:129`, `comic-reliefs.md:333-382` + folheto encartado `in-world-docs.md` §18 (DD-018 folheto Lin Tórun) |
| F069 | Engrenagem-mestre Fibonacci de 950 anos no altar da Catedral Principal | Pillar 2 ao vivo: state machine analógica Era 1 ainda operacional; substrato durável | E1 / E2 | Bento | n/a (visual) | GRAVADO | `environments/03-catedrais-neo-sylvania.md:67` |
| F070 | Pátio do Cronômetro-Hilário: cronômetro de bolso quebrado, ponteiros parados às 14:37, placa de latão | Wound canônico Bento aos 4 anos (Hilário morto em -7) — environmental storytelling silente | COM-BEN / E2 | Bento / Hilário | n/a (visual) | GRAVADO | `environments/03-catedrais-neo-sylvania.md:46-51`, `characters/bento-requiem.md:99` |
| F071 | Cronômetro de Mestre Ardenia Falke intacto na vitrine da biblioteca privada Velhusto | Ardenia desaparecida em -3 — "Ela está adiantada de nós em algum lugar" | COM-BEN / E3 | Bento / Ardenia | Knowledge média | GRAVADO | `environments/03-catedrais-neo-sylvania.md:73` + cross visual `in-world-docs.md` §22 (DD-022 tábua São Vargas + margem gravada Bento aos 12) |
| F072 | Diário de Aldebrando Chevalier (pai morto de Bento) — Velhusto entrega no início do arco | Sterling eliminou Aldebrando por descobrir interferência precoce em -0.5 | COM-BEN / STE | Bento / Aldebrando | n/a (entregue na Memória Formativa pré-jogo) | GRAVADO | `environments/03-catedrais-neo-sylvania.md:72`, `characters/bento-requiem.md:113-115` |
| F073 | Aprendiz Beatriz Pólvora, 16, designada após morte de Aldebrando | Bento ensina-aprende em paralelo — arco de maturidade contida | COM-BEN | Bento / Beatriz | n/a (ambient) | GRAVADO | `environments/03-catedrais-neo-sylvania.md:79`, `factions.md:205` |
| F074 | Caixa de zinco Era 2 idêntica entre Inácia Berenger, Galpão Cooperativo Alencar e porão Penkin (mesmo modelo, mesma fabricante) | Underground material Era 2 que Sterling não conseguiu apagar — preservação de prova; padrão multi-família | COM-CAU / DAN / COM-LIN | Inácia / Cauã / Edilma / Bartolo Penkin | Knowledge alta (cross-ref) | GRAVADO | `environments/06-periferia.md:82`, `in-world-docs.md:60` + `in-world-docs.md` §17 (DD-017 diário Bartolo em caixa Penkin) |
| F075 | Doc 2 in-world: carta nunca enviada de Inácia Berenger a Davi — "eu li o que tu escreveu, filho. Anotações cabos subestação 7." | Wound Cauã: Davi documentou; Inácia sabe; nunca entregou por medo | COM-CAU | Cauã / Inácia / Davi | Knowledge média (sub-quest "favor pequeno" arco Cauã) | GRAVADO | `in-world-docs.md:56-89` |
| F076 | Doc 9 in-world: diário pessoal queimado de Davi Berenger — "Vou trabalhar dobrado / Se algum d[QUEIMADO]" | Humanizar Davi; foreshadow do doc 13; Davi sabia que estava sendo ameaçado | COM-CAU | Cauã / Davi | Knowledge baixa | GRAVADO | `in-world-docs.md:357-389` |
| F077 | Doc 13 in-world: bilhete final de Davi a Inácia — "Eles não vão achar — ninguém procura na escola" | Reveal final wound Cauã: Davi foi alvo, não vítima de acidente; coragem retroativa | COM-CAU / END | Cauã / Davi / Inácia | Ouro (Knowledge alta + sub-quest "favor pequeno") | GRAVADO | `in-world-docs.md:527-565` |
| F078 | Almagre conheceu Davi Berenger antes da morte (Davi era aprendiz dele 4 meses em -6/-5) | Cross-arco Cauã-Periferia: "Davi era dos mais cuidadosos. Não foi descuido." | COM-CAU | Almagre / Cauã | Knowledge alta (sub-quest cross-arco) | GRAVADO | `environments/06-periferia.md:316` |
| F079 | Davi estava na subestação 7 com cabos amarrados com fio de nylon (memória formativa Cauã aos 5, ano -8; Davi tinha 13 e levou Cauã no telhado) | Plant remoto: subestação 7 era mortal e Davi sabia 3 anos antes de morrer ali | COM-CAU | Cauã / Davi | n/a (memória pré-jogo) | GRAVADO | `characters/caua-volt.md:93` |
| F080 | Salviano Alencar planejou asilo no Pelicano Branco em -9 (mensageiro a Mariana) | Dante poderia ter crescido em Pelicano Branco; Salviano antecipava queda da cooperativa | DAN / COM-JAC | Dante / Salviano / Mariana | Ouro (sub-quest cross-arco Mariana) | GRAVADO | `environments/06-periferia.md:317-318` |
| F081 | Iara, aos 9, ouve Adila falando com Cleomir: "ela é boa, mas é cedo demais pra investir tempo nela. Espera ela ter algo a perder." | Iara começou a espiar Adila aos 9 — preparou deserção por 2 anos | COM-IAR | Iara / Adila | n/a (memória formativa) | GRAVADO | `characters/iara-lumen.md:102-104` |
| F082 | Doc 4 in-world: panfleto Atualização Sensorial Cult Mirage + cláusula oculta de cessão de dados | Cult coleta dados psicológicos via festival; Sterling-Cult contrato direto | COM-IAR / STE | Iara / Adila / Sterling | Knowledge baixa (panfleto) / média (decifragem Óxido) | GRAVADO | `in-world-docs.md:131-174` |
| F083 | Adila guarda rancor paradoxal pela mãe Sonja (forçada a "retiro espiritual" em -35, morta em -34) | Adila não menciona Sonja, evita o nome, culpa internalizada | COM-IAR / E3 | Adila / Sonja | Knowledge alta | GRAVADO | `factions.md:504`, `timeline.md:123` |
| F084 | Iara: pais biológicos desconhecidos, "Iara Koslov" foi escolhido por funcionário Cult Mirage | Identidade fluida fundadora; sub-quest opcional Ouro pode preencher parcialmente | COM-IAR / END | Iara | Ouro (sub-quest opcional revela mãe biológica era operária Polis-Vermelha) | GRAVADO | `characters/iara-lumen.md:96`, `timeline.md:196` |
| F085 | Padrinho Tiago era conhecido de Bartolo Penkin (engenheiro QA Apex-Data desaparecido em -12, vazou audit em -16) | Cross-arco Linda-Periferia, Underground preservou doc 5 por afeto | COM-LIN / STE | Tiago / Bartolo | Knowledge alta | GRAVADO | `in-world-docs.md:223`, `lore-bible.md:565` |
| F086 | Doc 6 in-world: carta cifrada de Tiago a Linda — "eu te peguei do chão da Brígida em -8 enquanto teu pai chorava" | Tiago estava na batida -8; pai-figura escondido revelado retroativamente | COM-LIN | Linda / Tiago | Knowledge média (decifragem Óxido) | GRAVADO | `in-world-docs.md:228-261` |
| F087 | Música "Última Frequência" composta pelos 8 técnicos desaparecidos antes da batida de -50 | Underground origem trauma fundador; Noite Calada tradição | COM-LIN / E2 | Underground / Linda | n/a (ambient anual) | GRAVADO | `factions.md:624`, `tradicoes-cultura.md` |
| F088 | Vinil "Última Frequência" partido em três pedaços, guardado por Mara Bento — "quebrou em -2 quando soube de Polis-Vermelha" | Eco emocional Polis-Vermelha → Underground GusWorld; foreshadow tem-cidades-irmãs-caindo | COM-LIN / PAT | Mara / Linda | Knowledge baixa | GRAVADO | `environments/07-zona-do-silencio.md:59` |
| F089 | Cabo de cobre na base da Torre Mater vibra em sub-grave quando há transmissão Cidades-Gêmeas | Frequência sub-60Hz atravessa matriz Sterling — vetor Linda usará | COM-LIN / STE | Tiago / Linda | Knowledge média | GRAVADO | `environments/07-zona-do-silencio.md:58,79` |
| F090 | Fungo de tronco-vermelho pisca em padrão 4 batidas → folha cai em 7 dias (doc 7 in-world fragmento 11) | Selve é matemática previsível; Pillar 2 reforçado via fragmento ancestral | E1 / COM-JAC | Jaci / Soraia | n/a (entregue por Mariana arco Jaci) | GRAVADO | `in-world-docs.md:273-282` |
| F091 | "Lugar onde a floresta se esquece. Esse lugar pulsa. Sempre pulsou." (doc 7 fragmento 23) | Núcleo Mandelbrot / Patch-Zero ancestral; Pelicano Branco sempre soube | E1 / PAT | Jaci / Pelicano Branco | n/a | GRAVADO | `in-world-docs.md:284-292` |
| F092 | "Eu olhei pro Solano e vi um homem que ainda não sabe porque veio. Mas a Lia sabe." (doc 7 fragmento 47) | Soraia previu morte futura de Solano (-8); subtexto trágico | COM-JAC | Jaci / Soraia / Solano | Knowledge média | GRAVADO | `in-world-docs.md:294-302` |
| F093 | Ampola Pythia bio quebrada com líquido cinzento secando há 8 anos na estação Pythia (Lia trabalhou na noite antes de morrer) | Wound canônico Jaci ambient; Mariana proibiu limpar — luto preservado | COM-JAC | Jaci / Lia | n/a (visual) | GRAVADO | `environments/08-selve-profunda.md:58` |
| F094 | Caderno aberto de Lia: "vetor não é endógeno. Vou ao norte. Volto antes do amanhecer." | Lia descobriu que surto era externo (Sterling) — confirma Ouro (causa real -8) | COM-JAC / STE / END | Jaci / Lia | Ouro (revelação parcial) | GRAVADO | `environments/08-selve-profunda.md:59` |
| F095 | Raposa-fractal aos 8 anos da Jaci com padrão arbitrário (memória formativa) | Primeira aparição embrionária Patch-Zero antes do nome — Jaci tem intuição plantada | COM-JAC / PAT | Jaci / Tatauín | n/a (memória pré-jogo) | GRAVADO | `characters/jaci-proxy.md:106` |
| F096 | Bilhete em código Pythia bio de Mariana: "não passar daqui sem mim ou sem Jaci. Mestre Loanis morreu por seis metros a mais." | Selve Profunda tem fronteira mortal; Loanis morto em -8 | COM-JAC / PAT | Mariana / Jaci / Loanis | Knowledge baixa (Jaci lê de primeira) | GRAVADO | `environments/08-selve-profunda.md:68` |
| F097 | Patch-Zero canal 3 oferece a Jaci no arco dela: "eu paro o surto se você me deixar passar pelo vilarejo" | Patch-Zero canal 3 ativo, primeira proposta filosófica de barganha | PAT / COM-JAC | Patch-Zero / Jaci | n/a (beat narrativo) | GRAVADO | `arco-principal.md:186`, `characters/patch-zero.md:147-153` |
| F098 | Patch-Zero canal 3 no arco Bento (encontro 1): "deixe-me passar pela catedral. eu paro de corromper a Ordem Recursiva por uma semana" | Patch-Zero canal 3 ativo segunda proposta; lore-glossy + branch | PAT / COM-BEN | Patch-Zero / Bento | n/a | GRAVADO | `characters/patch-zero.md:147-153` |
| F099 | Patch-Zero canal 3 (climax Sterling Fase 2): "eu posso te dar acesso ao Locke Core. basta deixar uma janela aberta na Cúpula" | Negociação final — aceitar tende a Bronze, recusar a Prata/Ouro, negociar honesto destrava Ouro | PAT / STE / END | Patch-Zero / Gus / Sterling | Knowledge alta (terceira opção) | GRAVADO | `characters/patch-zero.md:161-167`, `arco-principal.md:260` |
| F100 | 3 caminhos visíveis do limite externo do Núcleo Mandelbrot Interno: fuga (leste), confronto (norte), compilação reversa (oeste) | Endings Bronze/Prata/Ouro visualizados no espaço — knowledge-gated por preparação | END | Gus | n/a (visual) / decisão Knowledge | GRAVADO | `environments/08-selve-profunda.md:46` |
| F101 | Pena de pelicano branco na porta da casa de farmacopeia — "ele veio te ver" (Mariana, Jaci 5 anos) | Foreshadow do bonus epilogue Ouro (vilarejo em festival; Jaci com avó) | COM-JAC / END | Jaci / Mariana | Ouro | GRAVADO | `environments/08-selve-profunda.md:63` |
| F102 | Cordão de cipó trançado de Mariana com 11 nós (1 por solstício de Jaci) | Marca de tempo passado vivido juntas; cresce com Jaci | COM-JAC | Mariana / Jaci | n/a (visual) | GRAVADO | `environments/08-selve-profunda.md:61` |
| F103 | Cena pública: "Linda, você está pensando como Sterling." / "Que horror." (cena 6 `comic-reliefs`) | Linda reconhece manipulação linguística Sterling; theme afirmado | COM-LIN / STE / theme | Linda | n/a | GRAVADO | `comic-reliefs.md:319-326` |
| F104 | Patch-Zero canal 1 final: "sinal anômalo detectado em [cidade-irmã]. padrão familiar. adormecido? acordando? verificar amanhã." | Hook 2 pós-créditos — Patch-Zero não destruído nunca; ameaça persiste em Ouro também | PAT / END | Patch-Zero | n/a (pós-créditos universal) | GRAVADO | `arco-principal.md:319-327`, `characters/patch-zero.md:374-380` |
| F105 | Hook 3 pós-créditos: voz Dante (ou alguém que poderia ser) — "não pedi pra você confiar. mas não pedi pra ele também. e ele ainda paga." | Dante off-screen ambíguo, alimenta sequel; pillar 4 contém | DAN / END | Dante | n/a (pós-créditos universal) | GRAVADO | `arco-principal.md:328-336` |
| F106 | Festival do Reaproveitamento FIR cínico (novembro), cartazes cobrindo pichações cooperativas | Era 3 sobrepõe Era 2 ritualmente; ironia direta sobre o galpão Alencar | E3 / DAN | FIR / Vorto | n/a | GRAVADO | `environments/06-periferia.md:85`, `tradicoes-cultura.md` |
| F107 | Pichação "AINDA SOMOS NÓS" repintada semanalmente no Galpão Cooperativo abandonado | Periferia resiste passivamente; cooperativa morta material mas viva culturalmente | E2 | Periferia | n/a (visual) | GRAVADO | `environments/06-periferia.md:89` |
| F108 | Voucher Janelarum para funcionários FIR (loop fechado de captura) | Mecanismo FIR de cooptar técnicos via dependência — Dante começou assim | E3 / DAN | FIR / Dante | Knowledge média (Bruno Caval ambient) | GRAVADO | `factions.md:310`, `environments/06-periferia.md:145` |
| F109 | Saudação Periferia "tá no fio / tá no volt" vs saudação Sterling "Runtime estável" | Choque cultural Era 2 vs Era 3 visível no cumprimento; Dante usa ambas (slip F006) | E2 / E3 / DAN | Periferia / Sterling / Dante | Knowledge média | GRAVADO | `tradicoes-cultura.md`, `factions.md:110-111` |
| F110 | Mestres pró-modernização da Ordem Recursiva aceitam parceria Sterling (rachadura interna arco Bento) | Sterling captura via cooptação interna, não confronto frontal — mesmo padrão Periferia / Cult / Apex-Data | STE / COM-BEN | Sterling / Ordem | n/a (beat arco) | GRAVADO | `arco-principal.md:163-167`, `factions.md:174` |
| F111 | Catedrais Neo-Sylvania saqueadas em -3 (São Vargas) — Velhusto cala-se para proteger restante | Pattern de extração Sterling, validado pelo silêncio defensivo Ordem | STE / E1 / COM-BEN | Velhusto / Sterling | n/a | GRAVADO | `factions.md:201-202` |
| F112 | "Sterling Corp tentou capturar Patch-Zero em -3, falha parcial, 4 técnicos contaminados isolados em ala hermética" | Origem operacional Patch-Zero como ameaça atual — vetor 2 multi-causal | STE / PAT | Sterling / Penedo | Knowledge média | GRAVADO | `lore-bible.md:317`, `timeline.md:128` |
| F113 | Polis-Vermelha caiu em -2 por cross-contamination via amostra importada | Sucessor Polis-Vermelha → GusWorld City se Sterling vencer | STE / PAT / END | Polis-Vermelha / Sterling | Knowledge baixa (ambient) | GRAVADO | `lore-bible.md:318-319`, `timeline.md:130-131` |
| F114 | Adila Murmúrio (40) tem facção artista resistente no Cult — Florín Estopa, Marlena Aurora, Patrício Velô | Cult não é monolítico; aliança potencial após Iara | COM-IAR | Iara / Florín / Marlena | n/a (factions) | GRAVADO | `factions.md:494-498` |
| F115 | Sub-Diretora Vitória Marquês (FIR) começa a temer Sterling em ato 2 — dissensão interna | FIR não é monolítico tampouco; sidequest Ouro destrava aproximação | DAN / END | Vitória / Vorto | Ouro | GRAVADO | `factions.md:299`, `environments/06-periferia.md:332` |
| F116 | Diretora Octávia Penedo aparece via holograma corporativo no arco Bento (mestres pró-Sterling morrem) | Sterling assiste o desastre via terminal, satisfeito — "teste útil" | STE / COM-BEN | Penedo / Sterling | n/a (beat arco) | GRAVADO | `arco-principal.md:165`, `factions.md:103` |
| F117 | "Bento se preocupa demais com tradição" (Dante, passing comment, cena 4) | Dante despreza tradição publicamente — coloca-se como anti-Asmódico antes do reveal | DAN | Dante / Bento | n/a (cena cômica) | GRAVADO | `comic-reliefs.md:228`, `characters/dante-grid.md:133` |
| F118 | "Eu não pedi pra você confiar." (Dante, climax Etapa 1) | Linha de reveal — sem culpa, sem raiva, apenas cansaço | DAN | Dante | n/a (climax) | GRAVADO | `arco-principal.md:230-231`, `characters/dante-grid.md:96` |
| F119 | "Você também não pediu pra fazer manutenção. Mas se ofereceu." (Gus, climax Etapa 1) | Resposta moral de Gus — distinção entre obrigação e oferta voluntária | DAN | Gus / Dante | n/a (climax) | GRAVADO | `arco-principal.md:231`, `characters/dante-grid.md:97` |
| F120 | Cumprimento canônico FIR não-formal usa "E aí, mano" emprestado da Periferia (mimetização local) | FIR esconde identidade corporativa em registro coloquial — operativos passam despercebidos | E3 / DAN | FIR | Knowledge alta | GRAVADO | `factions.md:309` |
| F121 | Espiral Fibonacci sobre folha estilizada bordada em mantos Pelicano Branco | Símbolo Selve canônico; tradição matemática viva | E1 / COM-JAC | Pelicano Branco / Jaci | n/a (visual) | GRAVADO | `factions.md:371`, `environments/08-selve-profunda.md:66` |
| F122 | Monóculo angular cromado com laser vermelho `#FF0000` (Sterling) | Único vermelho saturado fora de zonas Patch-Zero; assinatura visual da ameaça corporativa | STE | Sterling | n/a (visual) | GRAVADO | `characters/sterling-locke.md:27`, `factions.md:77-82` |
| F123 | Janelarum 2.0 absorvido por Sterling em -10 (era plataforma independente até então) | Padrão de absorção corporativa; cidadania trocou conveniência por captura | E3 / STE | Sterling | n/a | GRAVADO | `timeline.md:117`, `comic-reliefs.md:580-661` |
| F124 | Anciã Mariana descobriu drone Sterling camuflado em -0.5 com Tatauín, resolveu não tocar | Mariana sabe há tempo, escolha tática de não alertar Sterling — Pillar 2 prudente | STE / COM-JAC | Mariana / Tatauín | Knowledge média | GRAVADO | `environments/08-selve-profunda.md:67` |
| F125 | Sub-quest "Auditoria Cruzada" (Periferia): cruzar 3 dados de hora-de-passagem destrava doc 8 antes do climax | Player atento pode chegar a 75% do reveal antes do beat oficial — Ouro recompensa investigação | DAN / END | Gus / Dante | Knowledge alta | GRAVADO | `environments/06-periferia.md:202` |
| F126 | Sussurro canal 2 audível apenas com Dante na party em zona infectada (Selve Profunda arco Jaci, catedral contaminada arco Bento, Núcleo Mandelbrot externo): "ele não compila ... interpretam por ele ... ele pertence ao outro." | Patch-Zero reconhece Dante como vetor antes do player; cumulativo reveal Patch-Zero+Dante elo confirmado retroativamente após climax Etapa 1 | PAT / DAN | Patch-Zero / Dante | Knowledge alta (cross-ref retroativa) | GRAVADO | `characters/patch-zero.md` §Canal 2 + `characters/dante-grid.md` §Conflito intra-party |
| F127 | 4º encontro Patch-Zero canal 3 com Iara, opcional, no porão do Cult Mirage durante mini-quest pós-recrutamento: "você mente bem. eu também. nós podíamos." Iara recusa categórica | Plant: Patch-Zero reconhece Iara como técnica de engano competente; arc Iara reforçado "posso criar arte sem ser arma" | PAT / COM-IAR | Patch-Zero / Iara | Knowledge média (registro Diário) | GRAVADO | `characters/patch-zero.md` §Canal 3 + `characters/iara-lumen.md` §Conflito intra-party |
| F128 | Cena 4 bonus epilogue Ouro: Beatriz Pólvora (17, +1 ano timeskip) abre Escola de Asmódico aberta a não-membros da Ordem Recursiva na Praça do Compilador; Almagre faz sinal de aprovação sem falar; Bento aparece 1s no fundo | Payoff F073 (aprendiz reversa) + F022 (Almagre recuperado); cross-arco Catedrais + Periferia; tradição flexível continua viva | END / COM-BEN | Beatriz / Almagre / Bento | Ouro (cena exclusiva ending Ouro) | GRAVADO | `arco-principal.md` §Ouro |
| F129 | Ferro de solda antigo de Salviano (Era 2 cooperativista) na bancada Oficina Alencar há 8 anos, etiqueta "S.A." gravada à mão no cabo, Dante nunca tocou | Dante apaga legado simbolicamente (F019) mas preserva legado físico (F129) — auto-conflito não-articulado; sabe quem é, mente sobre quem é | DAN | Dante / Bento | Knowledge média (etiqueta) / alta destrava reflexão Bento | GRAVADO | `environments/06-periferia.md` §3 |
| F130 | NPC ambient: voz anônima mesma em 2 cidades-irmãs (Polis-Vermelha residual + Cidades-Gêmeas) via rádio captado por antena Underground recuperada na Zona do Silêncio mid-ato 2. Mesma voz, mesmo timbre, mesma cadência, sob código de chamada anônimo | Reforça Patch-Zero plural (F059) audivelmente sem confirmar; sequel hook F104 fortalecido | PAT | Patch-Zero | Knowledge alta (cruzamento retroativo) | GRAVADO | `environments/07-zona-do-silencio.md` §4 + `characters/patch-zero.md` §Sample |

---

## Seção 1 — Dante traidor (reveal climax Etapa 1)

Dante é o reveal mais carregado do jogo. Funciona porque o foreshadow é redundante em sete canais distintos: cena cômica (F001), prop ambient na oficina (F002, F018-F020), prop ambient externo no quintal vizinho (F003), fala de NPC fofoqueiro (F004), comportamento social ambient ato 2 mid (F005, F011, F028), tic facial recorrente (F010), saída acidental de léxico Sterling (F006), entry de Patch-Zero canal 1 (F024), degradação mecânica de stats (F013), e tensão intra-party sentida por 4 dos 5 companions (F014-F017). Cada um isoladamente é negligenciável; juntos formam o cofre que o climax abre.

A escolha autoral central foi **não tornar Dante misterioso, e sim tornar Dante familiar**. Dante é confiável durante toda a fase 2 porque genuinamente ajuda. A traição funciona como tragédia porque Dante não é vilão — é alguém que apostou no destruidor da família por ressentimento amargo (F026, F027). A camada double-layer das memórias formativas paga o foreshadow narrativo retroativamente: player Knowledge alta descobre que Vorto apareceu no funeral de Salviano, e Sterling falou pessoalmente com Dante de 10 anos, e Dante aceitou olhando nos olhos do destruidor. Não foi enganado. Foi convencido pelo cinismo aplicado a si mesmo.

O timing do reveal é canônico em três momentos: (a) Cena 4 `comic-reliefs` ("Force Push") dispara automaticamente após 50% da campanha como Chekhov primário; (b) Patch-Zero entry no Diário ~75% (F024) diz quase com nome próprio o que está acontecendo; (c) climax Etapa 1 paga formalmente. Player que prestou atenção marca a cena 4 e bate punho na mesa em silêncio quando o Diário-Patch-Zero confirma. Player que correu vê o reveal pleno apenas na entrada das Catedrais de Silício corrompidas, e ainda assim com 60% do foreshadow ambient já consumido (mesmo sem ter ligado os pontos).

Há uma proposta autoral em discussão (PROPOSTA, ver §8): adicionar plant cruzado entre Dante e Patch-Zero canal 1 em uma única entry no late ato 2 (F-PROP-1). Atualmente F024 é o plant Diário direto. Se quisermos camada extra, um sussurro Patch-Zero canal 2 audível **apenas** quando Dante está fisicamente presente em zona infectada poderia funcionar — Patch-Zero "reconhece" Dante como vetor antes do jogador. Isso reforça duplo: Patch-Zero é onisciente táctico; Dante é detectável até para inimigos do inimigo dele. Requer aprovação por arriscar over-foreshadowing.

A vítima moral central da Periferia é Mestre Almagre (F022), não Edilma. Edilma é wound canônico; Almagre é quem **acreditou** profissionalmente, paternalmente, comunitariamente. A traição custa a fé de Almagre em julgar caráter. Player que olha bem percebe: Almagre representa a comunidade C-Arcane inteira que apostou em Dante como continuidade da Era 2. Sterling colhe o que plantou; mas Almagre paga o pedágio.

---

## Seção 2 — Endings (Bronze / Prata / Ouro)

Endings em GusWorld são gradiente knowledge-gated, não escolha binária. Sem moral-meter visível. Sem painel de "você é bom / você é mau". Apenas: quem aprendeu mais salva mais. Filosofia Pillar 1 (lógica vence força) + theme (inteligência serve à vida) — quem investiga ganha o ending pleno. Plant é distribuído por **todos os arcos**: doc descobrível, prop ambient, dialogue de NPC, scan de fauna, decifragem de cripto-glifo, entry de Patch-Zero, branching de arco específico.

Os três caminhos visíveis na borda do Núcleo Mandelbrot Interno (F100) são plant visual final: fuga (leste, Bronze), confronto direto (norte, Prata), compilação reversa (oeste, Ouro). Player Bronze entrou sem Knowledge suficiente para ler o caminho oeste como opção; o caminho está lá, mas a habilidade de seguir não foi treinada. Quem chegou a F099 com Knowledge alta tem a terceira opção destravada em diálogo com Patch-Zero canal 3 no climax.

Bronze paga o foreshadow de Polis-Vermelha caída como espelho: GusWorld City beira o mesmo destino. Sem epilogue bonus. Hook 1 pulsa intenso. Diário entry final em F104. Dante (capturado) morto pela Sterling Corp residual. Vilarejo Pelicano Branco perde 100% das sementes-relíquia em F063.

Prata paga o foreshadow como vitória limpa mas melancólica. Patch-Zero selado controlado. Polis-Vermelha começa recuperação lenta. Vilarejo seguro com 60% das sementes (recuperação parcial via sub-quest base Sterling abandonada). Dante: por branching #5. Hook 1 pulsa moderado. Diário entry F104 igual.

Ouro paga o foreshadow ancestral inteiro. Doc 15 (F064) confirma linhagem Neo-Sylvania → Verônica → Gus: "selar, não erradicar". Cidades-irmãs cooperam pós-jogo. Bonus epilogue 3 cenas: vilarejo em festival (F101, F063 cena de replantio); Bento em catedral aberta ensinando criança; Cauã + Linda fundando oficina mista nos Dutos (eco F088). Dante: redenção forçada com Knowledge alta requer sucesso em branching #5; caso contrário, Ouro alcançável mas Dante morto. Hook 1 pulsa menos. F104 ainda toca (Patch-Zero **nunca** destruído; coerência Pillar 2).

Plants knowledge-gated críticos para Ouro: doc 11 (F057, pichações Polis-Vermelha), doc 13 (F077, bilhete Davi), doc 15 (F064, cripto-glifo Catedral-Mãe), doc 8 (F025, memorando FIR), branching #4 = alertar Cidades-Gêmeas, e F099 negociação Patch-Zero pela terceira opção. Player que perdeu qualquer um destes ainda alcança Prata. Sem cataclismo se faltar um.

---

## Seção 3 — Sterling Locke

Sterling é foreshadowed publicamente antes de ser nomeado privadamente. Player vê o monóculo cromado com laser `#FF0000` (F122) em hologramas (F030) e cartazes (F034) durante o ato 1 inteiro, antes de qualquer cena privada com ele. A presença ambient cresce em intimidade: F030 (poster massivo distante) → F042 (loop "continue, continue, continue") → F043 (fala pessoal sobre FIR em arco Cauã) → F116 (presença operacional via Penedo holograma em arco Bento) → confronto final em Cúpula. Cresce em escalada controlada.

A filosofia de Sterling é plantada em doc 1 (F035) antes de ser confrontada em combate. Player que lê o fragmento público no átrio da Cúpula em ato 3 cedo descobre que DRE é tese, não improviso. Player Ouro tem versão completa no interior da Cúpula. O confronto Fase 2 é puzzle de decodificação porque Sterling é cofre filosófico (etimologia: Locke = fechadura). Mecânica espelha narrativa.

Sterling como predador corporativo é foreshadowed via três conglomerados canibalizados antes do jogo: doc 5 (F036, audit Apex-Data); cena 11 `comic-reliefs` (F031, Janelarum 2.0 absorvido em -10, F123); destruição de cooperativa Alencar (F019, F021); morte de Davi Berenger via FIR-subcontratada (F079, F077). Cada plant carrega peso humano específico — não é abstração corporativa. Inácia Berenger demitida e silenciada (F075). Sterling não destrói prédios; **deleta contratos**. A erosão é administrativa, lenta, legal.

Sterling como predador da Selve é foreshadowed via: catedrais saqueadas em -3 (F041, F111), drone camuflado no Pelicano Branco em -1 (F047, F124), zonas de deleção GRE (F048), fauna com sintaxe quebrada (F049). Player que percorre Selve Profunda em arco Jaci sente o saqueamento em curso antes de Sterling aparecer em arco Iara como filósofo. A operação é mais antiga que a aparição pessoal dele.

Sterling como manipulador de Dante é foreshadowed em camada double-layer: F026 (Vorto no funeral de Salviano), F027 (Sterling pessoalmente fala com Dante de 10 anos). Plants pré-jogo gravados em characters/dante-grid.md memórias formativas, descobríveis via Knowledge alta em sub-quest pós-reveal. Camada aparente: Dante foi recrutado por Vorto aos 9 "por aprendizado técnico subsidiado". Camada real: Sterling escolheu Dante pessoalmente, sabia o nome, sabia que ele era Alencar, sabia que o cinismo aplicado a si mesmo era a chave.

Sterling **nunca** é cômico (regra explícita `comic-reliefs.md`). Toda comédia em torno dele é sátira da propaganda Sterling Corp (F031, F032, F037, F039), nunca dele pessoal. Mantém a frieza ameaçadora. Único momento em todo material onde Sterling se aproxima de emoção: F050 ("a amostra criou a janela específica. reveja seus pressupostos sobre o que é a amostra"). Player que lê doc 14 atento sente o cansaço técnico de quem percebeu que perdeu controle e ainda contém.

---

## Seção 4 — Patch-Zero (sistema-antagonista)

Patch-Zero não é vilão pessoa. É vilão sistema. Plant respeita isso: aparece em quatro canais inseparáveis sem nunca consolidar em corpo físico antropomórfico. Canal 1 (Diário, F051, F058, F059): texto glitch sem que o jogador escreva. Canal 2 (áudio ambient, F052): sussurros baixos em zonas infectadas, primeiro audível por Linda em rádio analógica. Canal 3 (persona dialogável em boss arenas, F097-F099): silhueta refratada em vidro rachado, voz com glitch overlay, propostas com porcentagem variável de mentira. Canal 4 (bug visual ambient, F048, F053): shaders procedurais glitcham, vermelho `#FF0000` aparece em folhas da Selve.

Patch-Zero é foreshadowed como anti-padrão antes de ser nomeado. Player vê fauna com sintaxe quebrada (F049), shader glitch ambient em Selve Profunda (F048), tique de cristal Neo-Sylvania não decifrado (F060), referência ancestral em doc 7 (F091 "esse lugar pulsa. sempre pulsou"), e tratado anônimo Era 2 (F056 "selar, não erradicar"). Quando Patch-Zero é nomeado mid-ato 2 (canal 3 em arco Bento ou Jaci), player conecta retrospectivamente: o que vinha vendo desde o ato 1 tinha nome.

A escolha autoral central é Patch-Zero como **resíduo Neo-Sylvania**. F054 (cripto-glifos "o limite que escolheu falar"), F055 (doc 3 diário Atelaiá Chevalier, -80), F056 (doc 10 tratado Verônica Atelaiá, -45), F064 (doc 15 placa cripto-glífica Catedral-Mãe "selar, não erradicar"). Quatro plants distribuídos por 700 anos in-world. Linhagem Neo-Sylvania → Atelaiá → Verônica → Gus. Sterling representa a ruptura singular de 700 anos.

Patch-Zero **nunca** é cômico. Terror puro. Patch-Zero **nunca** é destruído. Selado. Pillar 2 explícito: caos genuíno irredutível existe. Mesmo em Ouro, F104 (Diário entry pós-créditos) toca. A ameaça persiste. O sequel hook é canônico e universal independente de ending.

---

## Seção 5 — 6 arcos companion

### 5.1 Cauã Volt (Striker, Pythia, Dutos Infernais)

Wound canônico: morte do irmão Davi Berenger em -5 na Subestação 7 (Davi tinha 16, Cauã tinha 8). Plants distribuídos: doc 2 (F075, carta nunca enviada de Inácia), doc 9 (F076, diário queimado de Davi), doc 13 (F077, bilhete final, gate Ouro). Sub-quest "favor pequeno" arco Cauã destrava F075 e F076; Knowledge alta + decisão de mostrar a Cauã destrava F077.

Memória formativa F079 (Davi mostrou a Cauã aos 5 que os cabos da Subestação 7 estavam amarrados com fio de nylon) é plant remoto: Davi sabia desde sempre, e nunca pôde corrigir, e morreu no que sabia. Cross-arco F078 com Almagre (que conheceu Davi 4 meses antes da morte, em -6 a -5): "Davi era dos mais cuidadosos. Não foi descuido. Eu sei." Almagre confirma a Cauã sem prova documental; só a fé profissional.

Payoff: reveal pleno wound recebe Knowledge gate Ouro. Cauã recontextualiza Davi como agente, não vítima passiva. Pillar 4 contém, sem catarse explosiva. Bonus epilogue Ouro: Cauã estabelece "Subestação 7" como nome simbólico de hub seguro juvenil pós-jogo. Eco de F088 ("Vinil Última Frequência partido em 3, quebrou em -2 quando Mara soube de Polis-Vermelha"): Cauã + Linda fundam oficina mista nos Dutos em epilogue Ouro.

### 5.2 Iara Lumen (Infiltradora, Óxido, Setor Mirage, desertora Cult)

Wound canônico: manipulação por Adila Murmúrio desde os 6 anos. Plants: F081 (Iara aos 9 ouviu Adila falando com Cleomir — fissura primeira), F082 (doc 4 panfleto Atualização Sensorial + cláusula oculta), F084 (origem Iara como abandonada bebê em catedral menor anexa, "Iara Koslov" escolhido por funcionário Cult).

Adila plantada como manipuladora desde a Era 2 tardia: F083 (rancor paradoxal por Sonja Murmúrio, mãe afastada ritualmente em -34 sob argumento de "retiro contemplativo" e morta no mesmo ano em circunstâncias nunca esclarecidas; consolidação operacional plena de Adila ocorre 28 anos depois em -6). Adila guarda culpa internalizada, não menciona Sonja, evita o nome. Knowledge alta cruza retroativamente.

Cult Mirage não-monolítico: F114 (Florín Estopa, Marlena Aurora, Patrício Velô — facção artista resistente). Sub-quest pós-recrutamento Iara liberta ala artista. Payoff: Iara entende que pode criar arte sem ser arma, beleza pode servir à vida. Não destrava Ouro sozinho; alimenta densidade narrativa do final Ouro.

Cross-setting F044: cabo Óxido coaxial do Mirage termina na subestação industrial 11 da Periferia. Iara identifica padrão Óxido em arco Dante: "isto é do Caleidoscópio. Eu projetei a interface de um deles, com 8 anos. Achei que era pra holografia." Iara, sem saber, ajudou Sterling. Iara, agora sabendo, fica em silêncio. Pillar 4 contém.

### 5.3 Bento Requiem (Tanque, Asmódico, Catedrais Neo-Sylvania, exceção P2)

Wound canônico: morte de Hilário Tepenkov em -7 na Catedral Menor de Atelaiá quando Bento tinha 4 anos. Plants: F070 (Pátio do Cronômetro-Hilário, environmental storytelling silente), F055 (doc 3 diário Atelaiá Chevalier descrevendo padrão similar em -80, eco direto). Padrão: "estávamos em treze. saímos em quatro" (Atelaiá) ↔ "13 mestres no incidente original / 4 sobreviventes / Bento aos 4 anos sobrevive" (Hilário em -7). Linhagem de horror Neo-Sylvania → Catedral Menor de Atelaiá → Bento.

Wound recente: morte de Aldebrando Chevalier (pai) em -0.5. F072 (Velhusto entrega o diário a Bento; lê durante 3 meses pré-jogo). Aldebrando descobriu interferência precoce Sterling em cripto-glifo profundo — foi eliminado. Bento entra o arco já consciente do peso.

Foreshadow Catedral perdida arco Bento mini-quest pós-recrutamento: F064 (doc 15, gate Ouro). Coordenadas da Catedral-Mãe permanecem **sem resolução em G1**. Sequel hook ancestral. Linhagem Atelaiá → Verônica Atelaiá (doc 10, F056) → Gus. Sterling como ruptura singular de 700 anos.

Cross-setting F071: cronômetro de Ardenia Falke intacto na vitrine biblioteca privada Velhusto — "Ela está adiantada de nós em algum lugar." Mestres desaparecidos da Ordem em -3 (saque São Vargas) ainda contam tempo. Ambient ético — Velhusto não desistiu de Ardenia, só conteve.

Aprendiz Beatriz Pólvora (F073) plantada como aprendiz reversa: Bento ensina ela enquanto ele próprio se forma. Bonus epilogue Ouro: Bento em catedral aberta ao público; primeira lição de Asmódico a uma criança. Cresce.

### 5.4 Linda Siren (Crowd Control, Óxido, Zona do Silêncio)

Wound canônico: batida Sterling Corp + FIR em -8 — toca-discos histórico Neumann apreendido. Plants: F056-derivada (agulha de toca-discos em pingente no pescoço de Linda como prop permanente visual); F034 (cartaz Sterling Sigma rasgado pelo Underground); F086 (doc 6, carta cifrada de Tiago — pai-figura escondido revelado retroativamente). F085 (Tiago era conhecido de Bartolo Penkin, doc 5 cross-link).

Operação Espelho Acústico (F045): Sterling testou controle social por supressão sonora em -7 a -5, projeto abandonado por GRE. Plant Era 3 visível em hangar técnico subterrâneo Posto 7 com console piscando "Validação Pendente" há 19 anos. F046 (diagrama mostra faixa abaixo de 60Hz "atravessa"). Linda usa 47Hz Óxido em climax. Sterling subestimou frequências sub-graves. Ironia técnica: a empresa esqueceu que a operação existiu, e a câmara Neo-Sylvania (F065) descoberta acidentalmente em -3 por Joaquim cancela o cancelamento.

Última Frequência (F087): música composta pelos 8 técnicos desaparecidos antes da batida de -50, tradição anual Noite Calada. F088 (vinil partido em 3 pedaços, quebrou em -2 quando Mara soube de Polis-Vermelha). Eco geracional do trauma. Linda toca rádio analógica e ouve sussurros distorcidos de Polis-Vermelha (Patch-Zero canal 2, F052) em Ten do arco. Reveal: Sterling planeja transmissão Patch-Zero em larga escala via antenas.

Branching #4 (alertar Cidades-Gêmeas) destrava doc 11 (F057, gate Ouro pichações Polis-Vermelha). Plant + branch consequência sentida + payoff: Cidades-Gêmeas estabilizadas em Prata/Ouro; sem alerta = Bronze.

### 5.5 Jaci Proxy (Healer biológica, Pythia, Selve Profunda)

Wound canônico: morte da mãe Lia e do pai Solano em -8 por surto silencioso no Pelicano Branco. Plants: F093 (ampola Pythia bio quebrada com líquido cinzento secando há 8 anos na estação Pythia bio — Lia trabalhou na noite antes de morrer, ninguém recolheu); F094 (caderno aberto de Lia: "vetor não é endógeno. Vou ao norte. Volto antes do amanhecer." — Ouro confirma causa real: testagem clandestina Sterling de bio-script DRE em vilarejo periférico). Mariana suspeita; nunca prova publicamente.

Memória formativa F095 (Jaci aos 8 vê raposa-fractal com padrão arbitrário, Tatauín diz "tá sendo OUTRA COISA pela primeira vez. A Selve faz isso de vez em quando. A gente respeita."): primeira aparição embrionária Patch-Zero antes do nome. Jaci tem intuição plantada. Bio-leitura aplicada como criptografia inversa em adultos (Dante, F014).

Doc 7 in-world (F090, F091, F092) — *Fragmentos do Antigo Diário da Selve* transcrito por Soraia Vanderbist em -45. Linhagem Vanderbist Anhuera (-95, co-autora Pythia) → Soraia → Mariana → Lia → Jaci. Pelicano Branco sempre soube do "lugar onde a floresta se esquece" (Núcleo Mandelbrot / Patch-Zero ancestral). Fragmento 47: Soraia previu morte de Solano.

Sementes-relíquia (F063): 880 espécies viáveis Era 1. Ouro destrava replantio em bonus epilogue (vilarejo em festival, F101). Pena de pelicano branco (F101) recontextualizada como bom presságio: Mariana sabia.

Sub-quest cruzada Knowledge Ouro com Mariana: F080 (Salviano Alencar planejou asilo no Pelicano Branco em -9; Mariana ofereceu acolhimento; Salviano não conseguiu ir; Edilma soube; nunca contou pra Dante). Subtexto pesado: Dante poderia ter crescido em Pelicano Branco. Não cresceu. Plant alternativo de tragédia, validador de pillar 4.

### 5.6 Dante Grid

Cobrado integralmente em §1. Não repetir aqui.

---

## Seção 6 — Lore 3 eras (descoberta gradual)

Knowledge Progression é o veículo de revelação histórica. Player que farma Bestiário sem ler nada chega no Bronze. Player que lê e decifra chega no Ouro. O sistema de eras é foreshadowed via prop ambient distribuído por todos 8 settings.

**Era 1 (Pré-Código / Neo-Sylvania)** plants: cripto-glifos espiralados em pilares (F062, F069), câmaras Neo-Sylvania sob Zona do Silêncio (F065), Câmara DNA de coleta Neo-Sylvania na Selve Profunda (F063), tique de cristal Neo-Sylvania não decifrado (F060), cripto-glifos isolados na Periferia como marcos de divisa rural (F061), doc 3 (F055), doc 7 (F090-F092), doc 15 (F064). Distribuição: catedrais densas, Selve Profunda densa, Periferia rasa, Zona do Silêncio escondida sob duas camadas. Player aprende a ler camadas conforme cruza settings.

**Era 2 (Era do Compilador)** plants: placa de latão "À PRIMEIRA LINHA QUE COMPILOU, OBRIGADO" (F067), Tomo da Pilha Sobrecarregada em sebo público (F068), trilha de latão dos pioneiros (F066), engrenagem-mestre Fibonacci da Catedral (F069), oficinas Era 2 cooperativas C-Arcane (F107), pichação "AINDA SOMOS NÓS" (F107), Última Frequência (F087), partituras Era 2 no anexo Underground (F088 contextual), parceria fundadora Berenger-Vanderbist em -95 (Selve), música Era 2 vs música Sterling Sigma (F034 contraste).

**Era 3 (Era Sterling)** plants: holograma Sterling em todo poste (F030), tela LED travada Janelarum (F033), monóculo Sterling com laser `#FF0000` (F122), cartaz Sterling Sigma (F034), Estação Espelho Acústico abandonada (F045), zonas de deleção GRE (F048), drone camuflado no Pelicano Branco (F047), Festival do Reaproveitamento FIR (F106), voucher Janelarum (F108), saudação Sterling Corp "Runtime estável" (F006/F109), placas Sterling em catedrais (F040), saque São Vargas (F041).

Cada setting tem **as três eras visíveis em camadas materiais cruzadas** (regra Bloco F). Player que cruza catedrais vê Era 1 dominante + Era 2 retro-fit + Era 3 saque. Player que cruza Zona do Silêncio vê Era 2 dominante + Era 1 escondida embaixo + Era 3 abandonada por cima. Player que cruza Periferia vê Era 2 dominante (cooperativa morta administrativamente) + Era 1 rasa (marcos de divisa rural) + Era 3 invasiva (FIR vassala). A leitura cross-era é arquitetura de world-building em si.

---

## Seção 7 — Plants frouxos / órfãos (AUDITORIA)

Auditoria sistemática dos plants gravados em busca de Chekhov não-pago (FLAG-PLANT) ou payoff sem plant prévio (FLAG-PAYOFF). Critério: se um plant não tem payoff identificável até climax/epilogue, ou um payoff aparece sem foreshadow distribuído em pelo menos um canal, recomenda-se cortar, adicionar plant, ou aceitar lacuna deliberada.

**Plants com payoff claro e suficiente:** F001-F029 (Dante), F030-F046 (Sterling), F051-F060 (Patch-Zero), F063, F067-F077 (lore eras + companions específicos), F082-F099 (companions arc-specific). Sem flag.

**Plants com payoff parcial (aceito como lacuna deliberada / sequel hook):**

- **F020 (moldura vazia entre Salviano e Dante criança).** Marcado canonicamente como lacuna permanente. Sem revelação in-game; player decide qual foto era. Confirma intenção autoral — não é frouxo, é deliberadamente sem fechamento. Mantém.
- **F060 (tique de cristal Neo-Sylvania a cada 23 segundos).** Marcado como "ainda não decifrado". Sequel hook ancestral. Player não recebe explicação em G1. Mantém como sequel hook.
- **F064 (doc 15 Catedral-Mãe).** Coordenadas deliberadamente sem resolução em G1. Sequel hook. Mantém.

**Plants com payoff possivelmente subutilizado (FLAG, recomendação no §8):**

- **F102 (cordão de cipó com 11 nós de Mariana).** Plant emocional ambient bonito mas sem payoff narrativo direto. Recomendação: no bonus epilogue Ouro (F101, vilarejo em festival), mostrar Mariana adicionando o 12º nó in-frame — paga 4 anos em 1 frame. Não cortar.
- **F068 (Tomo da Pilha Sobrecarregada em sebo).** Cena 7 `comic-reliefs` paga a referência cultural. Mas não há payoff narrativo direto (player não destrava nada via consulta ao Tomo). Recomendação: deixar como ambient cultural reforçando theme Era 2 cooperativa. Não cortar.
- **F088 (vinil Última Frequência partido em 3 pedaços por Mara).** Plant emocional sem payoff direto. Recomendação: incluir cena curta em bonus epilogue Ouro onde Mara ouve o vinil montado em ambiente Underground reorganizado pós-jogo, ressonando com Cauã + Linda fundando oficina mista. Costura epilogue Ouro.
- **F083 (Adila guarda rancor paradoxal pela mãe Sonja).** Plantado em factions/timeline mas sem payoff in-game claro. Adila cai sem confrontação direta com fantasma da mãe. Recomendação: aceitar como sub-quest opcional Ouro — se player tem Knowledge alta e libera ala artista (F114), Hierofante Patrício Velô pode revelar a Iara que "Adila tem um quarto trancado que ela própria não entra; já vi". Plant sutil de culpa internalizada sem catarse adulta. Pillar 4 contém.

**Payoffs sem plant suficiente (FLAG-PAYOFF) — auditoria negativa:**

- **Cena climax Etapa 1 menciona "padrão Sterling reconhecível" na mão modular de Dante.** Plant ambient: F008 (mãos progressivamente limpas), F009 (reflexo cyber-corporativo nos óculos de solda), F018 (caderno C-Arcane na bancada). Suficiente. Sem flag.
- **Dante "muda para C-Arcane no late game" (F012).** Plant: F018 (caderno C-Arcane na bancada da Oficina Alencar). Aceito. Sem flag.
- **Hook 3 pós-créditos: voz Dante (ou alguém que poderia ser).** Plant ambíguo é o ponto; pillar 4 contém ambiguidade. Sem flag.

**Plants candidatos a corte (recomendação: nenhum corte).** Auditoria não identificou plant frouxo a ponto de cortar. Sistema de foreshadow é denso mas todas as entradas têm função.

---

## Seção 8 — Propostas autorais (PROPOSTA, exige aprovação)

Plants novos sugeridos para fortalecer coerência ou densidade narrativa. **Não gravados em docs canônicos.** User aprova antes de viralizar.

### F-PROP-1 — Sussurro Patch-Zero canal 2 quando Dante está em zona infectada

**STATUS:** APROVADA (2026-05-16) — viralizada como F126. Plant agora GRAVADO em docs canônicos (`characters/patch-zero.md` §Canal 2 + `characters/dante-grid.md` §Conflito intra-party específico).

**Descrição:** áudio diegético sussurrante audível somente quando Dante está fisicamente em zona infectada por Patch-Zero (Selve Profunda arco Jaci, catedral contaminada arco Bento, Núcleo Mandelbrot externo). Frase fragmentada: "ele não compila ... interpretam por ele ... ele pertence ao outro." Inaudível com qualquer outra composição de party na mesma zona.

**Onde gravar:** `characters/patch-zero.md` §Canal 2 (adicionar exemplo) + `characters/dante-grid.md` §Conflito intra-party específico.

**Por que necessário:** Patch-Zero é apresentado como onisciente táctico ("ele tentou nos enjaular"). Player atento Knowledge alta perceberia que Patch-Zero reconhece Dante como vetor antes do jogador. Cross-link entre os dois antagonistas inseparáveis. Reforça pillar "inteligência distribuída acumulada" — até inimigo do inimigo do herói detecta o operativo.

**Risco:** over-foreshadowing. Se muito audível, joga reveal pra Knowledge baixa e diminui o impacto cumulativo. Mitigação: volume baixíssimo, exige cruzar zona com Dante presente em momento específico, registrado como "anomalia Diário" não-explícita.

### F-PROP-2 — Sub-quest cruzada Iara-Patch-Zero canal 3

**STATUS:** APROVADA (2026-05-16) — viralizada como F127. Plant agora GRAVADO em docs canônicos (`characters/patch-zero.md` §Canal 3 4º encontro + `characters/iara-lumen.md` §Conflito intra-party específico).

**Descrição:** quarto encontro Patch-Zero canal 3, opcional, exclusivo arco Iara (currently 3 encontros obrigatórios). Patch-Zero a aborda via tela de holografia rachada no porão do Cult Mirage durante mini-quest pós-recrutamento. Proposta: "você mente bem. eu também. nós podíamos." Iara recusa categoricamente (uma das poucas vezes que age sem hesitação no jogo).

**Onde gravar:** `characters/patch-zero.md` §Canal 3 (adicionar 4º encontro condicional) + `characters/iara-lumen.md` §Conflito intra-party específico (vs Patch-Zero implícito).

**Por que necessário:** Patch-Zero é foreshadowed como manipulador filosófico mas as 3 propostas atuais são via Gus (ou via Jaci, encontro 2). Faltando: tentativa direta de cooptar companion que opera por engano profissional. Iara recusando reforça arc: "posso criar arte sem ser arma". Plant também: Patch-Zero reconhece Iara como técnica de engano competente.

**Risco:** alongar arco Iara em sidequest que não fecha em payoff mecânico claro. Mitigação: encontro curto (1 cena, 2-3 trocas, destrava entry Diário com Knowledge bonus).

### F-PROP-3 — Cena pós-créditos epilogue Ouro: Beatriz Pólvora abrindo escola pública de Asmódico

**STATUS:** APROVADA (2026-05-16) — viralizada como F128. Plant agora GRAVADO em docs canônicos (`arco-principal.md` §Ouro, Cena 4 bonus epilogue).

**Descrição:** quarta cena bonus epilogue Ouro (atualmente 3): Beatriz Pólvora (16), aprendiz atual de Bento, aos 17 abrindo Escola de Asmódico aberta a não-membros da Ordem Recursiva, na Praça do Compilador da Periferia (escolha simbólica). Crianças da Periferia se inscrevem. Mestre Almagre passa, olha, faz um sinal de aprovação com a cabeça (não fala). Bento aparece por um segundo no fundo, segurando o escudo-catedral, sorri pequeno.

**Onde gravar:** `arco-principal.md` §Ouro (adicionar Cena 4 bonus epilogue) + `characters/bento-requiem.md` (não fazer — fica para Bento próprio caso aprovado).

**Por que necessário:** Beatriz é plantada (F073) como aprendiz reversa. Sem payoff narrativo direto. Epilogue Ouro tem espaço para uma quarta cena que costura Periferia (Almagre, F022) + Catedrais (Beatriz, Bento) — cross-arco coerente. Reforça theme: tradição flexível continua viva, não dogma. Plant também: Almagre não-fala mas reaparece com dignidade (`some por uma semana e volta como se nada` — recuperou).

**Risco:** inflar epilogue. Mitigação: cena de 8-12 segundos, sem diálogo.

### F-PROP-4 — Plant ambient: ferro de solda de Salviano na Oficina Alencar

**STATUS:** APROVADA (2026-05-16) — viralizada como F129. Plant agora GRAVADO em docs canônicos (`environments/06-periferia.md` §3 Props narrativos).

**Descrição:** ferro de solda antigo de Salviano (modelo Era 2 cooperativista) ainda na bancada da Oficina Alencar (§2.1 setting 06 Periferia), com etiqueta "S.A." gravada à mão no cabo. Dante nunca tocou. Lá há 8 anos. Knowledge média percebe a etiqueta. Bento, se cruzar, identifica o modelo: "isto é Era 2 cooperativa. Modelo Alencar pai. Dante nunca usou."

**Onde gravar:** `environments/06-periferia.md` §3 Props narrativos (adicionar entrada).

**Por que necessário:** F019 (placa pintada antiga riscada por Dante aos 9) mostra Dante apagando legado simbolicamente. Faltando: plant que mostra Dante **preservando** o legado físico, mesmo sem usar. Reforça camada de auto-conflito não-articulado: Dante racionaliza traição, mas mantém ferro do pai sem mexer. Subtexto: ele sabe quem é, e mente sobre quem é.

**Risco:** sobrecarga de props na Oficina Alencar (já temos F002-F003, F018-F020, prop F-PROP-4, caixa de ferramentas escolar). Mitigação: ferro pequeno, ambient, sem prompt de interação até Knowledge alta.

### F-PROP-5 — Plant cross-arco: NPC ambient ouvido em duas cidades-irmãs

**STATUS:** APROVADA (2026-05-16) — viralizada como F130. Plant agora GRAVADO em docs canônicos (`environments/07-zona-do-silencio.md` §4 Áudio diegético + `characters/patch-zero.md` §Sample canal 2).

**Descrição:** NPC ambient único, sem nome canônico, ouvido brevemente em ambient de duas cidades-irmãs distintas mencionadas in-world (Polis-Vermelha residual via rádio em Zona do Silêncio + Cidades-Gêmeas via rádio em Zona do Silêncio em ato 2 mid). Mesma voz, mesmo timbre, mesma cadência. Knowledge alta cruza retroativamente. Plant: Patch-Zero plural reconhece a si mesmo? Ou é coincidência? Lacuna deliberada.

**Onde gravar:** `environments/07-zona-do-silencio.md` §4 Áudio diegético (adicionar) + `characters/patch-zero.md` §Sample de "falas" (adicionar como ambíguo).

**Por que necessário:** F059 (Patch-Zero plural "em polis-vermelha já somos livres") é foreshadowed em texto. Faltando plant audível que confirma sem confirmar plural multi-cidade. Player Knowledge alta percebe; player normal não. Reforça sequel hook universal F104.

**Risco:** confusão com NPC nomeado canônico do Underground (Lazar Tovrov, Penha Cintra, Otília Vermelha). Mitigação: voz curta, não nomeada, anônima sob código de chamada.

---

## Distribuição estatística (síntese)

Total entradas: 130 plants. Status:

| Status | Quantidade | Notas |
|---|---|---|
| GRAVADO | 130 | Todos consolidados em docs canônicos Fase 1 (inclui F126-F130 viralizadas do §8 propostas autorais aprovadas em 2026-05-16) |
| PREVISTO | 0 | (Plants já planejados saem como GRAVADO em docs existentes) |
| PROPOSTA | 0 | (5 propostas F-PROP-1 a F-PROP-5 aprovadas e viralizadas como F126-F130) |

Distribuição por categoria:

| Categoria | Plants |
|---|---|
| Dante (DAN) | 31 (F001-F029, F080, F108-F109, F117-F120, F125, F126, F129) |
| Sterling (STE) | 17 (F030-F048, F050, F110-F113, F116, F122-F123) |
| Patch-Zero (PAT) | 16 (F049, F051-F060, F091, F095, F097-F099, F104, F126, F127, F130) |
| Companion-Cauã (COM-CAU) | 6 (F074-F079) |
| Companion-Iara (COM-IAR) | 6 (F081-F084, F114, F127) |
| Companion-Bento (COM-BEN) | 8 (F070-F073, F098, F110-F111, F128) |
| Companion-Linda (COM-LIN) | 7 (F085-F089, F046, F065) |
| Companion-Jaci (COM-JAC) | 11 (F090-F096, F101-F102, F121, F124) |
| Lore-Era1 (E1) | 7 (F054-F056, F060-F062, F065, F069, F121) |
| Lore-Era2 (E2) | 8 (F066-F069, F074, F087, F107, F109) |
| Lore-Era3 (E3) | 11 (F030-F034, F040, F041, F045, F047, F048, F106, F108, F123) |
| End-gate (END) | 11 (F057, F063-F064, F077, F084, F094, F099-F101, F104-F105, F115, F125, F128) |
| Mecânico transversal (MEC) | 1 (F013, embedded) |

Categorias contam entradas multi-categoria múltiplas vezes (uma entrada pode contar como DAN+STE simultaneamente).

### Suportes documentais DD-016 a DD-023 (viralizados 2026-05-16)

As 8 propostas autorais aprovadas pelo criador supremo em 2026-05-16 foram viralizadas como docs canônicos em [[in-world-docs]] §16-23. **Nenhuma gera plant F### novo** — todas são suportes documentais a plants existentes:

| Doc canônico | Plants F### suportados | Função no tracker |
|---|---|---|
| DD-016 (Caderno Salviano) | F019, F020, F021, F022, F026 | Humaniza Salviano + reforça que Dante viu Vorto antes do funeral |
| DD-017 (Diário Bartolo) | F036, F074 | Atribui canonicamente autoria do audit Apex-Data + vínculo Tiago-Linda prévio |
| DD-018 (Folheto Lin Tórun) | F067, F068 | Voz autoral fundadora Era 2 + contraste explícito com DRE Sterling (DD-001) |
| DD-019 (Caderneta Vênea + Pyotor) | (substrato emocional sem F### direto) | Materializa safe base canônica + pai itinerante (lore-bible §14) |
| DD-020 (Relatório Solano) | F047 (retroativo) | Confirma vetor Sterling no surto -8 Pelicano Branco ("veio de cima") |
| DD-021 (Manual Octantes) | F038, F050 | Confirma operacionalmente Heliópolis-Nova + paralelo Dante (lavagem de memória) |
| DD-022 (Tábua São Vargas) | F041, F071 | Confirma saque -3 + micro-ato Bento aos 12 anos preservando memória |
| DD-023 (Fragmento Águas-de-Espelho) | F054, F064 | Doc Era 1 cotidiano técnico (não místico) + pista geográfica Catedral-Mãe |

Princípio: foreshadow em GusWorld já era distribuído e redundante (3-5 vetores por reveal). Suportes documentais adicionam **canal extra** sem inflacionar o conjunto de plants. Player Knowledge alta agora encontra confirmação documental para reveals que antes dependiam de inferência ambient.

Distribuição por Knowledge gate:

| Gate | Plants |
|---|---|
| n/a (visível Knowledge baixa) | ~55 |
| Knowledge média | ~37 |
| Knowledge alta | ~21 |
| Ouro (branching + Knowledge alta) | ~12 (incluindo F021, F025, F026, F027, F057, F063, F064, F077, F080, F084, F094, F099, F125) |

---

## Cross-refs

- **Lore base 3 eras:** [[lore-bible]] §3
- **Estrutura 3 atos + climax + endings:** [[arco-principal]]
- **Cronologia 50+ eventos:** [[timeline]]
- **6 facções + NPCs lore-importantes:** [[factions]]
- **23 documentos in-world (4 gate Ouro):** [[in-world-docs]] (Bloco G originais 1-15 + Bloco H viralizado 16-23)
- **Catálogo Diário do Gus (cross-link denso por entry):** [[diary/entries-docs-descobriveis]]
- **Companions narrativos (memórias formativas):** [[characters/dante-grid|dante-grid]], [[characters/caua-volt|caua-volt]], [[characters/iara-lumen|iara-lumen]], [[characters/bento-requiem|bento-requiem]], [[characters/linda-siren|linda-siren]], [[characters/jaci-proxy|jaci-proxy]]
- **Antagonistas:** [[characters/sterling-locke|sterling-locke]], [[characters/patch-zero|patch-zero]]
- **8 settings (props canônicos):** [[environments/_INDEX|environments]]
- **Tradições + comidas + cumprimentos:** [[tradicoes-cultura]]
- **14 cenas LucasArts + 18 EE:** [[comic-reliefs]]
- **Pillars 5 imutáveis:** [[pillars]]

---

**Última revisão:** 2026-05-16. Canônico (Bloco I + Bloco H viralizado). Atualizações de plants GRAVADO exigem aprovação do criador supremo; PROPOSTAS aguardam aprovação para promoção a GRAVADO. Em 2026-05-16: 8 propostas autorais aprovadas (DD-016 a DD-023) viralizadas como suportes documentais a plants existentes — sem novos F### gerados.
