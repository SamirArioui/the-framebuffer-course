# Solution : exercice 1 — La couche plateforme terminée, sur votre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La couche plateforme terminée, sur votre machine](../../lessons/part-1/lesson-043-demo.md) de la leçon 043.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-043/ex1.patch}}
```

## Visite guidée

Le cumul dit ce que coûtent les frames ; l'instrument ajoute *combien de temps*
l'exécution a duré et ce que cela donne en moyenne — une ligne qui transforme un
cumul par frame en un rythme comparable d'une machine à l'autre :

```
engine: 21 frames — avg 0.904 ms (update 0.000, render 0.439, present 0.465)
engine: worst frame 1.797 ms (frame 1); present is 51% of the frame
engine: 1.82 s measured, 11.5 frames/s while awake
```

(Cette exécution : 21 frames en 1,82 seconde de *fonctionnement*, pilotées par
une entrée scriptée. « Pendant l'éveil » est le qualificatif honnête — le moteur
dort entre deux nouvelles, donc le rythme est un rythme de frames, pas de CPU.)

Maintenant le portage. Sur un vrai bureau, la même exécution donne des nombres
différents, et ces différences sont votre rédaction :

- **Le coût du present bouge le plus.** Sous Xvfb, la copie se fait en quelques
  centaines de microsecondes vers un écran virtuel. Sur un bureau, il y a un
  vrai pilote, peut-être un compositeur, et le rafraîchissement d'un vrai
  moniteur sur le chemin. Les nombres de la leçon 036 étaient honnêtes pour
  *leur* machine ; les vôtres le sont pour la vôtre.
- **Le coût du rendu bouge à peine.** `ClearBuffer` et le marqueur sont de
  l'arithmétique du moteur — ce nombre parle de votre CPU, pas de votre écran.
- **Le rythme est celui de votre entrée.** Les frames arrivent quand les
  nouvelles arrivent (la note de rythme de la leçon 034) — donc l'auto-repeat
  d'un clavier mécanique, un pavé tactile lent et un collègue qui martèle les
  flèches donnent tous des rythmes de frames différents pour le même moteur.

Consignez la machine sur laquelle vous avez mesuré (c'est ce qu'une mesure
*est*), et comparez la forme plutôt que les valeurs : le present en part de la
frame, la pire frame contre la moyenne, l'update toujours négligeable. Si votre
forme diffère — disons, le present à 90 % de la frame — ce n'est pas une erreur
à corriger ; c'est le sujet du travail de budget de frames de la partie 5.

*Page traduite de la version anglaise `book/solutions/lesson-043/ex1.md`,
révision `5ec1553`.*

<!-- translation-source: book/solutions/lesson-043/ex1.md @ 5ec1553 -->
