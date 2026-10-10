# Solution : exercice 1 — La démo de la partie 2 sur votre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La démo de la partie 2 sur votre machine](../../lessons/part-2/lesson-057-demo.md) de la leçon 057.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-057/ex1.patch}}
```

## Visite guidée

Le cumul dit ce que coûtent les frames ; l'instrument ajoute *combien de temps*
l'exécution a duré et ce que cela donne en moyenne — une ligne qui transforme un
cumul par frame en un rythme comparable d'une machine à l'autre :

```
engine: 26 frames — avg 2.068 ms (update 0.022, render 1.436 incl. sprites 0.001, text 0.007, tilemap 0.963, present 0.610)
engine: 4.77 s measured, 5.4 frames/s while awake
```

(Cette exécution : 26 frames en 4,77 secondes de *fonctionnement*, pilotées par
une entrée scriptée. « Pendant l'éveil » est le qualificatif honnête — le moteur
attend entre deux nouvelles, donc c'est un rythme de frames, pas de CPU.)

Maintenant le portage. Sur un vrai bureau, la même démo donne des nombres
différents, et ces différences sont votre rédaction :

- **Le coût du present bouge le plus.** Sous Xvfb, la copie se fait en quelques
  centaines de microsecondes vers un écran virtuel ; sur un bureau, il y a un
  vrai pilote, un compositeur, et le rafraîchissement d'un moniteur sur le
  chemin. La leçon 043 de la partie 1 avait trouvé la même forme — le present
  est la part de la frame qui appartient à la machine.
- **La ligne du tilemap est la vôtre.** `tilemap 0.963 ms` est du travail
  CPU : 1 536 blits à travers un build `-O0`. Elle bougera avec votre CPU et
  avec la taille du monde — la dernière question de l'exercice (redimensionnez
  le monde : modifiez `assets/map.txt` pour 64×64 cases) la transforme en
  courbe de mise à l'échelle sur votre machine.
- **Les phases nommées sont le sujet.** Avec le render découpé en
  `sprites / text / tilemap`, comparer des machines revient à comparer des
  *lignes*, pas seulement des totaux. Si votre render vaut 3× celui du livre
  mais que votre ligne tilemap vaut 3× elle aussi, ce n'est pas le moteur de
  rendu qui est plus lent — c'est le parcours.

Consignez la machine avec les nombres (CPU, options du build, Xvfb ou bureau).
La colonne du livre et la vôtre sont deux mesures honnêtes du même code ; la
forme — le tilemap qui domine le render, sprites et texte quasi gratuits, le
present propriétaire d'une part qui dépend de la machine — est ce qui doit
survivre au voyage.

*Page traduite de la version anglaise `book/solutions/lesson-057/ex1.md`,
révision `b2e596c`.*

<!-- translation-source: book/solutions/lesson-057/ex1.md @ b2e596c -->
