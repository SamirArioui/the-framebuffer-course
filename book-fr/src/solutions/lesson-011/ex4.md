# Solution : exercice 4 — Un seul seau

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Un seul seau](../../lessons/part-0/lesson-011-hashtable.md) de la leçon 011.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-011/ex4.patch}}
```

## Visite guidée

Le patch fait du nombre de seaux un argument de la ligne de commande — 1024
par défaut — aussi les deux côtés de l'expérience n'ont-ils pas besoin de
recompiler. Le texte ici est un fichier généré : 300 000 jetons de mots tirés
d'un vocabulaire de 10 000 mots distincts (le `wc -w` du fichier et le compte
de lignes de sortie du compteur donnent directement les deux nombres). Mesuré
sur la machine de l'auteur :

```
NBUCKETS=1024   real 0.03s
NBUCKETS=64     real 0.08s
NBUCKETS=1      real 4.07s
```

L'exécution à un seul seau est environ 135 fois plus lente, et le ratio est
la longueur moyenne des chaînes rendue visible. Avec 10 000 mots distincts
dans un seul seau, chaque recherche marche une chaîne jusqu'à 10 000 entrées
— environ 5 000 `strcmp` en moyenne, quelque 1,5 milliard pour ce fichier. À
1024 seaux, la chaîne moyenne contient environ dix entrées et une recherche
sonde une poignée. Le ratio d'horloge est plus doux que le ratio de sondages
parce que chaque exécution paie les mêmes coûts fixes — découper 300 000
caractères, hacher, faire grandir les chaînes — et seule la partie marche
suit l'échelle.

Deux notes de bas de page tirées des mesures. La sortie était identique octet
par octet quels que soient les nombres de seaux (`diff` propre) : les seaux
changent *où* vit une clé, jamais la réponse. Et l'exécution intermédiaire à
64 seaux montre que la courbe est graduée, pas binaire — c'est un curseur de
« table de hachage » vers « liste », et le facteur de charge lit le curseur.

*Page traduite de la version anglaise `book/solutions/lesson-011/ex4.md`,
révision `11ce1ee`.*

<!-- translation-source: book/solutions/lesson-011/ex4.md @ 11ce1ee -->
