# Solution : exercice 2 — Le fichier qui ment sur sa taille

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le fichier qui ment sur sa taille](../../lessons/part-1/lesson-039-virtual-memory.md) de la leçon 039.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-039/ex2.patch}}
```

## Visite guidée

D'abord le bug. `ReadFile` fait confiance à `fstat` : il prend la taille
rapportée, alloue exactement cela et lit exactement cela. Pour un fichier
ordinaire, la taille dit vrai. Pour un **fichier virtuel** — `/proc/self/maps`
en est un, et c'est le même fichier que lit la plongée en profondeur — la
taille est un *mensonge* : le noyau rapporte 0 octet, puis produit des
kilooctets quand on le lit. Notre lecteur de fichier entier renvoie donc
**0 octet, présenté comme un succès** — exactement l'issue que le contrat de la
leçon 037 interdit : des données partielles déguisées en succès.

La correction consiste à cesser de traiter la taille comme la destination et à
la traiter comme un indice. Lisez jusqu'à ce que le fichier dise qu'il a fini —
`read()` qui renvoie 0 — et faites grandir le tampon chaque fois qu'il se
remplit (le `realloc` de la leçon 008, faisant le travail pour lequel il a été
enseigné). L'ancienne vérification « un octet après la fin » disparaît : la fin
de fichier *est* la fin du fichier, quelle que soit la taille annoncée.

Avec la correction et une sonde qui lit le second argument de ligne de
commande :

```
$ DISPLAY=:99 ./build/game rt.bin /proc/self/maps
engine: page size 4096 bytes
engine: /proc/self/maps: 5766 bytes read
```

5 766 octets — le vrai contenu du fichier de carte (sa taille varie d'une
exécution à l'autre : les mappages du processus en décident). Avant la
correction, le même appel rapportait 0.

Que signifie le contrat maintenant ? « Fichier entier » n'a jamais été « le
nombre dans le stat » — c'est *tout ce que le fichier donne*, et un fichier qui
donne plus que la taille annoncée se lit complètement. Les fichiers ordinaires
atterrissent toujours dans une seule allocation de la bonne taille ; les
fichiers virtuels grandissent à partir d'un premier morceau. Le lecteur est
honnête dans les deux cas, et « jamais de données partielles présentées comme
un succès » tient désormais pour les fichiers qui mentent.

(La même leçon vaut pour `/proc` en général, pour sysfs et pour tout ce que le
noyau synthétise à la lecture — ce qui vaut la peine d'être retenu bien avant
que la partie 4 ne commence à lire de vraies ressources : sachez quel genre de
fichier vous tenez.)

*Page traduite de la version anglaise `book/solutions/lesson-039/ex2.md`,
révision `00824d2`.*

<!-- translation-source: book/solutions/lesson-039/ex2.md @ 00824d2 -->
