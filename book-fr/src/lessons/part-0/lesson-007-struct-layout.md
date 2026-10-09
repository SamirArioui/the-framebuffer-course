# Leçon 007 — structures : sizeof, alignement et remplissage

{{#include ../../stability-horizon.md}}

## Prose

Cette leçon démarre le deuxième programme de la partie 0 : `ds-kit`, un petit
kit de structures de données qui grandit, leçon après leçon, jusqu'à devenir
un compteur de fréquences de mots sur un fichier texte. Aujourd'hui, le kit
est un type et un ensemble de questions sur lui. Le type est l'élément que le
kit stockera :

```c
struct Item {
    char key[16];
    long value;
};
```

Si vous venez de Python ou de Ruby, une structure n'est pas une classe. Elle
n'a ni méthodes, ni références, ni en-tête caché : une structure est une
*étendue de mémoire typée*, et sa déclaration est une recette de disposition
que le compilateur suit pour décider quel octet contient quel champ.
`item.key` et `item.value` avec un point ne sont pas des lectures d'attribut
à travers un dict — ce sont des décalages que le compilateur a déjà calculés.
Une structure est une valeur : en affecter une à une autre copie chaque octet,
et en passer une à une fonction la copie à nouveau. Rien n'est partagé dans
votre dos.

Demandez à la machine ce que la recette a produit et elle répond à la
compilation. `sizeof(struct Item)` est le nombre total d'octets qu'occupe la
structure ; `offsetof(struct Item, value)` — de `<stddef.h>` — est la distance
en octets depuis le début de la structure jusqu'à ce champ ;
`alignof(struct Item)` — le `<stdalign.h>` de C11 — est l'alignement que la
structure exige de son adresse. Le `ds-kit.c` de cette leçon imprime
exactement ces observations, plus une comparaison qui est le vrai sujet du
jour.

Parce que l'alignement n'est pas l'invention du compilateur — c'est la règle
de la machine. Chaque type a un **alignement** : une adresse où il doit vivre,
une puissance de deux. Un `int` veut une frontière de 4 octets, un `long` une
frontière de 8 octets sur cette machine. La règle, en entier : chaque membre
se place à un décalage multiple de son propre alignement ; l'alignement de la
structure est le plus grand de ceux de ses membres ; la taille de la
structure est arrondie à un multiple de cet alignement pour que les éléments
consécutifs d'un tableau restent alignés aussi. Quand la recette laisse un
membre vouloir une place que le membre précédent occupe déjà, le compilateur
insère du **remplissage** — des octets qui n'appartiennent à aucun champ.

C'est ce que démontrent `struct Scattered` et `struct Compact` dans le
programme : les mêmes trois champs, un `char`, un `long`, et un `char`, dans
deux ordres. `Scattered` place `score` au décalage 8 — sept octets de
remplissage après `tag` — et en gaspille sept autres après `flag`, si bien
qu'un élément fait 24 octets et dix d'entre eux 240. `Compact` met les deux
`char` ensemble d'abord et ne paie que le trou intérieur avant `score` :
16 octets par élément, 160 pour dix. Réordonner les champs a économisé 40 %
sans changer une seule ligne de logique de programme. C'est la seule
optimisation de ce livre que vous obtenez gratuitement.

Deux mises en garde avant d'exécuter. D'abord, la machine décide les nombres :
le programme imprime `sizeof(struct Item)` comme 24 ici parce que `long` fait
8 octets sur cette machine. Sur une construction 32 bits, ou sur Windows
64 bits où `long` fait 4 octets, le même source imprime 20. Rien ne s'est
cassé — l'alignement est la règle de la *machine*, et le compilateur obéit
simplement à la machine que vous avez demandée. Ensuite, le remplissage est
une raison pour laquelle les octets d'une structure ne sont pas un format de
fichier : faites un `fwrite` d'un `Scattered` sur disque et relisez-le sur une
machine avec des alignements différents et vous lisez de la camelote. Les
dispositions d'octets sont l'affaire de la leçon 013 ; retenez seulement
ceci : les structures sont pour la mémoire, les formats sont pour les octets.

Une commande de construction, depuis l'intérieur de `sandbox/ds-kit/`, avec
l'ensemble d'options que vous connaissez déjà de `wordcount` — `-O0 -g` est
arrivé avec gdb, et le reste avec la première commande que vous ayez jamais
exécutée :

```
gcc -std=c11 -O0 -g -Wall -Wextra ds-kit.c -o ds-kit
```

Exécutez `./ds-kit` et lisez ses observations à côté de ce texte.

## Étape de code

Un seul changement pour cette leçon : la totalité de `ds-kit.c`, commitée avec
ce texte. Son état final est étiqueté `lesson-007`.

```diff
diff --git a/sandbox/ds-kit/ds-kit.c b/sandbox/ds-kit/ds-kit.c
new file mode 100644
index 0000000..455e975
--- /dev/null
+++ b/sandbox/ds-kit/ds-kit.c
@@ -0,0 +1,57 @@
+// ds-kit.c — the data-structures kit of Part 0, starting with its element
+// type and the memory it lives in.
+//
+// Lesson 007: structs as laid-out memory — sizeof, offsetof, padding.
+#include <stdio.h>
+#include <stddef.h>   // offsetof
+#include <stdalign.h> // alignof (C11)
+
+struct Item {
+    char key[16];
+    long value;
+};
+
+// The same three fields in two different orders.
+struct Scattered {
+    char tag;
+    long score;
+    char flag;
+};
+
+struct Compact {
+    char tag;
+    char flag;
+    long score;
+};
+
+int main(void)
+{
+    printf("== scalars ==\n");
+    printf("sizeof(char) = %zu\n", sizeof(char));
+    printf("sizeof(long) = %zu\n", sizeof(long));
+
+    printf("== struct Item ==\n");
+    printf("sizeof(struct Item)  = %zu\n", sizeof(struct Item));
+    printf("alignof(struct Item) = %zu\n", alignof(struct Item));
+    printf("Item.key   offset %zu\n", offsetof(struct Item, key));
+    printf("Item.value offset %zu\n", offsetof(struct Item, value));
+    // key is 16 bytes, so the gap before value is its offset minus 16.
+    printf("padding before value: %zu bytes\n",
+           offsetof(struct Item, value) - 16);
+
+    printf("== same fields, two orders ==\n");
+    printf("struct Scattered { tag, score, flag }: sizeof %zu\n",
+           sizeof(struct Scattered));
+    printf("  tag   offset %zu\n", offsetof(struct Scattered, tag));
+    printf("  score offset %zu\n", offsetof(struct Scattered, score));
+    printf("  flag  offset %zu\n", offsetof(struct Scattered, flag));
+    printf("struct Compact { tag, flag, score }: sizeof %zu\n",
+           sizeof(struct Compact));
+    printf("  tag   offset %zu\n", offsetof(struct Compact, tag));
+    printf("  flag  offset %zu\n", offsetof(struct Compact, flag));
+    printf("  score offset %zu\n", offsetof(struct Compact, score));
+    printf("Scattered[10] = %zu bytes, Compact[10] = %zu bytes\n",
+           sizeof(struct Scattered[10]), sizeof(struct Compact[10]));
+
+    return 0;
+}
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Du remplissage dans une structure neuve *(predict-the-output)*

Ajoutez un `struct Triple` contenant exactement un `char a`, un `int b`, et un
`char c`, dans cet ordre, et imprimez son `sizeof` avec les décalages de `b`
et `c`. Avant de compiler quoi que ce soit, notez les trois nombres que vous
attendez sur votre machine — votre premier réflexe pour la taille est très
probablement faux. Puis construisez, exécutez, et accordez chaque différence
entre prédiction et sortie avec la règle d'alignement de cette leçon, en
montrant l'arithmétique.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-007/ex1.md)

### Exercice 2 — Le remplissage est de la vraie mémoire *(extend-the-code)*

Les octets de remplissage vivent à l'intérieur de l'objet — prouvez-le.
Ajoutez un auxiliaire `DumpBytes` qui imprime n'importe quelle région de
mémoire en octets hexadécimaux à deux chiffres séparés par des espaces, et
videz exactement `sizeof(struct Scattered)` octets d'une instance de
`Scattered` dont vous venez d'affecter les trois membres à la main. Quels
octets appartiennent aux membres, et lesquels sont de la camelote ? Exécutez
deux fois et vérifiez si la camelote est stable. Puis faites un `memset` de
la structure vers zéro avant d'affecter les membres, videz à nouveau, et
expliquez la différence.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-007/ex2.md)

### Exercice 3 — Pourquoi la machine insiste *(explain-in-prose)*

Écrivez une courte explication — un paragraphe chacune — de (1) pourquoi le
compilateur place `score` de `Scattered` au décalage 8 au lieu de juste après
`tag` ; (2) ce qui se passerait mal au niveau du CPU s'il plaçait `score` au
décalage 1 ; (3) pourquoi écrire des structures `Scattered` dans un fichier
avec `fwrite` et les relire sur une autre machine n'est pas un format
portable. Appliquez ensuite la vérification confirmante de la solution et
assurez-vous que votre explication s'accorde avec les nombres qu'elle imprime.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-007/ex3.md)

### Exercice 4 — La disposition de l'autre machine *(port-to-your-own-machine)*

Sur votre machine `struct Item` fait 24 octets avec `value` au décalage 16,
parce que `long` fait 8 octets ici. Prédisez `sizeof(struct Item)`, le
décalage de `value`, et `sizeof(struct Scattered)` sur une machine où `long`
fait 4 octets — une construction 32 bits ou Windows 64 bits. Vérifiez partout
où vous pouvez en atteindre une : `gcc -m32` sur un système avec les
bibliothèques 32 bits, la machine d'un ami, un conteneur. Si aucune telle
machine n'est atteignable, vérifiez à la main depuis la règle d'alignement et
montrez votre travail. La vérification de la solution imprime une empreinte
de machine ; lancez-la sur chaque machine que vous touchez et comparez.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-007/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 006 — comportement indéfini et débordements de tampon](lesson-006-undefined-behavior.md) ·
**Suivante :** [Leçon 008 — croissance de dynarray : realloc et capacité](lesson-008-dynarray.md) ·
**Étiquette de code :** [`lesson-007`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-007)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-007-struct-layout.md`, révision `fa0fc1a`.*

<!-- translation-source: book/lessons/part-0/lesson-007-struct-layout.md @ fa0fc1a -->
