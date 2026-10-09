# Solution : exercice 1 — Comptez dessus

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Comptez dessus](../../lessons/part-0/lesson-011-hashtable.md) de la leçon 011.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-011/ex1.patch}}
```

## Visite guidée

La prédiction, vérifiée contre la vraie exécution :

```
and 1
bird 1
cat 1
dog 1
don 1
stop 1
t 1
the 3
```

Huit lignes, et chacune est une conséquence des deux règles silencieuses du
découpeur. La casse plie d'abord : `The` et `AND` comptent comme `the` et
`and`, ce qui fait que `the` atteint 3. La ponctuation sépare : `Don't` n'est
pas un mot pour ce compteur — l'apostrophe est un séparateur, aussi les
lettres avant et après deviennent *deux* mots, `don` et `t`. C'est la paire
dont l'énoncé vous avertissait, et elles apparaissent comme des entrées
ordinaires avec un compteur de 1 chacune. Le `.` et le `!` de fin de ligne
séparent de la même façon et ne laissent rien derrière.

Le patch de confirmation journalise chaque jeton au moment où le découpeur
l'émet :

```
token: the
token: cat
token: and
...
token: don
token: t
```

Dix jetons à partir de neuf mots apparents — et notez que le journal est sur
`stderr`, aussi les compteurs sur `stdout` restent-ils propres et
tubables. Que `don`/`t` soit un défaut ou une définition dépend de votre
texte : les vrais compteurs de mots pré-traitent les apostrophes exactement à
cause de cela. La table elle-même s'en moque — pour `HtPut` ce sont juste des
clés.

*Page traduite de la version anglaise `book/solutions/lesson-011/ex1.md`,
révision `11ce1ee`.*

<!-- translation-source: book/solutions/lesson-011/ex1.md @ 11ce1ee -->
