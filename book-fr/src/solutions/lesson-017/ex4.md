# Solution : exercice 4 — Par octet contre par ligne

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Par octet contre par ligne](../../lessons/part-0/lesson-017-image-file.md) de la leçon 017.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-017/ex4.patch}}
```

## Visite guidée

`WriteBmpFast` fait le même travail — échange BGR et remplissage inclus —
mais dans un tampon de ligne émis avec un seul `fwrite` par ligne. Sur le
dégradé de 1024×1024 (3,1 Mo de pixels), cinq répétitions chacun, cette
machine rapporte

```
putc per byte: 7.7 ms/write
fwrite rows:   4.6 ms/write
```

les exécutions atterrissant dans les bandes 6,8-8,7 et 4,3-4,9 ms :
l'écrivain par ligne est environ 1,6× plus rapide, et
`cmp bench1.bmp bench2.bmp` confirme que les fichiers sont identiques octet
par octet. Où va le temps ? Pas sur disque — le cache de pages absorbe les
deux. L'écrivain par octet fait 3,1 millions d'appels à `putc`, chacun un
appel de fonction avec une vérification d'argument et une étape de comptabilité
de tampon ; l'écrivain par ligne fait mille `fwrite` plus la même boucle
d'échange. L'écart est du coût d'appel, pas de la bande passante mémoire — ce
qui explique aussi que ce soit un modeste 1,6× et pas le folklore du 10× et
plus des temps du stdio non mis en tampon. Mesurez sur votre machine ; si
`fwrite` n'y est pas en tête, la mesure mérite d'être lue deux fois.

*Page traduite de la version anglaise `book/solutions/lesson-017/ex4.md`,
révision `c6657c1`.*

<!-- translation-source: book/solutions/lesson-017/ex4.md @ c6657c1 -->
