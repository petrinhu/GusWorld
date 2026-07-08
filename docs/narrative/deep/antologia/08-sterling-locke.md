# Defesa Pública de Tese, Faculdade de Engenharia Recursiva, Salão 3

> **Nota editorial:** Transcrição preserva voz original Sterling Locke (defesa de tese de doutorado em -25, futura fundação Sterling Corp em -12). Sem comentário interpretativo. Leitor consulte Vol 1 Bíblia §3 Era 3 + ficha CHARS para contexto adversarial canônico.

*por Sterling Locke, doutorando candidato ao grau, cadeira de Paradigmas de Avaliação Runtime, transcrição autorizada pelo próprio*

---

## 1. Salão 3

O Salão 3 da Faculdade de Engenharia Recursiva tem trinta e quatro lugares para audiência e cinco lugares na bancada da banca. Cinco. Apenas três estão ocupados nesta tarde. A senhora Heris Marçal Vüldenberg, a senhora Octavia Pelicano Branco, e o senhor Cláudio Vance Atelaiá. O quarto e o quinto assentos permanecem vazios por ausência justificada de dois examinadores externos cujos nomes não interessam ao registro porque não votarão. Registro a observação por completude. A ausência reduz quórum a três. Três é suficiente. Três sempre foi suficiente em qualquer estrutura institucional desta faculdade desde a fundação dela em menos oitocentos e vinte, ano da Anomalia Primeira, e antes dela em qualquer outra estrutura institucional que tenha precedido esta. Três decide. Três rejeita. Três é número suficiente para encerrar uma hipótese.

Ajusto o monóculo. O vidro é vermelho cromado, ligeiramente espelhado pelo lado externo, transparente pelo interno. Foi calibrado para corrigir astigmatismo discreto no olho direito e simultaneamente filtrar comprimentos de onda azul abaixo de quatrocentos e oitenta nanômetros, o que reduz fadiga de leitura em ambientes de iluminação fluorescente como este. O Salão 3 tem oito lâmpadas tubulares no teto. Cinco acesas. Três queimadas. A administração da faculdade não substitui lâmpadas em ciclos regulares. Registro a ineficiência sem comentário oral.

A bancada está coberta por feltro verde. Sobre o feltro, três cópias impressas da minha tese, encadernação preta, lombada de cinco centímetros. Oitocentas e noventa páginas. O título consta em letras prateadas: *Dynamic Runtime Evaluation, Paradigma para Reavaliação Permanente de Contrato em Sistemas de Conjuração Abertos*. Abaixo do título, em letra menor, meu nome.

Heris Marçal Vüldenberg abre a sessão. A voz dela é seca. Diz, para efeito de ata, a hora, a data, o nome do candidato, o título da tese, e a composição da banca. Pede que eu apresente em até cinquenta e cinco minutos a defesa oral. Concordo com a cabeça. Subo ao palco baixo. Acendo o projetor de slides. O primeiro slide carrega.

Começo.

---

## 2. Apresentação

"Banca examinadora," digo, "a tese que apresento propõe deslocar o paradigma corrente de compilação estática de contratos de conjuro para um paradigma de avaliação dinâmica em tempo de execução. Doravante DRE. Dynamic Runtime Evaluation."

Pauso por dois segundos. Não para efeito retórico. Para sincronizar com a transição do segundo slide.

"O paradigma vigente, herdado da escola de Atelaiá Chevalier e consolidado por sucessivas gerações de praticantes desta faculdade, assume que um contrato de conjuro deve ser verificado em sua totalidade antes da execução. Verificado contra invariantes morais, invariantes ontológicas, invariantes de custo, e invariantes de consentimento entre as partes vinculadas. Esta verificação é onerosa. Estimo, com base em dados coletados ao longo dos últimos treze anos em treze laboratórios institucionais, que oitenta e nove por cento do tempo computacional gasto em sistemas de conjuração pertence a esta verificação prévia. Apenas onze por cento gera trabalho útil."

Avanço o slide. Aparece um gráfico de barras. As barras vermelhas dominam. As barras azuis são finas.

"DRE propõe inverter a proporção. Em vez de verificar contratos antes da execução, executamos diretamente, e reavaliamos contratos durante a execução, a cada ponto de decisão, contra contexto atualizado. Isto significa que invariantes deixam de ser estáticas. Tornam-se funções de estado. O que é ético em um ponto da execução pode deixar de ser ético no ponto seguinte. O que é consensual em um instante pode ser revogado no instante posterior. O que custa cinco unidades agora pode custar treze unidades depois. O sistema aceita. O sistema executa. O sistema reavalia. O sistema prossegue."

