# Solution : exercice 3 — Qui possède les octets

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Qui possède les octets](../../lessons/part-0/lesson-004-heap-buffers.md) de la leçon 004.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-004/ex3.patch}}
```

## Visite guidée

L'histoire, ancrée dans la sortie de l'instrument. Chaque appel de
`CountStream` possède un `struct Buffer` sur sa trame de pile et, derrière
lui, un bloc de tas que `malloc` (via `realloc(NULL, …)`) a créé et que
chaque `BufferGrow` a remplacé. `realloc` retire l'ancien bloc quand il
déplace et l'étend quand il peut — sur ces fichiers il n'a jamais déplacé,
ce qui explique que chaque croissance ait imprimé la même adresse (vos
nombres différeront ; c'est la similitude qui est la nouvelle) :

```
$ ./wordcount huge.txt
grown to 64 at 0x55ec6d332490
grown to 128 at 0x55ec6d332490
...
grown to 8192 at 0x55ec6d332490
1 1 5001 5000 huge.txt
```

`fclose` rend l'objet `FILE` et ses tampons stdio — dont aucun n'est le
nôtre. Ce qui est vivant quand `main` renvoie : exactement un bloc par
fichier nommé sur la ligne de commande — la capacité finale du tampon de
chaque fichier, atteignable depuis rien, parce que la trame de pile qui
portait le `Buffer` est morte au retour de `CountStream`. C'est la fuite, et
la leçon 005 la nommera et la comptera. Une commande ponctuelle peut se
l'offrir — le système d'exploitation récupère tout le tas à la sortie ; le
dommage est fait au principe et à quiconque copie le motif. Une boucle de jeu
qui fuit un bloc par trame ne le peut pas : 60 trames à la seconde × 8 Ko
font près d'un demi-mégaoctet chaque seconde — un mégaoctet toutes les deux
secondes, pour toujours. Libérez ce que vous allouez ; à partir de la leçon
005, les outils insisteront.

*Page traduite de la version anglaise `book/solutions/lesson-004/ex3.md`,
révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-004/ex3.md @ bb8d4ab -->
