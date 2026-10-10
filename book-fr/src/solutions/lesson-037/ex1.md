# Solution : exercice 1 — Lire dans votre propre mémoire

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Lire dans votre propre mémoire](../../lessons/part-1/lesson-037-file-read.md) de la leçon 037.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-037/ex1.patch}}
```

## Visite guidée

`ReadFile` distribue des octets que la *plateforme* a alloués et que
l'appelant doit libérer — parfait pour une lecture ponctuelle, gênant quand le
moteur sait déjà d'où sa mémoire doit venir. Le chargement de ressources de la
partie 4 lira dans un arena ; la version d'aujourd'hui lit dans n'importe quel
tampon qui appartient à l'appelant.

`ReadFileInto(path, into, capacity)`, c'est le même contrat pointé vers la
mémoire de quelqu'un d'autre : les octets complets du fichier atterrissent dans
`into`, ou la réponse est un échec typé — `FILE_UNREADABLE` quand le fichier ne
tient pas. Rien n'est alloué et rien n'a besoin d'être libéré : les octets sont
à vous parce que la mémoire était à vous depuis toujours. (Le champ
`FileData.data` pointe vers le tampon de l'appelant — ou vaut 0 en cas
d'échec, pour que les mêmes contrôles marchent pour les deux fonctions.)

Vérifié dans les deux sens en une seule exécution :

```
$ DISPLAY=:99 ./build/game README.md /tmp/opencode/xcheck/empty.txt
engine: read /tmp/opencode/xcheck/empty.txt into my own memory: 0 bytes
engine: read README.md: 6113 bytes, 143 lines
```

et le chemin d'échec — un fichier trop gros pour le tampon est une réponse
typée, pas une lecture tronquée. Un fichier de 100 000 octets contre le tampon
de 64 Kio :

```
$ DISPLAY=:99 ./build/game README.md /tmp/opencode/xcheck/big.bin
engine: /tmp/opencode/xcheck/big.bin: does not fit in 64 KiB
engine: read README.md: 6113 bytes, 143 lines
```

Les deux fonctions sont une seule conception : **c'est l'appelant qui décide
où vivent les octets**. Que la plateforme vous prête de la mémoire un instant
ou que vous lui tendiez la vôtre, la lecture est tout le fichier ou bien c'est
un échec — et quand l'appelant sera un arena dans la partie 4, c'est la seconde
forme qui conviendra.

*Page traduite de la version anglaise `book/solutions/lesson-037/ex1.md`,
révision `c9b4169`.*

<!-- translation-source: book/solutions/lesson-037/ex1.md @ c9b4169 -->