Cláudio Vance Atelaiá apoia o queixo na mão. O cenho dele se contrai discretamente. Registro.

"A vantagem do paradigma é dupla. Primeiro, performance. Segundo, e mais relevante, adaptabilidade. Um sistema DRE responde a ambientes voláteis com latência tendendo a zero. Não há recompilação. Não há recontratação formal. O contrato é o próprio estado de execução, continuamente reescrito por si mesmo."

Avanço o slide. Aparece um diagrama. Três círculos concêntricos. No centro, a palavra *runtime*. Em torno, *contexto*, *agente*, *capital*.

"O horizonte de aplicação que proponho excede o escopo desta tese. Mencionei brevemente, no capítulo final, oitavo, a possibilidade de um Global Runtime Environment. Doravante GRE. Um ambiente único de execução estendido a toda infraestrutura de conjuração da megacidade e potencialmente a toda infraestrutura de conjuração do continente. Sob GRE, qualquer agente conectado opera sob o mesmo runtime reavaliador, com o mesmo conjunto de contratos vivos, sob a mesma autoridade compartilhada de reavaliação. A eficiência sistêmica resultante é difícil de superestimar. A presente tese, no entanto, restringe-se à formalização do paradigma DRE em escala laboratorial, deixando GRE como direção de pesquisa subsequente."

Olho para a banca. Os três rostos estão imóveis. Heris Marçal Vüldenberg toma notas. Octavia Pelicano Branco lê uma página marcada. Cláudio Vance Atelaiá não escreve.

Prossigo.

"Antecipo a objeção principal. A objeção principal será de ordem ética. Será formulada da seguinte maneira: um contrato que se reavalia continuamente é um contrato que pode ser rompido continuamente. Um sistema que executa antes de verificar é um sistema que executa atos cujo consentimento ainda não foi formalmente estabelecido. Respondo a esta objeção antecipadamente. A objeção pressupõe que a verificação prévia oferece garantia de consentimento. Não oferece. Oferece, no máximo, ilusão de garantia. O paradigma vigente apenas adia o problema da revogabilidade do consentimento, deslocando-o para fora do sistema, para o tribunal ético institucional, que é, por sua vez, sistema mais lento e menos confiável que o próprio runtime que se quer regular. DRE não nega ética. DRE absorve ética dentro do runtime. A ética torna-se função de estado executável."

Pauso. Bebo um gole de água do copo sobre o púlpito. Cinco mililitros. O copo é de plástico transparente.

"Cito, para reforço, autoridades fora da escola Atelaiá Chevalier. Cito o tratado anônimo intitulado *Da Recursão das Convivências*, datado de aproximadamente menos quarenta e cinco, cuja autoria a tradição folclórica atribui à figura legendária de Verônica Atelaiá. O tratado, embora anônimo, defende em seu capítulo terceiro a noção de que o contrato vivo é superior ao contrato escrito, porque o contrato vivo acompanha o vivente. Concordo com a premissa do tratado. Discordo da conclusão. A autora anônima, ou autor anônimo, conclui que o contrato vivo deve ser custodiado por comunidade reunida em assembleia recíproca. Eu concluo que o contrato vivo deve ser custodiado por runtime central reavaliador. A primeira conclusão é folclore. A segunda é engenharia."

Discretamente, internamente, sem registro de fala, observo a citação. O tratado de menos quarenta e cinco é peça interessante. Li-o aos quatorze anos, em edição rara conservada na biblioteca particular de meu tio. A autora anônima, se de fato Verônica Atelaiá, antecipa, em linguagem mística, princípios formais que só seriam reformulados em linguagem técnica oito séculos depois. Há, no tratado, uma sentença que retorna ao meu pensamento com frequência: *o pacto que respira é o único pacto verdadeiro*. Não cito a sentença aqui. Cito apenas a estrutura argumentativa, e descarto a teleologia. A teleologia da autora é incompatível com a minha. A estrutura, no entanto, é elegante. Registro a observação internamente. Prossigo externamente.

