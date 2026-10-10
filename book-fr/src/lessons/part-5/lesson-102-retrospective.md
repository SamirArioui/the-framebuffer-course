# Leçon 102 — la rétrospective : notre moteur face aux moteurs réels

{{#include ../../stability-horizon.md}}

## Prose

Le jeu est terminé. La checklist figée du MVD est complète ligne par
ligne, et le rapport de la clôture attribue chaque sous-système de chaque
frame que le jeu terminé a dépensée — mesuré, jamais modélisé, sur une
machine nommée (design D12). Avant que la dernière leçon passe le moteur,
celle-ci regarde en arrière honnêtement puis en avant vers l'horizon :
**ce qu'est ce moteur, face aux moteurs sur lesquels les jeux sont
réellement livrés** — et **la carte de l'épilogue**, les deux pièces de
travail qui viendraient ensuite si le cours continuait. Toutes deux sont
du matériel d'après-cours par décision, pas par périmètre. La règle du
MVD depuis le premier jour de la partie 5 : une idée hors du contrat figé
est consignée comme extra et n'entre jamais dans le périmètre de la
partie. Cette leçon est là où cette règle est tenue — par écrit.

### Notre moteur face aux moteurs réels

Comparez ce qui est comparable, et avec des nombres nommés. Tout ce qui
suit est ce moteur à l'état de `lesson-101`, mesuré sur cette machine
(WSL2, Xvfb `:99`, pas de matériel sonore, la compilation `-O0` du cours)
— la table de la clôture, `2,583` vraies frames du jeu terminé joué à
fond.

**Les pixels.** Le nôtre est un framebuffer logiciel : le CPU écrit
chacun des 640×480 pixels d'une frame. `ClearBuffer` remplit les 307 200
pixels, `DrawTileMap` parcourt les cellules de la caméra et blitte chaque
tuile à travers `BlitSprite`, les sprites et le texte de `FontGlyph` se
posent par-dessus, et `platform::Present` copie tout le tampon vers la
fenêtre une fois par frame — l'unique boucle de copie du moteur. La
clôture a mesuré cette frame : `render 0.799 ms` (dont `clear 0.244` et
`tilemap 0.538`), `present 0.438 ms` de temps mural pour `0.004 ms` de
CPU — l'attente de la couture sur le serveur X — `1.313 ms` une frame de
jeu au total. Un jeu 2D livré dessine à travers un GPU : le CPU construit
une liste de commandes, les textures vivent en VRAM, et les pixels de la
frame ne sont jamais ceux du CPU ; la copie de l'affichage est un échange
de tampons. Ce que la différence leur achète, c'est l'échelle —
résolution, effets, nombres de particules, tout sur le budget du GPU — et
ce qu'elle leur coûte, c'est une pile de pilotes, des chaînes d'outils de
shaders, du débogage propre au GPU, et des frames plus difficiles à
attribuer. Ce que le nôtre achète à la place, c'est la lisibilité :
chaque octet est à nous de lire, et le compte de frame nomme le coût de
chaque phase à trois décimales.

**Les entités.** Le nôtre est un magasin unique et figé — `ENTITY_CAP`
lignes, créées à partir de définitions de table, refusées de façon typée
quand il est plein, sans jamais voler — avec une marche qui exprime une
fois le travail par entité et des comportements (`chase`/`keep`/`flee`,
et le schéma du boss) qui écrivent une requête de déplacement à travers
le mover comme l'entrée du joueur écrit celle du héros. Les moteurs de
production font tourner des entity-component systems, des graphes de
scène et des ordonnanceurs de tâches sur des milliers d'entités. Ce que
la différence leur achète, c'est la composition et l'échelle ; ce que le
nôtre achète à la place, c'est une histoire de mémoire qui ne change
jamais pendant que le jeu tourne — aucune allocation après le démarrage,
un refus au lieu d'une surprise, et la politique que le magasin a
enseignée à la leçon 074 : le travail cosmétique peut être abandonné, le
travail de gameplay non.

**Les données.** Les nôtres sont du texte écrit à la main : les tables
sous `assets/` nomment leurs colonnes dans un en-tête, se chargent
entières dans l'arena, et échouent de façon typée, complet ou nommé ; le
jeu change quand le fichier change, sans recompilation. Les pipelines de
production cuisinent des ressources binaires à travers des importateurs,
des validateurs et des outils de contenu. Ce que la différence leur
achète, c'est la sûreté de pipeline à l'échelle d'une équipe ; ce que le
nôtre achète à la place, c'est un format lisible de bout en bout — et un
seul chargeur, dans une seule paire de fichiers, que vous avez déjà lu.

**Le son.** Le nôtre est notre propre mixeur : seize canaux
(`AUDIO_MIXER_CHANNELS`), des WAV chargés par notre propre chargeur, une
alimentation mono 16 bits à 44.1 kHz à travers la couture, et — sur cette
machine — tout le mixage rendu dans le silence parce qu'il n'y a pas de
périphérique sonore. Les jeux se livrent sur des middlewares (FMOD,
Wwise) : bus, effets, occlusion, streaming et outils de création. Ce que
la différence leur achète, c'est l'outillage de production ; ce que le
nôtre achète à la place, c'est un chemin de signal où chaque échantillon
est trouvable.

**La boucle.** La nôtre est un seul thread, pilotée par les événements,
sous la loi du langage : aucune allocation pendant que le jeu tourne,
aucune exception, rien dans le dos de la couture. Il n'y a pas de pas de
temps fixe ni d'interpolation — des non-objectifs nommés des designs
antérieurs, pas des accidents. Les moteurs livrés font tourner des
systèmes de tâches sur plusieurs cœurs et interpolent leurs mouvements.
Ce que la différence leur achète, c'est l'échelle des cœurs et un
mouvement régulier à n'importe quelle fréquence de frames ; ce que le
nôtre achète à la place, c'est que les bugs de concurrence ne peuvent pas
exister ici, et que l'enregistrement de frame (le contrat de la leçon
079 : le temps mural à n'importe quelle échelle) reste honnête.

**La couture.** La nôtre est `platform.h` — le contrat qu'un second OS
implémente, la réponse d'un OS en deux fichiers (`platform_x11.cpp`,
`platform_alsa.cpp`), et `tools/check-boundary.sh` qui impose qu'aucun
fichier du moteur ne nomme un OS. SDL et GLFW couvrent des dizaines de
plateformes et de périphériques ; les leurs achètent la portée, la nôtre
achète une couture assez petite pour tenir dans une tête — et un second
OS est un travail borné, pas un portage.

**La discipline.** La nôtre, c'est le compte de frame plus `gprof` plus
le recensement de ce que le compilateur a réellement émis — et le menu
figé en trois passes : mesurer, corriger exactement ce que la mesure a
nommé, rapporter ce qu'on n'a pas corrigé. Les deux passes ont dépensé
leurs correctifs sur le dessin de la carte et le clear
(`1.437 → 0.782 ms` ensemble) et n'ont rien touché d'autre ; tout le
reste, mesuré et nommé, siège au registre du travail futur. Les moteurs
livrés ont des render-docs, des profileurs de plateforme et des budgets
appliqués en CI. Ce que la différence leur achète, c'est la précision sur
toute la pile ; ce que le nôtre achète à la place, c'est la même
discipline sans aucun outil à apprendre d'abord — et l'habitude survit à
chaque outil.

Lisez toute la comparaison en une phrase : **ce moteur échange l'échelle
contre la lisibilité**, et l'échange est délibéré. Une mise en garde
avant de citer un nombre : les nôtres ont été mesurés en `-O0` sur une
machine sans écran — les *formes* sont les affirmations (les lignes, les
parts, le registre), pas les millisecondes, qui bougent avec chaque
machine (D12).

### La carte de l'épilogue — après-cours, pas périmètre

Si le cours continuait, deux pièces de travail viendraient ensuite. Elles
ne font **pas** partie du périmètre de ce cours et ne sont **pas** des
obligations de votre jeu. Elles sont consignées ici pour que le registre
soit honnête sur l'endroit où est l'horizon.

**Un portage GPU.** Ce qu'il changerait : les octets de `Framebuffer`
cessent d'être les pixels — le clear, le dessin de la carte, les
sprites et le texte deviennent du travail pour une surface possédée par
le GPU ; `platform::Present` devient un échange ; les lignes du compte de
frame changent de forme (les lignes du render cessent d'être un coût
CPU) ; et l'élément « format des pixels des tuiles » du registre se
résout de lui-même, parce qu'une texture est au format du GPU au
chargement. Ce qui survivrait intact : la couche de jeu entière — états,
vagues, combat, IA, la boîte à outils — les tables et leur format, les
politiques du magasin, le temps de jeu, la discipline du compte de frame,
et la couture (`platform.h` grandit d'une moitié rendu, ou le portage vit
derrière `Present`). Son premier jalon, s'il arrive un jour : garder le
contrat de pixels du moteur (des octets BGRA) et ne remplacer que le
chemin de présentation, en mesurant à chaque étape — et il n'arrive que
quand la mesure nomme une ligne dont le CPU ne peut pas se délester.
« Les GPU sont plus rapides » n'est pas une mesure.

**Un module plateforme Windows.** Le contrat de `platform.h` dans les
termes d'un OS : une fenêtre (ouvrir, fermer, le titre contre lequel
l'exécution rapporte), l'état des touches (`KeyDown`/`KeyPressed`, les
neuf touches mappées), le focus, `Now`, `PageSize`, la paire de
réservation de mémoire, la paire lecture/écriture de fichier, la pompe à
événements, la demande de fermeture, `Present` — les mêmes octets BGRA,
la même promesse « la copie a eu lieu » — et le contrat d'alimentation de
la sortie audio mappé sur WASAPI ou DirectSound. Deux fichiers, un par
responsabilité d'OS, exactement comme `platform_x11.cpp` et
`platform_alsa.cpp` sont deux fichiers aujourd'hui ; la liste de
`tools/check-boundary.sh` grandit pour les nommer ; et **aucun fichier du
moteur ne change** — la promesse auditée par la leçon 042. Ce que le
portage doit garder : les échecs typés (chaque nom d'erreur est celui du
contrat), chaque rapport d'exécution, et les démonstrations qui tournent à
nouveau sans changement. Ce qui peut légitimement différer : le
cadencement de la boucle (un vrai périphérique sonore cadence
l'alimentation ; notre machine était cadencée par des secousses à
~25 fps), le partage CPU/attente de la ligne `present`, et les noms
physiques des touches. Celui-ci n'est pas de l'architecture spéculative —
c'est la couture existante, implémentée une seconde fois.

**Tout le reste est extra — consigné, jamais ajouté.** Le registre, tel
qu'il se présente à la clôture du cours :

- la copie du present (un present à double tampon ou MIT-SHM — un
  changement de couche plateforme que le menu n'a pas fait ; le portage
  GPU ou le module Windows le résoudrait) ;
- le format des pixels des tuiles (le changement de disposition de
  « copier plus large » — la mesure doit le demander) ;
- le mixage audio à pleine charge (les seize canaux jouant tous
  ensemble — non mesuré ici) ;
- l'update à un magasin plein de 64 emplacements (non mesuré ici) ;
- l'impression des rapports eux-mêmes à l'intérieur des phases mesurées
  (une exécution plus silencieuse mesure des phases moins chères) ;
- un cinquième effet de ressenti (hors périmètre par définition — la
  boîte à outils en compte quatre, et c'est la règle que la leçon 092 a
  tenue et que cette leçon tient encore) ;
- un pas de temps fixe, l'interpolation, un entity-component system, des
  threads, un éditeur, du multijoueur, un second mode de jeu (des
  non-objectifs de design, consignés pour que personne ne les redécouvre
  comme des idées).

Chaque ligne ci-dessus est une décision de *ne pas* construire — tenue
comme le MVD a dit de la tenir : consignée comme extra, jamais ajoutée.
Le périmètre est une promesse ; la checklist décide. Votre jeu aura
besoin du même registre — c'est le travail de passation de la dernière
leçon.

### Ce que cette page a vérifié, et ce qu'elle n'a pas vérifié

- **Les nombres de la rétrospective ne sont pas de nouvelles mesures.**
  Chaque nombre ci-dessus est un nombre que les leçons antérieures ont
  mesuré et cité avec leur machine (D12) : le rapport de la clôture
  (l'exécution de dix manches de la leçon 101), le partage CPU/attente de
  la couture (l'exercice 1 de la leçon 098), les lignes avant/après des
  deux passes (leçons 099-100). Cette page n'a rien mesuré.
- **La page elle-même a été vérifiée là où une page peut l'être** : elle
  se rend (`mdbook build`), et ses affirmations sur le code nomment des
  symboles et des structures qui existent à l'état final de cette leçon —
  dont le diff contre `lesson-101` est vide (l'étape de code ci-dessous).

Ce que cette page n'a **pas** vérifié, c'est quoi que ce soit de la carte
de l'épilogue : le portage GPU et le module Windows sont du travail cadré
qui n'a pas été tenté, et rien ici ne doit se lire comme une promesse
qu'ils fonctionnent. Voilà ce que veut dire « du matériel d'après-cours
plutôt que du périmètre », dit à voix haute.

## Étape de code

**L'étape de code de cette leçon est vide, à dessein.** La rétrospective
n'ajoute aucun comportement : rien sous `src/` ni `assets/` ne change, et
`git diff lesson-101 lesson-102 -- src/` n'imprime rien du tout. Le bloc
de diff du gabarit serait vide — voici l'énoncé honnête au lieu d'un bloc
vide. L'étiquette `lesson-102` marque le même code que `lesson-101`, avec
cette page à côté ; la règle du co-commit tient dans les deux cas (le
code et le texte à la leçon N, et ici le contenu de l'étape de code est :
aucun). Si vous attendiez une modification symbolique pour justifier
l'étiquette — il n'y en a aucune à faire, et en inventer une serait la
malhonnêteté.

## Exercices

Deux, et tous deux sont de l'écriture et du jugement plutôt que du code —
cette leçon n'enseigne aucun nouveau mécanisme. Chacun se termine par sa
solution après l'énoncé.

### Exercice 1 — Notre moteur, honnêtement comparé *(explain-in-prose)*

Choisissez un sous-système de ce moteur — les pixels, les entités, les
données, le son, la boucle ou la couture — et écrivez sa note honnête
d'une page : au plus une page, trois affirmations sur ce que fait le
nôtre, chacune portant un nombre que le cours a mesuré (nommez la leçon
où il a été mesuré et la machine sur laquelle il l'a été), trois faits
sur ce que font les moteurs de production à la place, et un jugement
final dans vos mots : ce que l'écart leur achète et leur coûte, et ce que
le nôtre achète à la place. Le livrable est la page ; il n'y a pas de
code à écrire dans cet exercice.

> **Solution :** [ex1 — visite guidée](../../solutions/lesson-102/ex1.md)

### Exercice 2 — La carte de l'épilogue sur votre machine *(port-to-your-own-machine)*

La carte de l'épilogue ci-dessus a été cadrée sur une machine sans GPU,
sans périphérique sonore et avec un seul OS. Lancez le moteur terminé sur
votre machine — une vraie fenêtre, un vrai périphérique sonore si vous en
avez un — et réordonnez la carte pour votre bureau : laquelle des deux
pièces votre machine exige-t-elle en premier, et quelles lignes du
rapport le décident ; puis cadrez le premier jalon de cette pièce dans
les termes de votre machine (pour le module : les fonctions du contrat
dans les API de votre OS ; pour le portage : quelles lignes du rapport de
budget de frames meurent, lesquelles survivent, et qu'est-ce qui
remplace la boucle de copie). Dites ce que votre machine a changé dans le
tableau. Le livrable est la carte ; il n'y a pas de code à écrire dans
cet exercice.

> **Solution :** [ex2 — visite guidée](../../solutions/lesson-102/ex2.md)

---

**Partie :** [Partie 5 — le jeu](../../index.md) ·
**Précédente :** [Leçon 101 — passe 3 : le rapport de budget de frames](lesson-101-frame-budget.md) ·
**Suivante :** [Leçon 103 — maintenant, faites VOTRE jeu](lesson-103-your-game.md) ·
**Étiquette de code :** [`lesson-102`](https://github.com/SamirArioui/the-framebuffer-course/tree/lesson-102)

*Page traduite de la version anglaise `book/lessons/part-5/lesson-102-retrospective.md`,
révision `4ef60c6`.*

<!-- translation-source: book/lessons/part-5/lesson-102-retrospective.md @ 4ef60c6 -->
