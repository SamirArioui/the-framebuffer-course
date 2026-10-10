# Solution : exercice 4 — Des suites, pas des cellules

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Des suites, pas des cellules](../../lessons/part-0/lesson-022-double-buffer.md) de la leçon 022.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-022/ex4.patch}}
```

## Visite guidée

Le vidage différentiel payait un échappement de position de curseur par
cellule changée — environ 7 octets d'adressage pour livrer 1 octet de
caractère. Le patch change l'unité de transmission de la cellule à la *suite* :
en trouvant une cellule changée, le vidage émet un `ESC [ row ; col H`, puis
écrit cette cellule et chaque voisine changée consécutive avec de simples
appels `fputc`, s'arrêtant à la première cellule inchangée. La boucle interne
possède `col` jusqu'à la fin de la suite ; la boucle externe reprend juste
après. `fputc` suffit parce que le terminal avance le curseur tout seul à
mesure que les caractères arrivent.

Mesuré sur les mêmes exécutions : `./snek 60` tombe de 6 949 à 1 127 octets
et `./snek 120` de 7 289 à 1 324 — environ 6× moins de trafic, aucun
changement de comportement. Le premier vidage profite le plus (des lignes
entières s'effondrent en un échappement plus leurs caractères), et le marqueur
en mouvement — deux cellules adjacentes — devient une courte suite. C'est la
même forme que chaque rastériseur de terminal sérieux et, d'ailleurs, que
chaque protocole réseau qui regroupe les petites écritures : quand le coût est
par message, rendez les messages dignes d'être envoyés.

*Page traduite de la version anglaise `book/solutions/lesson-022/ex4.md`,
révision `eea0461`.*

<!-- translation-source: book/solutions/lesson-022/ex4.md @ eea0461 -->
