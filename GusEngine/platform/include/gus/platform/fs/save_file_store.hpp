// gus/platform/fs/save_file_store.hpp
//
// I/O REAL em disco do SAVE do jogo (M2-SAVE-IO, ADR-012 Onda 2): ~/.gusworld/saves/,
// permissoes 0700 (diretorio) / 0600 (arquivo), mesma tecnica de degradacao segura do
// gus/platform/fs/settings_file_store.hpp (settings.json), aplicada aqui ao save de
// verdade. DIFERENCA DE FORMATO deliberada: settings.json e texto JSON legivel (config
// de conforto); o SAVE e BINARIO proprio (envelope GDS2 + HMAC, ADR-006) - o consumo e
// via gus::domain::save::load_save (nao-lancante, ADR-007 T1.1), NUNCA
// gus::domain::save::deserialize_save (lancante, reservado a testes legados/uso
// interno do proprio load_save).
//
// FsSaveStore implementa o port gus::domain::save::SaveStore (save_store.hpp) sobre
// arquivos reais dentro de um diretorio: cada NOME LOGICO ("autosave", "save_1",
// "save_1.backup2"...) vira um arquivo "<dir>/<nome>.sav". A politica de slots
// (save_slots.hpp) e de rotacao de backup (save_backup.hpp, N=3) SAO REUSADAS sem
// alteracao - este arquivo so fecha a fronteira de bytes<->disco que faltava.
//
// FRONTEIRA fail-fast vs degradacao segura (mandato backend-engineer):
//   - Slot invalido / SaveData com invariante violada = ERRO DE PROGRAMACAO do
//     chamador (contrato do dominio, ja fail-fast la). save_game/load_game DEIXAM
//     PASSAR essas excecoes (std::out_of_range / std::invalid_argument) - nao e
//     um caso de I/O, e um bug a corrigir no caller.
//   - Falha de I/O de verdade (disco cheio, permissao negada, race de FS) NUNCA
//     lanca: save_game devolve false; load_game devolve std::nullopt (tratado pelo
//     chamador como "sem save neste slot" - fluxo de novo jogo). Um arquivo
//     PRESENTE mas corrompido/adulterado NAO e "ausente": load_game devolve o
//     LoadOutcome do dominio (Corrupt/HmacInvalid/etc.) para a app decidir o que
//     avisar (a JANELA de aviso em si e decisao de UI/UX fora deste escopo - ver
//     TODO.md M2).
//
// Cross-ref: gus/domain/save/save_store.hpp (port), save_slots.hpp (nomes logicos),
//            save_backup.hpp (rotacao N=3), save_serializer.hpp (load_save/
//            LoadOutcome, T1.1/T1.2), gus/platform/fs/settings_file_store.hpp
//            (mesmo padrao de permissoes/degradacao, para settings.json), ADR-006,
//            ADR-007, ADR-012.

#ifndef GUS_PLATFORM_FS_SAVE_FILE_STORE_HPP
#define GUS_PLATFORM_FS_SAVE_FILE_STORE_HPP

#include <optional>
#include <string>
#include <vector>

#include "gus/domain/save/save_data.hpp"
#include "gus/domain/save/save_serializer.hpp"  // LoadOutcome
#include "gus/domain/save/save_store.hpp"       // SaveStore (port)

namespace gus::platform::fs {

// Implementacao REAL do port SaveStore sobre arquivos: nome logico "X" -> arquivo
// "<dir>/X.sav". NAO cria o diretorio sozinho (quem grava o 1o byte e save_game,
// via ensure_saves_dir interno) - a classe so traduz nome<->path e move bytes.
class FsSaveStore final : public gus::domain::save::SaveStore {
   public:
    explicit FsSaveStore(std::string dir);

    [[nodiscard]] bool exists(const std::string& name) const override;
    [[nodiscard]] std::vector<std::uint8_t> read(const std::string& name) const override;
    void write(const std::string& name, const std::vector<std::uint8_t>& bytes) override;
    void move(const std::string& from, const std::string& to) override;
    void remove(const std::string& name) override;

