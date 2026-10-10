# Solution : exercice 2 — La facture de la transcription

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La facture de la transcription](../../lessons/part-5/lesson-097-debt.md) de la leçon 097.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-097/ex2.patch}}
```

## Visite guidée

Le diff est l'outil, et l'outil est tout l'enjeu : faites-en `git apply` à
n'importe quel état de l'arbre et il réduit le journal d'une exécution à la
forme dont une refactorisation ne peut rien changer — le bavardage du harnais
supprimé, les lignes de journal par frame supprimées (leur nombre est celui du
cadencement, pas du jeu ; la table du budget de clôture garde le coût de la
frame), chaque chiffre remplacé pour que les valeurs mesurées cessent de
différer, et les lignes répétées réduites pour qu'un rapport déclenché une fois
et un déclenché cent fois se lisent pareil.

La mesure, dans l'ordre que l'exercice exige — le plancher de bruit **d'abord**.
Deux exécutions du build de la leçon 096 sur le même scénario scripté (construit
dans un worktree à l'étiquette, `git worktree add ../wt096 lesson-096`) :

```
$ tools/transcript-normalize.sh before-1.log > before-1.shape
$ tools/transcript-normalize.sh before-2.log > before-2.shape
$ wc -l before-1.shape before-2.shape
  105 before-1.shape
  105 before-2.shape
$ diff before-1.shape before-2.shape
33d32
< engine: fire: spitter -> shell (damage N, range N)
42d40
< engine: hero velocity -N,N (t=N.N)
54a53,54
> engine: screen: death fade arrived at N,N,N (its own color)
> engine: screen: death: "GAME OVER" / "SCORE N TIME N:N WAVE N/N" / "ENTER: TITLE"
59d58
< engine: shell at N,N — N px of the hero (t=N.N)
64d62
< engine: shot shell retired — wall
77a76
> engine: state play -> death (the hero's health reached zero)
103a103
> engine: world: spark ends at N,N — N px of the hero
```

Huit gabarits d'écart — quatre dans chaque direction — et chacun d'eux est le
*combat* qui se décide lui-même : dans la seconde exécution le héros est mort,
donc l'écran de mort est apparu et le shell du spitter n'a jamais volé. Deux
exécutions du même binaire, et elles diffèrent. Voilà le plancher ; la facture
d'une refactorisation doit se lire contre lui.

Puis le build refactorisé, le même scénario deux fois :

```
$ diff before-1.shape after-1.shape
68a69
> engine: hero unblocked at N,N (t=N.N)
77d77
< engine: hero unblocked at N,N (t=N.N)
$ for f in before-1 before-2 after-1 after-2; do
      sort $f.shape | md5sum; done
42c2b6b10349238e7840774698d52958  before-1.shape
30a2f5f2b7660514ecad5ab1d2268faf  before-2.shape
42c2b6b10349238e7840774698d52958  after-1.shape
42c2b6b10349238e7840774698d52958  after-2.shape
```

Le verdict, plancher en vue : **la facture est nulle.** L'ensemble de rapports
des exécutions refactorisées est identique octet pour octet à celui de la
première ancienne exécution — les mêmes 105 gabarits, chaque rapport que le jeu
sait donner se déclenchant dans les deux — et l'unique différence séquentielle
est une sonde `hero unblocked` qui atterrit quelques lignes plus tôt dans la
séquence du combat, de la gigue exactement du genre que les anciennes exécutions
montrent entre elles. Les anciennes exécutions sont à huit gabarits l'une de
l'autre ; les nouvelles sont à zéro gabarit de l'ancienne. Une refactorisation
qui aurait changé le comportement ne survivrait pas à cette comparaison — le
bruit du combat n'est pas une cachette, parce que l'*ensemble* des rapports (une
transition d'état, un écran, un fondu, une vague, un impact, une rafale) est le
vocabulaire du jeu, et le vocabulaire ne dépend pas du timing.

Une note de méthode pour le compte rendu que vous devez à cet exercice :
rapportez le plancher à côté du verdict, toujours. « Les transcriptions
concordaient » ne veut rien dire tant que le lecteur ne sait pas ce que
« concorder » devait battre.

*Page traduite de la version anglaise `book/solutions/lesson-097/ex2.md`,
révision `3e7f026`.*

<!-- translation-source: book/solutions/lesson-097/ex2.md @ 3e7f026 -->
