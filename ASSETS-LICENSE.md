# Licença dos assets (arte, música, lore in-game)

> Fronteira explícita entre **código** e **assets**. Decisão do líder supremo em 2026-06-21 (ver [ADR-005](docs/tech/adr/ADR-005-license-gpl3-assets-ccbysa.md)).

GusWorld é **freeware** (grátis, pra sempre). Tem partes sob licenças diferentes. Este arquivo define qual licença cobre o quê.

---

## Resumo

| Parte | Licença | Onde |
|---|---|---|
| Código-fonte | GNU GPL v3.0 (GPLv3) | ver [LICENSE](LICENSE) |
| Assets (arte, áudio, texturas, textos in-game) | Creative Commons BY-SA 4.0 | este arquivo |
| Livros-companheiros (Vol 1 + Vol 2) | Direitos reservados (obra à parte) | ver exceção abaixo |

---

## Código = GPLv3

Cobre todo o código-fonte vigente e os scripts de build:

- Fontes C++ (`.cpp`, `.h`, `.hpp`)
- Scripts de tooling/editor
- Scripts de build (CI, shell, empacotamento)

**Histórico (2026-07-22):** fontes C# (`.cs`) e scripts GDScript (`.gd`) da era Godot
não existem mais no repositório atual (decommission no marco M8; preservados na tag
`pre-m8-godot-legacy`), mas **continuavam cobertos pela GPLv3 enquanto existiram**;
esta seção não remove nem enfraquece aquela cobertura retroativa.

A licença completa está em [LICENSE](LICENSE) (texto GPLv3 verbatim, imutável). Titular: petrinhu, 2026.

---

## Assets = CC-BY-SA 4.0

Cobre toda a criação artística e narrativa **usada dentro do jogo**:

- Sprites, modelos 3D, texturas, atlas, materiais visuais
- Música, SFX, áudio
- Textos in-game (diálogos, lore exibida no jogo, descrições, UI)
- Conteúdo dentro de `assets/` e a lore canônica empregada no jogo

Licença: **Creative Commons Atribuição-CompartilhaIgual 4.0 Internacional (CC-BY-SA 4.0)**.
Texto e termos oficiais: <https://creativecommons.org/licenses/by-sa/4.0/>

Em resumo (não substitui o texto oficial): pode usar, copiar, modificar e redistribuir, **desde que** atribua o crédito (petrinhu, 2026) e **mantenha a mesma licença** (CC-BY-SA 4.0) na obra derivada.

Parte destes assets nasce de ferramentas de IA generativa de terceiro (PixelLab, Tripo3D, Suno, Gemini/Grok); nesses casos, o CC-BY-SA só se aplica na medida em que os Termos de Serviço daquela ferramenta cedem a titularidade do conteúdo gerado ao criador. Rastreabilidade por asset/lote em [`docs/tech/ai-assets-provenance.md`](docs/tech/ai-assets-provenance.md).

---

## Exceção: livros-companheiros = direitos reservados

Os **dois livros-companheiros** NÃO entram no CC-BY-SA. São obra literária à parte, com **todos os direitos reservados** ao autor (petrinhu, 2026):

- **Vol 1**: bíblia de worldbuilding
- **Vol 2**: antologia de contos

Ficam (no todo ou em parte) em `docs/book/`. Reprodução, distribuição ou obra derivada exigem autorização escrita do autor. O fato de a lore aparecer no jogo (sob CC-BY-SA) NÃO estende essa licença ao texto dos livros: são suportes distintos, com licenças distintas.

---

## Regra de fronteira (histórica): arquivos de cena Godot (.tscn / .tres)

> **Nota (2026-07-22):** o stack Godot/C# foi decommissionado no marco M8; arquivos
> `.tscn` e `.tres` não existem mais no repositório (histórico preservado na tag
> `pre-m8-godot-legacy`). A regra abaixo fica como registro de como a fronteira
> código/asset foi resolvida naquela era; não se aplica a nenhum arquivo do
> repositório atual.

Arquivos `.tscn` e `.tres` podem ser **código** ou **asset** conforme o conteúdo:

- **Com lógica** (script anexado, expressões, máquina de estado, configuração de comportamento) = **código**, sob GPLv3.
- **Dado ou arte puro** (apenas valores, referências de recurso, posições, paleta, dado de balanceamento sem lógica) = **asset**, sob CC-BY-SA 4.0.

Na dúvida sobre um `.tscn`/`.tres` específico, vale a natureza dominante do arquivo. Se carrega comportamento, trate como código.

---

## Atribuições de terceiros

Bibliotecas e fontes de terceiros têm licenças próprias, listadas em [THIRD-PARTY-LICENSES.md](THIRD-PARTY-LICENSES.md).

---

*Recomendação técnica de fronteira de licença; a validação jurídica formal cabe ao titular. Releases passadas distribuídas sob AGPL-3.0 permanecem sob aquela licença (ver ADR-005).*