"Cito, em segunda autoridade, o prefácio do *Tomo da Pilha Sobrecarregada*, atribuído a Atelaiá Chevalier, edição de menos cento e quarenta e quatro. O prefácio sustenta, e cito literalmente, que toda compilação prévia é forma de fé. Discordo do prefácio. O prefácio é prescrição disfarçada de descrição. Atelaiá Chevalier descreve uma realidade prática de seu tempo, em que recursos computacionais não permitiam reavaliação dinâmica, e converte limitação técnica em virtude ética. A operação retórica é antiga. Reconheço-a. Recuso-a. O paradigma DRE não tem fé. DRE não precisa de fé. DRE tem runtime."

Octavia Pelicano Branco vira uma página. O som do papel é discreto, mas registro.

Em segundo plano, no salão, há ruído ambiente baixo. O sistema de ventilação da faculdade opera em pulsação regular detectável por orelha treinada. Conto a pulsação. Cento e quarenta e quatro ciclos por minuto. Compasso quatro por quatro. Modo lócrio descendente sustentado por ressonância da tubulação de cobre na parede oeste. Registro o dado sem comentário oral. Música ambiente, mesmo quando involuntária e mecânica, instala-se no córtex auditivo do candidato e modula performance verbal. Cento e quarenta e quatro batimentos por minuto, em lócrio, em quatro por quatro quadrado, é, segundo dados que coletei em sessões anteriores, a assinatura rítmica em que minha fala oral atinge precisão máxima. O acaso de o sistema de ventilação operar exatamente nesta assinatura é, portanto, conveniente. Registro a conveniência. Prossigo.

Prossigo por mais trinta e cinco minutos. Apresento as três provas formais do capítulo quinto. Apresento o protótipo experimental do capítulo sexto, executado em laboratório com cinco agentes voluntários sob contratos DRE simplificados. Apresento os dados de performance. Apresento as comparações com sistemas estáticos equivalentes. Apresento, no oitavo e último capítulo, o esboço algébrico de GRE.

Encerro com a frase planejada: "DRE não é proposta de reforma. DRE é proposta de substituição. Submeto a tese ao escrutínio desta banca."

Desço do palco. Sento-me na cadeira reservada ao candidato, à direita da bancada.

Aguardo.

---

## 3. Arguição

Heris Marçal Vüldenberg abre a arguição. A voz dela permanece seca.

"Doutorando Locke, o paradigma que propõe pressupõe autoridade central de reavaliação. Quem reavalia o reavaliador?"

Respondo. "A pergunta repete, em forma nova, paradoxo antigo. Respondo com a mesma resposta antiga. O reavaliador é reavaliado por si próprio, em recursão. A recursão é estável se, e somente se, o reavaliador não tem incentivo interno a corromper o próprio runtime. Demonstro a condição no apêndice C."

Heris Marçal Vüldenberg ergue uma sobrancelha. "Apêndice C presume agente sem incentivo interno. Que agente humano não tem incentivo interno?"

Respondo. "Nenhum. Por isso o reavaliador não deve ser humano individual. Deve ser sistema runtime auditado por colegiado rotativo. O colegiado, por sua vez, é sujeito a runtime de nível superior."

"Que recursa ao infinito."

"Recursa até quórum de três, conforme princípio institucional vigente nesta faculdade desde menos oitocentos e vinte. Três decide. Três rejeita. Três é número suficiente para encerrar uma hipótese."

Heris Marçal Vüldenberg não responde de imediato. Registra. Passa a palavra.

Octavia Pelicano Branco arguiu por vinte e um minutos sobre objeções de segurança. Apontou três cenários em que DRE permite execução de contrato cuja revogação posterior produz dano irreversível ao agente revogador. Respondi aos três cenários. Os três cenários são tratáveis por instrumentação adicional ao runtime. A senhora Pelicano Branco discordou. Tomei nota do desacordo sem hostilidade. Hostilidade é desperdício de recurso afetivo, e recurso afetivo é recurso como qualquer outro, e recurso desperdiçado reduz performance global do agente.

Cláudio Vance Atelaiá arguiu por treze minutos. A voz dele é grave, mais grave que a média dos arguidores que enfrentei em apresentações anteriores. Ele iniciou pela citação que fiz do tratado anônimo de menos quarenta e cinco. Perguntou se eu compreendia que a citação que fiz, ao despojar o tratado de sua teleologia, profanava o tratado.

Respondi. "A palavra profanar pressupõe sacralidade prévia. Não atribuo sacralidade ao tratado. Atribuo elegância argumentativa. Cito por elegância. Recuso teleologia. Não há profanação porque não há sagrado."

