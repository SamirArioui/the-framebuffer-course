# Leçon 002 — gdb : points d'arrêt, pas à pas, trames de pile

{{#include ../../stability-horizon.md}}

## Prose

Le sujet de cette leçon n'est pas `wordcount` ; c'est la mécanique qui fait
tourner votre programme quand votre programme est faux. Jusqu'ici, le seul
outil d'observation a été `fprintf` : afficher une valeur, deviner, réafficher.
Cela fonctionne jusqu'au moment où le bug est un plantage dans du code que
vous n'avez pas écrit, ou une valeur qui est correcte à une extrémité d'une
fonction et fausse à l'autre. Un débogueur remplace la devinette par
l'observation : il démarre le programme, l'arrête là où vous dites, et vous
montre l'état de la machine exactement tel qu'il est à cet instant.

Commençons par la forme d'un appel. Quand `main` appelle `CountBytes`, le
contrôle ne se téléporte pas — la machine empile une *trame de pile* sur la
pile d'appels : les arguments en cours de transmission (`f`), les variables
locales de l'appelé (`bytes`, `c`), et l'adresse de retour qui dit où reprendre
dans `main`. Quand `CountBytes` renvoie, sa trame est dépilée et `main`
continue depuis exactement là. Python et Ruby vous montrent cette chaîne gelée
au pire moment possible : la trace d'appel imprimée avec une exception, la
trame la plus interne en premier. C a la même chaîne à chaque instant de
chaque exécution, plantage ou non, et `backtrace` l'imprime à la demande. Le
gain au débogage : chaque trame est sa propre portée. `bytes` et `c` vivent
dans la trame de `CountBytes`, `i` et `argv` dans celle de `main`, et
s'interroger sur une variable veut vraiment dire s'interroger *dans quelle
trame*.

C'est pour cela que l'étape de code de cette leçon existe. La boucle de
comptage qui vivait dans `main` vit désormais dans sa propre fonction,
`static unsigned long CountBytes(FILE *f)`, appelée une fois par fichier — non
par propreté, pour le débogueur. Une boucle dans `main` n'a pas de trame à
elle : il n'y a rien *dans quoi* mettre un point d'arrêt, aucune frontière
entre « le code qui compte » et tout le reste que fait `main`, et rien qu'une
backtrace puisse nommer. Une fonction donne à ce code un nom sur lequel
s'arrêter, des arguments à inspecter, et sa propre trame pour apparaître entre
`main` et la bibliothèque C dans une trace de pile. Le mot-clé `static` est un
détail de classe de stockage à garder : il donne à la fonction un *lien
interne*, ce qui veut dire que le nom est visible dans ce fichier et nulle part
ailleurs — dans un programme d'un seul fichier, c'est surtout une note au
lecteur pour dire que ceci n'est pas une interface.

Maintenant, la commande de construction gagne deux options. Depuis
l'intérieur de `sandbox/wordcount/` :

```
gcc -std=c11 -O0 -g -Wall -Wextra wordcount.c -o wordcount
```

- `-g` écrit des *informations de débogage* dans l'exécutable : la
  correspondance des instructions machine aux positions du code source, les
  noms et emplacements de pile des variables, les informations de type dont
  `print` a besoin pour formater sa sortie. Sans `-g`, gdb peut toujours
  exécuter le programme et s'arrêter sur des adresses, mais chaque question de
  cette leçon revient avec « no debugging symbols ». Les informations de
  débogage coûtent de l'espace disque et rien au moment de l'exécution ; les
  binaires de distribution les laissent de côté.
- `-O0` désactive l'optimisation. L'optimisation, c'est le compilateur qui
  réarrange, fusionne et supprime des instructions pour rendre le programme
  plus rapide — utile, et fatal au débogage, car aux réglages supérieurs « la
  ligne de source suivante » ne veut plus rien dire : les instructions de
  plusieurs lignes s'entremêlent, et certaines lignes disparaissent
  complètement. `-O0` compile le code source tel qu'il est écrit, de sorte que
  pas à pas dedans, c'est pas à pas dans *lui*. Le coût est un binaire plus
  lent — c'est pour cela que les versions de production optimisent, et
  l'optimiseur revient comme contenu réel plus loin dans la partie 0.

Fabriquez deux fichiers — `printf 'hi\n' > a.txt` et `printf 'hello\n' > b.txt`
— et démarrez le débogueur avec `gdb ./wordcount`. gdb affiche une bannière et
quelques conseils ; à sa première exécution, le gdb d'Ubuntu demande aussi si
l'on veut activer *debuginfod*, un service de téléchargement à la demande des
fichiers de source et de débogage. Répondez `n` (ou `set debuginfod enabled
off` dans `~/.gdbinit`) et il ne redemandera plus. L'invite `(gdb)` est le
REPL du débogueur ; `quit` le quitte.

La première compétence est *s'arrêter quelque part volontairement* :

```
(gdb) break CountBytes
Breakpoint 1 at 0x11d9: file wordcount.c, line 8.
(gdb) run a.txt b.txt
...
Breakpoint 1, CountBytes (f=0x5555555592a0) at wordcount.c:8
8	    unsigned long bytes = 0;
(gdb) backtrace
#0  CountBytes (f=0x5555555592a0) at wordcount.c:8
#1  0x00005555555552d2 in main (argc=3, argv=0x7fffffffdaf8) at wordcount.c:29
```

`break CountBytes` pose le *point d'arrêt 1* sur la fonction — s'arrêter quand
le contrôle l'atteint. `run` démarre le programme ; quand le point d'arrêt
frappe, gdb s'arrête avant que la première ligne du corps de la fonction ait
été exécutée et vous montre où vous êtes. `backtrace` (ou `bt`) imprime la
chaîne des trames : la trame `#0` est là où le programme est maintenant, la
trame `#1` son appelant, et ainsi de suite jusqu'à `main`. Notez ce que chaque
trame vous montre : le nom de la fonction, ses arguments avec les valeurs
(`argc=3` parce que deux fichiers plus le nom du programme étaient sur la ligne
de commande), et où dans son source elle est en pause. Les adresses sont la
réalité sans ramasse-miettes : `f` est le pointeur renvoyé par `fopen`, le
nombre dans la trame `#1` est une adresse de retour. Les nombres exacts
dépendent de votre machine ; la forme, non.

La deuxième compétence est *poser des questions à un programme arrêté* :

```
(gdb) info locals
bytes = 93824992247200
c = 0
(gdb) next
10	    while ((c = fgetc(f)) != EOF)
(gdb) print bytes
$1 = 0
(gdb) next
11	        ++bytes;
(gdb) next
10	    while ((c = fgetc(f)) != EOF)
(gdb) print bytes
$2 = 1
```

`info locals` liste les variables locales de la trame courante — et regardez ce
qu'il montre pour `bytes` à l'instant précis où le point d'arrêt a frappé : de
la camelote, parce que la ligne `unsigned long bytes = 0;` n'a pas encore
tourné. La mémoire non initialisée n'est pas aléatoire ; c'est les bits déjà
présents dans cet emplacement de pile (dans cette exécution, ils se sont
trouvés être une valeur de pointeur). C ne nettoie pas les variables pour vous,
et un débogueur est assez honnête pour vous le montrer. `next` exécute une
ligne de source et s'arrête à nouveau ; `print` évalue une expression dans la
trame courante et numérote le résultat `$1`, `$2`, … pour que vous puissiez
vous y référer. Pas à pas deux fois dans la boucle fait passer `$1 = 0` à
`$2 = 1`, un octet compté à la fois.

`step` ressemble à `next` mais diffère de la seule façon qui compte — il suit
les appels *à l'intérieur* de la fonction appelée au lieu de passer au-dessus :

```
(gdb) step
0x00007ffff7c8f062 in _IO_getc (fp=0x5555555592a0) at ./libio/getc.c:37
warning: 37	./libio/getc.c: No such file or directory
(gdb) finish
0x00005555555551f4 in CountBytes (f=0x5555555592a0) at wordcount.c:10
10	    while ((c = fgetc(f)) != EOF)
Value returned is $3 = 105
```

Sauter dans l'appel à `fgetc` atterrit dans la bibliothèque C — et gdb, sans
les sources de glibc installées, peut vous dire où vous êtes mais pas vous
montrer le code (`disassemble` le ferait). `finish` exécute jusqu'à ce que la
trame courante renvoie, vous redépose dans l'appelant, et rapporte la valeur
que la trame a renvoyée (`105` est le deuxième octet de `a.txt` : `i`). Quand
vous sautez dans un endroit sans intérêt, `finish` est le chemin du retour.
Après cela, `continue` reprend le programme jusqu'au prochain point d'arrêt ou
jusqu'à la fin, et `quit` quitte gdb (il demandera s'il faut tuer le programme
en cours ; la réponse est `y`).

Deux derniers éléments de vocabulaire. `next`, `step`, `print`, `info`,
`continue` et `quit` fonctionnent depuis n'importe quel point d'arrêt — un
point d'arrêt qui frappe, un signal, la fin de `finish` — le *point d'arrêt* et
la *trame courante* sont ce sur quoi les commandes agissent, et `up`/`down`
changent la trame courante sans déplacer le programme. Et quand une session
doit être reproductible, gdb prend ses commandes en arguments : `gdb -batch -ex
"break CountBytes" -ex "run a.txt" ./wordcount` exécute les mêmes commandes une
après l'autre et sort — la forme employée pour vérifier les sorties citées dans
ce livre. (Sur macOS, la chaîne d'outils livre *lldb* à la place de gdb ;
même travail, orthographes différents — l'exercice 4 de la leçon 003 y
revient.)

Toutes les transcriptions de cette leçon ont été capturées avec gcc 13.3.0 et
gdb 15.1 sur Linux x86-64 ; les nombres exacts différeront sur votre machine,
les formes non.

## Étape de code

Un seul changement pour cette leçon : la boucle de comptage sort de `main`
pour aller dans `static unsigned long CountBytes(FILE *f)`, appelée une fois par
fichier, commitée avec ce texte. Son état final est étiqueté `lesson-002`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
index a74772e..cd34962 100644
--- a/sandbox/wordcount/wordcount.c
+++ b/sandbox/wordcount/wordcount.c
@@ -1,8 +1,17 @@
 // wordcount.c — count bytes in every file named on the command line.
 //
-// Lesson 001: argv, file input, and the first gcc command.
+// Lesson 002: gdb — breakpoints, stepping, and stack frames.
 #include <stdio.h>
 
+static unsigned long CountBytes(FILE *f)
+{
+    unsigned long bytes = 0;
+    int c;
+    while ((c = fgetc(f)) != EOF)
+        ++bytes;
+    return bytes;
+}
+
 int main(int argc, char **argv)
 {
     if (argc < 2) {
@@ -17,10 +26,7 @@ int main(int argc, char **argv)
             continue;
         }
 
-        unsigned long bytes = 0;
-        int c;
-        while ((c = fgetc(f)) != EOF)
-            ++bytes;
+        unsigned long bytes = CountBytes(f);
 
         printf("%lu %s\n", bytes, argv[i]);
         fclose(f);
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Le deuxième arrêt ressemble au premier *(predict-the-output)*

Posez un point d'arrêt sur `CountBytes`, `run a.txt b.txt`, et `continue` une
fois — vous êtes maintenant arrêté au deuxième appel. Avant de taper
`backtrace`, écrivez exactement ce qu'il montrera : combien de trames, quels
noms de fonctions, quels arguments avec quelles valeurs, et si `f` contiendra
la même adresse qu'au premier arrêt ou une adresse différente. Puis
exécutez-le et accordez chaque ligne de la sortie réelle avec votre prédiction
— et expliquez celle qui vous a surpris. (Une impression d'instrumentation de
votre cru, pour distinguer les deux appels, est permise et utile.)

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-002/ex1.md)

### Exercice 2 — Le fichier qui se ferme deux fois *(fix-the-crash)*

Un collègue a décidé que la fonction qui lit un fichier devrait aussi le
fermer, et a ajouté un `fclose(f);` dans `CountBytes` juste avant son `return`
— mais a laissé le `fclose` de `main` tranquille. `./wordcount a.txt` compte
désormais correctement puis meurt. Trouvez le plantage dans gdb (`run`, puis
`backtrace`, puis remontez jusqu'à la première trame qui est notre code plutôt
que la bibliothèque C) et dites en une phrase pourquoi le programme s'arrête
en catastrophe. Puis corrigez le programme pour que le fichier soit fermé
exactement une fois, en laissant la règle de propriété évidente pour le
lecteur suivant.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-002/ex2.md)