   private:
    [[nodiscard]] std::string path_for(const std::string& name) const;

    std::string dir_;
};

// Resolve o DIRETORIO de saves pra uso REAL do jogo: env GUSWORLD_HOME (override de
// teste/CI, mesmo espirito de resolve_settings_dir/GUSWORLD_ASSETS) > $HOME/.gusworld.
// So monta a STRING (dir dos SAVES = "<base>/saves"); nao cria nada em disco.
[[nodiscard]] std::string resolve_saves_dir();

// true se ha um save gravado no slot (arquivo PRIMARIO presente; nao conta backups).
// Consulta pura de existencia - nao valida integridade (isso e load_game). Slot
// invalido lanca std::out_of_range (fail-fast, contrato do dominio).
[[nodiscard]] bool has_save(int slot, const std::string& dir);

// Grava `data` no slot: serializa (gus::domain::save::serialize_save, V4 atual) e
// escreve via write_with_backup_rotation (rotaciona backup1..backup3 antes de
// sobrescrever o primario) - a MESMA cadeia ja provada em domain/tests/save_backup_test.
// Cria o diretorio sob demanda com permissao 0700; escreve cada arquivo com 0600
// (LGPD - dado do jogador so ele le/escreve).
//
// CONTRATO do chamador: `data.slot_id` deve estar setado igual a `slot` ANTES de
// chamar (SaveData::slot_id e o campo SELADO no payload que load_game compara contra
// o slot fisico no load - T1.2). Este arquivo nao sobrescreve slot_id: nao e dono da
// semantica do dominio, so grava o que recebe.
//
// Fail-fast (propaga, nao e I/O): slot invalido (std::out_of_range) ou SaveData
// com invariante violada (std::invalid_argument, ver SaveData::validate()).
// Degradacao segura (NUNCA lanca por causa de I/O): qualquer falha de disco
// (permissao, disco cheio, etc.) devolve false; o estado em MEMORIA da sessao
// continua valido, so a persistencia falhou (o chamador decide avisar/tentar de novo).
[[nodiscard]] bool save_game(const gus::domain::save::SaveData& data, int slot,
                              const std::string& dir);

// Carrega o slot. Devolve:
//   - std::nullopt: nenhum save gravado neste slot ainda (1a execucao / slot vazio,
//     fluxo de "novo jogo") OU uma falha de I/O inesperada ao ler um arquivo que
//     `exists()` reportou presente (degradacao segura: tratada igual a slot vazio,
//     nunca lanca por causa disso).
//   - presente: o gus::domain::save::LoadOutcome de gus::domain::save::load_save
//     (chamado com expected_slot = `slot`, T1.2) - Ok / HmacInvalid (adulterado) /
//     Corrupt (malformado) / VersionTooNew (forward-only) / Invalid (invariante
//     violada) / WrongSlot (arquivo trocado de slot). A app decide o que avisar ao
//     jogador para cada caso (fora do escopo desta funcao).
//
// Fail-fast (propaga, nao e I/O): slot invalido lanca std::out_of_range.
[[nodiscard]] std::optional<gus::domain::save::LoadOutcome> load_game(
    int slot, const std::string& dir);

// SAVE-LOAD-AVISOS (aviso #1, "Tentar recuperar"): tenta carregar o slot a
// partir da CADEIA DE BACKUP (backup_logical_name(slot, 1..kBackupChainDepth),
// save_backup.hpp), NAO do primario - uso: o primario ja falhou (load_game
// devolveu HmacInvalid/Corrupt/VersionTooNew/Invalid/WrongSlot) e o jogador
// pediu recuperacao explicita. Percorre as geracoes NA ORDEM (backup1 = mais
// recente, ate kBackupChainDepth = mais antiga) e devolve o LoadOutcome da
// PRIMEIRA que carregar Ok - a recuperacao para no primeiro sucesso, nao
// procura "a melhor" entre varias Ok (backup1 e sempre a mais fresca das
// boas).
//
// Devolve std::nullopt se NENHUMA geracao de backup carregar Ok (todas
// ausentes, OU presentes mas tambem corrompidas/versao-incompativel/etc.) - o
// CHAMADOR trata como "recuperacao falhou" (mensagem dedicada, ver
// save_load_menu.hpp), DIFERENTE do nullopt de load_game (que significa "slot
// vazio"). Degradacao segura identica a load_game: falha de I/O ao ler um
// backup PRESENTE conta como aquela geracao reprovando (segue pra proxima),
// nunca lanca por causa disso.
//
// Fail-fast (propaga, nao e I/O): slot invalido lanca std::out_of_range
// (mesmo contrato de load_game/save_game/delete_save).
[[nodiscard]] std::optional<gus::domain::save::LoadOutcome> load_game_from_backup(
    int slot, const std::string& dir);

// Apaga TODO o save do slot: o arquivo PRIMARIO + a cadeia INTEIRA de backup
// (backup1..backup{kBackupChainDepth}, save_backup.hpp) - o slot fica completamente
// vazio (nao sobra rastro pra um load acidental reviver via backup depois, feature
// "Apagar" aprovada pelo lider, SAVE-LOAD-UI etapa 6). Idempotente: nomes ja ausentes
// (slot ja vazio, ou so o primario apagado mas backups nao) sao no-op seguro em cada
// um (mesmo contrato de FsSaveStore::remove).
//
// Degradacao segura (NUNCA lanca por causa de I/O): devolve true se o PRIMARIO nao
// existe mais ao final (VERIFICADO via exists(), nao so "tentei remover") - false
// sinaliza que a remocao nao pegou (permissao negada etc., raro); o CHAMADOR decide
// o que avisar. Fail-fast (propaga, nao e I/O): slot invalido lanca std::out_of_range
// (mesmo contrato de has_save/save_game/load_game).
[[nodiscard]] bool delete_save(int slot, const std::string& dir);

// MODOS-MORTE Fase 0 (Camada 3 essencial, docs/design/mecanicas/modos-morte.md
// §2.3): sobrescreve o SELO do envelope GDS2 (MAGIC+LENGTH no INICIO + HMAC no
// FIM, ver save_serializer.hpp) do PRIMARIO + da cadeia INTEIRA de backup
// (backup1..backup{kBackupChainDepth}) - torna cada arquivo INCARREGAVEL (o
// load_save/load_game_from_backup JA rejeita qualquer payload sem selo valido,
// comportamento EXISTENTE hoje - nao precisa de codigo novo de rejeicao). Depois
// do overwrite: UNLINK dos 4 arquivos (nao sobra nem o rastro incarregavel em
// disco) + ZERA o buffer em RAM que leu o conteudo antigo (nao deixa o payload
// sensivel residente na memoria do processo).
//
// NAO e o crypto-shred do ADR-014 (AEAD/machine-binding/ancora out-of-band) - isso
// e fase FUTURA (Hardcore, so buildavel pos-fim-de-jogo, ver modos-morte.md §6
// Fase 4). Isto aqui e a versao byte-overwrite sobre o selo HMAC ATUAL (GDS2),
// disponivel JA (memoria project_morte_dificuldade_canon).
//
// Degradacao segura (NUNCA lanca por causa de I/O, MESMO espirito de
// save_game/load_game/delete_save): um arquivo AUSENTE em qualquer nome logico e
// no-op seguro (nada a apagar - idempotente). Devolve true se, ao final, NENHUM
// dos 4 arquivos (primario + N backups) existe mais em disco - false sinaliza que
// algum sobrou (permissao negada etc., raro); o CHAMADOR decide o que avisar.
// Fail-fast (propaga, nao e I/O): slot invalido lanca std::out_of_range (mesmo
// contrato de delete_save/has_save/save_game/load_game).
[[nodiscard]] bool secure_wipe_save(int slot, const std::string& dir);

}  // namespace gus::platform::fs

#endif  // GUS_PLATFORM_FS_SAVE_FILE_STORE_HPP
