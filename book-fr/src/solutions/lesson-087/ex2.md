# Solution : exercice 2 — Le projectile sans art

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le projectile sans art](../../lessons/part-5/lesson-087-projectiles-weapons.md) de la leçon 087.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-087/ex2.patch}}
```

## Visite guidée

Le plantage se reproduit exactement comme l'exercice le dit : un fichier de
projectiles dont l'en-tête ne nomme aucune colonne `sprite` charge très bien
(c'est la promesse du format — une ligne d'arme ne nomme pas d'art non plus), et
l'exécution meurt à l'instant où son type est tiré :

```
engine: state title -> play (the player started)
engine: fire: hero -> bolt (damage 1, range 160)
Segmentation fault (core dumped)
```

Où et pourquoi : l'image de la définition est chargée depuis la colonne
`sprite`, donc une ligne sans art distribue `image = 0`. `EntityFromDef` la
copie dans l'entité, et le tout premier pas de vol lit la taille du sprite — la
boîte de collision du mover est l'art de l'entité, `ANIM_FRAME_W ×
sprite->height` — et un sprite nul n'a aucune taille à lire. Le projectile est
une entité, et chaque entité dessine et entre en collision *comme son art* :
une définition sans art ne peut pas répondre aux questions auxquelles une entité
doit répondre.

Le correctif est la réponse habituelle du moteur à une valeur dont il ne peut
pas se porter garant : **la refuser typée**. `EntityCreate` répond désormais
`ENTITY_NO_ART` pour une définition sans image, avant que quoi que ce soit en
lise une — et le tir le nomme comme chaque autre refus :

```
engine: fire refused — the kind has no art
```

Rien de légitime ne casse : les lignes d'armes ne portent pas d'art et ne sont
jamais créées comme entités, donc le refus ne se déclenche jamais pour elles.
L'exécution continue de tourner (la ligne ci-dessus vient d'une gâchette
maintenue — un refus par pression, le monde intact) — la défaillance est
redevenue une valeur, pas un plantage. C'est la règle de la leçon 071 appliquée
une couche plus bas : le mauvais jeu qui marche est pire qu'un échec typé qui se
nomme.

*Page traduite de la version anglaise `book/solutions/lesson-087/ex2.md`,
révision `3fd99b7`.*

<!-- translation-source: book/solutions/lesson-087/ex2.md @ 3fd99b7 -->