### Exercice 3 — Les conditions ne voient qu'une trame *(explain-in-prose)*

Avec deux fichiers sur la ligne de commande, essayez de vous arrêter seulement
au deuxième appel : `break CountBytes if i == 2`. gdb refuse. Expliquez
pourquoi `i` n'est pas visible depuis ce point d'arrêt, pourquoi
`break CountBytes if bytes == 0` *est* accepté mais ne se déclenchera
(typiquement) jamais à l'entrée de `CountBytes`, et ce que
`set $hits = 0` suivi de `break CountBytes if ++$hits == 2` fait à la place —
y compris quelle sorte de chose est `$hits` et où elle vit. Deux paragraphes.

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-002/ex3.md)

### Exercice 4 — Un compteur dans deux portées *(extend-the-code)*

Ajoutez un compteur d'appels au programme : une variable de portée fichier que
`CountBytes` incrémente à l'entrée, et une ligne dans `main`, après le dernier
fichier, rapportant combien de fois la fonction a tourné. Vérifiez ensuite
dans gdb que `print` peut voir le compteur depuis la trame de `CountBytes`
comme depuis celle de `main`, tandis que `i` ne peut être imprimé que depuis
`main`. Rapportez ce que vous avez vu et quelle est la différence entre les
deux variables.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-002/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** [Leçon 001 — argv et saisie de fichiers : votre première commande `gcc`](lesson-001-first-program.md) ·
**Suivante :** [Leçon 003 — tampons de caractères : les chaînes à la main](lesson-003-char-buffers.md) ·
**Étiquette de code :** [`lesson-002`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-002)

*Page traduite de la version anglaise `book/lessons/part-0/lesson-002-gdb.md`,
révision `a3b4a6a`.*

<!-- translation-source: book/lessons/part-0/lesson-002-gdb.md @ a3b4a6a -->
