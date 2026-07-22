// gus/domain/save/save_store.hpp
//
// PORT (interface injetavel) de armazenamento de saves, sobre NOMES LOGICOS de
// arquivo. A camada de I/O real (platform/, futura) implementa este port sobre
// arquivos de verdade; a politica de backup (save_backup.hpp) so depende DESTE
// port, ZERO disco, ZERO Qt. Permite testar slots/backup com um fake em memoria.
//
// Contrato: o store mapeia NOME LOGICO ("autosave", "save_1", "save_1.backup1"...)
// -> bytes. Sem semantica de versao/HMAC (isso e do serializer); o store so move
// bytes opacos.
//
// Cross-ref: game/scripts/foundation/save_system/SaveManager.cs (FileAccess era o
//            "store" concreto no C#), ADR-006.
// (o C# original nao existe mais: repo do submodulo engine/ apagado no M8;
//  referencia historica)

#ifndef GUS_DOMAIN_SAVE_SAVE_STORE_HPP
#define GUS_DOMAIN_SAVE_SAVE_STORE_HPP

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace gus::domain::save {

// Port de armazenamento por nome logico. Implementado pelo platform/ (FS real) e
// pelo InMemorySaveStore (testes).
class SaveStore {
   public:
    virtual ~SaveStore() = default;

    // true se ha bytes gravados sob o nome logico.
    [[nodiscard]] virtual bool exists(const std::string& name) const = 0;

    // Le os bytes do nome logico. Lanca std::out_of_range se ausente (use exists()).
    [[nodiscard]] virtual std::vector<std::uint8_t> read(
        const std::string& name) const = 0;

    // Grava (sobrescreve) os bytes no nome logico.
    virtual void write(const std::string& name,
                       const std::vector<std::uint8_t>& bytes) = 0;

    // Move os bytes de from para to (sobrescreve to; from deixa de existir).
    // No-op se from nao existir. Espelha o rename do FileAccess do C#.
    virtual void move(const std::string& from, const std::string& to) = 0;

    // Remove o nome logico se existir (no-op se ausente).
    virtual void remove(const std::string& name) = 0;
};

// Fake em memoria do SaveStore para testes (e referencia de comportamento esperado
// do port). ZERO disco.
class InMemorySaveStore final : public SaveStore {
   public:
    [[nodiscard]] bool exists(const std::string& name) const override {
        return data_.find(name) != data_.end();
    }

    [[nodiscard]] std::vector<std::uint8_t> read(
        const std::string& name) const override {
        const auto it = data_.find(name);
        if (it == data_.end())
            throw std::out_of_range("InMemorySaveStore: nome ausente: " + name);
        return it->second;
    }

    void write(const std::string& name,
               const std::vector<std::uint8_t>& bytes) override {
        data_[name] = bytes;
    }

    void move(const std::string& from, const std::string& to) override {
        const auto it = data_.find(from);
        if (it == data_.end()) return;  // no-op se origem ausente
        data_[to] = it->second;
        data_.erase(it);
    }

    void remove(const std::string& name) override { data_.erase(name); }

   private:
    std::map<std::string, std::vector<std::uint8_t>> data_;
};

}  // namespace gus::domain::save

#endif  // GUS_DOMAIN_SAVE_SAVE_STORE_HPP
