# Solution : exercice 4 — Des octets, pas des caractères

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Des octets, pas des caractères](../../lessons/part-0/lesson-001-first-program.md) de la leçon 001.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-001/ex4.patch}}
```

## Visite guidée

La prédiction : `./wordcount five.bin` affiche `5 five.bin`, et `wc -c
five.bin` est d'accord — `5 five.bin`. Le fichier contient cinq octets, et le
programme compte des octets. Sur ce système, il n'y a rien de plus à dire.

La boucle instrumentée est l'endroit où la leçon se cache. Elle affiche, dans
l'ordre :

```
c=72
c=105
c=0
c=10
c=255
```

`72` est `H`, `105` est `i`, `0` est l'octet NUL, `10` est le retour à la
ligne, et `255` est `0xFF` — cinq valeurs, puis la boucle s'arrête sans rien
afficher pour la fin du fichier. Cette dernière valeur est exactement
pourquoi `fgetc` renvoie un `int` et pas un `char`. S'il renvoyait un `char`,
la valeur `0xFF` arriverait comme `-1` sur tout système où le `char` simple
est signé — le même nombre que `EOF` — et ce fichier de cinq octets serait
compté pour quatre octets. La largeur supplémentaire de `int` garde les 257
résultats possibles de « un octet » séparés de l'unique valeur qui signifie
« il ne reste rien ».

La condition de boucle est sûre telle qu'écrite pour la même raison : `c` est
un `int`, donc chaque valeur d'octet — y compris `0x00` et `0xFF` — survit
intacte à la comparaison contre `EOF`. Si vous voyez un jour du code C
stocker le résultat de `fgetc` dans un `char`, il porte ce bug.

*Page traduite de la version anglaise `book/solutions/lesson-001/ex4.md`,
révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-001/ex4.md @ bb8d4ab -->