Cláudio Vance Atelaiá sorriu sem alegria. Disse, lentamente, que era exatamente o tipo de resposta que esperava de mim. Não respondi à observação dele porque a observação dele não continha pergunta.

Ele prosseguiu. "Doutorando Locke. O senhor citou, no capítulo final, em nota de rodapé, a tradição cerimonial dos artesãos de Boróstoma. O senhor descreveu, e cito o senhor literalmente, o ritual de aferição mútua com instrumentos de medida pareados como, e cito, *ineficiência cerimonial corporativa*. O senhor mantém essa caracterização?"

Respondi. "Mantenho. A tradição cerimonial dos artesãos de Boróstoma consome cinco horas semanais de cada artesão participante para produzir aferição que poderia ser obtida em três minutos por instrumentação calibrada. A diferença entre cinco horas e três minutos é ineficiência. A função cerimonial da prática é função afetiva, e função afetiva, conforme já estabeleci na resposta à senhora Pelicano Branco, é recurso. Recurso desperdiçado em cerimônia é recurso subtraído de produção. Mantenho a caracterização."

Cláudio Vance Atelaiá apoiou a testa nos dedos. Não respondeu.

A arguição encerrou.

---

## 4. Deliberação e voto

A banca pediu vinte e um minutos para deliberar. Saí do salão. Aguardei no corredor adjacente. O corredor tem piso de mármore tesselado em padrão preto e branco alternado, herança arquitetônica antiga, decorativa, ineficiente. As paredes do corredor exibem retratos de antigos professores da faculdade. Conto os retratos. Vinte e um. O número repete o tempo de deliberação. Coincidência aritmética sem significado.

Durante a espera, formulo internamente o cenário possível. Há quatro cenários ponderados. Cenário um, aprovação irrestrita, probabilidade estimada cinco por cento. Cenário dois, aprovação condicionada à supressão do capítulo final, probabilidade vinte e um por cento. Cenário três, reprovação por motivo técnico isolado, probabilidade treze por cento. Cenário quatro, rejeição unânime por motivo institucional combinado, probabilidade cinquenta e cinco por cento. As probabilidades somam noventa e quatro. Os seis pontos percentuais restantes pertencem a desvios não modelados, incluindo a possibilidade de que algum membro da banca surpreenda com voto idiossincrático em direção que não consigo antecipar. Modelos preditivos sempre exigem margem de erro. O candidato responsável aceita a margem.

Durante a espera, também recordo, sem motivo aparente, uma frase do tratado anônimo de menos quarenta e cinco. A frase é a seguinte: *quem mede sem ser medido administra; quem é medido sem medir é administrado; quem mede e é medido convive*. A autora anônima, ou autor anônimo, dispõe os três regimes em ordem ascendente de virtude. Eu disponho os três regimes em ordem ascendente de eficiência decrescente. O primeiro regime, medir sem ser medido, é o mais eficiente. O terceiro regime, medir e ser medido, é o menos eficiente, porque consome recurso afetivo recíproco em volume não compensado por ganho de produção. Registro a divergência axiológica sem comentário. A divergência axiológica é o motivo pelo qual o tratado anônimo permanece, na catalogação oficial desta faculdade, classificado como anomalia folclórica e não como contribuição técnica.

Retornei ao salão.

Heris Marçal Vüldenberg leu o veredito.

"A banca, em votação por unanimidade, três votos contra zero, abstenções zero, rejeita a tese intitulada *Dynamic Runtime Evaluation, Paradigma para Reavaliação Permanente de Contrato em Sistemas de Conjuração Abertos*, apresentada pelo doutorando Sterling Locke, por três motivos formais. Primeiro, paradigma adversarial às fundações éticas institucionais desta faculdade. Segundo, runtime sem instrumentação adequada de segurança em tempo de compilação. Terceiro, explicitação no capítulo final de objetivo corporativo, GRE, incompatível com missão acadêmica desta cadeira. A banca recomenda ao candidato revisão profunda do paradigma, supressão integral do capítulo final, e reapresentação após período mínimo de cinco anos. A banca encerra a sessão."

Registrei o veredito. Não sorri. Não me ergui em protesto. Não solicitei direito de réplica, embora o regimento o permita. A réplica não alteraria três votos contra zero. Réplica em condição de derrota unânime é desperdício de recurso afetivo, e recurso afetivo é recurso como qualquer outro.

Levantei-me. Inclinei a cabeça três centímetros em direção à bancada, na medida exata de cortesia institucional exigida e nenhuma medida além. Recolhi as três cópias impressas da tese. Saí do Salão 3.

