# Solution : exercice 2 — La largeur de registre de votre compilateur

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La largeur de registre de votre compilateur](../../lessons/part-2/lesson-049-simd.md) de la leçon 049.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-049/ex2.patch}}
```

## Visite guidée

Le patch ajoute la colonne de largeur : pour chaque fonction que le
recensement liste, le registre vectoriel le plus large que son code touche —
16 octets pour `xmm`, 32 pour `ymm`, 64 pour `zmm`. La prédiction avant de
lancer quoi que ce soit : sur la cible x86-64 de base, chaque fonction
devrait afficher **16 o** (SSE2 est garanti ; AVX non), et avec
`-march=native` sur le CPU de cette machine, les parcours de copie et les
fonctions à forte densité arithmétique devraient s'élargir à **32 o** —
tandis que tout ce qui n'utilise que quelques registres vectoriels
(l'horloge, l'arena) n'a aucune raison de s'élargir du tout.

La construction `-O3` par défaut :

```
$ ./tools/disasm.sh --census
  188  16 B  engine::Run()
   14  16 B  engine::AccountFrame(engine::FrameStats&, engine::FrameRecord const&)
    6  16 B  platform::Now()
    5  16 B  engine::LoadSprite(engine::Arena&, char const*)
    2  16 B  engine::CopyStrided(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  16 B  engine::CopySequential(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  16 B  engine::ArenaInit(engine::Arena&)
```

Toutes les largeurs à 16 o, comme prévu : la cible portable peut supposer
SSE2 et rien de plus. Avec `-march=native` — un drapeau qui dit au
compilateur que ce binaire peut utiliser tout ce que *ce* CPU possède :

```
  207  32 B  engine::Run()
   11  32 B  engine::AccountFrame(engine::FrameStats&, engine::FrameRecord const&)
    6  32 B  engine::LoadSprite(engine::Arena&, char const*)
    5  16 B  platform::Now()
    4  32 B  engine::CopyStrided(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    4  32 B  engine::CopySequential(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  16 B  engine::ArenaInit(engine::Arena&)
```

Les copies se sont élargies (leur liste montre maintenant
`vmovdqu (%rsi,%rdi,1),%ymm0` — 32 octets par déplacement — avec des queues
`xmm` pour le reste de 16 octets), les comptes ont bougé (l'abaissement AVX
fait d'autres choix — les 188 de `Run` sont devenus 207), et `platform::Now`
et `ArenaInit` sont restés à 16 o parce que leur poignée d'opérations
scalaires sur double ne gagnent rien à des registres plus larges.

Maintenant les mesures — la ligne 4 Ko de la sonde de caches, l'ensemble de
travail qui tient dans L1d et est donc limité par le débit d'instructions
plutôt que par la mémoire :

| Construction | Ligne 4 Ko | Ligne 12 Mo |
| ------------ | ---------- | ----------- |
| `-O3` | 102,5 Go/s | 16,9 Go/s |
| `-O3 -march=native` | **169,6 Go/s** | 16,2 Go/s |

La ligne résidente en L1 a bougé de deux tiers — les registres larges
divisent par deux le nombre d'instructions de la boucle interne de la copie
et le cœur retraite plus d'octets par cycle. La ligne 12 Mo a à peine bougé :
cette ligne, c'est la vitesse de la mémoire principale (la courbe de la
leçon 047), et aucune largeur de registre ne bat la physique. C'est la forme
de toute affirmation « le SIMD l'a accéléré » que vous lirez jamais : **la
largeur vectorielle paie là où le travail est limité par le calcul ou les
instructions, et pas là où il l'est par la mémoire** — c'est pourquoi les
leçons de localité de la leçon 047 viennent en premier.

Un avertissement que l'exercice gagne à la dure : `-march=native` fige le
jeu d'instructions de *ce* CPU dans le binaire. C'est le bon drapeau pour une
sonde que vous lancez sur votre propre machine, et le mauvais pour un jeu que
vous livrez sur la machine de quelqu'un d'autre — la colonne portable `-O3`
est celle qui tourne partout. Consignez les deux colonnes avec la machine qui
les a produites ; ce sont deux produits différents, pas un meilleur et un
moins bon.

*Page traduite de la version anglaise `book/solutions/lesson-049/ex2.md`,
révision `8c45291`.*

<!-- translation-source: book/solutions/lesson-049/ex2.md @ 8c45291 -->
