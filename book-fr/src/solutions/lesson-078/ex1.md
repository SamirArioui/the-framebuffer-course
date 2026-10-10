# Solution : exercice 1 — Un hitstop qui se termine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Un hitstop qui se termine](../../lessons/part-4/lesson-078-game-time.md) de la leçon 078.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-078/ex1.patch}}
```

## Visite guidée

Le diff fait du hitstop une chose *déclenchable* : la touche espace règle
l'échelle à un quart et pose une échéance dans la comptabilité de l'exécution —
`hitstop_until` — et l'update vérifie l'échéance à chaque frame et restaure la
pleine vitesse. Le screenshake se repose pour l'exercice (sa démo sur espace
cède la touche) ; tout le reste de l'exécution est intact.

Maintenant la question que l'appel impose : **une durée de quoi ?** L'échéance
est `platform::Now() + 0.2` — du **temps mural** — et c'est le bon choix ici,
pour deux raisons dont la seconde tranche :

1. *Le ressenti se mesure comme il est vécu.* Le hitstop existe pour être un
   moment que le joueur ressent — 0,2 s de sa vraie attention — donc il dure
   0,2 s de temps réel. (Un compte à rebours en temps de jeu à l'échelle 0.25
   durerait 0,8 s de temps réel : le ralentissement étirerait l'effet même qui
   l'a causé.)
2. *Un compte à rebours en temps de jeu ne peut rien terminer pendant que le
   jeu est arrêté.* À l'échelle 0, le pas de jeu vaut 0, donc une minuterie
   avancée par le temps de jeu n'avance jamais — un hitstop programmé en temps
   de jeu se terminerait exactement jamais. Tout effet qui doit *restaurer*
   l'échelle doit tourner sur une horloge que l'échelle n'atteint pas. C'est le
   même argument que la leçon 079 fait à propos de l'enregistrement de frame,
   et c'est pourquoi l'horloge murale de la couture reste l'unique horloge de
   mesure du moteur.

L'exécution, depuis l'état final de cette leçon plus le patch — un appui sur
espace au début de l'exécution :

```
engine: game-time: hitstop fired — scale 0.25 for 0.2 s of wall time
engine: game-time: hitstop over — scale 1.00
```

L'échelle est revenue toute seule, environ 0,2 s d'horloge murale plus tard (la
frame suivante après l'échéance — la boucle de cette machine ne tourne que
quand des nouvelles d'entrée arrivent, donc « toute seule » signifie « à la
prochaine frame dont l'horloge a dépassé l'échéance »). Les pas du héros
rampent pendant la fenêtre du hitstop et reviennent à plein ensuite ; le script
d'échelle de l'exécution tourne alors le bouton de nouveau à trois secondes, ce
qui est l'affaire de la démo et non celle du hitstop.

Le détail de design à voler pour la partie 5 : l'état du hitstop est un nombre
(`hitstop_until`) et la restauration est une comparaison. Quand L11 ajoutera le
screenshake à côté, la même forme fonctionne — une échéance en temps mural,
vérifiée dans l'update — et aucun des deux effets n'a besoin d'une seconde
horloge, d'un callback ou d'une pause dans la boucle.

Une chose que cet exercice laisse délibérément ouverte : que se passe-t-il si
un hitstop est déclenché *pendant qu'un autre tourne* ? Ici, l'échéance est
simplement repoussée et l'échelle re-réglée. C'est un choix de politique —
rafraîchir, ignorer ou empiler — et il appartient au jeu, pas au service,
exactement comme la politique de refus du magasin l'a fait.

Rien ici ne touche le service de l'échelle, la marche ou l'enregistrement de
frame : le patch est un effet déclenchable et son échéance.

*Page traduite de la version anglaise `book/solutions/lesson-078/ex1.md`,
révision `cc66198`.*

<!-- translation-source: book/solutions/lesson-078/ex1.md @ cc66198 -->
