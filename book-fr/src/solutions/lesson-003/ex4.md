# Solution : exercice 4 — Le débogueur sur votre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Le débogueur sur votre machine](../../lessons/part-0/lesson-003-char-buffers.md) de la leçon 003.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-003/ex4.patch}}
```

## Visite guidée

L'instrument affiche chaque longueur que `LineLen` calcule, aussi
confirme-t-il ce que vos pas à pas disent sur n'importe quelle machine et
n'importe quel débogueur :

```
$ ./wordcount story.txt
LineLen: 11
LineLen: 8
2 4 21 11 story.txt
```

S'arrêter sur `LineLen` sous gdb sur cette machine montre la marche
directement — notez que gdb imprime la chaîne à laquelle un `char *` pointe
dans le cadre de la trame :

```
(gdb) break LineLen
Breakpoint 1 at 0x1219: file wordcount.c, line 16.
(gdb) run story.txt
...
Breakpoint 1, LineLen (s=0x7fffffffd830 "hello world") at wordcount.c:16
16	    unsigned long n = 0;
(gdb) next
17	    while (s[n] != '\0')
(gdb) next
18	        ++n;
(gdb) print n
$1 = 0
```

La même session sous lldb est un exercice d'orthographe. Les traductions à
essayer (vérifiez `help` sur votre machine — ce livre vérifie gdb et ne cite
que des sorties de gdb) : `gdb ./wordcount` → `lldb ./wordcount` ;
`break LineLen` → `breakpoint set --name LineLen` ; `run`, `next`, `step`,
`finish` et `continue` gardent leur nom ; `backtrace` → `bt` ;
`info locals` → `frame variable` ; `print n` → `print n`. Les développeurs
Windows rencontrent le débogueur dans Visual Studio ou WinDbg, où les cinq
mêmes gestes — point d'arrêt, exécution, pas à pas, inspection, backtrace —
sont tous présents sous d'autres noms. Rapportez tout ce que vous n'avez pas
pu traduire ; cette liste est une carte de l'outillage que le cours gardera
en marge.

*Page traduite de la version anglaise `book/solutions/lesson-003/ex4.md`,
révision `1772957`.*

<!-- translation-source: book/solutions/lesson-003/ex4.md @ 1772957 -->
