# Solution : exercice 2 — La passe de mesure sur votre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La passe de mesure sur votre machine](../../lessons/part-5/lesson-098-measure.md) de la leçon 098.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-098/ex2.patch}}
```

## Visite guidée

Le diff est la fiche — `tools/profile-card.sh`, qui transforme un `gmon.out` en
un bloc collable : les premières lignes du profil plat et le CPU profilé
additionné depuis la colonne des secondes propres, pour que les réponses de
deux machines à « qu'est-ce qui est chaud » se comparent ligne à ligne. La
fiche de cette machine, de l'exécution de mesure de la leçon elle-même :

```
$ tools/profile-card.sh build-pg/game gmon.out
 59.77      2.11     2.11  3501190     0.00     0.00  engine::BlitSprite(engine::Framebuffer&, engine::Sprite const&, int, int)
 37.68      3.44     1.33     2584     0.00     0.00  engine::ClearBuffer(engine::Framebuffer&, unsigned char, unsigned char, unsigned char)
  1.13      3.48     0.04     2204     0.00     0.00  engine::DrawTileMap(engine::Framebuffer&, engine::TileMap const&, engine::TileSheet const&, int, int)
  0.57      3.50     0.02    14745     0.00     0.00  engine::BlitSpriteFrame(engine::Framebuffer&, engine::Sprite const&, int, int, int, int)
  0.28      3.51     0.01 29905680     0.00     0.00  engine::(anonymous namespace)::ChannelFrame(engine::Channel&)
  0.28      3.52     0.01     2584     0.00     0.00  platform::Present(platform::Window*, unsigned char const*, int, int)
  0.28      3.53     0.01     2543     0.00     0.00  engine::MixBuffer(engine::Mixer&, short*, int)
  0.00      3.53     0.00   160083     0.00     0.00  engine::(anonymous namespace)::ReadFrame(unsigned char const*)
profiled CPU: 3.53 s in 353 samples of 0.01 s
```

— et le bilan des frames de la *même* exécution (la fiche ne le remplace pas ;
les deux instruments répondent à des questions différentes, le découpage de
l'exercice 1 étant la différence) :

```
engine: frame budget — 2583 frames, avg 1.834 ms, worst 3.488 ms (frame 2322)
engine:     clear      0.443      24%
engine:     tilemap    0.835      46%
engine:   present      0.433      24%
```

Ce pour quoi le portage est fait, c'est la comparaison. Sur cette machine
l'ordre est le dessin de la carte d'abord (`BlitSprite` + `DrawTileMap`, ~61 %
du CPU) et le clear ensuite (`ClearBuffer`, 37,7 %), le present étant presque
absent du CPU (0,28 %) bien qu'il détienne un quart du temps mural de la frame.
Sur la vôtre, attendez-vous aux *deux mêmes noms* — le jeu dessine la même
carte et efface la même frame partout — mais surveillez trois choses avant de
conclure quoi que ce soit d'un ordre différent :

- **Le découpage du present.** La copie d'un affichage local coûte du vrai CPU
  *au processus* ; l'aller-retour au serveur X de cette machine ne lui en coûte
  presque pas et facture la différence comme attente. `platform::Present` qui
  monte sur votre fiche, c'est la couture plus proche, pas le moteur plus lent.
- **L'alimentation du périphérique son.** Cette machine mixe en silence (pas de
  `/dev/snd` digne de ce nom). Une vraie sortie fait montrer à `MixBuffer` et
  `ChannelFrame` le *même* CPU qu'ici — le mixage tourne dans les deux cas (la
  règle de la leçon 095) — mais `SubmitSamples` et le cadencement du
  périphérique peuvent déplacer où les frames atterrissent.
- **Le niveau de votre compilateur.** La fiche ci-dessus est en `-O0 -pg`. En
  `-O3`, la boucle de copie de la carte change entièrement de forme (la leçon
  049 a chiffré cet écart : plusieurs fois), alors nommez le build à côté des
  nombres — une fiche sans ses drapeaux est un nombre sans machine.

Rapportez la fiche, la table des frames, les drapeaux du build et le nom de la
machine — et si votre top 2 diffère de celui du livre, la question intéressante
n'est jamais « qui a raison » mais *quelle mesure explique la différence*.
Cette question est toute la passe de mesure, apprise sur votre propre matériel.

*Page traduite de la version anglaise `book/solutions/lesson-098/ex2.md`,
révision `17073ed`.*

<!-- translation-source: book/solutions/lesson-098/ex2.md @ 17073ed -->