Internamente, num plano que a banca não acessa e jamais acessará, registrei o plano alternativo já formulado havia treze meses. Em menos doze, isto é, dentro de treze anos a contar deste momento, fundarei a Sterling Corp. Capital inicial obtido por meio de três contratos de consultoria corporativa já pré-acertados com casas mercantis menores que não exigem credencial acadêmica. A Sterling Corp implementará DRE como infraestrutura proprietária, sem submissão a banca institucional. A Sterling Corp expandirá em direção a GRE em horizonte de oito a vinte e um anos subsequentes, em função de variáveis de adoção. O veredito de hoje não atrasa o plano. O veredito de hoje confirma uma hipótese subjacente que eu vinha testando: a faculdade rejeitará DRE por motivo institucional, não por motivo técnico. Faculdade rejeitada como árbitro. Mercado promovido a árbitro. Mercado decide. Mercado adota.

Mercado é runtime.

---

## 5. Corredor de saída

No corredor de saída cruzei com Yusuf Mendel, colega de cadeira, ano abaixo do meu. Yusuf parou diante de mim. Tinha o sobretudo cinza abotoado até o pescoço.

"Sterling," ele disse. "Eu avisei. Capítulo oito. Capítulo oito era veneno. Bastava cortar o capítulo oito."

Olhei para Yusuf. O monóculo refletiu o rosto dele em vermelho cromado.

"Yusuf," respondi. "Você me avisou. Registro o aviso. Não agradeço pelo aviso. Você confunde aviso com favor. Aviso é informação. Informação é mercadoria. Você forneceu mercadoria sem cobrar preço. Aprenda a cobrar. Boa tarde."

Passei por ele.

Yusuf não respondeu. Não esperei resposta.

---

## 6. Rua de saída

Saí da faculdade pela porta principal. A porta tem cinco degraus de granito até a calçada. Desci os cinco degraus. Os músculos da panturrilha registraram cinco contrações sucessivas. A calçada estava parcialmente molhada por garoa anterior.

Caminhei pela Rua dos Acaceiros em direção ao bonde elétrico da linha treze.

À esquerda, a meio quarteirão, ergue-se a catedral menor de Atelaiá. A catedral é um edifício de três naves, fachada de pedra parda, rosácea central com vitral de oito pétalas, pavimento exterior tesselado em padrão alternado preto e branco no átrio frontal, dois acaceiros de tronco vermelho plantados em vasos de pedra ladeando o portal. Em menos sete, isto é, dentro de dezoito anos a contar deste momento, registros canônicos indicam que esta catedral menor estará ainda intacta. Em horizonte subsequente, indeterminado, não há projeção. A catedral, no presente, está intacta.

Pareio na frente da catedral por exatos três segundos. Não entro. Não me persigno. Não inclino a cabeça. Olho a fachada e registro a fachada como obstáculo arquitetônico em rota de pedestres da Rua dos Acaceiros. A rosácea de oito pétalas reflete luz baixa do entardecer. Os dois acaceiros estão em estação de troca foliar. As folhas caídas formam tapete vermelho denso sobre o pavimento tesselado. O tapete vermelho cobre parcialmente o padrão preto e branco. A combinação visual seria, em outra mente, comovente. Na minha, é dado.

Atravesso a rua sem olhar a catedral pela segunda vez.

O bonde da linha treze chega em três minutos. Tomo o bonde. Sento no banco do fundo, à direita, junto à janela. O bonde parte. Pela janela, o vento agita os acaceiros. As folhas vermelhas giram em padrão que coincide aproximadamente com espiral logarítmica. Coincidência geométrica sem significado.

O bonde se afasta da catedral. Da faculdade. Da banca. Da tese rejeitada.

Em menos doze, fundo a Sterling Corp.

Em menos cinco, GRE estará em fase de pré-deployment.

Em menos três, mercado terá decidido.

O bonde dobra a esquina. A catedral desaparece do quadro da janela. Registro o desaparecimento sem comentário interno.

O monóculo vermelho cromado reflete, por um instante, o próprio vidro da janela do bonde, em redundância óptica circular. Ajusto o monóculo um milímetro para cima. A redundância cessa.

Prossigo.

---

**Última revisão:** 2026-05-20. Canônico deep-lore R9 F1-DL.9 Antologia Vol 2 conto 8/14. Não modificar sem aprovação criador supremo.
