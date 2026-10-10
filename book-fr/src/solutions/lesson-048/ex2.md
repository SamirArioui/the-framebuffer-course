# Solution : exercice 2 — La signature du compilateur

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La signature du compilateur](../../lessons/part-2/lesson-048-assembly.md) de la leçon 048.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-048/ex2.patch}}
```

## Visite guidée

Le patch ajoute une ligne au rapport de démarrage — la macro prédéfinie du
compilateur lui-même, `__VERSION__`, imprimée à côté des nombres qu'il a
produits :

```
engine: compiler 13.3.0
```

C'est GCC 13.3.0 — la même chaîne d'outils que celle qui a mesuré les
tableaux du livre. Désormais, chaque exécution que votre machine produit dit
quel compilateur a fabriqué le code qu'elle chronomètre ; deux exécutions
qui diffèrent en *forme* (pas seulement en chiffres) se comparent d'abord
par leur signature. Une mesure sans sa machine est une rumeur, et le
compilateur fait partie de la machine.

Maintenant la comparaison elle-même. Construisez avec votre compilateur et
lancez `./tools/disasm.sh` ; puis parcourez les trois mêmes repères que la
leçon a lus dans la liste de GCC :

1. **Le prologue et les déversements d'arguments.** Comment votre
   compilateur ramène-t-il `fb`, `sprite`, `x`, `y` à la maison ? Le x86-64
   fixe les *registres* — `%rdi`, `%rsi`, `%edx`, `%ecx` relèvent de l'ABI
   de la plateforme, pas de l'idée d'un compilateur — mais que les valeurs
   soient déversées immédiatement vers `-0xNN(%rbp)`, restent dans des
   registres ou soient copiées deux fois est entièrement l'affaire du
   compilateur. À `-O0`, la plupart des compilateurs déversent (c'est ce que
   veut dire « pas d'optimisation »), et les décalages d'emplacements
   différeront.
2. **Les bornes du découpage.** La colonne du livre montre `test`/`cmovs` et
   `cmovle` — des déplacements conditionnels, sans branchement. Un autre
   compilateur à `-O0` peut émettre comparaison-et-saut à la place :
   charger, `test`, `jns` par-dessus un `mov`, puis continuer. Même
   comportement, forme différente ; ici un branchement se prédit
   trivialement (la borne ne se déclenche presque jamais), donc la
   différence n'a pas d'importance — jusqu'à ce qu'elle en ait une, et c'est
   pourquoi on lit plutôt qu'on ne suppose.
3. **Les quatre stockages de la copie.** Attendez-vous au même ordre de
   stockage (l'ordre des énoncés est observable — le compilateur n'a pas le
   droit de réordonner les stockages vers la même adresse d'une manière que
   le C++ interdit) et au même décalage `× 4` pour l'adresse de destination.
   Ce qui *différera*, c'est quels registres transportent les octets et
   comment l'arithmétique d'adresse est repliée — les instructions `lea` qui
   calculent `base + index*4 + 1` en une seule étape sont courantes.

La ligne de partage que l'énoncé demande : **les noms de registres, les
adresses et les choix d'instructions sont au compilateur ; le comportement
qu'ils produisent est au langage.** Le contrat de `BlitSprite` — quels
pixels changent et lesquels ne changent pas — doit sortir identique de la
liste de chaque compilateur, sinon le compilateur est cassé. La liste est une
opinion ; les pixels sont un fait. C'est la règle du comme-si de la leçon
018, et c'est pourquoi ce cours vous apprend à lire la liste au lieu d'en
mémoriser une.

Un repère de plus, à ajouter à votre comparaison pendant que vous y êtes :
la taille des fonctions. Passez `wc -l` sur le désassemblage de `BlitSprite`
et de `ClearBuffer` sur les deux compilateurs. La taille des fonctions est
l'endroit où les personnalités des compilateurs se montrent le plus
clairement, et « mon compilateur a fait la boucle deux fois plus longue »
est le début d'une conversation sur la génération de code que la leçon 049
poursuit.

*Page traduite de la version anglaise `book/solutions/lesson-048/ex2.md`,
révision `38c0b7a`.*

<!-- translation-source: book/solutions/lesson-048/ex2.md @ 38c0b7a -->
