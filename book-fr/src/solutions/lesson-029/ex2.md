# Solution : exercice 2 — Le registre

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le registre](../../lessons/part-1/lesson-029-clean-close.md) de la leçon 029.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-029/ex2.patch}}
```

## Visite guidée

Le registre fait une ligne par acquisition et une par libération, imprimées
depuis l'intérieur de l'implémentation, là où chacune se produit réellement.
Avant de lancer quoi que ce soit, la prédiction à faire est un bilan pour
chaque sortie : combien d'acquisitions, et quelles libérations les
équilibrent.

**La fermeture de l'utilisateur — la demande (`ClientMessage`) :** trois
acquisitions (display, window, signal handler), trois libérations
correspondantes.

```
platform: take display
platform: take window
platform: take signal handler
platform: release window
platform: release display
platform: release signal handler
```

**La fenêtre détruite (`DestroyNotify`) :** les mêmes trois acquisitions — mais
aucune ligne `release window`. La surprise que la prédiction doit attraper :
l'OS a libéré cette ressource lui-même quand il a détruit la fenêtre. Le
registre le dit (`window released by the OS`), et `CloseWindow` saute la
destruction qu'il ne possède plus. Le bilan se clôt quand même — trois
acquisitions, trois libérations — dont une effectuée par l'OS.

```
platform: take display
platform: take window
platform: take signal handler
platform: window released by the OS
platform: release display
platform: release signal handler
```

**Ctrl+C (`SIGINT`) :** identique à la fermeture propre — l'interruption est
une nouvelle qui devient la même fermeture, et les mêmes trois libérations
s'exécutent. C'est la sortie que la leçon 028 ne pouvait pas gérer du tout.

```
platform: take display
platform: take window
platform: take signal handler
platform: release window
platform: release display
platform: release signal handler
```

**Le display manquant :** le registre est *vide*. La panne arrive avant la
première acquisition, donc il n'y a rien à libérer — la règle tient
trivialement, et la ligne d'erreur du moteur est la seule sortie.

```
engine: no display to open a window on
```

Le motif à travers les quatre : les libérations équilibrent les acquisitions
sur chaque chemin, et la seule ligne qui bouge est *qui* a libéré la fenêtre.
C'est ce que « ressources de l'OS libérées à chaque sortie » veut dire quand
on le mesure au lieu de l'espérer — et le registre est le plus petit
instrument qui le mesure.

*Page traduite de la version anglaise `book/solutions/lesson-029/ex2.md`, révision `71a2428`.*

<!-- translation-source: book/solutions/lesson-029/ex2.md @ 71a2428 -->
