# Leçon 001 — argv et saisie de fichiers : votre première commande `gcc`

{{#include ../../stability-horizon.md}}

## Prose

Cette leçon construit le premier programme de la partie 0 : `wordcount`, un
tout petit `wc`. Il prend des noms de fichiers sur la ligne de commande et
affiche combien d'octets contient chacun. Tout, dans la partie 0, vit sous
`sandbox/` — des programmes jetables, aucune source du moteur, compilés et
jetés à la main. L'enjeu du jour n'est pas le programme ; c'est la mécanique
autour : comment un programme C démarre, comment il lit des fichiers, et
comment `gcc` transforme un fichier de texte en quelque chose que vous pouvez
exécuter.

Un programme C démarre sur `main`. Il n'y a ni garde
`if __name__ == "__main__"`, ni mécanisme d'import qui déciderait de ce qui
s'exécute : l'éditeur de liens enregistre `main` comme point d'entrée, et le
système d'exploitation y commence l'exécution. `main` reçoit la ligne de
commande sous la forme de deux valeurs : `argc`, le nombre d'arguments, et
`argv`, le tableau des chaînes d'arguments. Comparez avec `sys.argv` en Python
ou `ARGV` en Ruby : même idée, cérémonie différente. `argv[0]` est le nom du
programme lui-même — le message d'utilisation de `main` l'affiche au lieu de
coder un nom en dur — et les noms de fichiers qui nous intéressent vont de
`argv[1]` à `argv[argc - 1]`. La boucle qui les parcourt est un `for` ordinaire
; C n'a pas d'itérateurs.

Deux types liés aux arguments ont l'air exotiques venant de Python ou de Ruby.
`char *` est ce qu'est une chaîne en C : un pointeur d'octets — pour
aujourd'hui, lisez « une chaîne » ; les leçons 003 et 004 ouvrent les pointeurs
proprement. `char **argv` est un pointeur vers ces pointeurs de chaînes, le
tableau que le shell nous remet. Et `FILE *f` de `fopen` est une poignée sur un
fichier ouvert — un pointeur opaque que vous devez fermer vous-même. C n'a ni
ramasse-miettes, ni bloc `with` : `fopen` acquiert une ressource, `fclose` la
libère, et chaque chemin dans la boucle doit faire les deux.

Regardez ce que `fopen` renvoie en cas d'échec et ce que `main` en fait. Il
n'y a pas d'exceptions en C : l'échec est une valeur — `NULL` ici — que vous
testez. Le message d'erreur part sur `stderr` et la boucle passe au fichier
suivant, ce qui fait qu'un fichier manquant n'arrête pas l'exécution. Envoyer
les diagnostics sur `stderr` et les résultats sur `stdout` est une habitude à
prendre maintenant : c'est ce qui permet à `./wordcount report.txt 2>/dev/null`
de garder une sortie propre pendant que la réclamation disparaît.

La boucle de lecture est un appel à `fgetc` par octet, et son type de retour
mérite une pause. `fgetc` renvoie un `int`, pas un `char` : un `char` ne peut
pas contenir toutes les valeurs d'octet *plus* le signal de « fin de fichier »,
aussi le marqueur de fin `EOF` a besoin de sa propre place. La condition de
boucle `while ((c = fgetc(f)) != EOF)` affecte et teste en un seul geste.
C'est seulement quand la boucle se termine que le programme affiche son
compteur avec `printf` — `%lu` pour le compteur `unsigned long`, `%s` pour le
nom de fichier — et ferme le fichier.

Enfin, la commande qui donne l'existence à tout cela. Depuis l'intérieur de
`sandbox/wordcount/` :

```
gcc -std=c11 -Wall -Wextra wordcount.c -o wordcount
```

Lisez-la option par option — aucune n'est de la décoration :

- `gcc` exécute toute la chaîne sur `wordcount.c` : le **préprocesseur** déplie
  `#include <stdio.h>` dans les déclarations derrière `printf`, `fopen` et les
  autres ; le **compilateur** traduit le programme en assembleur ; l'
  **assembleur** transforme cela en code machine ; l'**éditeur de liens** coud
  le code machine aux implémentations de la bibliothèque C et enregistre
  `main` comme point d'entrée.
- `-std=c11` fixe la version du langage sur C11, pour que le compilateur soit
  d'accord avec ce livre sur ce que signifie le code, aujourd'hui et sur votre
  machine.
- `-Wall -Wextra` activent les avertissements du compilateur. Les
  avertissements sont du cours, ici : un avertissement, c'est le compilateur
  qui vous dit que votre programme est légal mais suspect, et la partie 0 prend
  chacun d'eux au sérieux.
- `-o wordcount` nomme le programme produit au lieu du `a.out` par défaut.

Exécutez-le avec `./wordcount fichier.txt fichier2.txt` — ou `./wordcount`
seul, pour voir le message d'utilisation que vous obtenez quand `argc` est trop
petit.

## Étape de code

Un seul changement pour cette leçon : la totalité de `wordcount.c`, commitée
avec ce texte. Son état final est étiqueté `lesson-001`.

```diff
diff --git a/sandbox/wordcount/wordcount.c b/sandbox/wordcount/wordcount.c
new file mode 100644
index 0000000..a74772e
--- /dev/null
+++ b/sandbox/wordcount/wordcount.c
@@ -0,0 +1,30 @@
+// wordcount.c — count bytes in every file named on the command line.
+//
+// Lesson 001: argv, file input, and the first gcc command.
+#include <stdio.h>
+
+int main(int argc, char **argv)
+{
+    if (argc < 2) {
+        fprintf(stderr, "usage: %s FILE...\n", argv[0]);
+        return 1;
+    }
+
+    for (int i = 1; i < argc; ++i) {
+        FILE *f = fopen(argv[i], "rb");
+        if (f == NULL) {
+            fprintf(stderr, "%s: cannot open %s\n", argv[0], argv[i]);
+            continue;
+        }
+
+        unsigned long bytes = 0;
+        int c;
+        while ((c = fgetc(f)) != EOF)
+            ++bytes;
+
+        printf("%lu %s\n", bytes, argv[i]);
+        fclose(f);
+    }
+
+    return 0;
+}
```

## Exercices

Quatre courts exercices. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — L'entrée standard *(extend-the-code)*

À l'heure actuelle, `./wordcount` sans argument de fichier affiche le message
d'utilisation et s'arrête. Faites-lui lire l'entrée standard à la place et
afficher seulement le nombre d'octets, pour que `./wordcount < notes.txt`
fonctionne comme le fait `wc`.

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-001/ex1.md)

### Exercice 2 — Le répertoire silencieux *(fix-the-crash)*

`./wordcount .` affiche `0 .` comme si le répertoire était un fichier vide.
Ouvrir un répertoire réussit sur ce système, mais le lire échoue — et
l'échec est invisible dans la sortie. Faites détecter au programme une erreur
de lecture après la boucle (regardez ce que `ferror` rapporte sur le flux) et
affichez un message `cannot read` pour ce fichier au lieu d'un compteur.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-001/ex2.md)

### Exercice 3 — Un caractère à la fois *(measure-the-performance)*

Fabriquez un gros fichier — `yes | head -c 100000000 > big.txt` — et chronométrez
le programme dessus avec `time ./wordcount big.txt`. Remplacez ensuite la boucle
de comptage `fgetc` par une lecture par blocs : un tampon `char` de quelques
kilooctets rempli avec `fread`, compté par morceaux. Chronométrez à nouveau.
De combien est-ce plus rapide, et qu'est-ce que la machine fait de différent ?

> **Solution :** [ex3 — diff + visite guidée](../../solutions/lesson-001/ex3.md)

### Exercice 4 — Des octets, pas des caractères *(predict-the-output)*

Créez un fichier contenant exactement les cinq octets `0x48 0x69 0x00 0x0A
0xFF` — `printf 'Hi\0\n\xff' > five.bin` fait cela. Prédisez la ligne de sortie
exacte que `./wordcount five.bin` affiche, et prédisez en quoi elle diffère de
`wc -c five.bin`. Ajoutez ensuite une ligne `fprintf(stderr, "c=%d\n", c);`
dans la boucle de lecture, exécutez sur ce fichier, et accordez ce que vous
voyez avec votre prédiction — en particulier la dernière valeur que la boucle
rapporte.

> **Solution :** [ex4 — diff + visite guidée](../../solutions/lesson-001/ex4.md)

---

**Partie :** [Partie 0 — Fondations en C](../../index.md) ·
**Précédente :** — ·
**Suivante :** [Leçon 002 — gdb : points d'arrêt, pas à pas, trames de pile](lesson-002-gdb.md) ·
**Étiquette de code :** [`lesson-001`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-001)

*Page traduite de la version anglaise
`book/lessons/part-0/lesson-001-first-program.md`, révision `57b9b3e`.*

<!-- translation-source: book/lessons/part-0/lesson-001-first-program.md @ 57b9b3e -->
