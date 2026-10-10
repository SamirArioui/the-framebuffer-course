# Solution : exercice 1 — La secousse qui décroît

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La secousse qui décroît](../../lessons/part-2/lesson-054-camera.md) de la leçon 054.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-054/ex1.patch}}
```

## Visite guidée

Le patch transforme l'onde carrée en paliers : la secousse dure 30 frames et
son amplitude descend 6 → 4 → 2 → 0 par tiers, pendant que le signe continue
d'alterner. Le rapport imprime l'additif chaque fois qu'il *change*, si bien
que le journal est le chemin complet de l'effet plutôt qu'une ligne par frame.
Appui sur espace, puis une entrée maintenue pour que les frames tournent :

```
engine: camera additive 6,0 (shake starts)
engine: camera additive 6,0
engine: camera additive -6,0
engine: camera additive 6,0
engine: camera additive -6,0
engine: camera additive 6,0
engine: camera additive -4,0
engine: camera additive 4,0
engine: camera additive -4,0
engine: camera additive 4,0
engine: camera additive -4,0
engine: camera additive 4,0
engine: camera additive -4,0
engine: camera additive 4,0
engine: camera additive -2,0
engine: camera additive 2,0
engine: camera additive -2,0
engine: camera additive 2,0
engine: camera additive -2,0
engine: camera additive 2,0
engine: camera additive -2,0
engine: camera additive 2,0
engine: camera additive 0,0
engine: camera additive 0,0 (at rest)
```

Le chemin est exactement le design : six pour le premier tiers, quatre pour le
deuxième, deux pour le dernier, avec alternance de signe à chaque frame — puis
exactement `0,0`. (La ligne `0,0` avant « at rest » est le rapport de
changement qui se déclenche quand le dernier palier d'amplitude tombe ; la
ligne entre parenthèses est l'exécution qui dit où vit le hook.)

Ce que la décroissance apporte au *ressenti* : une secousse constante se lit
comme un bug — l'écran vibre et continue de vibrer. Une secousse qui décroît
se lit comme un *événement* — quelque chose est arrivé, et sa force s'est
dépensée. L'œil suit l'enveloppe (l'amplitude qui descend) plus que
l'oscillation ; c'est l'enveloppe qui fait ressentir l'effet comme une
conséquence plutôt que comme du bruit. C'est la forme que la boîte à outils de
la partie 4 généralise (des courbes d'easing au lieu de paliers, une durée au
lieu d'un nombre de frames) — mais l'*idée* est ici, en quatre lignes.

Ce que le *rapport* apporte : le journal ci-dessus est une trace complète de
la machine à états du hook, une ligne par changement. Quand la boîte à outils
du juice pilotera plus tard ce champ depuis trois effets à la fois, le même
rapport montrera quel effet a écrit quoi, et quand — et une secousse qui ne
revient jamais à zéro (le bug classique) se voit d'un coup d'œil : un journal
qui se termine sans la ligne entre parenthèses. Rapportez le hook, pas
seulement la frame.

Une dernière chose que l'exercice met au jour : la secousse ne tourne que tant
que les frames tournent — et les frames arrivent quand les nouvelles arrivent
(la note de rythme de la leçon 034). C'est le maintien d'une touche pendant la
secousse qui a permis aux 30 frames de s'accomplir dans la transcription
ci-dessus ; une machine au repos étalerait ces 30 frames sur des minutes. Une
décroissance fondée sur le temps (`shake_seconds` compté sur l'horloge de la
plateforme plutôt qu'en nombre de frames) est ce que veut un vrai effet — et
c'est une excellente prochaine étape à partir d'ici.

*Page traduite de la version anglaise `book/solutions/lesson-054/ex1.md`,
révision `b7f3628`.*

<!-- translation-source: book/solutions/lesson-054/ex1.md @ b7f3628 -->
