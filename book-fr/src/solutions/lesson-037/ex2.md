# Solution : exercice 2 — Le fichier qui ne finit jamais

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le fichier qui ne finit jamais](../../lessons/part-1/lesson-037-file-read.md) de la leçon 037.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-037/ex2.patch}}
```

## Visite guidée

Trois prédictions à formuler avant de lancer quoi que ce soit — que répond
`ReadFile` pour un répertoire, pour `/dev/zero`, et pour un fichier qui
n'existe pas ?

Une fois l'instrument en place, l'implémentation raconte la réponse de l'OS
elle-même en chemin vers l'échec typé :

```
$ DISPLAY=:99 ./build/game src
platform: not a regular file (directory)
engine: src: unreadable

$ DISPLAY=:99 ./build/game /dev/zero
platform: not a regular file (special)
engine: /dev/zero: unreadable

$ DISPLAY=:99 ./build/game no-such-file.txt
platform: open failed, errno=2 (No such file or directory)
engine: no-such-file.txt: file not found
```

Le répertoire et le fichier manquant sont les cas ordinaires : `open` échoue
avec `errno=2` (`ENOENT`) pour le chemin manquant — la seule erreur de l'OS qui
se mappe vers `FILE_NOT_FOUND` — et un répertoire *s'ouvre* très bien mais
échoue au contrôle `S_ISREG`, donc il devient `FILE_UNREADABLE`.

`/dev/zero` est la prédiction intéressante. Il s'ouvre, il se lit — pour
toujours. Une boucle naïve de fichier entier (`while (read(...) > 0) keep
going`) n'en revient jamais ; « tous les octets » est une réponse infinie.
L'implémentation ne pose jamais la question ainsi : elle prend d'abord la
*taille* du fichier (`fstat`), alloue exactement cela, lit exactement cela,
puis exige que l'octet suivant n'existe pas. `/dev/zero` est un périphérique
caractère de taille 0 — il échoue au contrôle de fichier régulier avant qu'un
seul octet ne soit lu. Le contrat de fichier entier n'est pas « lire jusqu'à ce
que le fichier s'arrête » ; c'est « le fichier a une taille, et vous l'obtenez
exactement, ou vous obtenez un échec ».

Les lignes de l'instrument sont le vocabulaire de l'OS (`errno`, les genres de
fichiers) ; les lignes du moteur sont celles de la couture (`file not found`,
`unreadable`). La couture ne laisse pas fuir le vocabulaire de l'OS dans celui
du moteur — tout l'intérêt de l'échec typé est que le moteur n'a jamais besoin
de savoir ce que `errno=2` veut dire.

*Page traduite de la version anglaise `book/solutions/lesson-037/ex2.md`,
révision `c9b4169`.*

<!-- translation-source: book/solutions/lesson-037/ex2.md @ c9b4169 -->
