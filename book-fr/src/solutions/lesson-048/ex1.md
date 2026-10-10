# Solution : exercice 1 — Le budget d'instructions

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le budget d'instructions](../../lessons/part-2/lesson-048-assembly.md) de la leçon 048.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-048/ex1.patch}}
```

## Visite guidée

D'abord le compte, directement depuis la liste que la leçon parcourt. Le
chemin de copie de la boucle interne, section par section :

| Section | De | À | Instructions |
| ------- | -- | - | ------------ |
| adresse source (`((j−y)*width + (i−x)) * 3`) | `17e6` | `181a` | 18 |
| test de la clé (trois chargements, trois comparaisons) | `181e` | `185d` | 20 |
| la copie (destination `× 4`, quatre stockages) | `185f` | `18ce` | 34 |
| incrément et test (`++i`, `i < right`) | `18d1` | `18db` | 4 |
| **par pixel opaque** | | | **76** |

Le chemin de saut prend l'arithmétique d'adresse et le test de la clé, puis
le `nop` de `18d0` et le même incrément et test : environ **43**. Notre
sprite a 130 pixels opaques et 126 de la couleur-clé, donc un dessin
retraite à peu près `130 × 76 + 126 × 43 ≈ 15,300` instructions.

La prédiction naïve : une instruction par cycle à 3,26 GHz (0,31 ns) donne
`76 × 0.31 ≈ 23 ns` par pixel opaque — et un sprite autour de 6 µs. Le
patch le mesure avec dix mille dessins chronométrés :

```
engine: blit bench: 10000 draws in 8.819 ms — 881.9 ns per sprite, 3.45 ns per pixel
```

**3,45 ns par pixel**, pas 23. La prédiction se trompe d'un facteur presque
sept, et l'endroit où vit ce facteur est toute la leçon : le cœur à
exécution dans le désordre d'un CPU moderne exécute *plusieurs* instructions
par cycle quand elles sont indépendantes et simples. Par sprite,
l'arithmétique est brutale — 15 300 instructions en 882 ns font 2 875 cycles
à 3,26 GHz, donc ce code retraite environ **5,3 instructions par cycle**. Pas
mal pour du code auquel on a dit de ne pas réfléchir ; les chargements et
les stockages touchent tous L1 (le sprite fait 12 lignes de cache et la
destination est chaude depuis le clear), donc rien dans la boucle n'attend
la mémoire.

Ce que la prédiction a eu juste et faux mérite d'être retenu :

- **Juste :** le nombre d'instructions prédit l'*ordre*. Le chemin de copie
  fait ~1,8× les instructions du chemin de saut, et les pixels opaques
  coûtent vraiment à peu près cela de plus que les pixels de la couleur-clé.
- **Faux :** les instructions ne sont pas des cycles. À `-O0`,
  l'ordonnancement du compilateur est arbitraire (un énoncé, une salve
  d'instructions), mais le *matériel* court toujours devant — les chargements
  sont émis tôt, les additions se chevauchent. Un compte d'instructions au
  dos d'une enveloppe est un plancher du travail, pas un chronomètre.

La seconde moitié de la réponse, c'est ce que `-O0` coûte en *nature*, pas
en nombre : chaque variable fait l'aller-retour par `-0xNN(%rbp)`, donc la
boucle est pleine de chargements et de stockages qu'une construction
optimisée garderait dans des registres. C'est exactement l'axe que la leçon
049 fait basculer — même C++, `-O3`, et une liste où une instruction fait ce
que vingt faisaient ici.

*Page traduite de la version anglaise `book/solutions/lesson-048/ex1.md`,
révision `38c0b7a`.*

<!-- translation-source: book/solutions/lesson-048/ex1.md @ 38c0b7a -->
