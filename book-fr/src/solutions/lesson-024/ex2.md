# Solution : exercice 2 — Entrée est un retour chariot

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Entrée est un retour chariot](../../lessons/part-0/lesson-024-command-table.md) de la leçon 024.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-024/ex2.patch}}
```

## Visite guidée

Le piège est une traduction. `printf '\n' | ./snek 20` démarre la partie, aussi
la ligne `'\n'` a-t-elle l'air juste — mais la touche Entrée d'un vrai terminal
envoie CR (`0x0D`), pas LF (`0x0A`), et le mode brut de cette leçon efface
`ICRNL`, le drapeau de la discipline de ligne qui réécrirait discrètement CR en
NL. Le programme voit désormais ce que le clavier a réellement envoyé, et la
table n'a aucune ligne pour cela : Entrée ne fait rien. (Nourrissez le tube
avec `printf '\r'` et vous voyez le même échec sans aucun terminal.)

Reproduisez à travers un pty avec l'entrée arrivant *après* que le mode brut
est en place — les octets pré-nourris sont traduits avant que le programme ne
puisse éteindre la traduction :

```
( sleep 0.5; printf '\r' ) | script -qec './snek 20' /dev/null
```

Sans la correction, la frame 20 rapporte encore `state=title`. La correction
est une ligne — `{ '\r', CmdStart }` — et la même exécution atteint
`state=play`, avec `'\n'` conservé pour que les tubes et les vieilles
habitudes continuent de fonctionner. Les deux orthographes de « confirmer »
vivent désormais dans la table, ce qui est la table de commandes qui gagne son
salaire : une bizarrerie de compatibilité coûte une ligne de données. La leçon
plus profonde est que « brut » veut dire que le programme voit les octets tels
que le matériel les a envoyés — chaque traduction que le terminal faisait
(CR→NL, XON/XOFF, les bizarreries du retour arrière) est désormais votre
contrat avec le clavier.

*Page traduite de la version anglaise `book/solutions/lesson-024/ex2.md`,
révision `bd716a9`.*

<!-- translation-source: book/solutions/lesson-024/ex2.md @ bd716a9 -->
