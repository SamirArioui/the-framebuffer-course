# Solution : exercice 1 — L'en-tête qui ment

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — L'en-tête qui ment](../../lessons/part-2/lesson-044-sprite-bytes.md) de la leçon 044.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-044/ex1.patch}}
```

## Visite guidée

D'abord, la prédiction. L'en-tête de la copie corrompue dit `P6 / 32 32 / 255`
— chaque champ *s'analyse* encore : les octets magiques sont bons, `32` et `32`
sont des nombres parfaitement valides, `255` est la valeur maximale que le
chargeur attend. Rien dans l'en-tête n'a l'air d'un mensonge. Le mensonge ne
devient visible que lorsque l'annonce de l'en-tête rencontre la longueur du
fichier : `32 × 32 × 3 = 3072` octets de pixels annoncés, 768 présents. La
vérification qui refuse est donc **le compte des pixels**, et le rapport doit le
dire.

Le diff scinde l'unique `SPRITE_MALFORMED` en les deux mensonges que
l'analyseur peut réellement distinguer — `SPRITE_BAD_HEADER` (les champs ne
forment pas un en-tête P6) et `SPRITE_TRUNCATED` (l'en-tête est bon ; les pixels
ne sont pas tous là) — et redonne ses arguments à `Run` pour qu'on puisse
braquer le chargeur vers n'importe quel fichier. Puis les trois mensonges,
chacun se nommant lui-même :

```
$ ./build/game lying-32.ppm
engine: lying-32.ppm: the pixel bytes do not fill the header's claim
$ ./build/game bad-magic.ppm
engine: bad-magic.ppm: the header fields are not a P6 header
$ ./build/game no-such.ppm
engine: no-such.ppm: missing or unreadable
```

`lying-32.ppm` est la confirmation de la prédiction : les champs de l'en-tête
sont passés, le compte a refusé. (`bad-magic.ppm` commence par `P5` — la
variante *texte* du PPM, dont les pixels sont des nombres décimaux ; un octet
d'écart dans les octets magiques, un format complètement différent.) L'exécution
par défaut charge toujours `assets/sprite.ppm` et imprime les mêmes quatre
lignes d'inspection que la leçon.

Maintenant la question que posent les octets. Si le chargeur avait fait
confiance à l'en-tête, il aurait cru à 3072 octets de pixels et il en aurait
copié autant depuis un tampon de fichier qui n'en contient que 768. Les 2304
octets en trop ne sont ni des zéros ni une erreur — ce sont **tout ce que le
tampon de fichier de l'OS contenait par hasard au-delà de la fin du fichier**.
Le sprite aurait été dessiné avec 2304 octets de mémoire étrangère comme lignes
du bas : des ordures qui changent d'une exécution à l'autre, d'une machine à
l'autre, d'une taille de fichier à l'autre.

Et ça empire au-delà des ordures. Les octets du tampon de fichier repartent vers
l'OS avec `ReleaseFile` dès la copie terminée — c'est la règle de propriété de
la prose de la leçon 044. Un chargeur qui rapporterait « succès » alors que ses
pixels dépassent le fichier serait un sprite dont les octets incluent de la
mémoire que le moteur a déjà rendue : le territoire aveugle à ASan de la leçon
041, affublé d'un costume 16×16. La vérification du compte n'est pas du zèle ;
c'est la ligne entre un échec typé et une séance de débogage qui finit en
chapitre sur le comportement indéfini.

C'est l'habitude que l'exercice vise : *l'en-tête est une affirmation, le
fichier est la preuve, et le chargement ne procède que lorsqu'elles sont
d'accord.*

*Page traduite de la version anglaise `book/solutions/lesson-044/ex1.md`,
révision `7e8ed7f`.*

<!-- translation-source: book/solutions/lesson-044/ex1.md @ 7e8ed7f -->
