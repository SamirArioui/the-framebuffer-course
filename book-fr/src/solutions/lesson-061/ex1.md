# Solution : exercice 1 — Réécrire le fichier

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Réécrire le fichier](../../lessons/part-3/lesson-061-wav.md) de la leçon 061.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-061/ex1.patch}}
```

## Visite guidée

L'écrivain est l'inverse du lecteur, et le patch le construit comme le chargeur
est construit : trois petits assistants `Put` dans l'espace de noms anonyme à
côté des assistants `Read`, puis une fonction qui assemble un fichier. Là où
`ReadU32` défait quatre octets petit boutiste, `PutU32` les repose ; là où
`ReadFrame` décode une valeur dont le signe est au bit 15, `PutFrame` prend
`(unsigned short)frame` — la valeur en complément à deux, trames négatives
comprises — et l'écrit octet de poids faible d'abord. Si ces deux-là ne
s'accordent pas sur ce qu'une trame *est*, rien d'autre dans le format ne peut
les sauver.

`WriteSample` écrit la marche de la leçon à l'envers : le tampon fait
`44 + frame_count * 2` octets dans l'arena, l'identifiant RIFF, la déclaration
`size - 8`, la forme WAVE, les seize octets de faits du chunk `fmt ` (l'étiquette,
le canal, la fréquence, le débit en octets qui vaut `AUDIO_RATE * 2`,
l'alignement de bloc qui vaut 2, les bits), puis l'identifiant `data` et la
taille propre des trames, puis les trames. Chaque champ qu'un lecteur vérifiera
est écrit pour passer cette vérification — l'écrivain n'est pas un second avis
sur le format, c'est le même avis dans l'autre sens.

Une habitude à remarquer : les octets assemblés sont de purs jetables, donc la
marque et le retour arrière les encadrent — gardé ou refusé, l'arena ne conserve
pas quarante-quatre kilo-octets d'image de fichier. `platform::WriteFile` prend
les octets avant le retour arrière, exactement comme les octets de `ReadFile`
repartent avec `ReleaseFile`.

L'aller-retour dans `Run` écrit l'échantillon chargé dans un fichier à lui,
recharge ce fichier avec `LoadSample`, et compare chaque trame. D'une vraie
exécution de l'état final de cette leçon plus le patch :

```
engine: sample: 22050 frames at 44100 Hz, 1 channel, first frames: 0 513 1024 1531 2032 2525 3009 3480
engine: round trip: 22050 frames written and read back, every frame agrees
```

Et le fichier lui-même, comparé à l'asset du cours en dehors de l'exécution :
`assets/tone-roundtrip.wav` fait 44144 octets et est **identique octet pour
octet** à `assets/tone.wav` — en-tête et trames, tout. L'écrivain n'a pas
seulement produit un fichier que le chargeur tolère ; il a produit *ce* fichier.

Voilà ce que l'aller-retour prouve qu'un vidage hexadécimal ne peut pas prouver.
Un vidage hexadécimal montre les octets d'un fichier et la patience d'un lecteur ;
l'aller-retour met deux implémentations du format face à un vrai fichier et
vérifie qu'elles s'accordent sur chaque champ et chaque trame — les déclarations
du conteneur comme les octets des trames. Un vidage hexadécimal n'aurait pas
attrapé un bogue de signe dans `PutFrame` (le fichier aurait encore « l'air »
d'un WAV) ; la comparaison de −1 contre −1 l'attrape aussitôt. Quand vous écrirez
plus tard un fichier que le chargeur refuse — oubliez la règle de remplissage,
mentez dans le débit en octets — les deux moitiés se disputeront en public, et
l'échec typé nommera quelle déclaration a perdu.

Rien ici ne touche le chargeur, la couture ou le flux de l'exécution :
l'aller-retour est une vérification à côté du chargement, et le fichier qu'il
écrit est à vous, à garder ou à supprimer.

*Page traduite de la version anglaise `book/solutions/lesson-061/ex1.md`,
révision `f3af761`.*

<!-- translation-source: book/solutions/lesson-061/ex1.md @ f3af761 -->
