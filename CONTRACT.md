# AI Coder Contract  -  Best Practices, Architecture & Standards


---

> **Audience:** AI coding agents (Claude, GPT, Gemini, Copilot, etc.)
> **Purpose:** Mandatory reference before writing, modifying, or reviewing any code.
> **Authority:** Rules use RFC 2119 keywords  -  MUST, MUST NOT, SHOULD, SHOULD NOT, MAY.
> **Testing & Audit:** All testing, quality and audit procedures are defined in [TESTES.md](TESTES.md).

---

## Table of Contents

1. [How to Use This Document](#1-how-to-use-this-document)
2. [OOP Fundamentals](#2-oop-fundamentals)
3. [SOLID Principles](#3-solid-principles)
4. [Design Patterns  -  Complete Reference](#4-design-patterns--complete-reference)
5. [Clean Code Rules](#5-clean-code-rules)
6. [Security](#6-security)
7. [Performance](#7-performance)
8. [Git Process for AI Coders](#8-git-process-for-ai-coders)
9. [Testing & Audit Mandate](#9-testing--audit-mandate)
10. [Language-Specific Rules](#10-language-specific-rules)
11. [Universal Engineering Principles](#11-universal-engineering-principles)
12. [Logging & Observability](#12-logging--observability)

---

## 1. How to Use This Document

**Before writing any code, the AI coder MUST:**

1. Read this document fully for the first task in a project.
2. Identify the target language and framework  -  apply sections 11 and 12 accordingly.
3. Identify the architecture layer being modified  -  layers now follow GODS_LAWS.md, L-17.
4. After completing any task: run the checklist in section 10.

**Decision flow:**

```
New task received
      │
      ▼
Read existing code before modifying ──► Understand context fully
      │
      ▼
Identify architecture layer (GODS_LAWS.md, L-17)
      │
      ▼
Apply SOLID + Design Pattern rules
      │
      ▼
Write code → Build → No errors/warnings?
      │
      ▼
Run applicable tests from TESTES.md
      │
      ▼
Commit with Conventional Commits format
      │
      ▼
Done
```

**What the AI coder MUST NOT do:**
- Write code without reading existing files first.
- Add features beyond what was requested.
- Introduce hardcoded secrets, credentials, or API keys.
- Skip the build step before committing.
- Use deprecated APIs, unsafe functions, or patterns marked ANTI-PATTERN below.
- Ignore compiler warnings.

---

## 2. OOP Fundamentals

### 2.1 The Four Pillars

**Encapsulation**
- MUST hide internal state. Expose only what callers need.
- MUST use private/protected for data members, public only for interface.

```cpp
// CORRECT
class GerenciadorCache {
public:
    bool salvar(const std::string& chave, const std::vector<std::byte>& dados);
    std::optional<std::vector<std::byte>> recuperar(const std::string& chave) const;
private:
    std::unordered_map<std::string, std::vector<std::byte>> m_cache;  // hidden
    int m_tamanho_max{1000};             // hidden
};

// INCORRECT  -  exposes internals
class GerenciadorCache {
public:
    std::unordered_map<std::string, std::vector<std::byte>> cache;   // direct access = violation
};
```

**Abstraction**
- MUST define interfaces (pure abstract classes) for every cross-layer boundary.
- MUST NOT let callers depend on implementation details.

**Inheritance**
- SHOULD prefer composition over inheritance.
- MUST NOT use inheritance for code reuse alone  -  only for true IS-A relationships.
- Inheritance depth MUST NOT exceed 3 levels.

**Polymorphism**
- MUST use virtual dispatch via interfaces, not type-checking (no `dynamic_cast` chains).
- MUST mark overrides with `override` keyword (C++) or `@Override` (Java/Kotlin).

### 2.2 Class Design Rules

- One class = one clearly stated responsibility.
- Class size: SHOULD NOT exceed 300 lines. If it does, decompose.
- Constructor MUST NOT perform heavy work (I/O, network, database). Use an `initialize()` method.
- MUST implement the Rule of Five/Zero (C++) or equivalent resource management.

---

## 3. SOLID Principles

### S  -  Single Responsibility Principle
> A class should have only one reason to change.

- Each class owns one concept: parsing, rendering, fetching, caching, validating  -  never all at once.
- Test: if you describe the class and use the word "and", split it.

```
CORRECT:  RepositorioProdutos  →  only fetches items
CORRECT:  CacheProdutos        →  only caches items
INCORRECT: RepositorioProdutos →  fetches AND caches AND validates
```

### O  -  Open/Closed Principle
> Open for extension, closed for modification.

- Add behavior via new classes, not by editing existing ones.
- Use Strategy, Decorator, or plugin patterns to extend without modifying.

```cpp
// CORRECT  -  add new API source without touching existing code
class IRepositorioProdutos {
public:
    virtual std::vector<Produto> buscar(const std::string& nome) = 0;
    virtual ~IRepositorioProdutos() = default;
};
class RepositorioApiA  : public IRepositorioProdutos { ... };
class RepositorioApiB : public IRepositorioProdutos { ... };  // extension, not modification
```

### L  -  Liskov Substitution Principle
> Subtypes must be substitutable for their base types without breaking behavior.

- An override MUST NOT weaken preconditions or strengthen postconditions.
- An override MUST NOT throw exceptions the base does not declare.
- MUST NOT override to do nothing or throw `NotImplementedException`.

```cpp
// INCORRECT  -  LSP violation: override does nothing
class RepositorioNulo : public IRepositorioProdutos {
    std::vector<Produto> buscar(const std::string&) override { return {}; } // silent failure
};

// CORRECT  -  explicit null object that documents intent
class RepositorioNulo : public IRepositorioProdutos {
    std::vector<Produto> buscar(const std::string&) override {
        std::cerr << "RepositorioNulo: operação não suportada\n";
        return {};
    }
};
```

### I  -  Interface Segregation Principle
> Clients should not depend on interfaces they do not use.

- Split fat interfaces into role-specific ones.
- Each interface SHOULD have ≤ 5 methods.

```cpp
// INCORRECT  -  one fat interface
class IRepositorio {
    virtual void salvar(Produto) = 0;
    virtual Produto buscar(std::string) = 0;
    virtual void deletar(std::string) = 0;
    virtual std::vector<Produto> listar() = 0;
    virtual void exportarCSV() = 0;   // unrelated to repo
    virtual void enviarEmail() = 0;   // completely unrelated
};

// CORRECT  -  segregated
class IRepositorioLeitura  { virtual Produto buscar(std::string) = 0; ... };
class IRepositorioEscrita  { virtual void salvar(Produto) = 0; ... };
class IExportador          { virtual void exportarCSV() = 0; ... };
```

### D  -  Dependency Inversion Principle
> Depend on abstractions, not concretions.

- High-level modules MUST NOT import low-level modules directly.
- Both MUST depend on interfaces.
- Inject dependencies via constructor (preferred), setter, or factory.

```cpp
// INCORRECT  -  high-level depends on concrete low-level
class ServicoProdutos {
    RepositorioApiA m_repo;  // concrete dependency
};

// CORRECT  -  depends on abstraction, injected via constructor
class ServicoProdutos {
    std::shared_ptr<IRepositorioProdutos> m_repo;
public:
    explicit ServicoProdutos(std::shared_ptr<IRepositorioProdutos> repo)
        : m_repo(std::move(repo)) {}
};
```

---

## 4. Design Patterns  -  Complete Reference

Apply patterns when they solve a real problem. MUST NOT apply patterns speculatively.

### 4.1 Creational Patterns

| Pattern | Use When | Key Rule |
|---------|----------|----------|
| **Singleton** | Exactly one instance needed (logger, config) | MUST be thread-safe. AVOID in testable code  -  prefer DI. |
| **Factory Method** | Subclasses decide which object to create | Define abstract `criar()`, override in subclasses. |
| **Abstract Factory** | Families of related objects (theme, platform) | One factory interface, multiple concrete factories. |
| **Builder** | Complex object construction with many optional params | Separate construction from representation. Fluent API. |
| **Prototype** | Clone expensive objects | Implement deep copy. Avoid shared mutable state. |

```cpp
// Builder example
class AtaqueBuilder {
    Ataque m_ataque;
public:
    AtaqueBuilder& nome(const std::string& n)   { m_ataque.nome = n; return *this; }
    AtaqueBuilder& dano(const std::string& d)   { m_ataque.dano = d; return *this; }
    AtaqueBuilder& custo(std::vector<std::string> c)     { m_ataque.custo = std::move(c); return *this; }
    Ataque build() { return std::move(m_ataque); }
};
// Usage:
auto ataque = AtaqueBuilder{}.nome("Tackle").dano("10").custo({"Colorless"}).build();
```

### 4.2 Structural Patterns

| Pattern | Use When | Key Rule |
|---------|----------|----------|
| **Adapter** | Incompatible interfaces must work together | Wrap external API to match internal interface. |
| **Bridge** | Separate abstraction from implementation | Decouple so both can vary independently. |
| **Composite** | Tree structures (UI hierarchy, file system) | Leaf and composite share same interface. |
| **Decorator** | Add behavior dynamically without subclassing | Wrap object, delegate, then extend. |
| **Facade** | Simplify complex subsystem access | One simple interface over many complex classes. |
| **Flyweight** | Many objects sharing common state (icons, fonts) | Separate intrinsic (shared) from extrinsic (unique) state. |
| **Proxy** | Control access, add lazy init, logging, caching | Same interface as real object. |

```cpp
// Composite: repository that tries primary then fallback
class RepositorioComposto : public IRepositorioProdutos {
    std::unique_ptr<IRepositorioProdutos> m_primario;
    std::unique_ptr<IRepositorioProdutos> m_fallback;
public:
    Produto buscar(const std::string& id) override {
        auto resultado = m_primario->buscar(id);
        if (resultado.id.empty()) resultado = m_fallback->buscar(id);
        return resultado;
    }
};
```

### 4.3 Behavioral Patterns

| Pattern | Use When | Key Rule |
|---------|----------|----------|
| **Chain of Responsibility** | Multiple handlers may process a request | Each handler decides to handle or pass forward. |
| **Command** | Encapsulate action as object (undo/redo, queue) | Separate invoker from receiver. |
| **Iterator** | Traverse collection without exposing internals | Use standard iteration protocol. |
| **Mediator** | Reduce coupling between many objects | Central hub coordinates communication. |
| **Memento** | Save/restore object state | Snapshot without violating encapsulation. |
| **Observer** | Notify dependents of state changes | Maintain a list of listeners and notify them on change. |
| **State** | Object changes behavior based on internal state | Replace conditionals with state objects. |
| **Strategy** | Switch algorithm at runtime | Extract algorithm family into interchangeable objects. |
| **Template Method** | Define skeleton, let subclasses fill steps | Base class controls flow, subclasses override steps. |
| **Visitor** | Add operations to object structure without modifying it | Separate algorithm from object structure. |
| **Interpreter** | Parse and evaluate a language/grammar | Define grammar as class hierarchy. |

### 4.4 Modern / Architectural Patterns

| Pattern | Use When |
|---------|----------|
| **Repository** | Abstract data source from business logic. |
| **Unit of Work** | Group related database operations into a transaction. |
| **CQRS** | Separate read (Query) from write (Command) operations. |
| **Event Sourcing** | Store state as sequence of events, not current state. |
| **Dependency Injection** | Provide dependencies from outside the class. |
| **Service Locator** | AVOID  -  hidden dependency, hard to test. Use DI instead. |
| **MVC** | Separate Model (data), View (UI), Controller (logic). |
| **MVP** | Like MVC but Presenter holds all UI logic, View is passive. |
| **MVVM** | ViewModel exposes state; View binds reactively. |
| **Null Object** | Avoid null checks  -  provide do-nothing default implementation. |
| **Specification** | Encapsulate business rules as composable predicates. |

---

> Arquitetura de camadas: ver [GODS_LAWS.md](GODS_LAWS.md), L-17.

---

## 5. Clean Code Rules

### 5.1 Naming

> Convenção de nomes de identificador e comentário: ver [GODS_LAWS.md](GODS_LAWS.md), L-22.

### 5.2 Functions

- MUST do one thing. If you can extract a sub-function with a meaningful name, do it.
- MUST NOT exceed 40 lines. If longer, decompose.
- MUST NOT take more than 4 parameters. Wrap in struct if needed.
- Return early to avoid deep nesting. MUST NOT exceed 3 levels of nesting.

```cpp
// INCORRECT  -  deep nesting
void processar(Item c) {
    if (c.valida()) {
        if (!cache.tem(c.id)) {
            if (api.disponivel()) {
                // actual logic buried here
            }
        }
    }
}

// CORRECT  -  early returns (guard clauses)
void processar(const Item& c) {
    if (!c.valida()) return;
    if (cache.tem(c.id)) return;
    if (!api.disponivel()) { reportarErro("API indisponível"); return; }
    // actual logic at top level
}
```

### 5.3 Comments

- MUST NOT comment what the code does. Comment **why** it does it.
- MUST comment every workaround, hack, or non-obvious decision.
- MUST update comments when changing the code they describe.

```cpp
// INCORRECT
i++;  // increment i

// CORRECT
// API externa usa páginas base-1; nossa API interna usa base-0
pagina++;
```

### 5.4 Error Handling

- MUST handle all error cases. MUST NOT silently swallow exceptions.
- MUST log errors with context (file, function, relevant data).
- MUST propagate errors to the caller  -  do not hide failures.
- MUST NOT use exceptions for control flow.
- Use `std::optional`, `std::expected`, or result types for expected failures.

### 5.5 Constants vs Magic Numbers

```cpp
// INCORRECT
if (tentativas > 3) retry();
img = redimensionar(img, 120, 160);

// CORRECT
constexpr int MAX_TENTATIVAS = 3;
constexpr int LARGURA_MINIATURA = 120;
constexpr int ALTURA_MINIATURA = 160;
if (tentativas > MAX_TENTATIVAS) retry();
img = redimensionar(img, LARGURA_MINIATURA, ALTURA_MINIATURA);
```

### 5.6 RAII and Resource Management

- MUST use RAII for all resources (memory, file handles, DB connections, mutexes).
- MUST prefer smart pointers (`unique_ptr`, `shared_ptr`) over raw `new`/`delete`.
- MUST NOT call `delete` manually in application code.
- MUST NOT store raw owning pointers.

### 5.7 DRY  -  Don't Repeat Yourself

**Regra de Três:** Na **primeira** ocorrência, escreva. Na **segunda**, registre a repetição. Na **terceira**, extraia.

```cpp
// 1ª e 2ª ocorrências: duplicação aceitável (WET  -  Write Everything Twice)
std::string formatarPreco(double v)  { return std::format("R$ {:.2f}", v); }
std::string formatarSaldo(double v)  { return std::format("R$ {:.2f}", v); }

// 3ª ocorrência: EXTRAIA  -  nomeie a razão comum de mudança
// Razão: "formatação de valores monetários em BRL"
std::string formatarBRL(double valor, int casas = 2) {
    return std::format("R$ {:.{}f}", valor, casas);
}
```

**Duplicação real vs. coincidência:**

| Tipo | Definição | Regra |
|------|-----------|-------|
| **Duplicação real** | Mesmo conceito, mesma razão de mudar | MUST extrair |
| **Coincidência** | Parece similar hoje; divergirá amanhã | MUST NOT unificar |

```cpp
// COINCIDÊNCIA  -  NÃO unificar mesmo tendo lógica idêntica hoje:
// Regras de negócio distintas; mudarão de forma independente
bool validarIdade(int anos)        { return anos >= 0 && anos <= 120; }
bool validarAnoFabricacao(int ano) { return ano  >= 0 && ano  <= 120; }

// DUPLICAÇÃO REAL  -  MUST extrair:
// Mesma razão de mudar: "validar quantidade positiva com teto de estoque"
bool validarQuantidade(int qtd)  { return qtd > 0 && qtd <= 9999; }
bool validarEstoque(int estoq)   { return estoq > 0 && estoq <= 9999; }
// → bool validarQuantidadePositiva(int val) { return val > 0 && val <= 9999; }
```

**Rules (RFC 2119):**

- MUST name the *common reason to change* when extracting  -  similarity in code alone is insufficient justification.
- MUST NOT unify logic with distinct business meanings even if syntactically identical.
- SHOULD prefer WET (Write Everything Twice) over a premature abstraction that fits neither caller.
- MUST NOT create generic helpers to avoid two similar lines; three real occurrences are required.
- MAY tolerate duplication in tests when each test independently documents a distinct behavior.

---

## 6. Security

> Full procedures in [TESTES.md](TESTES.md)  -  sections T8, T12.

### 6.1 OWASP Top 10  -  AI Coder Checklist

| # | Risk | Rule |
|---|------|------|
| A02 | Cryptographic Failures | MUST use AES-256+ for data at rest. NEVER MD5/SHA1 for security. |
| A04 | Insecure Design | MUST threat-model new features before implementing. |
| A06 | Vulnerable Components | MUST check CVEs before adding any dependency (see T12 in TESTES.md). |
| A08 | Software Integrity | MUST verify integrity of downloaded dependencies (checksums, signatures). |
| A09 | Logging Failures | MUST log security events. MUST NOT log passwords, tokens, or PII. |

### 6.2 Hardcoded Secrets  -  Zero Tolerance

```cpp
// INCORRECT  -  NEVER commit this
const std::string API_KEY = "sk-live-abc123xyz789";
std::string url = "https://api.com?token=mypassword";

// CORRECT  -  load from secure storage or environment
const std::string apiKey = gerenciadorChaves.recuperar("limitless_api_key");
```

### 6.3 Input Validation

- MUST validate all data arriving from: user input, file reads, environment variables.
- MUST NOT trust data from any external source.
- MUST constrain string lengths, numeric ranges, and allowed characters at entry points.

---

## 7. Performance

### 7.1 General Rules

- MUST profile before optimizing. No premature optimization.
- MUST cache results of expensive operations (disk, computation).
- MUST use lazy loading for data not immediately needed.
- MUST NOT copy large objects unnecessarily  -  use references and move semantics.

### 7.2 Memory

- MUST release resources when they go out of scope (RAII).
- MUST NOT hold large objects in memory indefinitely  -  use LRU cache with size limit.

### 7.3 Rendering

- MUST NOT do layout calculations on the UI thread unnecessarily.
- Avoid creating/destroying widgets in tight loops  -  reuse or use virtual scrolling.
- Heavy image operations MUST be done off-thread, result posted to UI thread.

---

## 8. Git Process for AI Coders

### 8.1 Before Writing Any Code

```bash
# MUST: read current state of files to be modified
# MUST: understand existing patterns before introducing new ones
# MUST: check if a build passes before starting
cmake --build build -j$(nproc) 2>&1 | grep -E "error:|warning:"
```

### 8.2 Conventional Commits (MANDATORY)

Format: `<type>(<scope>): <description>`

| Type | When to use |
|------|------------|
| `feat` | New feature added |
| `fix` | Bug fix |
| `refactor` | Code change without feature or fix |
| `docs` | Documentation only |
| `test` | Adding or fixing tests |
| `chore` | Build, CI, dependencies, tooling |
| `perf` | Performance improvement |
| `style` | Formatting, no logic change |
| `revert` | Reverting a previous commit |

```bash
# CORRECT examples
git commit -m "feat(filtros): add real-time chip filter applied to loaded grid"
git commit -m "fix(tema): type icon not rendering in the results grid via data URI"
git commit -m "docs: add TESTES.md with complete audit and quality guide"
git commit -m "chore: convert type images webp→png, remove webp files"

# INCORRECT
git commit -m "fix stuff"
git commit -m "WIP"
git commit -m "changes"
```

### 8.3 Branch Naming

```
feat/nome-da-feature
fix/descricao-do-bug
refactor/modulo-afetado
docs/nome-do-documento
chore/ferramenta-ou-dep
test/modulo-testado
```

### 8.4 Commit Checklist (MUST complete before every commit)

```
[ ] Build passes with zero errors
[ ] Zero new compiler warnings introduced
[ ] No hardcoded secrets, tokens, or credentials
[ ] .gitignore excludes build artifacts, IDE files, .env files
[ ] Commit message follows Conventional Commits format
[ ] Files staged are only those related to the current task
[ ] No unrelated changes mixed in (separate commits for separate concerns)
```

### 8.5 What the AI MUST NEVER Do in Git

- MUST NOT force-push to `main` or `master`.
- MUST NOT commit files containing secrets (`.env`, credentials, private keys).
- MUST NOT amend published commits (use a new commit instead).
- MUST NOT use `--no-verify` to skip hooks unless explicitly instructed.
- MUST NOT batch unrelated changes into one commit.
- MUST NOT commit generated files (build artifacts).

### 8.6 Pull Request Description Template

```markdown
## What
[One sentence: what this PR does]

## Why
[One sentence: why it was needed]

## How
- [Key technical decision 1]
- [Key technical decision 2]

## Checklist
- [ ] Build passes
- [ ] Tests pass (reference TESTES.md sections run)
- [ ] No new warnings
- [ ] No secrets committed
- [ ] CHANGELOG.md updated (if user-facing change)
```

---

## 9. Testing & Audit Mandate

> Full procedures, commands, and tools: **[TESTES.md](TESTES.md)**

### When to Run Tests

| Event | Required tests |
|-------|---------------|
| Every commit | Build passes, zero warnings |
| Feature complete | T1 (unit), T2 (static analysis), T4 (ASan/UBSan) |
| Before any release/deploy | T1-T12 full suite + A1-A10 full audit |
| New dependency added | T5 (dependency scan) + T12 (CVE check) |
| Crypto code changed | T8 (secrets), A5 (dynamic analysis) |
| UI changed | A3 (UX/accessibility) |
| Architecture changed | A2 (layer audit), A7 (coupling), A9 (SOLID) |

### Minimum Quality Gates (MUST pass before release)

```
[ ] T1  Unit tests: 0 failures
[ ] T2  Static analysis: 0 security/bugprone errors
[ ] T4  ASan: 0 ERROR SUMMARY
[ ] T8  Secrets scan: 0 detected
[ ] T12 CVE scan: 0 CRITICAL unpatched
[ ] A2  Architecture: 0 layer violations
[ ] A10 Audit report generated and reviewed
```

### Post-Release Cleanup Prompt (MANDATORY)

Após uma release ser efetivamente lançada (tag publicada + artefatos anexados + CI verde no remoto), o agente DEVE perguntar ao usuário se deseja apagar pastas desnecessárias geradas durante o ciclo de build/test.

**Quando perguntar:** somente após confirmação de release publicada (não após push comum, não após build local de teste, não antes de CI verde).

**Pergunta padrão:**

> "Release lançada. Quer apagar pastas desnecessárias geradas pelo ciclo de build/test (ex.: `build/`, `dist/`, `.venv/`, `target/`, `node_modules/`, caches)?"

**Se o usuário disser não:** registrar e não tocar em nada.

**Se o usuário disser sim:**

1. Varrer a raiz do projeto buscando candidatos comuns por stack:
   - **Genéricos:** `build/`, `dist/`, `out/`, `tmp/`, `.cache/`, `coverage/`, `htmlcov/`
   - **Python:** `.venv/`, `__pycache__/`, `*.egg-info/`, `.pytest_cache/`, `.mypy_cache/`, `.ruff_cache/`, `.hypothesis/`, `.tox/`, `.import_linter_cache/`
   - **C++/CMake:** `build/`, `build-*/`, `cmake-build-*/`, `_deps/`, `CMakeFiles/`, `Testing/`
   - **Rust:** `target/`
   - **Node/TS:** `node_modules/`, `.next/`, `.turbo/`, `.nuxt/`, `.svelte-kit/`
   - **IDE/SO:** `.DS_Store`, `Thumbs.db`
2. Listar cada candidato encontrado com tamanho aproximado (`du -sh`).
3. **CONFIRMAR explicitamente cada grupo antes de remover** (operação destrutiva). Nunca apagar sem listar primeiro.
4. Excluir caminhos rastreados pelo git ou que contenham mudanças não commitadas. Checar com `git status --ignored` e `git ls-files`.
5. Excluir pastas que estão no `.gitignore` mas o usuário marcou como "manter" (ex.: `.venv/` em projetos que prefere preservar).
6. Após remoção, mostrar resumo: "removidos X paths, Y MB liberados".

**Regras invioláveis:**

- NUNCA apagar pasta versionada (`src/`, `tests/`, `docs/`, etc.).
- NUNCA apagar `dist/` se a release ainda não anexou os artefatos no servidor.
- NUNCA apagar `.git/`, `.github/` (config persistente).
- Sempre listar antes, confirmar depois.

---

## 10. Language-Specific Rules

### 10.1 C++ *(Mandatory)*

**Version:** C++23 minimum.

**Memory:**
```cpp
// MUST use smart pointers
auto obj = std::make_unique<MeuObjeto>();
auto shared = std::make_shared<Servico>(dep);

// MUST NOT
MeuObjeto* raw = new MeuObjeto();  // who owns this?
delete raw;                         // manual delete = leak risk
```

**Modern C++23 MUST-use features:**
```cpp
std::optional<Item>     // instead of nullptr checks
std::variant<Ok, Erro>   // instead of error codes
[[nodiscard]]            // on functions whose return value must be checked
const auto&              // prefer const references
if (auto val = buscar(); val.has_value())  // init-statement in if
```

**MUST NOT use:**
```cpp
NULL           // use nullptr
(Type*)ptr     // use static_cast<Type*>(ptr)
printf/scanf   // use std::cout/std::cin or std::format
gets()         // buffer overflow risk
strcpy/strcat  // use std::string
```

---

## 11. Universal Engineering Principles

> Complement to SOLID and DRY. Apply across all languages and stacks to produce robust, professional, and high-performance code.

### 11.1 KISS  -  Keep It Simple, Stupid

The simplest solution that correctly solves the problem is the right solution. Complexity is debt.

**Rules:**
- MUST NOT add layers of abstraction without a concrete, present reason.
- MUST NOT use a design pattern just because it fits  -  only when it removes real pain.
- When two solutions work, MUST choose the one a new team member understands in 30 seconds.

### 11.2 YAGNI  -  You Aren't Gonna Need It

Build what is required **now**. Future requirements arrive with their own context.

**Rules:**
- MUST NOT implement features, flags, or extension points for hypothetical future use.
- MUST NOT add configuration options that no current caller uses.
- MUST NOT generalize a function until the third real use case exists (see DRY § 5.7).

### 11.3 Fail Fast

Detect violations of preconditions as early as possible  -  at the system boundary, not buried in domain logic.

**Rules:**
- MUST validate all external input at the entry point (API, CLI, file, IPC, queue).
- MUST NOT silently coerce invalid input into a valid-looking value.
- MUST crash or return error immediately when an invariant is violated  -  never defer.
- MUST include the violated condition and the actual value in the error message.
- MUST NOT use default values to mask missing required input.

### 11.4 Law of Demeter  -  Principle of Least Knowledge

A unit MUST only talk to its immediate collaborators. No reaching through the object graph.

**The "one dot" rule:** `a.fazAlgo()` is fine. `a.getB().getC().fazAlgo()` is a violation.

**Rules:**
- MUST NOT chain more than one method/property access on a foreign object.
- MUST NOT reach through an object to access its internals  -  ask the object to act.
- SHOULD expose behavior, not structure (Tell, Don't Ask).

### 11.5 CQS  -  Command-Query Separation

A function either **changes state** (command) or **returns data** (query). Never both.

**Rules:**
- Commands MUST return `void` (or `Result`/`Error` for success/failure only  -  no business data).
- Queries MUST be pure: same input → same output, no side effects.
- MUST NOT have a function that returns meaningful data AND produces a side effect.
- Exception: language-idiomatic patterns like `pop()` (stack), `next()` (iterator) are accepted.

### 11.6 Composition over Inheritance

Prefer assembling behavior from small collaborators over deep class hierarchies.

**Rules:**
- MUST NOT create inheritance hierarchies deeper than 2 levels (base + one concrete).
- MUST NOT use inheritance to share implementation  -  use composition or free functions.
- SHOULD use interfaces/traits/protocols to define behavior contracts.

### 11.7 Immutability by Default

Treat data as immutable unless there is a concrete reason to mutate it.

**Rules:**
- MUST declare variables as immutable (`const`, `val`, `let`, `final`, `const&`) by default.
- MUST NOT mutate function parameters.
- SHOULD return new values instead of modifying existing ones in domain logic.
- Shared mutable state MUST be protected by a synchronization primitive (mutex, atomic, lock).

### 11.8 Explicit over Implicit

Behavior MUST be visible at the call site. Magic and hidden side effects are failure modes.

**Rules:**
- MUST NOT rely on global mutable state or thread-local singletons silently affecting behavior.
- MUST pass dependencies explicitly (constructor/function parameters), not via globals.
- Configuration that affects runtime behavior MUST be explicit, not inferred from environment magic.
- MUST NOT use hidden default arguments that change behavior without callers knowing.

### 11.9 High Cohesion, Low Coupling

- **Cohesion:** everything inside a module belongs together  -  one clear purpose.
- **Coupling:** modules depend on each other as little as possible, and only through stable interfaces.

**Rules:**
- MUST NOT place logic in a module because it is *convenient*, only because it *belongs*.
- Coupling between modules MUST go through interfaces, not concrete types.
- A module MUST be testable in isolation without instantiating the full system.
- Circular dependencies between modules MUST NOT exist.

### 11.10 Idempotency

An operation that can be retried N times MUST produce the same result as running it once.

**Rules:**
- MUST design write operations (HTTP PUT/DELETE, DB upserts, file overwrites) to be idempotent.
- MUST NOT accumulate state on repeated calls (e.g., double-append on retry).
- SHOULD use idempotency keys for operations that cannot be made naturally idempotent.
- MUST test the "call twice" scenario for all mutation endpoints.

### 11.11 Tell, Don't Ask

Don't query an object's state to make a decision externally  -  tell the object to act and let it decide internally.

> Distinct from Law of Demeter (§ 11.4): LoD governs *how far* you reach into the object graph; TDA governs *where decisions live*. Both can be violated independently.

**Rules:**
- MUST NOT extract state from an object, compute a decision outside, then push the result back in.
- MUST place the decision inside the object that owns the relevant data.
- SHOULD expose behavior-revealing methods (`aprovar()`, `descontar()`) over state-revealing getters (`getStatus()`, `getValor()`).

### 11.12 POLA  -  Principle of Least Astonishment

A function, method, or API MUST behave exactly as its name and signature lead the caller to expect. Surprising behavior is a bug, even when documented.

**Rules:**
- MUST NOT perform side effects that the name does not indicate (`buscar*` MUST NOT write; `calcular*` MUST NOT mutate).
- MUST NOT return a different type or shape depending on a hidden flag or global state.
- MUST NOT silently ignore parameters  -  if a parameter is accepted, it MUST affect behavior.
- MUST use names that reveal what the unit does at the abstraction level of the caller.
- Boolean parameters that change the *kind* of operation MUST be replaced by two separate functions.

---

## 12. Logging & Observability

- Structured JSON logging.
- NO PII in logs.

---
*This contract is the authoritative reference for all code written in this project.*
