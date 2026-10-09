# Solution : exercice 3 — Un caractère à la fois

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Un caractère à la fois](../../lessons/part-0/lesson-001-first-program.md) de la leçon 001.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-001/ex3.patch}}
```

## Visite guidée

Sur une machine, le fichier de 100 Mo a pris environ 0,15 s via `fgetc` et
environ 0,01 s via `fread` — un ordre de grandeur. Vos nombres différeront,
et le rapport aussi : le cache et le stockage le font varier. Ce qui survit à
chaque machine, c'est l'ordre de grandeur.

Les deux boucles font la même arithmétique, aussi le temps n'est pas dans le
comptage. Chaque appel à `fgetc` est un appel de fonction qui vérifie l'état
du tampon du flux et renvoie un octet ; fait 100 millions de fois, ce coût par
octet est le programme. `fread` déplace 4096 octets par appel et laisse la
mise en tampon de la bibliothèque C faire son travail en vrac — sous le
capot, les deux lisent le fichier par les mêmes appels système, mais la
version par blocs demande un kilooctet à la fois et n'appelle presque rien par
octet.

C'est la forme de la plupart du travail de performance en C : mesurer d'abord
(c'est à cela que sert `time` ici), changer une chose, mesurer à nouveau. Le
résultat du comptage est inchangé — la boucle par blocs gère encore la
lecture courte à la fin du fichier, parce que `fread` renvoie le nombre
d'éléments réellement obtenus, et un retour de `0` est la fin de la boucle.
Garder le test `ferror` de l'exercice 2 est facultatif ici, mais l'habitude ne
coûte rien.
*Page traduite de la version anglaise `book/solutions/lesson-001/ex3.md`, révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-001/ex3.md @ bb8d4ab -->
