# Solution : exercice 2 — Les octets dont la planche n'a jamais entendu parler

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Les octets dont la planche n'a jamais entendu parler](../../lessons/part-2/lesson-050-font.md) de la leçon 050.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-050/ex2.patch}}
```

## Visite guidée

La prédiction, à partir de l'arithmétique d'index de `FontGlyph` :
`index = (int)(unsigned char)c - 32`, puis une vérification de bornes contre
`FONT_COUNT = 96`. La réponse est donc arithmétique :

| Sonde | Octet | Index | Réponse |
| ----- | ---- | ----- | ------ |
| `A` | 65 | 33 | **trouvé** — cellule 33, ligne 2 de la planche |
| saut de ligne | 10 | −22 | **manquant** — sous la plage |
| octet `0` | 0 | −32 | **manquant** — sous la plage |
| octet `200` | 200 | 168 | **manquant** — au-delà de 96 |
| octet `195` | 195 | 163 | **manquant** — au-delà de 96 |

L'exécution confirme les cinq :

```
engine: glyph for byte  65: found
engine: glyph for byte  10: missing
engine: glyph for byte   0: missing
engine: glyph for byte 200: missing
engine: glyph for byte 195: missing
```

Voici maintenant le cas que l'énoncé demande — celui où un `char` signé calcule
un index différent de celui que l'octet mérite. Sur cette plateforme, `char`
est signé, donc l'octet `200` *en tant que `char`* vaut −56. Sans le cast
`(unsigned char)`, l'index serait `−56 − 32 = −88` ; avec lui, `200 − 32 =
168`. Les deux sont hors de la plage aujourd'hui, donc les deux retournent `0`
— le bug est invisible *parce que la planche est petite*. Faites grandir la
planche au-delà de 128 caractères (une police avec des lettres accentuées, des
cellules 0..191, et la vérification de bornes contre 192) et la version sans
cast commencera à retourner `missing` pour des glyphes parfaitement valides
dans le tiers supérieur de la plage, tandis que la version avec cast les
trouve. Le cast n'est pas du style défensif ; c'est l'arithmétique d'index qui
sait ce qu'est un octet. L'habitude de la partie 0, qui paie encore.

Et la sonde d'étiquette :

```
engine: label of 6 bytes: 4 drew glyphs, 2 skipped
```

`"SCÖRE"` ressemble à cinq caractères pour un humain et fait **six octets** sur
disque : `Ö` est deux octets UTF-8 (`0xC3 0x96`), et le moteur lit des octets.
La boucle dessine quatre glyphes (`S`, `C`, `R`, `E`) et saute les deux octets
du `Ö` — chacun manquant indépendamment de la planche. C'est l'état honnête
d'un moteur orienté octets avec une planche de 96 glyphes : il ne sait pas ce
qu'est un point de code, et il le dit en ne dessinant rien.

La leçon 051 transforme ce « ne dessine rien » en *règle* : un caractère
manquant ne dessine rien **et les caractères qui le suivent gardent leurs
positions** — pas de trou, pas de décalage, pas de crash. Le compteur de sauts
de ce patch est cette règle avant qu'elle ait un nom : `drew`/`skipped` est
exactement la comptabilité dont la boucle de disposition aura besoin, et la
sonde UTF-8 est exactement l'entrée qui la testera.

*Page traduite de la version anglaise `book/solutions/lesson-050/ex2.md`,
révision `2230eb0`.*

<!-- translation-source: book/solutions/lesson-050/ex2.md @ 2230eb0 -->
