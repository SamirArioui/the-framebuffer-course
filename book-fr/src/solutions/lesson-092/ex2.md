# Solution : exercice 2 — La pause qui rencontre le hitstop

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La pause qui rencontre le hitstop](../../lessons/part-5/lesson-092-hitstop-shake.md) de la leçon 092.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-092/ex2.patch}}
```

## Visite guidée

**La prédiction, écrite d'abord.** L'échelle du temps de jeu est l'échelle de
l'état *multipliée par* le facteur du hitstop. Le jeu est à `1.00` ; la pause à
`0.00` ; le hitstop à `0.25` tant qu'il dure. Pendant la pause, le produit est
donc `0.00 × 0.25 = 0.00` — le monde reste **immobile**, pas rampant : zéro fois
n'importe quoi fait zéro, et la pause l'emporte purement et simplement. Le
compte à rebours du hitstop, lui, est en temps mural (l'horloge de la leçon 078)
— il continue donc de tourner pendant la pause et `hitstop rested` atterrit
*pendant que le jeu est figé*. Et la reprise : le hitstop est alors depuis
longtemps parti, si bien que le jeu reprend à pleine vitesse sans rien suspendu
au-dessus de lui.

**L'exécution** — le coup fatal de l'effectif jetable, `Escape` pressé juste
après que le coup a atterri, la sonde imprimant les deux facteurs et le compte à
rebours propre du hitstop à chaque frame de la fenêtre :

```
engine: hit: bolt hits bag — damage 1, health 1 -> 0
engine: feel: hitstop fired (0.25x, 0.15s)
engine: feel: shake fired (5 px, 0.25s)
engine: feel: hitstop fired (0.25x, 0.30s)
engine: feel: shake fired (10 px, 0.50s)
frame 11: step 42.239 ms, …
engine: probe: state play, scale 1.00 x hitstop 0.25 = 0.25 (hitstop 0.26s left)
frame 12: step 11.024 ms, …
engine: probe: state play, scale 1.00 x hitstop 0.25 = 0.25 (hitstop 0.21s left)
frame 13: step 10.805 ms, …
engine: probe: state play, scale 1.00 x hitstop 0.25 = 0.25 (hitstop 0.17s left)
frame 14: step 10.738 ms, …
engine: probe: state play, scale 1.00 x hitstop 0.25 = 0.25 (hitstop 0.13s left)
frame 15: step 10.803 ms, …
engine: state play -> pause (the player paused)
engine: probe: state pause, scale 0.00 x hitstop 0.25 = 0.00 (hitstop 0.08s left)
frame 16: step 0.000 ms, …
engine: probe: state pause, scale 0.00 x hitstop 0.25 = 0.00 (hitstop 0.08s left)
frame 17: step 0.000 ms, …
engine: probe: state pause, scale 0.00 x hitstop 0.25 = 0.00 (hitstop 0.04s left)
engine: feel: hitstop rested — full speed again
frame 18: step 0.000 ms, …
engine: probe: state pause, scale 0.00 x hitstop 1.00 = 0.00 (hitstop 0.00s left)
```

Chaque morceau de la prédiction est là. Avant la pause : le produit
`1.00 × 0.25 = 0.25` et le step à `10.8 ms` contre les `43 ms` cadencés de
l'exécution — la fenêtre au quart de vitesse de l'exécution de la leçon. La pause
tombe en plein hitstop : le produit de la sonde bascule à `0.00` (`0.00 × 0.25`),
le step passe à `0.000 ms`, et `hitstop rested — full speed again` s'imprime
**entre deux frames en pause** — l'horloge murale du hook a continué de tourner
pendant que le monde restait immobile, et il s'est reposé à sa propre échéance
sans que personne n'avance quoi que ce soit. La secousse a fait la même chose
quelques frames plus tard dans la même exécution : `engine: feel: shake rested at
0,0`, son décalage se posant à exactement zéro pendant que le jeu était encore en
pause.

Et la reprise :

```
engine: state pause -> play (the player resumed)
frame 53: step 22.696 ms, …
frame 54: step 6.239 ms, …
frame 55: step 13.629 ms, …
frame 56: step 42.818 ms, …
frame 57: step 43.688 ms, …
```

De nouveau à pleine vitesse cadencée — aucun ralentissement résiduel. (Les frames
53–55 sont le propre tremblement du pilote scripté, des frames arrivant en rafale
autour de la touche de reprise : leurs steps sont petits parce que leurs pas
muraux étaient petits, pas à cause d'une échelle. Les frames cadencées d'après
sont les `43 ms` habituels de l'exécution.)

**Pourquoi le produit, et pas un if-else.** Une pause qui vérifierait « un
hitstop tourne-t-il ? » et le traiterait à part serait une règle de plus à tenir
en phase avec les hooks. Le produit n'a besoin d'aucune règle : quoi que dise
l'état, quoi que dise le hitstop, une multiplication tranche — et les cas limites
tombent gratuitement. Un hitstop ne peut pas survivre à lui-même sous une pause
(son horloge est celle du mur). Une pause ne peut pas être annulée par un hitstop
(zéro fois n'importe quoi fait zéro). Et l'enregistrement de frame reste honnête
de bout en bout — `step 0.000` est le monde qui n'avance réellement rien,
exactement comme le contrat de la leçon 079 l'a promis, à toute échelle.

*Page traduite de la version anglaise `book/solutions/lesson-092/ex2.md`,
révision `6f25fd1`.*

<!-- translation-source: book/solutions/lesson-092/ex2.md @ 6f25fd1 -->
