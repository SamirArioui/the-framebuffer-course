# Solution : exercice 2 — La capacité que vous choisiriez

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La capacité que vous choisiriez](../../lessons/part-4/lesson-074-store.md) de la leçon 074.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-074/ex2.patch}}
```

## Visite guidée

Le diff est une ligne de comptabilité : l'exécution nomme la taille d'un
emplacement et le total du magasin (store), pour que « la capacité est une
décision » soit une décision prise contre un chiffre. D'une vraie exécution de l'état final
de cette leçon plus le patch :

```
engine: store: live 64 of 64 — the hero and 63 from the script
engine: store: creation refused (full), arena 1253648 -> 1253648 — creation allocates nothing
engine: store: 64 slots of 56 bytes — 3592 bytes decided up front
```

Cinquante-six octets par emplacement. C'est le chiffre sur lequel tient toute la
discussion : une entité, c'est son nom, cinq `int`, un pointeur de sprite et le
drapeau `live`, avec le remplissage que la machine décide d'ajouter. Tout ce qui
suit est de l'arithmétique avec lui.

**Le compte, pour le jeu terminé.** Le monde du MVD contient : le héros (1) ; de
quoi remplir une vague d'ennemis vivants à la fois — trois types plus le boss et
ses parties (disons 32) ; les projectiles en vol de deux armes (64, c'est une
seconde chargée) ; et les rafales de particules (la conception laisse
délibérément ouverte la question de savoir si les rafales auront leur propre
magasin, plus étroit — la L12 de la partie 5 tranche). Cela fait environ une
centaine d'entités avec de la marge pour un mauvais moment, donc **256
emplacements** est la capacité que choisit cette solution : 256 × 56 = 14 336
octets, plus le compte — moins de 15 Ko d'une arena de 32 Mo, et les mêmes 15 Ko
que le jeu fasse un jour apparaître cent entités ou une seule.

**Quand elle est trop petite.** Les requêtes au-delà de la capacité sont
refusées comme des valeurs typées (l'exercice 1 montre à quoi ressemble le
traitement d'un refus). Le jeu décide ce que signifie chaque refus : un
projectile qui ne peut pas être créé, c'est le tir qui n'est pas parti — la
version honnête est de dépenser la requête là où elle compte (le tir du joueur)
et de l'abandonner là où elle ne compte pas (la cinquième particule d'une
rafale), et de *compter* les refus dans le rapport de l'exécution, pour qu'un
magasin plein se voie en test plutôt qu'en revue. Ce que le jeu ne doit jamais
faire, c'est ce sur quoi l'exercice 1 a planté : utiliser l'entité d'une requête
qu'il n'a pas vérifiée.

**Quand elle est trop grande.** Le coût, ce sont les octets du magasin, pris à
l'avance et jamais rendus — 3,5 Ko à 64 emplacements, 14 Ko à 256, 56 Ko à 1024.
Contre une arena de 32 Mo, ce n'est rien ; contre une machine plus petite ou un
plus gros magasin d'autre chose, non. La règle empirique que le moteur applique
partout : *la capacité est une décision*, prise là où le besoin est connu, et
nommée là où un relecteur la trouvera (la revue de clôture de cette partie nomme
celle-ci).

**Pourquoi une mauvaise capacité est du réglage, pas de la correction.** Trois
choses dans la conception du magasin, toutes dans le code de cette leçon : le
refus est *typé* — un magasin plein répond `ENTITY_FULL` et le jeu le traite, au
lieu d'écrire quelque part où il ne devrait pas ; il ne *vole* jamais — aucune
entité vivante n'est silencieusement remplacée, donc rien de ce que le joueur
peut voir ne disparaît parce qu'une chose invisible a demandé la place ; et il
n'*alloue* jamais — la mémoire de l'exécution ne bouge pas sous lui, donc un
magasin plein ne peut pas devenir un magasin lent ou fragmenté. Trompez-vous sur
le nombre et le jeu se joue moins bien ou fait apparaître moins de choses. Il ne
corrompt rien, ne fuit rien, ne fait rien disparaître — c'est ce qui sépare un
bouton de réglage d'un bug.

Si votre propre compte diffère, c'est l'exercice qui fonctionne : le nombre
appartient à la pire seconde de *votre* jeu, pas à cette leçon. Changez
`ENTITY_CAP`, relancez le script et lisez la ligne du magasin — la capacité est
une constante et le rapport vous dit ce qu'elle coûte.

*Page traduite de la version anglaise `book/solutions/lesson-074/ex2.md`,
révision `cc57259`.*

<!-- translation-source: book/solutions/lesson-074/ex2.md @ cc57259 -->
