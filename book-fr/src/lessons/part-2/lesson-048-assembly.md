# Leçon 048 — l'assembleur compilé du blitter

{{#include ../../stability-horizon.md}}

## Prose

Vous avez écrit `BlitSprite`. Cette leçon lit **ce que le compilateur en a
fait** — les instructions qui s'exécutent réellement quand un sprite est
dessiné. C'est la plongée de lecture d'assembleur, et sa discipline est la
même que celle de toute lecture de ce cours : le texte devant vous est de la
*sortie réelle* de la construction que vous pouvez lancer, citée ligne pour
ligne. Vous n'apprenez pas à écrire de l'assembleur (ce n'est pas une
compétence au programme de ce cours) ; vous apprenez à *lire* la réponse de
la machine à votre code — parce que les réponses à « pourquoi est-ce lent »
et « qu'a fait le compilateur de ma boucle » sont écrites exactement dans
cette langue. La leçon 049 lira la réponse optimisée ; celle d'aujourd'hui
est la réponse fidèle.

### L'outil

L'étape de code est `tools/disasm.sh` — une commande qui imprime les
instructions d'une fonction, extraites de `build/game` :

```
$ ./tools/disasm.sh              # the blitter
$ ./tools/disasm.sh ClearBuffer  # anything else, by substring
```

Elle enveloppe `objdump -d -C --no-show-raw-insn`, restreint à la définition
de la fonction (les sites d'appel nomment eux aussi les fonctions ; ce n'est
pas ce que nous lisons), et n'imprime rien que le compilateur n'ait émis. Ce
qui suit est sa sortie pour `BlitSprite`, construite avec le `./build.sh`
par défaut du cours — `-O0`, la construction qu'ont utilisée toutes les
leçons jusqu'ici.

### Le prologue : comment arrive un appel

```
0000000000001764 <engine::BlitSprite(engine::Framebuffer&, engine::Sprite const&, int, int)>:
    1764:	endbr64
    1768:	push   %rbp
    1769:	mov    %rsp,%rbp
    176c:	mov    %rdi,-0x38(%rbp)
    1770:	mov    %rsi,-0x40(%rbp)
    1774:	mov    %edx,-0x44(%rbp)
    1777:	mov    %ecx,-0x48(%rbp)
```

Sept instructions et toute la convention d'appel est sur la table :

- **`endbr64`** est un point d'atterrissage : le x86-64 moderne marque les
  cibles de saut valides pour que le CPU puisse refuser les retours et les
  sauts vers nulle part (control-flow enforcement — la défense du matériel,
  présente dans chaque fonction).
- **`push %rbp; mov %rsp,%rbp`** est le pointeur de trame : la fonction
  garde une ancre fixe pour ses variables locales. C'est précisément grâce à
  ces deux instructions que vous pouvez demander à un débogueur de parcourir
  les trames.
- **Les quatre `mov` sont les arguments qui rentrent à la maison.** La
  convention x86-64 passe les six premiers arguments entiers ou pointeurs
  dans des registres — ici `%rdi` (le `Framebuffer&`), `%rsi` (le `Sprite&`),
  `%edx` (x), `%ecx` (y) — et à `-O0`, le compilateur déverse immédiatement
  chacun dans son emplacement de pile. Chaque variable locale de cette
  fonction vit à `-0xNN(%rbp)` : voilà ce que `-O0` *signifie*. Aucune
  variable n'a reçu de registre ; la pile est la maison de tout.

### Le rectangle de découpage, compilé

```
    177a:	mov    -0x44(%rbp),%eax
    177d:	mov    $0x0,%edx
    1782:	test   %eax,%eax
    1784:	cmovs  %edx,%eax
    1787:	mov    %eax,-0x20(%rbp)
```

C'est `int left = x < 0 ? 0 : x;` — lisez-le ainsi : charger x, charger
zéro, tester le signe de x, **déplacer conditionnellement** zéro par-dessus x
s'il était négatif. `cmovs` (« conditional move if sign ») est une
comparaison et une affectation *sans branchement* — le CPU exécute les
données des deux chemins et en choisit un. La même forme compile `top` ; et
les deux autres bornes :

```
    179a:	mov    -0x38(%rbp),%rax
    179e:	mov    0x8(%rax),%edx
    17a1:	mov    -0x40(%rbp),%rax
    17a5:	mov    0x8(%rax),%ecx
    17a8:	mov    -0x44(%rbp),%eax
    17ab:	add    %ecx,%eax
    17ad:	cmp    %eax,%edx
    17af:	cmovle %edx,%eax
    17b2:	mov    %eax,-0x18(%rbp)
```

`right = x + s.width < fb.width ? x + s.width : fb.width`, en une seule
passe. Deux détails méritent une pause : les décalages de champs —
`0x8(%rax)` est `width`, `0xc(%rax)` est `height`, *la disposition de
structure de la leçon 007 rendue visible* (un `Framebuffer`, c'est `pixels`
à 0, `width` à 8, `height` à 12) ; et la comparaison est `cmovle`, ce qui
correspond au `<` du C++ retourné — le compilateur lit `a < b` à la façon
`b <= a` et choisit le déplacement en conséquence. La condition est la
même ; la forme est celle du compilateur.

### Les boucles sont des branchements avec de la comptabilité

```
    17d0:	mov    -0x1c(%rbp),%eax
    17d3:	mov    %eax,-0x28(%rbp)
    17d6:	jmp    18e5 <engine::BlitSprite(...)+0x181>
    17db:	mov    -0x20(%rbp),%eax
    17de:	mov    %eax,-0x24(%rbp)
    17e1:	jmp    18d5 <engine::BlitSprite(...)+0x171>
```

`j = top; goto the_test; i = left; goto its_test`. Les boucles `for` du C++
se compilent selon la forme classique : initialiser, sauter vers le *bas*,
là où vit le test, pour que la boucle ne fasse sa vérification qu'une fois
par itération et non deux. `-0x28(%rbp)`, c'est `j` ; `-0x24(%rbp)`, c'est
`i`. À partir d'ici, « la boucle » désigne ces deux emplacements qu'on
incrémente et qu'on compare au bas et à la droite du rectangle de découpage.

### Un pixel : l'adresse source et le test de la clé

```
    17e6:	mov    -0x40(%rbp),%rax
    17ea:	mov    (%rax),%rcx
    17ed:	mov    -0x28(%rbp),%eax
    17f0:	sub    -0x48(%rbp),%eax
    17f3:	movslq %eax,%rdx
    17f6:	mov    -0x40(%rbp),%rax
    17fa:	mov    0x8(%rax),%eax
    17fd:	cltq
    17ff:	imul   %rax,%rdx
    1803:	mov    -0x24(%rbp),%eax
    1806:	sub    -0x44(%rbp),%eax
    1809:	cltq
    180b:	add    %rax,%rdx
    180e:	mov    %rdx,%rax
    1811:	add    %rax,%rax
    1814:	add    %rdx,%rax
    1817:	add    %rcx,%rax
    181a:	mov    %rax,-0x10(%rbp)
```

Dix-huit instructions pour
`&s.pixels[(((j - y) * s.width) + (i - x)) * 3]`. Toute l'arithmétique est
là : `sub` calcule `j − y` ; `movslq`/`cltq` **étendent le signe** de l'`int`
de 32 bits vers un index de 64 bits (la vérité aux proportions de la
leçon 007 : l'arithmétique `int` élargie là où vivent les pointeurs) ;
`imul` fait le pas
de ligne ; `add %rax,%rax; add %rdx,%rax` est `× 3` — une multiplication par
une petite constante compilée en décalage-et-addition. Le résultat atterrit
dans `-0x10(%rbp)` : `src`, le pointeur.

Puis le test de la clé — la règle de transparence, en trois comparaisons :

```
    181e:	mov    -0x10(%rbp),%rax
    1822:	movzbl (%rax),%edx
    1825:	mov    -0x40(%rbp),%rax
    1829:	movzbl 0x10(%rax),%eax
    182d:	cmp    %al,%dl
    182f:	jne    185f <engine::BlitSprite(...)+0xfb>
```

`movzbl`, c'est « move byte, zero-extend to long » — charger un octet non
signé dans un registre entier pour que la comparaison soit propre.
`0x10(%rax)` est `key_r` : les champs de la structure `Sprite` aux décalages
0 (pixels), 8 (width), 12 (height), 16-18 (la clé). Le `jne 185f` saute
*par-dessus la copie* à la première différence — et les deuxième et
troisième comparaisons (key_g, key_b) font de même. C'est seulement si les
trois concordent que le flux atteint `je 18d0` — le saut : un unique `nop`,
le `continue` du C++.

### La copie elle-même

```
    185f:	mov    -0x38(%rbp),%rax
    1863:	mov    (%rax),%rcx
    ...
    1881:	shl    $0x2,%rax
    1885:	add    %rcx,%rax
    1888:	mov    %rax,-0x8(%rbp)
```

L'adresse de destination : la même arithmétique d'index en unités de
framebuffer, et `shl $0x2,%rax` est `× 4` — quatre octets par pixel, le
décalage remplaçant la multiplication. Puis les quatre stockages, **dans
l'ordre où le C++ les a écrits** :

```
    188c:	mov    -0x10(%rbp),%rax
    1890:	add    $0x2,%rax
    1894:	movzbl (%rax),%edx
    1897:	mov    -0x8(%rbp),%rax
    189b:	mov    %dl,(%rax)
```

`dst[0] = src[2];` — charger le *troisième* octet de la source (rouge), le
stocker au premier de la destination (bleu). L'échange RGB→BGR est là, sous
vos yeux : `add $0x2` choisit l'octet, le stockage le place. Les paires
suivantes font `dst[1] = src[1]` et `dst[2] = src[0]`, et enfin
`movb $0x0,(%rax)` est `dst[3] = 0` — le stockage d'un zéro littéral, large
d'un octet.

La boucle se referme sur quatre instructions que vous savez maintenant lire
à froid :

```
    18d1:	addl   $0x1,-0x24(%rbp)
    18d5:	mov    -0x24(%rbp),%eax
    18d8:	cmp    -0x18(%rbp),%eax
    18db:	jl     17e6 <engine::BlitSprite(...)+0x82>
```

`++i`, le charger, le comparer à `right`, revenir en arrière s'il est
inférieur. La version de la boucle externe est la même contre `bottom`. Et
la fonction se termine comme elle a commencé : `nop; nop; pop %rbp; ret` —
la trame démontée, le retour.

### Ce que la lecture vous apprend

Comptez la boucle interne sur le chemin de copie — l'arithmétique d'adresse
(18 instructions), le test de la clé (20), la copie (34), l'incrément et le
test (4) — et un pixel opaque coûte **76 instructions** ; un pixel de la
couleur-clé, environ 43 (l'adresse et le test de la clé, puis le `nop` du
saut). C'est le prix `-O0` de « une variable par emplacement de pile et un
énoncé par instruction exécutée ».

La mesure dit ce que cela coûte : dessiner un sprite 16×16 — 130 pixels
opaques et 126 de la couleur-clé — prend environ 882 ns, soit **3,45 ns par
pixel de sprite** en moyenne sur ce mélange. À 3,26 GHz sur cette machine,
cela fait environ 11 cycles par pixel contre ~76 instructions. L'écart,
c'est le cœur moderne à exécution dans le désordre qui retraite plusieurs
instructions simples par cycle ; le *rapport* est ce que `-O0` perd et que
`-O3` récupère. La ligne plate à 1,5 Go/s du tableau de la leçon 047, c'est
cette boucle vue de l'autre côté.

Et la frontière honnête de cette lecture : la norme C++ ne promet que le
**comportement observable** — le compilateur peut calculer n'importe quoi,
dans n'importe quel ordre, par n'importe quelles instructions, tant que les
pixels et les échecs tombent juste. Ce que vous venez de lire est la réponse
*actuelle* d'un compilateur à `-O0`, pas une promesse sur celle de demain, et
pas la seule réponse. La leçon suivante lira une seconde réponse au même
C++ — la réponse vectorisée — et les deux sont vraies à la fois. La passe de
profilage de la partie 5 finit encore ici : quand le blit apparaît dans le
top 2 des points chauds, c'est dans cette liste que la correction se lit
avant de s'écrire.

## Étape de code

Un seul changement pour cette leçon : `tools/disasm.sh` — l'outil de lecture
d'assembleur qui imprime les instructions compilées d'une fonction, extraites
de `build/game` (`objdump -d -C --no-show-raw-insn`, restreint à la
définition de la fonction). Le code du moteur n'est pas touché. L'état final
de l'outil est étiqueté `lesson-048`.

```diff
diff --git a/tools/disasm.sh b/tools/disasm.sh
new file mode 100755
index 0000000..f54a09c
--- /dev/null
+++ b/tools/disasm.sh
@@ -0,0 +1,32 @@
+#!/usr/bin/env bash
+#
+# disasm.sh — print what the compiler made of one function.
+#
+# Lesson 048: the assembly-reading tool. build.sh compiles the engine; this
+# prints the instructions that ended up in build/game for one symbol —
+# exactly the compiler's output, nothing added. The default symbol is the
+# blitter, whose copy loop lessons 048-049 read.
+#
+# Usage:
+#   ./tools/disasm.sh                 # the blitter (BlitSprite)
+#   ./tools/disasm.sh ClearBuffer     # any other symbol by substring
+#
+# build/ must be current: run ./build.sh first. To read the optimized
+# build's instructions (lesson 049), build with the flags first:
+#   CXXFLAGS="-std=c++17 -O3 -g -Wall -Wextra" ./build.sh
+
+set -euo pipefail
+
+cd "$(dirname "$0")/.."
+
+SYMBOL="${1:-BlitSprite}"
+
+if [ ! -x build/game ]; then
+    echo "disasm: build/game is missing — run ./build.sh first" >&2
+    exit 1
+fi
+
+# Only definition lines start at column 0 with an address and end in ':' —
+# call sites name the symbol too, and those are not what we are reading.
+objdump -d -C --no-show-raw-insn build/game |
+    sed -n "/^[0-9a-f]* <.*${SYMBOL}.*>:/,/^\$/p"
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — Le budget d'instructions *(predict-the-output)*

La liste est une nomenclature ; chiffrez-la. Comptez vous-même les
instructions du chemin de copie de la boucle interne (l'adresse source, le
test de la clé, la copie, l'incrément — la leçon donne les sections, vous
donnez les totaux), puis prédisez les nanosecondes que devrait coûter un
sprite 16×16 à la fréquence de votre CPU, sans rien supposer du parallélisme.
Ajoutez le benchmark de la leçon au bloc de démarrage — dessinez le sprite
dix mille fois, chronométré, imprimé en nanosecondes par pixel — et comparez
votre prédiction à la mesure. Où vit l'écart, et que dit-il du comptage
d'instructions sur un cœur moderne ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-048/ex1.md)

### Exercice 2 — La signature du compilateur *(port-to-your-own-machine)*

Cette liste est l'opinion d'un compilateur — la colonne du livre est GCC
13.3.0. Faites en sorte que votre construction consigne la sienne aussi :
imprimez `__VERSION__` (la macro prédéfinie du compilateur lui-même) dans le
rapport de démarrage de l'exécution, pour que chaque nombre que votre machine
produit porte le compilateur qui l'a produit. Lancez ensuite
`./tools/disasm.sh` sur votre construction — même C++, votre compilateur —
et comparez trois repères avec la liste du livre : le prologue de la fonction
(les déversements d'arguments), les bornes du découpage (déplacements
conditionnels ou branchements ?), et les quatre stockages de la copie.
Qu'est-ce qui est resté, qu'est-ce qui a bougé, et laquelle des deux listes
est le langage, laquelle est le compilateur ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-048/ex2.md)

---

**Partie :** [Partie 2 — le rendu logiciel](../../index.md) ·
**Précédente :** [Leçon 047 — plongée dans les caches](lesson-047-caches.md) ·
**Suivante :** [Leçon 049 — la lentille SIMD](lesson-049-simd.md) ·
**Étiquette de code :** [`lesson-048`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-048)

*Page traduite de la version anglaise `book/lessons/part-2/lesson-048-assembly.md`,
révision `38c0b7a`.*

<!-- translation-source: book/lessons/part-2/lesson-048-assembly.md @ 38c0b7a -->
