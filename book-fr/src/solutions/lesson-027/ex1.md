# Solution : exercice 1 — La fenêtre prend votre titre

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La fenêtre prend votre titre](../../lessons/part-1/lesson-027-first-window.md) de la leçon 027.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-027/ex1.patch}}
```

## Visite guidée

Trois fichiers, une idée : un titre est du texte ordinaire, donc il appartient
à la couture comme n'importe quelle autre donnée ordinaire. `platform.h`
élargit `OpenWindow` d'un `const char *` — toujours aucun idiome d'OS, toujours
aucun en-tête d'OS. `platform_x11.cpp` est le seul fichier qui sait de quoi un
titre est fait sur cet OS : le paramètre atterrit directement dans
`XStoreName`, à côté de l'identifiant de fenêtre auquel il s'applique.
`main.cpp` passe la chaîne au site d'appel et ne change rien d'autre.

La fenêtre du moteur est désormais trouvable par son nom. Dans la vérification
sans écran :

```
$ DISPLAY=:99 xdotool search --name "lesson 027"
2097153
$ DISPLAY=:99 xdotool getwindowgeometry 2097153
Window 2097153
  Position: 0,0 (screen: 0)
  Geometry: 640x480
```

Comptez les fichiers que le changement a touchés : une interface, une
implémentation, un site d'appel. Une implémentation Win32 de la couture
prendrait le même `const char *` et le passerait à ce que Win32 appelle un
titre — et le site d'appel dans `main.cpp` ne s'en apercevrait pas. Lequel des
trois fichiers a le droit de savoir de quoi un titre est fait ? Seulement
`platform_x11.cpp`. La couture porte des données ; les implémentations leur
donnent un sens sur leur OS.

*Page traduite de la version anglaise `book/solutions/lesson-027/ex1.md`, révision `a8e1cba`.*

<!-- translation-source: book/solutions/lesson-027/ex1.md @ a8e1cba -->
