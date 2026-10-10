# Solution : exercice 1 — La démo de la partie 3 sur votre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La démo de la partie 3 sur votre machine](../../lessons/part-3/lesson-069-demo.md) de la leçon 069.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-069/ex1.patch}}
```

## Visite guidée

Le cumul dit ce que l'exécution a fait et ce qu'elle a coûté ; l'instrument
ajoute *combien de temps* elle a duré et ce à quoi ses deux moitiés aboutissent
en moyenne — une ligne qui transforme les comptes de la démo en taux par
lesquels une machine peut être comparée :

```
engine: demo: 466 frames measured, 466 buffers fed, 21 effects fired, 2 music wraps
engine: demo: 8.00 s run, 58.3 frames/s while awake, sound fed at 58.3 buffers/s
engine: frame budget — 466 frames, avg 2.129 ms, worst 3.612 ms (frame 115)
engine:   subsystem   avg ms    share
engine:   update       0.001       0%
engine:   render       1.403      66%
engine:     sprites    0.002       0%
engine:     text       0.008       0%
engine:     tilemap    0.974      46%
engine:   present      0.687      32%
engine:   total        2.129     100%
```

(Cette exécution : 466 frames en 8.00 secondes, non pilotée — 58.3 frames/s
pendant qu'elle est éveillée, et le son alimenté exactement aux mêmes
58.3 tampons/s. « Pendant qu'elle est éveillée » est la nuance honnête : la
boucle attend entre les nouvelles, donc ce sont des taux de l'exécution, pas du
CPU. Les deux taux qui coïncident sont la signature de l'attente cadencée — une
boucle non pilotée alimente un tampon par frame, à un horizon d'écart.)

Maintenant, le portage. Sur un vrai bureau, la même démo donne des nombres
différents, et les différences font le compte rendu :

- **Le coût de la présentation bouge le plus.** Sous Xvfb, la copie fait
  quelques centaines de microsecondes contre un écran virtuel ; sur un bureau,
  il y a un vrai pilote, un compositeur et le rafraîchissement d'un moniteur
  sur le chemin. La leçon 058 de la partie 2 a trouvé la même forme — `present`
  est la ligne qui appartient à la machine.
- **La ligne du tilemap est la vôtre.** `tilemap 0.974 ms` est du travail CPU :
  1 536 blits à travers un build `-O0`. Elle bouge avec votre CPU et avec la
  taille du monde ; tout le reste du render est du bruit à côté.
- **Les nombres audio ont deux nouvelles questions que seul votre matériel
  peut trancher.** Sur cette machine, la soumission revient immédiatement —
  `null` prend les échantillons et les jette — donc la phase se lit comme le
  coût du mixage et les deux taux ci-dessus coïncident. Sur du vrai matériel,
  une soumission peut attendre de la place dans le tampon du périphérique. Si
  la vôtre le fait, ça se voit exactement ici : le taux du son reste épinglé
  près de 60 tampons/s par l'horizon du tampon tandis que les frames/s et les
  nombres `audio` par frame grandissent autour de l'attente. Comparez la
  colonne `audio` de vos lignes `frame N:` aux 0.028-0.034 ms du livre — une
  phase qui lit parfois des millisecondes n'est pas un mixage plus lent, c'est
  un périphérique qui pousse en retour.

Consignez la machine avec les nombres (CPU, options de compilation, Xvfb ou
bureau, et le nom du périphérique ALSA — `aplay -l` vous dit ce qu'il y a
vraiment). Et la dernière question est à vous seul : cette machine de rédaction
n'a aucun haut-parleur — `null` a accepté chaque échantillon et n'a produit
aucun son — donc ce que la démo *donne à entendre*, la musique qui boucle sous
une rafale d'effets, est le seul fait que le livre ne peut pas imprimer.

*Page traduite de la version anglaise `book/solutions/lesson-069/ex1.md`,
révision `f5e9029`.*

<!-- translation-source: book/solutions/lesson-069/ex1.md @ f5e9029 -->
