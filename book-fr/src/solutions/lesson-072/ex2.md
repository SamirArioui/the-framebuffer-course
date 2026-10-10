# Solution : exercice 2 — Le coût du chargement, mesuré

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le coût du chargement, mesuré](../../lessons/part-4/lesson-072-load.md) de la leçon 072.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-072/ex2.patch}}
```

## Visite guidée

Le diff, c'est la mesure et rien d'autre : quatre chargements du même chargeur à
quatre tailles de table, chacun chronométré sur l'horloge de la plateforme —
`platform::Now()` autour de l'appel, le même instrument sur lequel
l'enregistrement de frame est construit — et comptabilisé dans l'arena. Les
quatre fichiers sont des entrées jetables que vous générez une fois, hors de
l'exécution :

```
$ for n in 2 20 200 2000; do { printf 'name x y facing speed health sprite\n'; i=0; while [ $i -lt $n ]; do printf 'def%d %d %d %d %d %d assets/sprite.ppm\n' $i $((i%480)) $((i%320)) $((i%4)) $((60+i%200)) $((1+i%10)); i=$((i+1)); done; } > assets/rows$n.txt; done
```

(Ils sont à vous de supprimer ensuite ; rien dans le moteur ne les nomme en
dehors de cette sonde.)

Les nombres, d'une vraie exécution de l'état final de cette leçon plus le patch
sur la machine de rédaction :

| fichier | lignes | croissance de l'arena | chargement |
| ------- | ------ | --------------------- | ---------- |
| `assets/rows2.txt` | 2 | 200 octets | 0.0041 ms |
| `assets/rows20.txt` | 20 | 2000 octets | 0.0088 ms |
| `assets/rows200.txt` | 200 | 20000 octets | 0.1561 ms |
| `assets/rows2000.txt` | 2000 | 200000 octets | 10.4368 ms |

Deux choses à lire avant la troisième. La **croissance de l'arena est exacte** à
chaque taille : `rows × sizeof(EntityDef)` — cent octets par définition, pas de
capacité et pas de gaspillage. Et le **temps n'est pas linéaire** : dix fois les
lignes coûtent deux fois le temps au petit bout (0.0041 → 0.0088 ms), dix-huit
fois au milieu (0.0088 → 0.1561 ms), et soixante-sept fois au sommet
(0.1561 → 10.4368 ms). Un chargement linéaire lirait 10× à chaque pas.

Où passe le temps, le code sous les yeux. Les deux marches sont linéaires — une
passe pour compter, une pour remplir — et les analyseurs de valeurs touchent
chaque octet une fois. La partie qui n'est pas linéaire est la vérification des
noms en double : le nom de chaque ligne est comparé aux noms de toutes les
lignes qui la précèdent, donc une table de *n* lignes fait environ *n²/2*
comparaisons de chaînes. À vingt lignes, c'est quelques centaines de
comparaisons et le chargement se termine en microsecondes ; à deux mille, c'est
deux millions de comparaisons, et c'est le chargement. Le saut de 0.1561 ms à
10.4368 ms, c'est cette vérification qui arrive.

Changeriez-vous quoi que ce soit pour les tables de ce jeu ? **Non — et les
nombres disent pourquoi.** Une table contient les *définitions* d'un jeu, pas
ses entités : le héros, trois types d'ennemis, le boss, une arme ou deux — une
douzaine de lignes, deux tout au plus dans les dizaines. À ces tailles, le
chargement prend quelques microsecondes et le terme quadratique n'est pas
mesurable par rapport à la lecture du fichier. Les entités elles-mêmes ne sont pas des
lignes de cette table du tout — elles sont l'affaire du magasin, la leçon 074 —
et c'est *là* qu'appartiennent des milliers de choses vivantes.

Quelle serait votre réponse à dix mille lignes ? Mesurée, elle serait à peu près
250× le nombre de 2000 lignes — un quart de seconde passé dans la seule
vérification des noms — et à ce stade c'est la vérification qui change, pas le
format :
triez les noms une fois après le remplissage et comparez les voisins, ou
vérifiez-les pendant le remplissage contre une petite table fixe de hachages.
Les deux gardent la règle (un nom, une définition) et ramènent le coût à
linéaire ou linéarithmique. L'habitude qui compte est celle avec laquelle cet
exercice a commencé : le changement est justifié par une mesure que vous avez
prise, puis mesurée de nouveau, jamais par la forme de la courbe sur un coin de
nappe.

Rien ici ne touche le chargeur, le format ou le jeu : la sonde est quatre
chargements à côté de ceux de l'exécution, et les fichiers qu'elle lit sont à
vous, à garder ou à supprimer.

*Page traduite de la version anglaise `book/solutions/lesson-072/ex2.md`,
révision `2794ee0`.*

<!-- translation-source: book/solutions/lesson-072/ex2.md @ 2794ee0 -->
