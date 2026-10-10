# Solution : exercice 2 — Où se trouve la frontière

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Où se trouve la frontière](../../lessons/part-1/lesson-027-first-window.md) de la leçon 027.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-027/ex2.patch}}
```

## Visite guidée

L'affirmation — *le code du moteur ne nomme jamais un type d'OS* — n'est pas
une règle de style à prendre pour argent comptant ; elle est visible dans les
objets que le compilateur produit. Construisez l'état final de la leçon et
regardez les deux fichiers objet (`build.sh` garde un objet par source, donc
la frontière apparaît sous forme de deux fichiers) :

```
$ nm build/obj/main.o | c++filt
0000000000000000 T engine::Run()
0000000000000000 r engine::WINDOW_WIDTH
0000000000000004 r engine::WINDOW_HEIGHT
                 U platform::OpenWindow(int, int)
                 U platform::CloseWindow(platform::Window*)
                 U fprintf
                 U getchar
000000000000009b T main
                 U printf
                 U puts
                 U stderr
```

La prédiction tient : `main.o` référence les deux fonctions de la couture et
la bibliothèque standard C, et pas un seul nom `X*`. Les deux lignes `U` qu'il
a vraiment sont la couture elle-même — `platform::OpenWindow` et
`platform::CloseWindow`, avec `platform::Window*` qui dé-mangle vers un
*pointeur vers un type incomplet* : le fichier objet porte le nom et aucune
idée de ce qu'il y a dedans. (`puts` est là parce que le compilateur a
transformé un `printf` d'une seule ligne constante en cet appel — le même
genre de réflexion « en quoi ça se compile » sur laquelle la loi du langage
est bâtie.)

L'autre côté de la frontière :

```
$ nm build/obj/platform_x11.o | c++filt
                 U XCloseDisplay
                 U XCreateSimpleWindow
                 U XDestroyWindow
                 U XFlush
                 U XMapWindow
                 U XOpenDisplay
                 U XStoreName
0000000000000000 T platform::OpenWindow(int, int)
000000000000018a T platform::CloseWindow(platform::Window*)
0000000000000000 b platform::window_state
                 U __stack_chk_fail
```

Chaque référence `X*` de tout le programme vit dans ce seul objet — les
symboles indéfinis sont exactement les appels Xlib que l'implémentation fait,
et les deux symboles `T` sont les mêmes fonctions de la couture, définies ici.

Ce qu'un portage Win32 toucherait, alors, est décidé par l'étape de liaison :
une implémentation Win32 fournit `platform::OpenWindow` et
`platform::CloseWindow` avec les mêmes signatures, déménage les entrailles de
`window_state` vers les propres handles de Win32, et `main.o` est *réutilisé
tel quel* — il a été compilé contre `platform.h` et ne connaît rien d'autre.
Seul le fichier jumeau de `platform_x11.cpp` (et l'unique bibliothèque d'OS de
la ligne de liaison) change.

Le diff ci-dessus est le plus petit changement de confirmation qui soit : une
ligne d'instrumentation à l'intérieur de l'implémentation, pour qu'une
exécution nomme sa propre frontière —

```
platform: x11 implementation active
engine: window 640x480 open — press enter to close
```

— pendant que le moteur reste muet sur l'OS à qui il parle. Il ne peut pas le
savoir. C'est ça, la couture.

*Page traduite de la version anglaise `book/solutions/lesson-027/ex2.md`, révision `a8e1cba`.*

<!-- translation-source: book/solutions/lesson-027/ex2.md @ a8e1cba -->
