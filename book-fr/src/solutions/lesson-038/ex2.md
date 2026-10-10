# Solution : exercice 2 — Le périphérique qui est toujours plein

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le périphérique qui est toujours plein](../../lessons/part-1/lesson-038-file-write.md) de la leçon 038.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-038/ex2.patch}}
```

## Visite guidée

Trois chemins, trois prédictions à écrire d'abord : que répond `WriteFile`
pour `/dev/full`, pour un chemin dans un répertoire qui n'existe pas, et pour
un chemin dans un répertoire où l'OS ne vous laisse pas écrire ?

L'instrument raconte la réponse de l'OS lui-même sur le chemin de chaque échec
typé :

```
$ DISPLAY=:99 ./build/game /dev/full
platform: write failed, errno=28 (No space left on device)
engine: /dev/full: could not write

$ DISPLAY=:99 ./build/game /no-such-dir/out.bin
platform: write open failed, errno=2 (No such file or directory)
engine: /no-such-dir/out.bin: could not write

$ DISPLAY=:99 ./build/game /usr/out.bin
platform: write open failed, errno=13 (Permission denied)
engine: /usr/out.bin: could not write
```

`/dev/full` est le plus intéressant : le fichier *s'ouvre* très bien — c'est
l'écriture qui échoue, avec `ENOSPC` à chaque octet. C'est le cas pour lequel
la boucle d'écriture existe : un `write()` qui renvoie une erreur ne veut pas
dire « en a écrit une partie », et la réponse du contrat est un échec typé, pas
un fichier plus court.

Les deux autres échouent à `open` : `errno=2` parce que le répertoire parent
n'existe pas (l'OS ne peut pas créer de fichier dans rien), et `errno=13` parce
que le répertoire existe et dit non. La même réponse typée de la couture
(`could not write`), trois raisons différentes de l'OS en dessous.

Remarquez où se situe la frontière du vocabulaire : `errno=28`, `errno=2`,
`errno=13` sont les nombres de l'OS et ils ne franchissent jamais la couture —
le moteur imprime `could not write` et sort, exactement comme pour toute
écriture qui échoue. Si le moteur doit un jour distinguer « disque plein » de
« permission refusée », c'est un nouvel échec *typé* à concevoir — pas un errno
à transmettre tel quel. (Et une note honnête : une écriture qui échoue peut
laisser un fichier partiel derrière elle sur le disque. La réponse du moteur
est correcte — il sait que l'écriture a échoué — mais les octets sur le disque
sont les restes de l'OS. Matière à exercice pour une leçon future : rendre les
écritures tout-ou-rien.)

*Page traduite de la version anglaise `book/solutions/lesson-038/ex2.md`,
révision `dabf156`.*

<!-- translation-source: book/solutions/lesson-038/ex2.md @ dabf156 -->
