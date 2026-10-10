# Solution : exercice 2 — Ce que le mot sait

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Ce que le mot sait](../../lessons/part-5/lesson-100-clear.md) de la leçon 100.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-100/ex2.patch}}
```

## Visite guidée

La sonde vérifie l'argument au démarrage — le moteur répondant aux trois
affirmations sur sa propre mémoire, sur le vrai tampon :

```
engine: probe: word fill — pixels aligned to 4: yes, row stride ok; cleared 1,2,3 reads 1,2,3 at the corners and 1,2,3 in the middle; raw bytes 3,2,1,0
```

**Affirmation 1 — alignement : le pointeur de mot a le droit d'écrire
cette mémoire.** Un stockage d'`unsigned int` veut une adresse alignée sur
4. Deux faits le garantissent, et la sonde vérifie les deux :
`GetFramebuffer` remet le tampon de pixels depuis l'arena avec un
alignement de 4096 octets (plus large que ce dont le stockage a besoin),
et le pas de ligne est `width × 4` — un multiple de 4 quelle que soit la
largeur, si bien que chaque ligne commence alignée si le tampon l'est. La
sonde imprime `pixels aligned to 4: yes, row stride ok`. Si l'un des deux
était faux — un framebuffer enveloppant de la mémoire choisie par
quelqu'un d'autre — le correctif aurait besoin d'un tampon de transit
aligné ou d'un repli sur les stockages d'octets : le chemin rapide est
une *revendication sur l'allocateur*, pas un fait du C++.

**Affirmation 2 — ordre des octets : les octets du mot atterrissent là
où les stockages d'octets les mettaient.** C'est l'affirmation pour
laquelle le `memcpy` existe. Le moteur définit un pixel comme quatre
*octets* dans l'ordre — bleu, vert, rouge, zéro — et ne dit jamais ce
qu'est un entier. Composer le mot en copiant ces quatre octets exacts
dans un `unsigned int` laisse la machine choisir sa propre
représentation ; stocker le mot de nouveau dans la même mémoire défait
exactement la représentation. Sur une machine petit boutiste, le mot du
`ClearBuffer(fb, 1, 2, 3)` de la sonde est `0x00010203` ; sur une machine
gros boutiste ce serait `0x03020100` — et les deux produisent les octets
`3, 2, 1, 0`, ce que la sonde lit au niveau brut. Si nous avions plutôt
écrit `color = r<<16 | g<<8 | b` et supposé la disposition, le code
serait plus rapide à lire et faux sur toute machine qui n'est pas
d'accord avec la nôtre. Les octets sont le contrat ; le mot est un détail
d'implémentation que nous laissons au compilateur.

**Affirmation 3 — aliasing : l'écriture peut passer par un type
différent.** Le tampon est de la mémoire d'arena — du stockage alloué
sans type déclaré — et un stockage à travers `unsigned int *` lui donne
ce type effectif ; relire des octets à travers `unsigned char *` est
légal quoi qu'il arrive (les types caractère peuvent inspecter la
représentation de n'importe quel objet). La sonde referme la boucle en
lisant la même mémoire des deux façons : `GetPixel` (comme pixels : 1,2,3
à deux coins et au milieu) et les octets bruts (`3,2,1,0` — exactement la
sortie des stockages d'octets). Lire des deux façons est l'argument :
quoi que l'optimiseur croie sur les types, la mémoire dit la même chose à
travers les deux fenêtres.

Pour une machine où une affirmation échoue, le changement est local :
mémoire non alignée → aligner l'allocation ou garder les stockages
d'octets (un plancher de correction sous toute vitesse) ; un ordre des
octets que le `memcpy` gère déjà — cette affirmation *ne peut pas*
échouer par construction, ce qui est toute la raison de composer le mot
ainsi ; et pour l'aliasing, la relecture est déjà l'énoncé le plus fort
que le C++ portable puisse faire — une assertion statique sur
`sizeof(unsigned int) == 4` est la seule garde qui vaut la peine d'être
ajoutée si vous voulez que la machine refuse un mot qui n'est pas large
de quatre octets.

Une note de bas de page honnête : la sonde efface le framebuffer au
démarrage — le clear de la frame suivante l'écrase, donc l'exécution
n'est pas affectée, mais une sonde qui écrit devrait le dire (celle-ci le
fait, dans son commentaire), et la vérification de forme de la
transcription dans une leçon ultérieure verra la ligne de la sonde comme
un gabarit de plus. Les sondes ont le droit de toucher ; elles n'ont pas
le droit de se taire à ce sujet.

*Page traduite de la version anglaise `book/solutions/lesson-100/ex2.md`,
révision `72d9add`.*

<!-- translation-source: book/solutions/lesson-100/ex2.md @ 72d9add -->
