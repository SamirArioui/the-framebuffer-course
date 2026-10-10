# Solution : exercice 1 — Le redimensionnement est une nouvelle aussi

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Le redimensionnement est une nouvelle aussi](../../lessons/part-1/lesson-028-event-pump.md) de la leçon 028.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-028/ex1.patch}}
```

## Visite guidée

La pompe est un aiguillage sur les nouvelles, et un redimensionnement est un
cas de plus. L'OS rapporte la nouvelle géométrie de la fenêtre dans un
événement `ConfigureNotify` qui porte sa nouvelle largeur et sa nouvelle
hauteur, et le pliage tient en deux affectations dans l'état de la fenêtre. Le
côté état de la couture gagne une interrogation, `WindowSize`, et le moteur la
lit avant de fermer.

L'état du moteur commence là où la fenêtre a commencé (`OpenWindow` enregistre
la taille qu'il a demandée), donc la réponse a toujours un sens — avant toute
nouvelle de redimensionnement, c'est simplement la taille que le moteur a
demandée.

Vérifié sans écran : demandez une autre taille à l'OS, puis fermez :

```
$ DISPLAY=:99 xdotool windowsize 2097153 800 600
$ DISPLAY=:99 xdotool windowclose 2097153
engine: window 640x480 open — waiting for news
engine: close reported
engine: closed at 800x600
```

Le moteur n'a jamais vu d'objet événement — il a scruté l'état de la couche
plateforme à la fin et y a trouvé la taille que l'OS a rapportée en dernier.
C'est le contrat d'état scruté de la leçon 032, qui arrive en avance : les
nouvelles entrent *dans* l'état, le moteur lit *l'état*.

Une frontière à garder : `WindowSize` renvoie de simples `int`, pas une
structure de géométrie X, pas un handle de display — la couture reste exempte
d'idiomes d'OS même dans la nouvelle fonction. Un second OS rapporte la taille
de sa propre fenêtre et l'appel du moteur ne change pas.

*Page traduite de la version anglaise `book/solutions/lesson-028/ex1.md`, révision `8904d11`.*

<!-- translation-source: book/solutions/lesson-028/ex1.md @ 8904d11 -->
