# Leçon 070 — le coût du mixage dans le budget de frames

{{#include ../../stability-horizon.md}}

## Prose

L'instrumentation que la leçon 060 a plantée était exactement pour ça : la
phase `audio` est mesurée à l'intérieur de chaque frame depuis que le son a
commencé — sommée dans le cumul, nommée dans chaque ligne `frame N:` — et
c'est ici que la table du budget la montre enfin. La ligne est **mesurée, pas
devinée** — c'est le titre de cette leçon et sa règle. Aucun nombre de la ligne
n'a été estimé, benchmarké à part ou recopié depuis une fiche technique ;
chacun est une somme sur les frames que cette exécution a réellement exécutées,
divisée par leur nombre, comme chaque ligne au-dessus et en dessous d'elle.

### La table

La démo tourne, et à la fin le cumul s'imprime en budget — avec enfin la ligne
du son, entre `update` et `render`, là où la phase se situe dans la frame :

```
engine: demo: 580 frames measured, 580 buffers fed, 25 effects fired, 3 music wraps
engine: frame budget — 580 frames, avg 2.302 ms, worst 3.803 ms (frame 538)
engine:   subsystem   avg ms    share
engine:   update       0.001       0%
engine:   audio        0.045       2%
engine:   render       1.523      66%
engine:     sprites    0.001       0%
engine:     text       0.008       0%
engine:     tilemap    1.047      45%
engine:   present      0.732      32%
engine:   total        2.302     100%
```

La *place* de la ligne est l'ordre de l'enregistrement lui-même. La frame est
mesurée dans l'ordre `update`, `audio`, `render`, `present` — c'est l'ordre
dans lequel la ligne `frame N:` les liste, et la table se lit maintenant de la
même façon. Il aurait été tout aussi facile d'imprimer la ligne en bas, près de
`present`, là où vit l'autre ligne liée à la machine ; la table refuse, parce
que les lignes sont les phases de la frame dans l'ordre où la frame les
exécute. Une table qui réordonne ses lignes est une table qui commence à mentir
sur ce qui arrive quand.

Tout le reste au sujet de la ligne, ce sont les règles existantes de la table,
exactement :

- **la ligne est une somme mesurée** — `audio_sum` sur les frames de
  l'exécution divisée par leur nombre, comme `update_sum` et `render_sum` avant
  elle ;
- **sa part est celle de la frame moyenne** — `0.045 / 2.302` fait 1,96 %,
  imprimé comme 2 % ;
- **l'en-tête est inchangé** — combien de frames ont été mesurées, ce qu'elles
  coûtent en moyenne, et la pire par numéro.

### Ce que la ligne inclut

La ligne `audio` enveloppe **le mixage *et* sa soumission** — chaque lecture
d'horloge autour de toute l'étape audio de l'exécution, du premier tirage de
canal du tampon jusqu'à ce que le périphérique prenne le flux fini. C'est
pourquoi la ligne est digne de confiance : elle mesure la phase telle que la
frame la paie, pas la partie facile à chronométrer.

Ce que la ligne *se lit comme* dépend du périphérique, et les deux cas valent
la peine d'être dits clairement :

- **sur cette machine, la soumission revient immédiatement.** Le périphérique
  `null` d'ALSA prend les échantillons et les jette — mesuré dans les notes de
  cette partie : cinq tampons de 100 ms acceptés en 0.0 ms de temps écoulé, à
  chaque taille de tampon essayée. La ligne se lit donc ici comme **le coût du
  mixage** : 0.045 ms pour 735 trames de sortie à travers tous les seize
  canaux, environ 11 760 tirages de canal — autour de 4 ns par tirage.
- **sur du vrai matériel, une soumission peut attendre** de la place dans le
  tampon du périphérique, et la ligne porte alors cette attente. Ce n'est pas
  une forme nouvelle : `present` a toujours inclus la synchronisation de la
  copie, pour la même raison — la ligne est la phase telle que la frame la
  paie. L'exercice 1 découpe la ligne pour que vous voyiez quelle moitié est
  laquelle sur votre machine.

Les pires frames de la phase disent la même chose de l'autre côté. Sur cette
exécution, les nombres `audio` se lisent plancher 0.026 ms, moyenne 0.045 ms,
pire 0.644 ms — et la pire n'est pas le mixage : le render et l'update de cette
frame sont hauts eux aussi (un hoquet de frame entière de la machine, sur une
exécution qui tient sinon 0.03-0.05 ms). La ligne est honnête sur sa pire de la
même façon que `present` — elle ne retaille pas ses valeurs aberrantes avant de
les moyenner.

### L'arithmétique de la frame se referme à nouveau

La prose de la leçon 058 disait que l'arithmétique de la frame se refermait —
`update + render + present = total`, les trois phases rendant compte de la
frame. Puis la leçon 060 a fait grandir une quatrième phase, et la table a
continué d'imprimer trois lignes : la ligne de journal de la frame elle-même
montrait `audio` dans chaque enregistrement tandis que les lignes du budget
sommaient à moins que le propre total du budget. L'écart n'a jamais été un
mystère — il a été nommé et daté (« les lignes de la table couvrent `update +
render + present` et pas la frame entière ; la leçon 070 est le travail de la
clôture ») — mais c'était un écart. Cette ligne le referme :

```
0.001 + 0.045 + 1.523 + 0.732 = 2.302
```

`update + audio + render + present = total`, à la précision imprimée. Dans les
sommes de l'enregistrement lui-même, la fermeture est encore plus serrée : les
phases font en moyenne 2.302252 ms contre 2.302274 ms pour le total — un résidu
de 0.000022 ms, qui est la mesure elle-même (l'horloge lit autour des phases),
exactement le résidu que la leçon 058 a nommé.

Et l'écart que cette ligne referme se voit dans la même table quand on la
retire : `0.001 + 1.523 + 0.732 = 2.256` des 2.302 — les lignes manquent au
total de 0.046 ms comme imprimé, 0.045 ms à la précision des sommes
elles-mêmes. Cette différence *est* la phase audio. Trois lignes ne décrivaient
pas cette frame ; quatre le font.

### Vérifiée contre le journal de l'exécution elle-même

Chaque nombre ci-dessus est vérifiable contre les lignes `frame N:` de
l'exécution — et la vérification de rédaction de cette leçon a fait exactement
cela : faire la moyenne des phases sur les 580 lignes de frame de l'exécution
reproduit la table ligne pour ligne.

| Ligne | Imprimé | Moyenné depuis le journal |
| ----- | ------- | ------------------------- |
| update | 0.001 | 0.001 |
| **audio** | **0.045** | **0.045** |
| render | 1.523 | 1.523 |
| sprites | 0.001 | 0.001 |
| text | 0.008 | 0.008 |
| tilemap | 1.047 | 1.047 |
| present | 0.732 | 0.732 |
| total | 2.302 | 2.302 |

Les lignes de frame contre lesquelles elle a été réconciliée ressemblent à
ceci — le nombre audio est celui que la nouvelle ligne somme :

```
frame 1: update 0.001 ms, audio 0.033 ms, render 1.738 ms (sprites 0.001, text 0.006, tilemap 0.914), present 0.751 ms, total 2.521 ms
frame 2: update 0.001 ms, audio 0.033 ms, render 1.522 ms (sprites 0.001, text 0.007, tilemap 1.123), present 0.807 ms, total 2.363 ms
frame 3: update 0.001 ms, audio 0.034 ms, render 1.409 ms (sprites 0.001, text 0.008, tilemap 0.977), present 0.872 ms, total 2.316 ms
```

Un budget dont les nombres sont en désaccord avec son propre registre est une
décoration ; celui-ci est le registre — maintenant pour le son aussi.

### Le format que la partie 5 fait grandir

La leçon 058 a appelé cette table un *premier jet* et a dit que la clôture la
fait grandir — nommant « le son » comme l'exemple d'une ligne qui arrive quand
son sous-système existe. Elle est arrivée, et ce qu'elle ajoute est exactement
ce qui était promis : **une phase nommée de plus, mesurée comme toutes les
autres**. Ce que cette ligne ne change *pas*, c'est la partie qui compte :

- les lignes sont des sommes mesurées sur les frames de l'exécution — jamais
  des estimations ;
- les parts sont celles de la frame moyenne ;
- l'en-tête porte le compte, la moyenne et la pire ;
- les intérieurs nommés d'une phase vivent à l'intérieur de sa ligne, jamais à
  sa place.

Ces règles sont ce qui fait que les nombres veulent dire ce qu'ils disent, et
elles sont inchangées depuis que la leçon 058 les a écrites. Ce que la clôture
ajoute à côté d'elles — des colonnes avant/après pour le menu d'optimisation en
trois passes, la machine et les options de compilation nommées avec les
nombres — se pose par-dessus. La ligne du son est la preuve qu'une nouvelle
phase peut rejoindre cette table sans négocier avec elle.

## Étape de code

Un seul changement pour cette leçon : le `PrintFrameBudget` de `frame.cpp`
accueille la ligne `audio` — la phase que l'enregistrement porte depuis la
leçon 060, sommée par le même cumul, désormais imprimée dans l'ordre de
l'enregistrement lui-même entre `update` et `render`, sous les règles
inchangées de la table. Rien d'autre ne bouge : la démo, les champs de
l'enregistrement, le format de la ligne de journal et le cumul sont ceux de la
leçon 069, intacts. Son état final est étiqueté `lesson-070`.

```diff
diff --git a/src/frame.cpp b/src/frame.cpp
index d15549b..6868e33 100644
--- a/src/frame.cpp
+++ b/src/frame.cpp
@@ -39,11 +39,12 @@ void PrintFrameBudget(const FrameStats &stats)
                 stats.worst_number);
 
     /* The attribution: every row a measured sum, every share of the
-       average frame. The named phases live inside render — they say
-       where it went, they do not replace it. The audio phase (lesson
-       060) is summed like the rest but gets no row here: the table's
-       sound rows are lesson 070's, and that is deliberate. */
+       average frame. The phases take the record's own order — update,
+       audio, render, present — and the named phases live inside render:
+       they say where it went, they do not replace it. Lesson 070: the
+       audio phase, measured since lesson 060, gets its row at last. */
     double update = stats.update_sum / n * 1e3;
+    double audio = stats.audio_sum / n * 1e3;
     double render = stats.render_sum / n * 1e3;
     double sprites = stats.sprites_sum / n * 1e3;
     double text = stats.text_sum / n * 1e3;
@@ -52,6 +53,8 @@ void PrintFrameBudget(const FrameStats &stats)
     std::printf("engine:   subsystem   avg ms    share\n");
     std::printf("engine:   update      %6.3f      %2.0f%%\n", update,
                 100.0 * update / (avg * 1e3));
+    std::printf("engine:   audio       %6.3f      %2.0f%%\n", audio,
+                100.0 * audio / (avg * 1e3));
     std::printf("engine:   render      %6.3f      %2.0f%%\n", render,
                 100.0 * render / (avg * 1e3));
     std::printf("engine:     sprites   %6.3f      %2.0f%%\n", sprites,
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre
l'état final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Les deux intérieurs de la ligne *(extend-the-code)*

La ligne `audio` enveloppe deux choses : le mixage et sa soumission. Découpez
la ligne comme le render est découpé — nommez les deux intérieurs dans
l'enregistrement de frame, portez-les dans la ligne `frame N:`, et imprimez-les
comme des lignes en retrait sous `audio`, à l'intérieur d'elle plutôt qu'à sa
place. Lancez la démo et réconciliez : les deux lignes s'additionnent en la
ligne audio exactement comme les phases nommées expliquent le render. Répondez
ensuite à la question de la ligne : l'intérieur de la soumission ne lit presque
rien sur cette machine — que porte-t-il sur du matériel qui pousse en retour,
et quelle ligne déjà dans la table est son jumeau structurel ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-070/ex1.md)

### Exercice 2 — Les deux populations de la ligne *(measure-the-performance)*

La ligne de la table est une moyenne sur toutes les frames, mais les frames ne
sont pas toutes pareilles : une frame qui a alimenté un tampon mixe 735 trames
de sortie à travers tous les seize canaux ; une frame qui n'a rien alimenté
n'en mixe aucune. Séparez les deux populations — comptez-les et gardez leurs
sommes audio à part — et rapportez la moyenne de chacune depuis une vraie
exécution. Réconciliez les frames d'alimentation avec le travail que représente
un tampon, et la moyenne de toute l'exécution avec la ligne de la table ; dites
pourquoi les deux s'accordent, en termes de tailles des populations. Si votre
exécution n'a qu'une seule population, dites pourquoi à partir de l'attente
cadencée — et ce qu'il faudrait pour voir l'autre. Terminez par la question du
budget : quelle part de la pire frame la phase a-t-elle prise, en quoi cela
diffère-t-il de sa part de la frame moyenne, et qu'est-ce qui ferait de cette
ligne la première à grandir ?

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-070/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 069 — la démo de clôture](lesson-069-demo.md) ·
**Suivante :** — ·
**Étiquette de code :** [`lesson-070`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-070)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-070-audio-row.md`,
révision `cc7fcb3`.*

<!-- translation-source: book/lessons/part-3/lesson-070-audio-row.md @ cc7fcb3 -->
