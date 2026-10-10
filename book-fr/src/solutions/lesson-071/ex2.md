# Solution : exercice 2 — La table réécrite

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La table réécrite](../../lessons/part-4/lesson-071-table.md) de la leçon 071.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-071/ex2.patch}}
```

## Visite guidée

L'écrivain est l'inverse du lecteur, et le patch le construit comme tout
chargeur de ce moteur est construit : deux petits assistants `Put` dans l'espace
de noms anonyme à côté des assistants `Read`, puis une fonction qui assemble un
fichier. Là où `ReadInt` prend des chiffres sur la ligne, `PutInt` pose des
chiffres — les restes dans un petit tampon, puis écrits à l'envers, ce qui est
la même arithmétique que le lecteur de la leçon 052 a menée dans l'autre sens.
Là où `ReadText` copie une suite d'octets sans espace dans un champ, `PutText`
copie les octets du champ vers l'extérieur. Si ces deux-là ne s'accordent pas
sur ce qu'*est* une valeur, rien d'autre dans le format ne peut les sauver.

`WriteTable` écrit la marche de la leçon dans l'ordre : l'en-tête que le format
de cette leçon nomme, un espace entre les noms de colonnes et un saut de ligne
après le dernier, puis une ligne par définition — name, x, y, facing, speed,
health, sprite — chaque valeur suivie du séparateur que le tokeniseur du lecteur
saute. Chaque champ qu'un lecteur vérifiera est écrit pour passer cette
vérification : l'écrivain n'est pas un second avis sur le format, c'est le même
avis dans l'autre sens. Notez que l'écrivain choisit une mise en page — l'ordre
canonique des colonnes du format — tandis que le lecteur accepte tout ordre que
l'en-tête nomme ; ils s'accordent sur les *champs*, ce qui est ce que « le
format » veut dire.

Une habitude à remarquer : les octets assemblés sont de purs jetables, donc la
marque et le retour arrière les encadrent — gardé ou refusé, l'arena ne conserve
pas l'image du fichier. `platform::WriteFile` prend les octets avant le retour
arrière, exactement comme les octets de `ReadFile` repartent avec
`ReleaseFile`.

L'aller-retour dans `Run` écrit la table chargée dans le frère de
`assets/entities.txt`, `assets/table-roundtrip.txt`, la relit avec `LoadTable`,
et compare chaque champ de chaque définition — en nommant le premier champ en
désaccord. D'une vraie exécution de l'état final de cette leçon plus le patch :

```
engine: table: 2 definitions
engine: def hero: x 312 y 232 facing 0 speed 240 health 3 sprite assets/sprite.ppm
engine: def slime: x 400 y 320 facing 2 speed 96 health 1 sprite assets/sprite.ppm
engine: round trip: 2 definitions written and read back, every field agrees
```

Et le fichier lui-même, comparé à l'asset du cours en dehors de l'exécution
(`cmp assets/table-roundtrip.txt assets/entities.txt`) : **identique octet pour
octet** — en-tête et lignes, tout. L'écrivain n'a pas seulement produit un
fichier que le chargeur tolère ; il a produit *ce* fichier.

Le chemin en désaccord est réel lui aussi, et bon marché à voir : faites que
l'écrivain mette `def.health + 1` dans l'image au lieu de `def.health`,
recompilez, et l'exécution dit
`engine: round trip: definition 0 disagrees on health` — le premier champ qui
diffère, nommé, avec la définition à laquelle il appartient. Cette sonde ne fait
pas partie du patch ; c'est la vérification de cinq minutes qui vérifie que la
comparaison compare.

Voilà ce que l'aller-retour prouve qu'une simple lecture ne peut pas prouver.
Une lecture montre une implémentation du format face à un fichier ; l'aller-retour
met deux implémentations face à un vrai fichier et vérifie qu'elles s'accordent
sur chaque champ. Un vidage hexadécimal du fichier écrit
n'attraperait pas un bogue d'ordre des chiffres dans `PutInt` (le fichier aurait
encore « l'air » d'une table) ; la comparaison de 240 contre 240 l'attrape
aussitôt. Quand vous écrirez plus tard une table que le chargeur refuse —
décalez les valeurs d'une colonne par rapport à l'en-tête, écrivez un `facing`
de 4 — les deux moitiés se disputeront en public, et l'échec typé nommera quelle
déclaration a perdu.

Rien ici ne touche le chargeur, l'analyse ou le format : l'aller-retour est une
vérification à côté du chargement, et le fichier qu'il écrit est à vous, à
garder ou à supprimer.

*Page traduite de la version anglaise `book/solutions/lesson-071/ex2.md`,
révision `84af294`.*

<!-- translation-source: book/solutions/lesson-071/ex2.md @ 84af294 -->
