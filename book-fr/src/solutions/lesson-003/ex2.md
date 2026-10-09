# Solution : exercice 2 — La ligne citée ment

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La ligne citée ment](../../lessons/part-0/lesson-003-char-buffers.md) de la leçon 003.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-003/ex2.patch}}
```

## Visite guidée

La tentative va mal de quatre façons, et le symptôme des deux premières est le
`"hi thererld"` que vous avez vu : la copie tourne pour *chaque* ligne au lieu
de seulement quand un nouveau plus long est trouvé, et elle écrit `len`
caractères mais jamais le NUL. Après la deuxième ligne de `story.txt`, le
tampon lit `hi there` de la nouvelle copie suivi de `rld` — des restes de
l'ancien, plus long `hello world` — et `%s` marche droit à travers. Troisième
point, rien n'enregistre la dernière ligne quand le fichier manque de retour
à la ligne final, aussi `partial.txt` cite-t-il la camelote que la pile
contient (sur une construction tranquille, souvent rien du tout). Quatrième
point, `longest_text[128]` ne peut pas contenir une ligne que le tampon
`line` a acceptée — la copie écrit jusqu'à 255 octets dans 128, un
débordement de pile qui se trouve être inoffensif sur cette construction ;
les leçons 005 et 006 enseignent les outils qui font hurler de telles choses.

La correction fait les choses évidentes et justes : un tampon de la taille de
`line`, `longest_text[0] = '\0'` dès le départ pour qu'un fichier vide cite du
vide, la copie sous la garde du nouveau-plus-long et copiant `i <= len` pour
que le terminateur suive, et le même enregistrement dans la branche de la
ligne finale. Vérifié sur toutes les entrées piégeuses à la fois :

```
$ ./wordcount story.txt
longest line: "hello world"
2 4 21 11 story.txt
$ ./wordcount partial.txt empty.txt
longest line: "no newline"
0 2 10 10 partial.txt
longest line: ""
0 0 0 0 empty.txt
```

*Page traduite de la version anglaise `book/solutions/lesson-003/ex2.md`,
révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-003/ex2.md @ bb8d4ab -->
