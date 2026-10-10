# Solution : exercice 1 — La marche qui retire devant

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — La marche qui retire devant](../../lessons/part-4/lesson-075-lifetime.md) de la leçon 075.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-075/ex1.patch}}
```

## Visite guidée

Le diff est une sonde, lancée deux fois : `ProbeWalk` construit un magasin
(store) jetable de six entités aux emplacements 0-5 et le parcourt avec un corps
qui retire *une autre entité* quand la marche atteint un emplacement donné — puis
rapporte la liste des visites et le compte des vivantes. Le premier appel retire
l'emplacement 3 en se tenant à l'emplacement 1 (devant la marche) ; le second
retire l'emplacement 1 en se tenant à l'emplacement 3 (derrière elle).

Les prédictions, avant toute exécution. Dans le **premier** cas, la marche
visite l'emplacement 0 puis l'emplacement 1 ; à l'emplacement 1, son travail
retire l'emplacement 3 ; la marche continue ensuite vers l'emplacement 2, trouve
l'emplacement 3 mort et le saute, puis visite 4 et 5. La liste est `0 1 2 4 5` —
cinq visites — et le compte des vivantes est 5. Dans le **second** cas, la
marche a déjà visité l'emplacement 1 avant que le travail de l'emplacement 3 le
retire, donc chaque emplacement est visité une fois et la liste est
`0 1 2 3 4 5` — six visites — tandis que le compte des vivantes retombe encore
sur 5.

Les exécutions, à partir de l'état final de cette leçon plus le patch :

```
engine: walk (retire slot 3 at slot 1) visited 0 1 2 4 5, live 5
engine: walk (retire slot 1 at slot 3) visited 0 1 2 3 4 5, live 5
```

Les deux tiennent. Remarquez ce que les deux listes partagent avec la marche de
la leçon : aucun emplacement n'est visité deux fois dans aucune d'elles, et
aucune entité vivante n'est sautée. L'asymétrie est exactement ce que dit le
contrat — une entité retirée *devant* la marche n'est jamais visitée dans cette
marche ; une entité retirée *derrière* elle a été visitée avant de partir et
n'est pas revue.

Ce qui rend les deux réponses sûres tient en une phrase : **les emplacements ne
bougent jamais.** L'indice de la marche est sa propre position dans le magasin,
et « cette entité est-elle vivante ? » se répond à l'arrivée — donc rien de ce
qu'un corps fait à un emplacement ne peut faire bouger le sol sous la marche.
Une marche sur une structure qui se compacte au retrait (un tableau qui se
décale, une liste qui relie à nouveau ses maillons) perdrait ou répéterait des
entités à l'instant où son corps en retirerait une — c'est exactement pourquoi
le magasin a des emplacements fixes et non une collection extensible. Le retrait
ici, c'est un drapeau et un compte ; la géométrie est intacte.

C'est aussi la réponse à l'objection « pourquoi ne pas simplement rassembler les
entités dans un tableau à chaque frame et parcourir celui-ci ? » — rassembler
est une copie par frame de la chose même que la marche existe pour éviter, et
cela déplace les entités à l'instant où vous voulez en retirer une. Le magasin
fixe fait de la marche, du retrait et de la réutilisation une règle chacun.

Rien ici ne touche à la marche de la démo, à la politique du magasin ou à la
boucle du jeu : la sonde est un magasin jetable à côté de celui de l'exécution.

*Page traduite de la version anglaise `book/solutions/lesson-075/ex1.md`,
révision `25e6c7f`.*

<!-- translation-source: book/solutions/lesson-075/ex1.md @ 25e6c7f -->
