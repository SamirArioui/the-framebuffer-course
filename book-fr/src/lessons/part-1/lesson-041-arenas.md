# Leçon 041 — les arenas

{{#include ../../stability-horizon.md}}

## Prose

La réservation donne de la mémoire au moteur ; aujourd'hui le moteur construit
**l'allocation** par-dessus — et elle est d'une simplicité presque gênante. Une
arena, c'est une réservation et un pointeur qui avance. Pas de listes libres,
pas de comptabilité par allocation, pas de `free`. Ce qu'elle coûte est une
discipline, et ce qu'elle cache aux outils fait l'objet de la dernière section
de la leçon.

### Le bump pointer

```c++
struct Arena {
    platform::Reservation memory;
    size_t used;   /* the bump pointer: bytes handed out */
};

void *ArenaAlloc(Arena &arena, size_t bytes, size_t align);
```

L'allocation tient en un seul pas d'arithmétique : arrondir le bump pointer à
l'alignement, distribuer les octets à cet offset, puis faire avancer le
pointeur au-delà. C'est tout le mécanisme. `align` est une puissance de deux,
donc l'arrondi est l'arithmétique de remplissage de la leçon 007 faite avec un
masque — l'alignement dont une structure a besoin, calculé au moment de
l'allocation.

Le mode d'échec est l'autre moitié du modèle : quand la demande ne tient pas,
`ArenaAlloc` renvoie **0**. Pas de bloc plus petit, pas de nouvelle tentative,
pas d'appel caché à quelque autre allocateur. L'arena possède exactement une
réservation et ne ment jamais en prétendant en avoir plus.

### Les marques et le retour arrière

Le bump pointer ne va qu'en avant — sauf quand le moteur déclare « cette
portion du passé est terminée » :

```c++
size_t mark = ArenaMark(arena);
... ArenaAlloc, ArenaAlloc, ArenaAlloc ...
ArenaRollback(arena, mark);
```

Le retour arrière n'est pas une libération ; c'est un *oubli*. Rien n'a été
détruit, aucune métadonnée n'a été mise à jour — le curseur est revenu en
arrière, et l'allocation suivante atterrit là où celle qui était marquée a
atterri. C'est la bonne forme pour de la mémoire qui a une portée : les données
temporaires d'une frame (retour arrière à la fin de la frame), les données d'un
niveau (retour arrière à la fin du niveau), une expérience (retour arrière
quand c'est fini). Le coût d'une allocation est une comparaison ; le coût d'en
libérer des milliers en est une aussi.

### L'arena, mesurée

Le démarrage du moteur exerce l'allocateur comme l'avenir s'en servira — des
allocations aux alignements différents, une marque et un retour arrière, et
l'échec honnête :

```
engine: arena over 65536 bytes (16 pages)
engine: alloc 100 (align 16) -> offset 0, used 100
engine: alloc 50 (align 32) -> offset 128, used 178
engine: mark at 178
engine: alloc 3 (align 1) -> offset 178, used 181
engine: rollback to 178 — used 178
engine: alloc 3 (align 1) -> offset 178, used 181
engine: exhausted: alloc 999999 -> 0 (as it should be)
```

Lisez les nombres : la seconde allocation atterrit à l'offset 128, pas à 100 —
`align 32` a arrondi le bump pointer de 100 au multiple de 32 suivant. Après le
retour arrière, la même demande atterrit de nouveau à l'offset **178** : la
même mémoire, distribuée deux fois, pour un coût nul. Et la demande de 999999
octets reçoit le seul échec que l'arena connaisse.

### La limite, enseignée honnêtement

Voici maintenant la partie que la conception a tenu à enseigner ici plutôt que
de la laisser découvrir plus tard : **AddressSanitizer ne voit pas à
l'intérieur d'une arena.**

Depuis la leçon 005, le sanitizer est la machine d'honnêteté du cours — il
attrape les fuites de mémoire, les use-after-free et les accès hors limites sur
le *tas*, parce qu'il instrumente chaque `malloc` et chaque `free`, et marque
ce qui est vivant dans une shadow memory. L'arena ne participe à rien de tout
cela : sa mémoire est un grand mappage anonyme (leçon 040), et une
« allocation » est de l'arithmétique dont le sanitizer n'est jamais informé.

L'exercice 2 en est la démonstration — un usage après retour arrière, exécuté
sous un vrai build ASan, qui corrompt une allocation à travers une autre et ne
rapporte **rien**. Le bug est réel ; l'outil est aveugle, exprès, parce que le
modèle qu'il connaît n'est pas ce modèle-ci.

Ce qui comble le trou :

- **Le contrat.** Un pointeur ramené en arrière est mort, comme un pointeur
  libéré par `free`. La discipline de l'arena *est* la sécurité — c'est
  l'envers de « pas de comptabilité par allocation ».
- **L'empoisonnement.** Une arena de débogage peut marquer son espace libre
  comme inaccessible (l'exercice 2 de la leçon 040 montre le mécanisme) ou
  l'estampiller d'une sentinelle — la façon dont les vrais moteurs rendent
  leurs arenas vérifiables.
- **Les hooks.** Les sanitizers ont des interfaces d'allocateur personnalisé
  exactement pour cela. Le service d'arena de la partie 4 pourra se doter d'un
  mode débogage qui leur parle.

Enseigner la limite ici, où l'arena est assez petite pour tenir dans votre
tête, c'est la différence entre un outil auquel vous faites confiance
aveuglément et un outil auquel vous faites confiance *en connaissance de
cause*.

### Pourquoi une arena, au fond

La douleur de la leçon 004 — chaque `malloc` exige un `free`, et la question
« qui possède ces octets » ne s'efface jamais — reçoit une réponse structurelle
pour les données du moteur. Les données d'un jeu ont des *portées* : une frame,
un niveau, une session. Une arena par portée signifie que les allocations ne
coûtent rien, que les libérations ne coûtent rien, et que la propriété est
réglée par construction : l'arena possède tout ce qu'elle contient, et la fin
de la portée est un retour arrière. La réservation du framebuffer (leçon 040)
et cet allocateur par-dessus (cette leçon) sont les deux moitiés de « le moteur
possède sa mémoire » — la partie 4 formalisera les deux en services.

## Étape de code

Un seul changement pour cette leçon : `arena.h` et `arena.cpp` accueillent
l'allocateur à bump pointer par-dessus une réservation — allocation alignée,
marques, retour arrière, et le 0 honnête en cas d'épuisement — et `main.cpp`
l'exerce au démarrage puis le libère avec le reste de ce que l'exécution a
pris. Son état final est étiqueté `lesson-041`.

```diff
diff --git a/src/arena.cpp b/src/arena.cpp
new file mode 100644
index 0000000..400e5d0
--- /dev/null
+++ b/src/arena.cpp
@@ -0,0 +1,51 @@
+// arena.cpp — the arena: the bump pointer, the alignment, the marks.
+//
+// Lesson 041: allocation this simple is three pieces of arithmetic and one
+// rule — the arena never hands out memory it does not have.
+
+#include "arena.h"
+
+namespace engine {
+
+void ArenaInit(Arena &arena, size_t bytes)
+{
+    arena.memory = platform::ReserveMemory(bytes);
+    arena.used = 0;
+}
+
+void ArenaRelease(Arena &arena)
+{
+    platform::ReleaseMemory(arena.memory);
+    arena.used = 0;
+}
+
+void *ArenaAlloc(Arena &arena, size_t bytes, size_t align)
+{
+    if (!arena.memory.bytes)
+        return 0;
+
+    /* The bump pointer, rounded up to the alignment (a power of two, so
+       the mask does what lesson 007's padding did by hand). */
+    size_t base = (size_t)(arena.memory.bytes + arena.used);
+    size_t aligned = (base + align - 1) & ~(align - 1);
+    size_t start = aligned - (size_t)arena.memory.bytes;
+
+    if (start + bytes > arena.memory.size)
+        return 0; /* out of room: the honest answer */
+
+    arena.used = start + bytes;
+    return arena.memory.bytes + start;
+}
+
+size_t ArenaMark(const Arena &arena)
+{
+    return arena.used;
+}
+
+void ArenaRollback(Arena &arena, size_t mark)
+{
+    if (mark <= arena.used)
+        arena.used = mark; /* nothing is freed; the cursor just moves back */
+}
+
+} /* namespace engine */
diff --git a/src/arena.h b/src/arena.h
new file mode 100644
index 0000000..f9dff98
--- /dev/null
+++ b/src/arena.h
@@ -0,0 +1,41 @@
+// arena.h — the arena: a bump allocator over a reservation.
+//
+// Lesson 041: allocation is a pointer that moves forward (and comes back
+// on demand). No free lists, no bookkeeping per allocation — the arena is
+// one reservation and a cursor in it. Memory comes from the OS reservation
+// of lesson 040; the allocator is ours.
+#ifndef ARENA_H
+#define ARENA_H
+
+#include <stddef.h>
+
+#include "platform.h"
+
+namespace engine {
+
+struct Arena {
+    platform::Reservation memory; /* the whole reserve, from the OS */
+    size_t used;                  /* the bump pointer: bytes handed out */
+};
+
+/* An arena over an OS reservation of at least this many bytes. */
+void ArenaInit(Arena &arena, size_t bytes);
+
+/* Gives the reservation back to the OS. */
+void ArenaRelease(Arena &arena);
+
+/* Allocates from the bump pointer, aligned to align (a power of two).
+   Returns 0 when the arena has no room — the honest answer, never a
+   smaller allocation. */
+void *ArenaAlloc(Arena &arena, size_t bytes, size_t align);
+
+/* A rollback mark: where the bump pointer is now. */
+size_t ArenaMark(const Arena &arena);
+
+/* Everything allocated since the mark is gone — and the next allocation
+   lands where the marked one did. */
+void ArenaRollback(Arena &arena, size_t mark);
+
+} /* namespace engine */
+
+#endif
diff --git a/src/main.cpp b/src/main.cpp
index c345bb4..06eba6e 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -7,6 +7,7 @@
 
 #include <cstdio>
 
+#include "arena.h"
 #include "framebuffer.h"
 #include "frame.h"
 #include "platform.h"
@@ -127,6 +128,37 @@ int Run(int argc, char **argv)
     std::printf("engine: arrow keys move the marker; close the window to stop\n");
     std::printf("engine: marker at %d,%d\n", (int)marker_x, (int)marker_y);
 
+    /* The arena: bump allocation over a reservation. This report is the
+       allocator's behavior — the same numbers Part 4's services will rely
+       on. */
+    Arena arena;
+    ArenaInit(arena, 65536);
+    std::printf("engine: arena over %zu bytes (%zu pages)\n",
+                arena.memory.size, arena.memory.size / page);
+
+    void *a = ArenaAlloc(arena, 100, 16);
+    std::printf("engine: alloc 100 (align 16) -> offset %ld, used %zu\n",
+                (long)((unsigned char *)a - arena.memory.bytes), arena.used);
+    void *b = ArenaAlloc(arena, 50, 32);
+    std::printf("engine: alloc 50 (align 32) -> offset %ld, used %zu\n",
+                (long)((unsigned char *)b - arena.memory.bytes), arena.used);
+
+    size_t mark = ArenaMark(arena);
+    std::printf("engine: mark at %zu\n", mark);
+    void *c = ArenaAlloc(arena, 3, 1);
+    std::printf("engine: alloc 3 (align 1) -> offset %ld, used %zu\n",
+                (long)((unsigned char *)c - arena.memory.bytes), arena.used);
+    ArenaRollback(arena, mark);
+    std::printf("engine: rollback to %zu — used %zu\n", mark, arena.used);
+    void *d = ArenaAlloc(arena, 3, 1);
+    std::printf("engine: alloc 3 (align 1) -> offset %ld, used %zu\n",
+                (long)((unsigned char *)d - arena.memory.bytes), arena.used);
+
+    void *e = ArenaAlloc(arena, 999999, 16);
+    std::printf("engine: exhausted: alloc 999999 -> %s\n",
+                e ? "handed out (!)" : "0 (as it should be)");
+    ArenaRelease(arena);
+
     /* The frame step: read news, update from polled state, draw, present.
        This is the shape every later part fills in — Part 2 draws into it,
        Part 5 measures it. Every phase is now measured: the frame record is
```

## Exercices

Deux extensions « faites-le vôtre ». Chacune se termine par sa solution — un
diff contre l'état final de cette leçon, plus une visite guidée — après
l'énoncé.

### Exercice 1 — La traînée du marqueur *(extend-the-code)*

Donnez au moteur une raison d'allouer : le marqueur se souvient de ses
passages. Allouez au démarrage une traînée de huit positions dans l'arena
(après avoir ramené l'expérience de démarrage à zéro par un retour arrière),
enregistrez chaque déplacement dans l'emplacement suivant — un anneau de huit,
avec retournement (wrap) — et dessinez la traînée derrière le marqueur sous
forme de points qui s'estompent, les plus anciens les plus ternes. Rapportez
d'où vient la mémoire de la traînée (son offset dans l'arena) et ce que vaut
`used` de l'arena ensuite. Dans votre rédaction : pourquoi un anneau *n'est-il
pas* un retour arrière, et quand utiliseriez-vous l'un ou l'autre ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-041/ex1.md)

### Exercice 2 — Le bug qu'ASan ne voit pas *(explain-in-prose)*

**Cet exercice corrompt la mémoire exprès** — un état pédagogique délibéré,
signalé comme tel. Prenez un pointeur périmé : allouez, faites un retour
arrière, allouez de nouveau, puis écrivez sur le nouveau bloc à travers le
pointeur périmé. Construisez avec `-fsanitize=address` comme la leçon 005 l'a
enseigné, exécutez, puis notez ce que le sanitizer a rapporté et *pourquoi* :
que suit réellement ASan, pourquoi la mémoire de l'arena lui est-elle
invisible, et qu'est-ce qui attraperait ce bug dans un vrai moteur (nommez au
moins deux défenses et leur coût) ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-041/ex2.md)

---

**Partie :** [Partie 1 — la couche plateforme](../../index.md) ·
**Précédente :** [Leçon 040 — des tampons adossés à une réservation](lesson-040-reservations.md) ·
**Suivante :** [Leçon 042 — l'interface comme contrat](lesson-042-contract.md) ·
**Étiquette de code :** [`lesson-041`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-041)

*Page traduite de la version anglaise `book/lessons/part-1/lesson-041-arenas.md`,
révision `3f18b81`.*

<!-- translation-source: book/lessons/part-1/lesson-041-arenas.md @ 3f18b81 -->
