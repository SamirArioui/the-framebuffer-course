# Solution : exercice 3 — CSI, SS3, et votre terminal

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 3 — CSI, SS3, et votre terminal](../../lessons/part-0/lesson-021-terminal-input.md) de la leçon 021.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-021/ex3.patch}}
```

## Visite guidée

Les flèches ont deux orthographes en octets. `ESC [ A` est une séquence
*CSI* (l'introducteur de séquence de contrôle) ; `ESC O A` en est une *SS3*,
envoyée quand le terminal est en mode « touches de curseur applicatives » —
un mode que les éditeurs plein écran comme vim activent pour pouvoir
distinguer le pavé numérique. Laquelle vos flèches produisent dépend du
terminal *et* de ce qui tourne dedans, aussi un parseur qui ne connaît que `[`
casse-t-il sur la machine de quelqu'un — et le travail de portabilité dans le
code terminal commence par demander au terminal ce qu'il envoie : `cat -v`
(ou `showkey -a` sur Linux) imprime `^[[A` pour CSI et `^[OA` pour SS3 — `^[`
est l'orthographe d'ESC de `cat -v`.

Le patch ajoute le chemin SS3 à l'état moyen du parseur : `[` ou `O` fait
passer la machine à « octet final de séquence ensuite », et le mapping
`A`-`D` est partagé par les deux orthographes. Vérifié ici avec
`printf '\033ODq' | ./snek 30` (SS3 gauche, puis quitter) :
`frame=1 tick=0 dir=left`, `done after 1 frames`, tandis que la forme CSI
fonctionne toujours inchangée. Sur Windows Terminal sous WSL le pty parle les
mêmes octets ; une construction en console Windows nue aurait besoin de
`ReadConsoleInput` au lieu de `termios` — un autre port, une autre API.

*Page traduite de la version anglaise `book/solutions/lesson-021/ex3.md`,
révision `2369864`.*

<!-- translation-source: book/solutions/lesson-021/ex3.md @ 2369864 -->
