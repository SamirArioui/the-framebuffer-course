# Leçon 061 — le conteneur WAV

{{#include ../../stability-horizon.md}}

## Prose

La leçon 059 a laissé une promesse dans son propre commentaire : les trames
qu'elle calculait reviendraient plus tard comme asset. Cette leçon la tient.
`assets/tone.wav` est sur le disque maintenant, et contient exactement ce que
`GenerateTone` a écrit — 22050 trames d'une sinusoïde de 440 Hz à l'amplitude
0,25, les huit premières trames du fichier valant
`0 513 1024 1531 2032 2525 3009 3480`, tout comme l'exécution de la leçon 059
imprimait les quatre premières de ces mêmes nombres depuis son tampon. Les octets
n'ont pas changé ; la boîte dans laquelle ils voyagent, si. L'idée de cette leçon
est donc unique, et elle porte sur le conteneur : **un fichier WAV est une boîte
de chunks, et le chargeur le parcourt à la main** — parce que les *déclarations*
d'un fichier et les *octets* d'un fichier sont deux choses différentes, et le
métier d'un chargeur est de vérifier qu'elles s'accordent.

### Le conteneur, chunk par chunk

Un fichier WAV est un fichier RIFF, et RIFF est un format de conteneur dont
toute la grammaire tient en un paragraphe : le fichier s'ouvre sur une étiquette
de type de quatre octets (`RIFF`), une taille de quatre octets et une étiquette
de forme de quatre octets (`WAVE`) — puis vient une séquence de **chunks**. Un
chunk est un identifiant de quatre octets, une taille de quatre octets, et
exactement autant d'octets de charge utile, complétés à une longueur paire. Telle
est la grammaire entière. Il n'y a ni table des matières ni décalage à croire :
un lecteur trouve n'importe quoi dans le fichier en parcourant les chunks l'un
après l'autre et en lisant chaque identifiant au fur et à mesure.

`assets/tone.wav` fait 44144 octets. Voici ses quarante-huit premiers :

```
$ xxd assets/tone.wav | head -4
00000000: 5249 4646 68ac 0000 5741 5645 666d 7420  RIFFh...WAVEfmt 
00000010: 1000 0000 0100 0100 44ac 0000 8858 0100  ........D....X..
00000020: 0200 1000 6461 7461 44ac 0000 0000 0102  ....dataD.......
00000030: 0004 fb05 f007 dd09 c10b 980d 620f 1c11  ............b...
```

Lisons-le la grammaire en main. `5249 4646`, c'est `RIFF`, et `68ac 0000` est la
déclaration du conteneur sur sa propre longueur : 0x0000ac68 = 44136, en petit
boutiste — l'octet de poids faible d'abord, le geste de la leçon 014, parce que
les octets du fichier ne sont pas ceux de la machine et que le chargeur dit qui
est qui. 44136, c'est exactement les 44144 octets du fichier moins les huit que
la déclaration ne couvre pas. `5741 5645`, c'est `WAVE` : la forme à l'intérieur
de la boîte.

Puis les chunks. `666d 7420`, c'est `fmt ` — espace compris, quatre octets comme
tout identifiant — et `1000 0000` dit que le chunk fait 16 octets de long. Ces
seize octets sont les faits du format, chacun un nombre petit boutiste de deux ou
quatre octets : `0100` l'étiquette de format (1, PCM linéaire), `0100` un canal,
`44ac 0000` = 44100 la fréquence, `8858 0100` = 88200 le débit en octets, `0200`
l'alignement de bloc, `1000` seize bits. Puis `6461 7461`, c'est `data`, et
`44ac 0000` dit que le chunk contient 44100 octets de trames — 22050 trames d'un
`short` chacune — et ces trames vont jusqu'à la fin du fichier. Les premières
sont déjà visibles dans le vidage : `0000 0102 0004 fb05` vaut 0, 513, 1024,
1531, en petit boutiste. Le rapport de l'exécution imprime les mêmes trames sous
forme de nombres, ce qui est tout l'intérêt de les imprimer.

### Les déclarations et les octets

Chaque champ ci-dessus est une **déclaration**, et une déclaration n'est pas des
octets. Le chargeur de la leçon 044 avait déjà l'habitude — rien dans un fichier
n'est supposé présent avant que les octets ne le disent — et ce chargeur la garde
face à des nombres autorisés à mentir :

- la taille RIFF doit être égale à la longueur du fichier moins huit — un fichier
  dont la déclaration est en désaccord avec sa propre longueur est malformé
  avant même que ses chunks ne soient parcourus ;
- la taille de chaque chunk doit tenir dans les octets que le fichier possède
  réellement. Un chunk `data` déclarant 44100 octets dans un fichier de 20000
  n'est pas « presque un échantillon » ; c'est une déclaration qui dépasse la
  fin, et la suivre est la façon dont un chargeur lit de la mémoire qui n'a
  jamais été celle de son fichier ;
- les faits de `fmt ` doivent être le format du moteur, de façon typée : PCM
  linéaire, un canal, seize bits, 44100 Hz. Rien ici ne rééchantillonne, ne
  convertit ni ne réinterprète une trame pour faire entrer un fichier étranger ;
- les nombres dérivés du chunk `fmt ` doivent s'accorder avec les faits dont ils
  dérivent — le débit en octets vaut 44100 × 1 × 2, l'alignement de bloc vaut
  1 × 2. Un conteneur qui se contredit est malformé avant qu'aucune trame ne
  soit lue ;
- la taille de `data` doit être un nombre entier de trames : une trame est un
  `short`, deux octets, et 44101 octets de données déclarent quelque chose que
  le format ne peut pas contenir ;
- et les trames doivent être les derniers octets du fichier — pas de lecture
  partielle présentée comme un échantillon, et rien après les trames que le
  chargeur comprendrait à moitié.

Pourquoi tant de rigueur ? Parce que les défaillances qu'elle refuse sont pires
que celles qu'elle provoque. Un fichier à 22050 Hz lu comme s'il était à 44100
est une note jouée une octave trop haut pendant moitié moins longtemps — *et
elle sonne bien* : le mauvais son qui marche est le mode de défaillance qu'aucun
rapport n'attrape jamais. Un fichier tronqué dont la queue manquante est lue dans
la mémoire qui se trouve au-delà joue du charabia à la fin d'un son et appelle ça
terminé. Un échec typé est la réponse bon marché et honnête : l'exécution nomme
le fichier, nomme l'échec, et aucun son n'est jamais joué dont le chargeur ne
pourrait se porter garant. « Un chargement produit un échantillon complet ou un
échec typé — jamais des données partielles présentées comme un succès » est la
même règle que la leçon 044 a donnée aux sprites, gardée à l'identique.

### Pourquoi à la main

L'analyse se fait à la main parce qu'aucune bibliothèque ne lit un fichier dans
du code moteur visible par l'élève — le même geste que l'en-tête PPM de la leçon
044, un chunk plus profond. Là-bas, l'en-tête faisait trois lignes d'ASCII ; ici,
ce sont des chunks binaires imbriqués et des entiers petit boutiste, et la
marche est l'idée nouvelle tandis que la lecture octet par octet est celle de la
leçon 014, réutilisée. Une bibliothèque RIFF cacherait exactement le sujet de
cette leçon derrière un appel : la différence entre ce qu'un fichier dit et ce
qu'un fichier a. Ce que l'habitude achète est sur la page ci-dessus — les octets
restent inspectables avec `xxd`, chaque vérification est une ligne que l'élève
peut pointer du doigt, et le chargeur entier est assez petit pour tenir en une
seule lecture. La loi du langage de la leçon 026 vaut au-dessus, inchangée :
aucune allocation en dehors de celle de l'arena, aucune exception, aucune
bibliothèque que la couture ne possède pas.

### Un chargement est complet ou nommé

Le contrat de `LoadSample` est celui de `LoadSprite`, et `SampleResult` le dit :
`SAMPLE_OK` exactement quand le pointeur de trames est non nul, et trois échecs
nommés à côté — `SAMPLE_MISSING` quand le fichier n'est pas là ou ne peut pas
être lu, `SAMPLE_MALFORMED` quand les octets ne sont pas un échantillon complet
dans le format du moteur, `SAMPLE_NO_ROOM` quand l'arena n'a pas de place pour
les trames. Les trames elles-mêmes sont copiées dans l'arena et les octets du
fichier repartent vers l'OS : ce que le moteur garde, c'est sa copie. Et la copie
est encadrée par une marque — `ArenaMark` avant elle, les chemins d'échec faisant
un retour arrière jusqu'à elle — donc un chargement qui refuse ne laisse **aucune
trame partielle derrière lui**. La comptabilité de l'arena dans l'exécution est
là où cette promesse se voit : après un chargement refusé, son compte d'octets
utilisés n'a pas bougé.

La structure que le chargement remplit porte les trames *et les faits dont la
lecture a besoin* — la longueur en trames d'échantillons, la fréquence, les
canaux. Ce n'est pas de la décoration. Les questions que pose la lecture — quand
cela se termine-t-il, et que veut dire en temps l'indice d'une trame — doivent
trouver leur réponse dans l'échantillon seul, jamais dans des suppositions sur le
fichier qui l'a produit. La leçon 062 est exactement cela : le mixeur des leçons
suivantes n'ouvre jamais un fichier et ne devine jamais ; il lit ces champs.

### Ce que cette exécution a vérifié, et ce qu'elle n'a pas vérifié

Tous les nombres ci-dessous viennent de vraies exécutions de l'état final de
cette leçon sur cette machine :

- **L'échantillon se charge complètement.** L'exécution rapporte
  `engine: sample: 22050 frames at 44100 Hz, 1 channel, first frames:
  0 513 1024 1531 2032 2525 3009 3480` — les faits du chunk `fmt ` et les trames
  de `data`, les huit mêmes nombres que la leçon 059 a calculés. Une sonde
  jetable (pas du code moteur) a comparé chacune des 22050 trames chargées aux
  octets du fichier lui-même : les 22050 correspondent exactement.
- **Un fichier manquant échoue de façon typée.** Pointez l'exécution vers un
  répertoire où `assets/tone.wav` est absent : elle dit
  `engine: assets/tone.wav: could not load (missing)` et se termine, le fichier
  nommé et l'échec nommé.
- **Un fichier malformé échoue de façon typée.** Quatre corruptions ont été
  taillées dans le vrai fichier, une déclaration chacune, et toutes rapportent
  `engine: assets/tone.wav: could not load (malformed)` : un fichier tronqué au
  milieu de `data` (20000 octets sur 44144) ; une taille de `data` de 44101 —
  pas un nombre entier de trames ; un chunk `fmt ` déclarant 22050 Hz avec un
  débit en octets qui s'accorde encore avec lui ; et une taille RIFF déclarant
  44137 contre la vraie longueur du fichier. La sonde jetable a fait passer les
  quatre mêmes par `LoadSample` en surveillant l'arena : après chaque refus, le
  compte d'octets utilisés n'avait pas bougé — la promesse du retour arrière,
  vérifiée.

Ce que cette leçon ne fait **pas**, c'est jouer quoi que ce soit. Le flux joue
toujours la tonalité calculée de la leçon 060 — le fichier est chargé et vérifié,
pas encore entendu — et aucun haut-parleur de cette machine n'a émis de son, ni
dans cette leçon ni dans une autre. Les octets sont vérifiés ; l'écoute est pour
vous. La leçon 062 joue le fichier.

## Étape de code

Un seul changement pour cette leçon, du fichier aux trames : `src/audio.h` /
`src/audio.cpp` accueillent `Sample`, ses échecs typés et `LoadSample` — le
conteneur RIFF/WAVE parcouru chunk par chunk et octet par octet, chaque
déclaration vérifiée contre les octets avant qu'aucune trame ne soit copiée.
`src/main.cpp` accueille le démarrage de l'exécution : `assets/tone.wav` est
chargé à côté des autres assets, un chargement réussi rapporte les faits de
l'échantillon et ses premières trames, et un échec termine l'exécution par son
nom comme tout autre asset. Le nouvel asset est l'autre moitié de l'étape de
code : `assets/tone.wav`, les trames que la leçon 059 a calculées, dans le
conteneur que cette leçon définit. Le flux, la boucle et la couture ne sont pas
touchés. Son état final est étiqueté `lesson-061`.

Les octets de l'asset sont binaires, et son diff l'est aussi — git imprime
`Binary files … differ` pour lui. L'en-tête et les premières trames sont le
vidage hexadécimal et la ligne d'exécution ci-dessus ; `git diff --binary`
imprime le patch complet.

```diff
diff --git a/assets/tone.wav b/assets/tone.wav
new file mode 100644
index 0000000..346da99
Binary files /dev/null and b/assets/tone.wav differ
diff --git a/src/audio.cpp b/src/audio.cpp
index e942411..82223bd 100644
--- a/src/audio.cpp
+++ b/src/audio.cpp
@@ -1,15 +1,21 @@
-// audio.cpp — the tone computed by code: arithmetic that becomes sound.
+// audio.cpp — the tone computed by code, and the sample read from a file.
 //
 // Lesson 059: every sample in the engine is a sequence of numbers, and
 // this file makes one out of a sine wave so the numbers can be read,
-// checked, and played before any file format is involved. The same bytes
-// come back later as an asset (lesson 061) and go into the mixer (lesson
-// 063); here they are simply written.
+// checked, and played before any file format is involved.
+//
+// Lesson 061: the same bytes now come back out of a file. LoadSample
+// walks a RIFF/WAVE container chunk by chunk — every size checked against
+// the bytes around it, every format fact checked against the engine's —
+// and copies the frames into the arena. The numbers are the point on both
+// sides: what GenerateTone writes, the loader reads back.
 
 #include "audio.h"
 
 #include <cmath>
 
+#include "platform.h"
+
 namespace engine {
 namespace {
 
@@ -20,6 +26,30 @@ constexpr double TURN = 6.283185307179586;
    by amplitude * 32767 lands inside the format at any amplitude <= 1.0. */
 constexpr double SAMPLE_PEAK = 32767.0;
 
+/* The container's numbers are little-endian — the file's bytes are not
+   the machine's bytes, and this is where the difference is resolved
+   (lesson 014): low byte first, assembled by hand. */
+unsigned ReadU32(const unsigned char *p)
+{
+    return (unsigned)p[0] | ((unsigned)p[1] << 8) | ((unsigned)p[2] << 16) |
+           ((unsigned)p[3] << 24);
+}
+
+/* One sample frame: two little-endian bytes whose count carries its sign
+   in bit 15 — 0x8000 and up are the format's negative frames. */
+short ReadFrame(const unsigned char *p)
+{
+    int count = p[0] | (p[1] << 8);
+    return (short)(count < 0x8000 ? count : count - 0x10000);
+}
+
+/* A four-byte chunk id, exactly — `fmt ` keeps its space. */
+bool IdIs(const unsigned char *p, const char *id)
+{
+    return p[0] == (unsigned char)id[0] && p[1] == (unsigned char)id[1] &&
+           p[2] == (unsigned char)id[2] && p[3] == (unsigned char)id[3];
+}
+
 } /* namespace */
 
 void GenerateTone(short *frames, int frame_count, double frequency,
@@ -35,4 +65,126 @@ void GenerateTone(short *frames, int frame_count, double frequency,
     }
 }
 
+SampleResult LoadSample(Arena &arena, const char *path)
+{
+    SampleResult result = { { 0, 0, 0, 0 }, SAMPLE_OK };
+
+    platform::FileData file = platform::ReadFile(path);
+    if (file.error != platform::FILE_OK) {
+        result.error = SAMPLE_MISSING;
+        return result;
+    }
+
+    const unsigned char *data = file.data;
+    size_t size = file.size;
+
+    /* The container's account of itself: RIFF names the form, claims a
+       length, and says the form inside is WAVE. A claim is only a claim —
+       this one is checked against the bytes the file actually has, and a
+       file whose RIFF size disagrees with its own length is malformed
+       before its chunks are even walked. */
+    bool ok = size >= 12 && IdIs(data, "RIFF") && IdIs(data + 8, "WAVE");
+    ok = ok && ReadU32(data + 4) == size - 8;
+
+    /* The walk: chunk after chunk — an id, a size, then exactly that many
+       bytes (padded to an even length). Chunks are never assumed to be
+       where they "should" be: the walk finds `fmt ` and `data` wherever
+       they are, steps over what it does not know, and refuses any chunk
+       whose own size disagrees with the bytes around it. */
+    size_t at = 12;
+    bool have_format = false, have_frames = false;
+    unsigned rate = 0, channels = 0, bits = 0;
+    size_t frames_at = 0, frames_bytes = 0;
+
+    while (ok && at + 8 <= size) {
+        const unsigned char *id = data + at;
+        size_t body = at + 8;
+        size_t chunk = ReadU32(data + at + 4);
+        ok = ok && chunk <= size - body;
+        if (!ok)
+            break;
+
+        if (IdIs(id, "fmt ")) {
+            /* The format chunk: at least the sixteen bytes of PCM facts.
+               Every fact is checked typed — not mono, not 16-bit, not
+               AUDIO_RATE is not the engine's format, and nothing here
+               will resample or reinterpret a frame to make it fit. The
+               chunk's own derived numbers are checked against the facts
+               they are derived from: the claims must agree with each
+               other as well as with the bytes. */
+            ok = ok && !have_format && chunk >= 16;
+            if (ok) {
+                unsigned format =
+                    (unsigned)data[body] | ((unsigned)data[body + 1] << 8);
+                channels =
+                    (unsigned)data[body + 2] | ((unsigned)data[body + 3] << 8);
+                rate = ReadU32(data + body + 4);
+                unsigned byte_rate = ReadU32(data + body + 8);
+                unsigned block_align = (unsigned)data[body + 12] |
+                                       ((unsigned)data[body + 13] << 8);
+                bits = (unsigned)data[body + 14] |
+                       ((unsigned)data[body + 15] << 8);
+                bool claims_agree =
+                    byte_rate == rate * channels * (bits / 8) &&
+                    block_align == channels * (bits / 8);
+                ok = ok && format == 1 /* linear PCM */ && channels == 1 &&
+                     bits == 16 && rate == (unsigned)AUDIO_RATE &&
+                     claims_agree;
+            }
+            have_format = true;
+        } else if (IdIs(id, "data")) {
+            ok = ok && !have_frames;
+            frames_at = body;
+            frames_bytes = chunk;
+            have_frames = true;
+        }
+        at = body + chunk + (chunk & 1); /* chunks pad to an even length */
+    }
+
+    /* The walk's verdict: both halves found, and the frames a whole
+       number of them — one frame is one short, and a data chunk whose
+       size is not a whole number of frames claims something the format
+       cannot hold. */
+    ok = ok && have_format && have_frames;
+    ok = ok && frames_bytes % 2 == 0;
+
+    if (!ok) {
+        result.error = SAMPLE_MALFORMED;
+        platform::ReleaseFile(file);
+        return result;
+    }
+    int frame_count = (int)(frames_bytes / 2);
+
+    /* The frames, into the arena — the engine keeps its own copy, and the
+       file's bytes go back to the OS. The mark is the load's transaction:
+       from here on a refusal rolls the arena back, and a failed load
+       leaves no partial frames behind. */
+    size_t mark = ArenaMark(arena);
+    short *frames = (short *)ArenaAlloc(arena, frames_bytes, sizeof(short));
+    if (!frames) {
+        result.error = SAMPLE_NO_ROOM;
+        platform::ReleaseFile(file);
+        return result;
+    }
+    for (int i = 0; i < frame_count; ++i)
+        frames[i] = ReadFrame(data + frames_at + (size_t)i * 2);
+    platform::ReleaseFile(file);
+
+    /* The last agreement: the frames are the file's last bytes. The file
+       holds exactly the sample — no short read presented as a sample, and
+       nothing after the frames the loader would half-understand. */
+    if (frames_at + frames_bytes != size) {
+        ArenaRollback(arena, mark);
+        result.error = SAMPLE_MALFORMED;
+        return result;
+    }
+
+    result.sample.frames = frames;
+    result.sample.frame_count = frame_count;
+    result.sample.rate = (int)rate;
+    result.sample.channels = (int)channels;
+    result.error = SAMPLE_OK;
+    return result;
+}
+
 } /* namespace engine */
diff --git a/src/audio.h b/src/audio.h
index 11264b0..f05c1b0 100644
--- a/src/audio.h
+++ b/src/audio.h
@@ -3,12 +3,19 @@
 // Lesson 059: sound is data before it is sound. A sample is a frame of
 // amplitude — one number saying where the speaker sits at that instant —
 // and a sound is a run of those frames at a fixed rate. Nothing here knows
-// about devices, files, or mixing: this is the format the engine's output
-// speaks, the format lesson 061's loader accepts and refuses everything
-// else, and the format the mixer of lessons 063-065 sums.
+// about devices or mixing: this is the format the engine's output speaks
+// and the format the mixer of lessons 063-065 sums.
+//
+// Lesson 061: the format is now loadable. A sample is authored as a file
+// — the same bytes GenerateTone computes, in a RIFF/WAVE container — and
+// the loader below either hands over the complete sample or names what
+// went wrong. From there on the sample carries its own playback facts, so
+// lesson 062 plays it from the sample alone.
 #ifndef AUDIO_H
 #define AUDIO_H
 
+#include "arena.h"
+
 namespace engine {
 
 /* The engine's sample format: 16-bit signed frames at this rate. One
@@ -32,6 +39,34 @@ constexpr int AUDIO_OUTPUT_CHANNELS = 1;
 void GenerateTone(short *frames, int frame_count, double frequency,
                   double amplitude);
 
+/* A loaded sample: the frames, and the facts playback needs carried with
+   them — its length in sample frames and its format. */
+struct Sample {
+    short *frames;
+    int frame_count;
+    int rate;
+    int channels;
+};
+
+enum SampleError {
+    SAMPLE_OK = 0,
+    SAMPLE_MISSING,   /* the file is not there or cannot be read */
+    SAMPLE_MALFORMED, /* the bytes are not a complete sample in the engine's format */
+    SAMPLE_NO_ROOM,   /* the arena had no room for the frames */
+};
+
+struct SampleResult {
+    Sample sample;
+    SampleError error; /* SAMPLE_OK exactly when sample.frames is non-0 */
+};
+
+/* Loads a sample from a RIFF/WAVE file. The container is walked chunk by
+   chunk and byte by byte — no library reads it — and anything that is not
+   a complete sample in the engine's format is refused typed. The frames
+   are copied into the arena and the file's own bytes go back to the OS:
+   what the engine keeps is its copy, and a refused load keeps nothing. */
+SampleResult LoadSample(Arena &arena, const char *path);
+
 } /* namespace engine */
 
 #endif
diff --git a/src/main.cpp b/src/main.cpp
index 5044acc..e46c5bd 100644
--- a/src/main.cpp
+++ b/src/main.cpp
@@ -126,6 +126,42 @@ int Run(void)
     }
     TileSheet &sheet = tiles_loaded.sheet;
 
+    /* Lesson 061: the run's sound as a file's bytes. A sample is frames
+       of amplitude in a container, and the load either yields the
+       complete sample or names what went wrong — like every asset above.
+       A failure ends the run by name, like every asset above. */
+    SampleResult sample_loaded = LoadSample(arena, "assets/tone.wav");
+    if (sample_loaded.error != SAMPLE_OK) {
+        switch (sample_loaded.error) {
+        case SAMPLE_MISSING:
+            std::fprintf(stderr,
+                         "engine: assets/tone.wav: could not load (missing)\n");
+            break;
+        case SAMPLE_MALFORMED:
+            std::fprintf(stderr,
+                         "engine: assets/tone.wav: could not load (malformed)\n");
+            break;
+        default:
+            std::fprintf(stderr,
+                         "engine: assets/tone.wav: could not load (no room)\n");
+            break;
+        }
+        platform::CloseWindow(opened.window);
+        ArenaRelease(arena);
+        return 1;
+    }
+    Sample &sample = sample_loaded.sample;
+
+    /* The byte-level check, before anything is played: the sample's facts
+       and its first frames — the same bytes lesson 059 computed, now read
+       from a file instead. */
+    std::printf("engine: sample: %d frames at %d Hz, %d channel%s, first frames:",
+                sample.frame_count, sample.rate, sample.channels,
+                sample.channels == 1 ? "" : "s");
+    for (int i = 0; i < 8 && i < sample.frame_count; ++i)
+        std::printf(" %d", (int)sample.frames[i]);
+    std::printf("\n");
+
     double sprite_x = 312.0, sprite_y = 232.0;
     double started = platform::Now();
     double last = started;
```

## Exercices

Deux exercices mixtes. Chacun se termine par sa solution — un diff contre l'état
final de cette leçon, plus une visite guidée — après l'énoncé.

### Exercice 1 — Réécrire le fichier *(extend-the-code)*

Le chargeur lit le conteneur de cette leçon ; apprenez au moteur à en écrire un.
Ajoutez un écrivain à côté du chargeur — l'en-tête que la leçon parcourt,
assemblé à la main en octets, et les trames en petit boutiste derrière, à travers
l'écriture de fichier entier de la couture — dans le format du moteur et aucun
autre. Faites ensuite de l'exécution un aller-retour : écrivez l'échantillon
chargé dans un fichier à vous, rechargez ce fichier avec `LoadSample`, et
comparez chaque trame avec l'échantillon sorti — rapportez la première trame qui
diverge, ou que toutes les trames s'accordent. Comparez le fichier que vous avez
écrit à `assets/tone.wav` en dehors de l'exécution. Que prouve l'aller-retour sur
le format qu'un vidage hexadécimal du fichier ne peut pas prouver ?

> **Solution :** [ex1 — diff + visite guidée](../../solutions/lesson-061/ex1.md)

### Exercice 2 — Le fichier qui finit trop tôt *(fix-the-crash)*

Une déclaration n'est pas des octets, et cet exercice rend ce coût visible. Dans
la marche des chunks de `LoadSample`, retirez le refus qui arrête un chunk dont
la taille dépasse le fichier. Puis coupez `assets/tone.wav` à 20000 octets — à
l'intérieur de son chunk `data` — avec sa taille RIFF qui suit la coupe, pour que
la première déclaration du conteneur s'accorde encore avec les octets et que la
déclaration du chunk `data` soit le mensonge qui reste. Reconstruisez avec
l'instrument de la leçon 013 — AddressSanitizer, via les surcharges `CXXFLAGS` et
`LDFLAGS` de `./build.sh` — et lancez. Le chargement n'échoue plus de façon
typée ; lisez ce que le sanitizer nomme. Corrigez ensuite le chargeur pour que le
dépassement soit impossible même là où une vérification est manquée : la copie
elle-même doit refuser de prendre une trame dans des octets que le fichier ne
possède pas, le chargement doit échouer de façon typée, et aucune trame partielle
ne doit survivre dans l'arena. Prouvez que la même exécution est propre sous le
sanitizer et rapporte l'échec typé.

> **Solution :** [ex2 — diff + visite guidée](../../solutions/lesson-061/ex2.md)

---

**Partie :** [Partie 3 — le son](../../index.md) ·
**Précédente :** [Leçon 060 — la forme du flux](lesson-060-stream.md) ·
**Suivante :** [Leçon 062 — les faits de lecture de l'échantillon](lesson-062-playback.md) ·
**Étiquette de code :** [`lesson-061`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-061)

*Page traduite de la version anglaise `book/lessons/part-3/lesson-061-wav.md`, révision `f3af761`.*

<!-- translation-source: book/lessons/part-3/lesson-061-wav.md @ f3af761 -->
