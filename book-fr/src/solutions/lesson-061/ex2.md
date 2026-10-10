# Solution : exercice 2 — Le fichier qui finit trop tôt

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le fichier qui finit trop tôt](../../lessons/part-3/lesson-061-wav.md) de la leçon 061.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-061/ex2.patch}}
```

## Visite guidée

D'abord l'expérience, car le correctif ne vaut rien sans elle. Le refus de la
marche — `ok = ok && chunk <= size - body;` — disparaît, et le fichier est coupé
à 20000 octets avec sa taille RIFF qui suit la coupe (19992) pour que la première
déclaration du conteneur s'accorde encore avec les octets. La déclaration du
chunk `data` est le mensonge qui reste : elle dit toujours 44100. Construit avec
l'instrument de la leçon 013 —

```
CXXFLAGS="-std=c++17 -O0 -g -Wall -Wextra -fsanitize=address" \
LDFLAGS="-lX11 -lasound -fsanitize=address" ./build.sh
```

— l'exécution n'échoue pas de façon typée. Elle avorte :

```
==377548==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x52a000011020
READ of size 1 at 0x52a000011020 thread T0
    #0 ... in ReadFrame src/audio.cpp:42
    #1 ... in engine::LoadSample(engine::Arena&, char const*) src/audio.cpp:169
    #2 ... in engine::Run() src/main.cpp:133
0x52a000011020 is located 0 bytes after 20000-byte region
allocated by thread T0 here:
    #1 ... in platform::ReadFile(char const*) src/platform_x11.cpp:291
```

Lisez les deux moitiés de ce rapport ensemble : la copie dans `LoadSample` lit un
octet au-delà du tampon de 20000 octets alloué par `ReadFile` — les trames que le
chunk `data` *déclare* courent 44100 octets après leur début, et la copie croit
la déclaration plutôt que les octets. AddressSanitizer remplit les régions issues
de `malloc` d'octets de garde empoisonnés et nomme la pile d'appels exacte, voilà
pourquoi l'avortement pointe droit sur `ReadFrame`. Sans le sanitizer, la même
lecture atterrit dans le mou du tas et l'exécution « marche » — un échantillon
dont la queue est tout ce que le tas avait sous la main. Un fichier tronqué qui
produit du charabia silencieux est strictement pire qu'un plantage, et c'est tout
l'objet de l'exercice : **une déclaration n'est pas des octets, et un chargeur
qui suit une déclaration au-delà de la fin d'un fichier n'est pas un chargeur.**

Le correctif a deux moitiés, et les deux comptent :

- **Le refus de la marche revient.** C'est la vérification propre à cette leçon,
  et il reste là où il était : un chunk dont la taille dépasse le fichier rend
  tout le chargement `SAMPLE_MALFORMED`, avant que quoi que ce soit ne soit
  copié. Le patch ne le montre pas parce que le sabotage l'avait supprimé et que
  le correctif le restaure exactement à la ligne que livre la leçon.
- **La copie se dote de sa propre borne** — la partie que le patch montre, elle.
  Juste avant que la première trame ne soit prise,
  `frames_at + frames_bytes > size` est refusé de façon typée, et le retour
  arrière (`ArenaRollback` jusqu'à la marque prise avant l'allocation) fait
  qu'un chargement refusé ne laisse aucune trame partielle derrière lui. La
  déclaration est désormais vérifiée là où elle est *faite* (la marche) et là où
  elle est *dépensée* (la copie) : la seconde vérification coûte une comparaison
  par chargement et garantit qu'aucune vérification manquée en amont ne peut
  devenir un dépassement.

La même exécution, avec le correctif, sous le même sanitizer :

```
engine: assets/tone.wav: could not load (malformed)
```

Code de sortie 1, aucun rapport d'AddressSanitizer, et l'échec typé nomme le
fichier et la classe d'échec — la même ligne que les autres corruptions de la
leçon impriment. Le mensonge du fichier coûte à l'exécution une valeur d'échec,
jamais sa sûreté.

Une note de frontière, puisque cet exercice appuie dessus : la sonde est celle du
compilateur, pas du moteur — `-fsanitize=address` n'apparaît jamais dans les
valeurs par défaut de `build.sh` et aucun code du moteur ne sait qu'elle existe.
Le chargeur de la leçon est inchangé en esprit par le correctif ; il cesse
simplement de croire une déclaration ne serait-ce que le temps d'une copie.

*Page traduite de la version anglaise `book/solutions/lesson-061/ex2.md`,
révision `f3af761`.*

<!-- translation-source: book/solutions/lesson-061/ex2.md @ f3af761 -->
