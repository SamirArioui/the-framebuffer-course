# Solution : exercice 4 — Où vivent les fonctions

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Où vivent les fonctions](../../lessons/part-0/lesson-024-command-table.md) de la leçon 024.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-024/ex4.patch}}
```

## Visite guidée

Un pointeur de fonction n'est pas magique — c'est l'adresse du code, tenue
dans des données. La répartition instrumentée l'imprime : presser espace
rapporte `key=32 run=0x589a21ff7962` et `q` rapporte
`key=113 run=0x589a21ff794d` sur une machine. Comparez avec la table de
symboles du binaire lui-même :

```
$ nm snek | grep -E 'Cmd(Quit|Start|Up)$'
000000000000194d t CmdQuit
0000000000001962 t CmdStart
00000000000019f2 t CmdUp
```

Les bits de poids faible correspondent exactement : `CmdQuit` est au décalage
`0x194d`, `CmdStart` à `0x1962` — les adresses à l'exécution sont ces décalages
plus l'adresse de chargement de l'exécutable (un binaire PIE est relocalisé au
démarrage ; `nm` montre des décalages). `commands[i].run()` se compile en un
appel indirect à travers cette adresse, et le membre de structure est là où
l'adresse est stockée. L'appeler et sauter dessus sont le même acte machine.

C'est pourquoi la table est la route C vers les interfaces : une ligne associe
des données (la touche) à du comportement (l'adresse d'une fonction), et le
balayeur ne sait ni ne se soucie de *quelle* fonction il appelle. Échangez
l'adresse, obtenez un comportement différent — sans recompiler le
répartiteur. La leçon 025 transforme cette forme en classe C++ avec une
méthode virtuelle, et le code machine est presque le même : une table
d'adresses de fonctions, un appel indirect, toute une caractéristique de
langage bâtie sur exactement ce que vous venez d'imprimer.

*Page traduite de la version anglaise `book/solutions/lesson-024/ex4.md`,
révision `bd716a9`.*

<!-- translation-source: book/solutions/lesson-024/ex4.md @ bd716a9 -->
