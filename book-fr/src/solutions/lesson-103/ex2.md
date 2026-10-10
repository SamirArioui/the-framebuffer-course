# Solution : exercice 2 — La carte de passation de votre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La carte de passation de votre machine](../../lessons/part-5/lesson-103-your-game.md) de la leçon 103.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-103/ex2.patch}}
```

## Visite guidée

Le plus petit changement qui confirme qui soit : le nom de la machine. Le
rapport l'imprime déjà (`RUN_MACHINE` dans `src/main.cpp` — la constante
introduite par l'étape de code de la leçon 101) et la carte de l'exercice n'est
honnête que quand le nom est le vôtre, donc le diff est une seule chaîne.
Remplissez le placeholder par une vraie description — le CPU, l'OS, l'écran, le
périphérique sonore — car c'est cette chaîne qui rend tous les autres nombres de
la carte citables (D12 : un nombre qui a perdu sa machine est une rumeur).

La carte elle-même, dans l'ordre où l'écrire :

1. **Le rapport tel que votre machine l'imprime** — toute la table, plus les
   lignes by state, budget et machine. Le nôtre, pour comparaison (WSL2, Xvfb
   `:99`, pas de matériel sonore, `-O0`) : `avg 1.283 ms` sur 2 583 frames,
   `tilemap 0.538 / clear 0.244 / sprites 0.006 / text 0.011` dans
   `render 0.799`, `present 0.438`, `0 of 2583` frames au-dessus du budget de
   16.667 ms.
2. **Ce que votre machine a changé, ligne par ligne** — et *pourquoi*, ce qui
   est le vrai contenu de la carte. Attendez-vous à des différences
   structurelles, pas scalaires : `present` change de forme sur un affichage
   local (notre serveur X facturait son attente en temps mural avec presque
   aucun CPU ; un bureau local peut au contraire la facturer au processus),
   `audio` devient un vrai mixage de périphérique au lieu du silence, et la
   *fréquence* de frame devient celle de l'alimentation audio (~60) plutôt que
   les ~25 de nos secousses — donc votre *nombre* de frames et votre rythme
   diffèrent avant même tout coût. Les décalages scalaires (chaque ligne CPU
   s'échelonne avec votre processeur et vos options de compilation) sont
   attendus ; les décalages de forme sont les lignes intéressantes.
3. **La première entrée du registre des extras** — une idée que votre jeu a eue
   pendant que vous l'exécutiez (il en aura une ; exécuter son propre jeu est un
   générateur d'idées), consignée, pas ajoutée : un nom, un périmètre d'une
   ligne, et les mots *hors périmètre*. Cette entrée est la discipline qui
   commence à fonctionner. Le registre de la leçon 102 montre la forme achevée.
4. **Ce que la carte demande en premier** — lisez-le sur vos *propres* lignes :
   la plus grosse phase sur votre machine est votre point chaud, et le menu en
   trois passes s'y applique comme il s'est appliqué ici (mesurez de nouveau
   avant la correction, ne corrigez que ce que la mesure a nommé, rapportez ce
   que vous n'avez pas corrigé). Si la réponse de votre machine est « rien n'est
   près du budget », c'est un constat, lui aussi — dites-le, et allez construire
   le jeu avant d'optimiser quoi que ce soit.

Une mise en garde que la carte existe pour prévenir : **ne comparez pas vos
millisecondes aux nôtres comme un verdict sur l'une ou l'autre machine.**
Matériel différent, options de compilation différentes, formes d'exécution
différentes — la comparaison n'a de sens que de forme de ligne à forme de ligne,
et chaque nombre appartient à son nom. Voilà ce que la carte de passation garde
honnête, et c'est tout le design D12 mis entre vos mains, en train de
fonctionner.

Quand la carte est remplie, la ligne de perf de la checklist attend votre
réponse — *ma machine tient 60 fps*, ou honnêtement non, avec les frames qui
ratent comptées (l'exercice 1 de la leçon 101 a enseigné l'instrument ; cette
carte est la discipline autour) — et le cours n'a plus d'affirmations à faire
sur votre jeu. Mesurez bien.

*Page traduite de la version anglaise `book/solutions/lesson-103/ex2.md`,
révision `cfeaecd`.*

<!-- translation-source: book/solutions/lesson-103/ex2.md @ cfeaecd -->
