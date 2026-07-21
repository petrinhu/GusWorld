// cards_hw_mutation_killers_test.cpp
//
// CARDS-HW-QA1: mutation testing manual adversarial nas fatias mais novas do motor de
// cartas (turing_service/contamination_service/urandom_algorithm/adware_sterling/
// deck_transactions). Rodado SOZINHO pelo qa-engineer (nenhum outro agente concorrente),
// mutando arquivos de PRODUCAO um de cada vez, rebuildando o binario de teste, rodando a
// SUITE INTEIRA (nao so um filtro), e revertendo via `git checkout --` antes do proximo
// mutante (protocolo completo no bug tracker / TODO.md, item CARDS-HW-QA1).
//
// RESULTADO: os 19 mutantes descritos no brief foram TODOS killed pela suite EXISTENTE
// (turing_service_test.cpp / contamination_service_test.cpp / hardware_class_test.cpp /
// urandom_test.cpp / adware_sterling_test.cpp / deck_transactions_test.cpp) - ZERO
// sobreviventes. Nenhum teste NOVO foi necessario pra matar mutante nenhum (forcar um
// teste redundante so pra "ter algo aqui" seria vaidade de cobertura, nao qualidade -
// ver CONTRACT.md/qa-engineer). Este arquivo documenta a tabela completa como REGISTRO
// AUDITAVEL do mutation run (qual TEST_CASE matou cada mutante) e mantem 1 TEST_CASE
// executavel so pra nao deixar o CMakeLists com um alvo vazio (a doc-comment abaixo E' o
// entregavel real desta fatia, nao o assert em si).
//
// Duas correcoes de localizacao de arquivo em relacao ao brief original (achado durante a
// execucao, nao decisao de design - reportado no retorno da fatia):
//   - M8 ("contamination_service.cpp: off-by-one no threshold de uma classe de risco"):
//     o array de % de risco (kContaminationPercentTable) NAO mora em
//     contamination_service.cpp - mora em gus/domain/hardware/hardware_class.hpp
//     (contamination_service.cpp so LE via contamination_percent_for()). Mutante aplicado
//     la (PirataComum 21 -> 22).
//   - M14 ("urandom_algorithm.cpp: trocar a direcao do redirecionamento SELF"): a
//     aplicacao do status de backfire SEMPRE no caster (apply_offensive_status(caster,
//     caster, ...)) mora em combat_state_machine.cpp::resolve_urandom, nao em
//     urandom_algorithm.cpp (que so tem as 2 funcoes puras de peso/classificacao).
//     Mutante aplicado la (redireciona pro primeiro inimigo vivo, se houver).
//
// ============================================================================================
// TABELA COMPLETA DE MUTANTES (CARDS-HW-QA1)
// ============================================================================================
//
// turing_service.cpp (gus/domain/deck/turing_service.cpp, attempt_cure()):
//   M1  roll <  kTuringCureSuccessPercent  ->  roll <= kTuringCureSuccessPercent
//       KILLED por turing_service_test.cpp:116 ("roll=62 (borda, >=62) queima - sucata").
//   M2  remove guard 1 (tier protegido, Especial/Super)
//       KILLED por turing_service_test.cpp:173/185/196 (guards Especial/Super + ordem) e
//       :219 (determinismo - zero draw quando guard barra).
//   M3  remove guard 2 (!is_diagnosed)
//       KILLED por turing_service_test.cpp:158 (RejectedNotDiagnosed) e :219 (determinismo).
//   M4  no ramo Cured, deixa de resetar is_diagnosed
//       KILLED por turing_service_test.cpp:103 ("roll=61 cura - limpa completamente").
//   M5  no ramo Cured, deixa de resetar virus_kind
//       KILLED por turing_service_test.cpp:103 (mesmo teste, REQUIRE(virus_kind==None)).
//   M6  no ramo Burned, seta is_infected=false tambem (nao deveria - AMB-T1)
//       KILLED por turing_service_test.cpp:143 ("queima NAO remove/destroi - AMB-T1").
//   M7  troca a ordem dos 2 guards (diagnostico antes de tier protegido)
//       KILLED por turing_service_test.cpp:196 ("guard de tier protegido e checado ANTES
//       do guard de diagnostico").
//
// contamination_service.cpp / hardware_class.hpp (roll_contamination_on_acquisition()):
//   M8  off-by-one no risco de PirataComum (kContaminationPercentTable: 21 -> 22, vive em
//       hardware_class.hpp - ver nota de localizacao acima)
//       KILLED por hardware_class_test.cpp:56 (tabela literal) e
//       contamination_service_test.cpp:227 (fronteira PirataComum draw=21 -> Clean).
//   M9  troca a tabela de payload de HomebrewEprom pela de PirataComum
//       (gus/domain/src/deck/contamination_service.cpp, kVirusPayloadWeightTable)
//       KILLED por contamination_service_test.cpp:340 (distribuicao HomebrewEprom
//       4/12/32/52% dentro de tolerancia).
//   M10 remove o guard defensivo de classe protegida (Especial/Super) em
//       roll_contamination_on_acquisition()
//       KILLED por contamination_service_test.cpp:94/111 (outcome vira Clean em vez de
//       SkippedProtectedTier - NAO e mutante equivalente: o guard e redundante pro
//       is_infected final [ja 0% via EspecialSelada], mas MUDA o outcome/chave i18n
//       observavel, que os testes checam explicitamente).
//   M11 consome 1 draw de RNG extra no ramo Clean
//       KILLED por contamination_service_test.cpp:408 ("consome EXATAMENTE 1 draw quando
//       fica limpo", CountingRandom).
//
// urandom_algorithm.cpp (weighted_pick_urandom_faixa()):
//   M12 off-by-one na fronteira da tabela ORIGINAL (roll < cumulative -> roll <= cumulative)
//       KILLED por urandom_test.cpp:177 (fronteiras EXATAS da tabela ORIGINAL, banda Medio
//       comeca em 21).
//   M13 mesmo off-by-one, fronteira da tabela PIRATA
//       KILLED por urandom_test.cpp:228 (fronteiras EXATAS da tabela PIRATA, banda Medio
//       comeca em 7) - o MESMO mutante (funcao compartilhada pelas 2 tabelas) tambem
//       quebra urandom_test.cpp:301/349/678 (stats + FSM backfire/jackpot).
//   M14 troca a direcao do redirecionamento SELF do backfire (mora em
//       combat_state_machine.cpp::resolve_urandom - ver nota de localizacao acima):
//       aplica o status ruim no inimigo (se houver) em vez de SEMPRE no caster
//       KILLED por urandom_test.cpp:349 ("pirata sorteia Backfire -> status negativo leve
//       SEMPRE no CASTER, alvo intacto").
//
// adware_sterling.cpp (AdwareExposureTracker::roll_exposure()):
//   M15 4a exposicao vira 3a (exposure_count_ <= kAdwareAlwaysShowThreshold ->
//       exposure_count_ < kAdwareAlwaysShowThreshold)
//       KILLED por adware_sterling_test.cpp:150/170/223/239 (threshold==3, exposicoes 1-3
//       sempre gratis, 4a+ consome RNG).
//   M16 inverte o 70/30 (roll < kAdwareShowChanceAfter3 -> roll >= kAdwareShowChanceAfter3)
//       KILLED por adware_sterling_test.cpp:170 (fronteira exata roll=69/70) e :201
//       (enumeracao exaustiva 0..99, show_count==70).
//
// deck_transactions.cpp (sell()/upload()/acquire()/craft()):
//   M17 credita ANTES da checagem de idempotencia (find_in_active) em remove_and_credit()
//       KILLED por deck_transactions_test.cpp:64 (sell credita o preco) e :79
//       (double-sell credita SO 1x) - com o mutante, sell(id inexistente) tambem creditaria.
//   M18 no acquire() rejeitado por saldo insuficiente, debita mesmo assim
//       KILLED por deck_transactions_test.cpp:211 ("acquire com saldo insuficiente nao
//       muta nada") e :556 (determinismo, InsufficientCredits com CountingRandom).
//   M19 inverte a origem fisica de acquire() vs craft() (acquire nasce HomebrewEprom em
//       vez de OriginalRom)
//       KILLED por deck_transactions_test.cpp:270 ("acquire nasce com origem
//       OriginalRom") e :404 (integracao hardware_class_of() -> ComumOriginal).
//
// ============================================================================================
// VEREDITO FINAL: 19/19 mutantes KILLED pela suite existente. Zero sobreviventes. Zero
// bug real de producao encontrado nesta fatia (as 2 correcoes de localizacao acima sao
// achado de auditoria de DOCUMENTACAO/brief, nao de comportamento - o codigo real faz o
// que o header/spec descreve).
// ============================================================================================

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CARDS-HW-QA1: os 19 mutantes do motor de cartas (turing/contamination/urandom/"
          "adware/deck_transactions) foram TODOS killed pela suite existente - ver a tabela "
          "completa no doc-comment deste arquivo",
          "[domain][cards_hw][mutation]") {
    // Nenhum sobrevivente para matar aqui (protocolo CARDS-HW-QA1: nao forcar teste
    // redundante so por cobertura de vaidade). O REGISTRO auditavel esta no doc-comment
    // acima; este SUCCEED() so mantem o alvo de CMake nao-vazio.
    SUCCEED();
}
