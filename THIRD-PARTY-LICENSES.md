# Licenças de terceiros (atribuição)

> Componentes de terceiros empacotados ou usados pelo GusWorld, com a respectiva licença e fonte. **Incluir este arquivo no pacote de release.**

O código próprio do GusWorld está sob GPLv3 (ver [LICENSE](LICENSE)); os assets próprios sob CC-BY-SA 4.0 (ver [ASSETS-LICENSE.md](ASSETS-LICENSE.md)). Os itens abaixo são de terceiros e mantêm a licença original de cada um.

---

## Tabela de atribuição

| Componente | Licença | Titular / fonte | Nota |
|---|---|---|---|
| Qt 6 | LGPLv3 (núcleo) + GPLv3 (alguns módulos) | The Qt Company / Qt Project | Alvo do pivot C++. Módulos GPL-only ficam OK (jogo todo é GPLv3). Disponibilizar o source do Qt usado. |
| Godot Engine | MIT | Juan Linietsky, Ariel Manzur e contribuidores | Engine da fase de transição (pre-pivot). |
| DialogueManager (addon) | MIT | Copyright (c) 2022-present Nathan Hoad | Addon Godot de diálogo. |
| Fontes Noto | SIL OFL 1.1 | Google e contribuidores | <https://openfontlicense.org> |
| Fonte Inter | SIL OFL 1.1 | The Inter Project Authors | <https://openfontlicense.org> |
| Fonte JetBrains Mono | SIL OFL 1.1 | JetBrains s.r.o. | <https://openfontlicense.org> |
| OpenSSL | Apache-2.0 | OpenSSL Project | Apenas se empacotado via Qt. |

---

## Links das licenças (texto oficial)

Referência por nome + URL; o texto legal completo de cada licença vive na fonte oficial e/ou é embarcado junto do respectivo componente no pacote de release.

- **MIT**: <https://opensource.org/license/mit>
- **LGPLv3**: <https://www.gnu.org/licenses/lgpl-3.0.html>
- **GPLv3**: <https://www.gnu.org/licenses/gpl-3.0.html>
- **SIL OFL 1.1**: <https://openfontlicense.org>
- **Apache-2.0**: <https://www.apache.org/licenses/LICENSE-2.0>

---

## Qt: oferta de source (LGPL/GPL)

Qt entra no projeto via pivot C++. O GusWorld é GPLv3, então o link (inclusive estático) com Qt LGPL/GPL é permitido sem licença comercial. Obrigações a cumprir no release:

- Identificar a versão exata do Qt empacotada.
- Disponibilizar o **source correspondente** do Qt (link para o tarball oficial da versão usada satisfaz a obrigação).
- Manter os avisos de copyright do Qt no pacote.

Fonte do Qt: <https://download.qt.io/official_releases/qt/>

---

## Manutenção

Atualizar esta tabela sempre que adicionar, remover ou trocar a versão de qualquer dependência de terceiro (addon Godot, fonte, biblioteca C++/NuGet, lib do sistema empacotada). Auditoria de licença = item RF-9 do pivot.

*Recomendação técnica de atribuição; validação jurídica formal cabe ao titular.*
