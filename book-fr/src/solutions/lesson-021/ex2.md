# Solution : exercice 2 — La touche qui a disparu

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La touche qui a disparu](../../lessons/part-0/lesson-021-terminal-input.md) de la leçon 021.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-021/ex2.patch}}
```

## Visite guidée

`printf '\033q' | ./snek 30` qui fait tourner les trente frames est tout le
symptôme : après un ESC solitaire, l'état moyen du parseur teste l'octet
suivant contre `'['` et, en cas de désaccord, le jette discrètement — le `q`
est mangé et le jeu ne quitte jamais. Le même trou avale toute touche pressée
juste après la touche Échap.

La correction restructure `OnByte` autour d'une règle : **un octet inattendu
n'est pas un octet de séquence, aussi doit-il être retraité comme une touche
ordinaire**. Les anciens contrôles d'état montent en tête et ne renvoient que
quand ils consomment réellement un octet ; quand l'état 1 voit autre chose que
`[`, il se réinitialise et retombe dans la gestion des touches ordinaires en
dessous — où le même octet est testé contre `0x1b` et `q` comme n'importe quel
autre. L'état 2 consomme toujours un octet inconditionnellement (c'est l'octet
final de la séquence) et mappe `A`-`D` aux directions.

Après la correction, `ESC q` quitte à la frame 1, `q` et `ESC [ A q` se
comportent exactement comme avant, un ESC solitaire laisse toujours le jeu
tourner, et même `ESC ESC [ A` se remet : le second ESC retombe et redémarre
la séquence proprement.

*Page traduite de la version anglaise `book/solutions/lesson-021/ex2.md`,
révision `2369864`.*

<!-- translation-source: book/solutions/lesson-021/ex2.md @ 2369864 -->
