# Solution : exercice 3 — Pourquoi la machine insiste

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — Pourquoi la machine insiste](../../lessons/part-0/lesson-007-struct-layout.md) de la leçon 007.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-007/ex3.patch}}
```

## Visite guidée

Les trois réponses, dans l'ordre. (1) `score` va au décalage 8 parce qu'il est
un `long` et que la règle de la machine — l'ABI, en fait un contrat entre le
compilateur et le CPU — dit qu'un `long` vit à une adresse multiple de 8. Le
décalage 8 est la première telle place après `tag`. (2) Au décalage 1, les
instructions de chargement du CPU devraient aller chercher `score` à travers
deux mots alignés : sur x86 cela fonctionne et c'est plus lent ; sur les
architectures plus strictes, un accès non aligné déclenche une faute. Les
compilateurs ne le choisissent donc jamais — l'alignement est cuit dans la
convention d'appel et dans chaque disposition de structure. (3) Un dump
`fwrite` contient les membres *et* le remplissage, disposés pour la taille de
`long` et le boutisme de cette machine. Une autre machine n'est d'accord sur
ni l'un ni l'autre, aussi les octets se relisent-ils comme du bruit.

La vérification confirmante imprime les nombres sur lesquels tourne la règle :

```
alignof(char) = 1, alignof(int) = 4, alignof(long) = 8
```

Les décalages sont des multiples de ceux-ci, les tailles de structures des
multiples du plus grand — chaque disposition de cette leçon découle de cette
seule ligne.

*Page traduite de la version anglaise `book/solutions/lesson-007/ex3.md`,
révision `fa0fc1a`.*

<!-- translation-source: book/solutions/lesson-007/ex3.md @ fa0fc1a -->
