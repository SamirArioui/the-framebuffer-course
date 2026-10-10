# Solution : exercice 4 — Le pixel qui n'est pas là

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Le pixel qui n'est pas là](../../lessons/part-0/lesson-013-raw-bytes.md) de la leçon 013.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-013/ex4.patch}}
```

## Visite guidée

`(0, 6)` calcule le décalage `6 * 24 = 144` — exactement un de trop pour un
tampon de 144 octets. Sans la garde le programme « fonctionne » : l'écriture
atterrit dans le remplissage du tas et `HexDump` ne la voit jamais. Sous
`-fsanitize=address` elle ne peut pas se cacher :

```
==98527==ERROR: AddressSanitizer: heap-buffer-overflow on address ...
WRITE of size 1 at ... thread T0
```

AddressSanitizer aborte au premier octet empoisonné touché, avec une trace de
pile nommant `PutPixel` et `main` — bien mieux qu'une image corrompue trois
fonctions plus tard. La correction fait que la fonction veille elle-même à son
contrat : une coordonnée hors de `w × h` est refusée avec un message sur
`stderr` et sans écriture, et la même exécution est propre sous
AddressSanitizer. `GetPixel` mérite la garde identique — une lecture hors
limites est tout aussi indéfinie. Une réserve à retenir : la garde transforme
la corruption silencieuse en plainte, mais elle coûte une branche par pixel ;
les vrais moteurs gardent des écrivains bruts et découpent aux sites d'appel —
le motif que la leçon 015 rend explicite.

*Page traduite de la version anglaise `book/solutions/lesson-013/ex4.md`,
révision `66eaafc`.*

<!-- translation-source: book/solutions/lesson-013/ex4.md @ 66eaafc -->
