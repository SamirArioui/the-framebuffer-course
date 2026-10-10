# Solution : exercice 2 — Un bouton, pas trois

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Un bouton, pas trois](../../lessons/part-4/lesson-078-game-time.md) de la leçon 078.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-078/ex2.patch}}
```

## Visite guidée

Le diff est une sonde qui rend la réponse vérifiable : à chaque transition
d'échelle, elle rapporte ce que l'update a *fait* sur cette frame — combien
d'entités la marche a visitées et combien d'horloge murale l'update a prise. Si
l'update était sauté ou ralenti, ces nombres le montreraient.

Les exécutions, depuis l'état final de cette leçon plus le patch — le script de
la démo tournant le bouton à trois, cinq et sept secondes :

```
engine: game-time: at scale 0.25 — 8 entities walked, update 0.019 ms of wall clock
engine: game-time: at scale 0.00 — 8 entities walked, update 0.018 ms of wall clock
engine: game-time: at scale 1.00 — 8 entities walked, update 0.019 ms of wall clock
```

Huit entités à chaque échelle, l'update coûtant les mêmes ~0.019 ms d'*horloge
murale* à chaque échelle. La frame de la pause a parcouru ses huit entités
exactement comme les frames de jeu ; la seule chose qui était nulle était le
pas que la marche a multiplié dans le mouvement.

**Ce que l'update fait encore à l'échelle 0**, donc : il lit l'entrée (l'état
scruté, leçon 032 — les touches de l'écran de pause sont ce qui permet de
reprendre), il parcourt le magasin (le travail par entité tourne encore ; le
pas dont il déplace les choses est nul), il dessine toute la scène, il alimente
l'audio (un jeu en pause garde sa musique — le mixeur ne connaît pas le temps
de jeu et ne devrait pas), il présente, et il mesure. Ce qu'il ne fait *pas*,
c'est avancer la simulation. Un écran de pause a besoin de tout cela : sans
entrée, il ne pourrait jamais être quitté ; sans dessin, il figerait une frame
en plein present ; sans mesure, le budget de frames mentirait sur ce que coûte
un jeu en pause.

**Ce qui casserait si l'échelle atteignait l'horloge de la plateforme** : deux
choses, toutes deux nommées dans la spécification de ce changement.
L'enregistrement de frame mesurerait des durées *mises à l'échelle* — un jeu en
pause rapporterait `total 0.000 ms` pour des frames qui ont pris 2 ms de temps
machine, et la table du budget de frames (et l'attribution de la leçon 081)
deviendraient de la fiction ; c'est le scénario « l'échelle n'atteint pas la
couture ». Et le contrat de la couture changerait sous la couche plateforme —
`Now()` cesserait d'être l'horloge de mesure monotone que la leçon 035 a fixée,
et chaque consommateur (le calendrier de l'attente cadencée, les horodatages du
rapport) hériterait du temps de jeu sans l'avoir demandé. La réponse du moteur
est celle que cette leçon met en œuvre : l'horloge mesure, le pas se met à
l'échelle, et les deux ne se rencontrent jamais.

**L'argument contre**, honnêtement : un jeu qui veut que la pause soit *totale*
trouverait ce design trop faible — une pause qui fait aussi taire la musique,
arrête le rendu (pour la batterie) et fige l'animation en temps réel d'une
cinématique n'est pas « une échelle de 0 », c'est « la boucle arrêtée », et
c'est un autre mécanisme (le drapeau `paused` de l'exécution décidant ce que
fait la frame). Ce moteur garde la pause comme une échelle parce que son écran
de pause — le L1 de la partie 5 — veut le monde arrêté et la *présentation*
vivante. Un design est « juste » pour le jeu qu'il sert ; le bouton est un seul
parce que les deux besoins de ce jeu (pause, hitstop) sont un seul besoin vu à
deux positions.

Rien ici ne touche le service de l'échelle, la forme de l'update ou
l'enregistrement de frame : la sonde est un rapport à côté de celui de la
leçon.

*Page traduite de la version anglaise `book/solutions/lesson-078/ex2.md`,
révision `cc66198`.*

<!-- translation-source: book/solutions/lesson-078/ex2.md @ cc66198 -->
