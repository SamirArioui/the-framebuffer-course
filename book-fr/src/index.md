# Bienvenue

{{#include stability-horizon.md}}

<p><em>Le cours existe aussi en anglais : <a href="../index.html">The
Framebuffer Course</a> — cette page d'accueil garde ce lien en clair pour les
navigateurs sans JavaScript ; ailleurs, le sélecteur de langue en haut de
chaque page mène à la page correspondante de l'autre édition.</em></p>

> **Note d'édition.** La traduction française avance par lots. Sont traduits
> à ce jour : cette page et les leçons 001 à 018 avec leurs solutions (les
> arcs `wordcount`, `ds-kit` et `paint`) ; la suite du cours est en anglais et
> les liens qui y mènent le signalent. La page anglaise fait foi ; chaque page
> traduite nomme sa source et la révision qu'elle suit.

Ceci est le site du cours **The Framebuffer Course** — *des fondations en C à
un jeu d'arcade 2D terminé, sur un moteur que vous avez écrit vous-même* : un
cours écrit, gratuit et ouvert, en anglais, d'environ 130 leçons moyennes, qui
mène un développeur Python ou Ruby sans expérience de C/C++ des fondations en
C à un jeu d'arcade 2D vue de dessus terminé, tournant sur un moteur entièrement
écrit à la main — aucune bibliothèque externe dans le code C/C++ visible par
l'élève.

## L'arc

- **Partie 0 — Fondations en C.** Quatre programmes jetables de `sandbox/` :
  la chaîne de compilation et la mémoire, la disposition et l'édition de
  liens, les octets et les pixels, les boucles et l'état.
- **Partie 1 — la couche plateforme.** La base de code naît d'un fichier vierge
  ; fenêtre, saisie, minuterie et E/S de fichiers face au système
  d'exploitation (Linux/X11 en premier).
- **Partie 2 — le rendu logiciel.** Chaque pixel est écrit par du code qui est
  le nôtre.
- **Partie 3 — le son.** Musique et effets via notre propre mixeur.
- **Partie 4 — les services.** Arenas, entités, formats de ressources — et une
  tranche verticale qui dessine un héros marchant sur une tilemap avec la
  caméra qui le suit.
- **Partie 5 — le jeu.** Feel, ennemis, vagues, et une optimisation fixe en
  trois passes se terminant par un rapport de budget de trame.

Les plongées obligatoires — les dessous du compilateur et de l'éditeur de
liens, la mémoire virtuelle, les caches, la lecture SIMD/assembleur — se
trouvent dans les parties qui en ont besoin.

## Comment ce site est organisé

Les leçons apparaissent dans l'ordre du programme. Chaque leçon contient une
prose, exactement une étape de code, et ses exercices, et vise 30 à 60 minutes
de lecture et de code. Chaque énoncé d'exercice se termine par un lien vers sa
solution : un diff contre l'état final de la leçon, plus une visite guidée.
Rien dans un énoncé ne gâche sa solution.

Le code des leçons vit dans une histoire linéaire étiquetée `lesson-NNN`. Si
une étiquette de leçon bouge sous vos pieds, resynchronisez-vous avec :

```
git checkout lesson-NNN -- src/
```

*Page traduite de la version anglaise `book/index.md`, révision `fa72f8c`.*

<!-- translation-source: book/index.md @ fa72f8c -->
