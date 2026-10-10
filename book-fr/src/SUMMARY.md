# Sommaire

[Accueil](index.md)

<!-- L'édition française du cours. Les entrées ci-dessous suivent exactement
     l'ordre du programme de book/SUMMARY.md ; les lots 1 à 7 (leçons 001-058)
     traduisent les parties 0 à 2, et les lots suivants ajoutent la suite
     (parties 3 à 5) sans rien réordonner. Les liens du cours non encore
     traduit mènent à l'édition anglaise, et le disent. -->

# Partie 0 — Fondations en C

- [Leçon 001 — argv et saisie de fichiers : votre première commande `gcc`](lessons/part-0/lesson-001-first-program.md)
  - [Solution : ex1 — l'entrée standard](solutions/lesson-001/ex1.md)
  - [Solution : ex2 — le répertoire silencieux](solutions/lesson-001/ex2.md)
  - [Solution : ex3 — un caractère à la fois](solutions/lesson-001/ex3.md)
  - [Solution : ex4 — des octets, pas des caractères](solutions/lesson-001/ex4.md)

- [Leçon 002 — gdb : points d'arrêt, pas à pas, trames de pile](lessons/part-0/lesson-002-gdb.md)
  - [Solution : ex1 — le deuxième arrêt ressemble au premier](solutions/lesson-002/ex1.md)
  - [Solution : ex2 — le fichier qui se ferme deux fois](solutions/lesson-002/ex2.md)
  - [Solution : ex3 — les conditions ne voient qu'une trame](solutions/lesson-002/ex3.md)
  - [Solution : ex4 — un compteur dans deux portées](solutions/lesson-002/ex4.md)

- [Leçon 003 — tampons de caractères : les chaînes à la main](lessons/part-0/lesson-003-char-buffers.md)
  - [Solution : ex1 — trois cents caractères](solutions/lesson-003/ex1.md)
  - [Solution : ex2 — la ligne citée ment](solutions/lesson-003/ex2.md)
  - [Solution : ex3 — le mot le plus long](solutions/lesson-003/ex3.md)
  - [Solution : ex4 — le débogueur sur votre machine](solutions/lesson-003/ex4.md)

- [Leçon 004 — malloc et free : faire grandir les tampons sur le tas](lessons/part-0/lesson-004-heap-buffers.md)
  - [Solution : ex1 — la suite des doublons](solutions/lesson-004/ex1.md)
  - [Solution : ex2 — un octet à la fois](solutions/lesson-004/ex2.md)
  - [Solution : ex3 — qui possède les octets](solutions/lesson-004/ex3.md)
  - [Solution : ex4 — l'entrée standard, revue](solutions/lesson-004/ex4.md)

- [Leçon 005 — les fuites rendues visibles avec les sanitizers](lessons/part-0/lesson-005-leaks.md)
  - [Solution : ex1 — trois verdicts](solutions/lesson-005/ex1.md)
  - [Solution : ex2 — ce que coûte la certitude](solutions/lesson-005/ex2.md)
  - [Solution : ex3 — comment le sanitizer sait](solutions/lesson-005/ex3.md)

- [Leçon 006 — comportement indéfini et débordements de tampon](lessons/part-0/lesson-006-undefined-behavior.md)
  - [Solution : ex1 — la valeur qui n'est pas promise](solutions/lesson-006/ex1.md)
  - [Solution : ex2 — la ligne vide qui lit à l'envers](solutions/lesson-006/ex2.md)
  - [Solution : ex3 — trois sorts](solutions/lesson-006/ex3.md)
  - [Solution : ex4 — le prix de la vérification](solutions/lesson-006/ex4.md)

- [Leçon 007 — structures : sizeof, alignement et remplissage](lessons/part-0/lesson-007-struct-layout.md)
  - [Solution : ex1 — du remplissage dans une structure neuve](solutions/lesson-007/ex1.md)
  - [Solution : ex2 — le remplissage est de la vraie mémoire](solutions/lesson-007/ex2.md)
  - [Solution : ex3 — pourquoi la machine insiste](solutions/lesson-007/ex3.md)
  - [Solution : ex4 — la disposition de l'autre machine](solutions/lesson-007/ex4.md)

- [Leçon 008 — croissance de dynarray : realloc et capacité](lessons/part-0/lesson-008-dynarray.md)
  - [Solution : ex1 — le calendrier des croissances](solutions/lesson-008/ex1.md)
  - [Solution : ex2 — le tableau à moitié libéré](solutions/lesson-008/ex2.md)
  - [Solution : ex3 — doubler contre un-à-la-fois](solutions/lesson-008/ex3.md)
  - [Solution : ex4 — ce que `realloc` promet vraiment](solutions/lesson-008/ex4.md)

- [Leçon 009 — pointeurs de fonction : comparateurs et hooks](lessons/part-0/lesson-009-function-pointers.md)
  - [Solution : ex1 — l'ordre inversé](solutions/lesson-009/ex1.md)
  - [Solution : ex2 — chercher par prédicat](solutions/lesson-009/ex2.md)
  - [Solution : ex3 — pourquoi le pont existe](solutions/lesson-009/ex3.md)

- [Leçon 010 — void\* : la généricité et ses peines](lessons/part-0/lesson-010-void-pointer.md)
  - [Solution : ex1 — les mêmes octets, autrement](solutions/lesson-010/ex1.md)
  - [Solution : ex2 — la mauvaise taille](solutions/lesson-010/ex2.md)
  - [Solution : ex3 — supprimer au milieu](solutions/lesson-010/ex3.md)
  - [Solution : ex4 — le contrat, par écrit](solutions/lesson-010/ex4.md)

- [Leçon 011 — la table de hachage : hachage, seaux, recherche](lessons/part-0/lesson-011-hashtable.md)
  - [Solution : ex1 — comptez dessus](solutions/lesson-011/ex1.md)
  - [Solution : ex2 — le classement](solutions/lesson-011/ex2.md)
  - [Solution : ex3 — la longueur de la marche](solutions/lesson-011/ex3.md)
  - [Solution : ex4 — un seul seau](solutions/lesson-011/ex4.md)

- [Leçon 012 — constructions multi-fichiers : unités de traduction et édition de liens](lessons/part-0/lesson-012-multi-file.md)
  - [Solution : ex1 — l'auxiliaire qui est entré en collision](solutions/lesson-012/ex1.md)
  - [Solution : ex2 — statistiques de table](solutions/lesson-012/ex2.md)
  - [Solution : ex3 — la lettre minuscule](solutions/lesson-012/ex3.md)

- [Leçon 013 — octets bruts et formats de pixel](lessons/part-0/lesson-013-raw-bytes.md)
  - [Solution : ex1 — trois pixels, à la main](solutions/lesson-013/ex1.md)
  - [Solution : ex2 — le même pixel en 32 bits compacté](solutions/lesson-013/ex2.md)
  - [Solution : ex3 — les décalages, à la main](solutions/lesson-013/ex3.md)
  - [Solution : ex4 — le pixel qui n'est pas là](solutions/lesson-013/ex4.md)

- [Leçon 014 — boutisme et disposition des en-têtes d'image](lessons/part-0/lesson-014-image-headers.md)
  - [Solution : ex1 — largeur 258](solutions/lesson-014/ex1.md)
  - [Solution : ex2 — dans le mauvais sens](solutions/lesson-014/ex2.md)
  - [Solution : ex3 — les structures ne font pas de fichiers](solutions/lesson-014/ex3.md)
  - [Solution : ex4 — interrogez votre propre machine](solutions/lesson-014/ex4.md)

- [Leçon 015 — remplir un rectangle dans un tampon mémoire](lessons/part-0/lesson-015-fill-rect.md)
  - [Solution : ex1 — le rectangle hors du bord gauche](solutions/lesson-015/ex1.md)
  - [Solution : ex2 — compter ce qui a survécu](solutions/lesson-015/ex2.md)
  - [Solution : ex3 — l'addition qui a mangé le découpage](solutions/lesson-015/ex3.md)
  - [Solution : ex4 — pourquoi plier d'abord](solutions/lesson-015/ex4.md)

- [Leçon 016 — tracer des lignes dans le tampon](lessons/part-0/lesson-016-lines.md)
  - [Solution : ex1 — la ligne verticale à travers tout](solutions/lesson-016/ex1.md)
  - [Solution : ex2 — des rectangles en contour](solutions/lesson-016/ex2.md)
  - [Solution : ex3 — flottant contre entier](solutions/lesson-016/ex3.md)
  - [Solution : ex4 — le terme d'erreur, observé](solutions/lesson-016/ex4.md)

- [Leçon 017 — écrire un vrai fichier image à la main](lessons/part-0/lesson-017-image-file.md)
  - [Solution : ex1 — les lignes sur disque](solutions/lesson-017/ex1.md)
  - [Solution : ex2 — aller-retour](solutions/lesson-017/ex2.md)
  - [Solution : ex3 — l'image retournée](solutions/lesson-017/ex3.md)
  - [Solution : ex4 — par octet contre par ligne](solutions/lesson-017/ex4.md)

- [Leçon 018 — l'optimiseur et le comportement indéfini](lessons/part-0/lesson-018-optimizer-ub.md)
  - [Solution : ex1 — prédisez les dégâts](solutions/lesson-018/ex1.md)
  - [Solution : ex2 — la garde qui est supprimée](solutions/lesson-018/ex2.md)
  - [Solution : ex3 — relisez le remplissage d'un collègue](solutions/lesson-018/ex3.md)
  - [Solution : ex4 — prouvez la correction](solutions/lesson-018/ex4.md)

- [Leçon 019 — la boucle de jeu](lessons/part-0/lesson-019-game-loop.md)
  - [Solution : ex1 — la vie de `strtoul`](solutions/lesson-019/ex1.md)
  - [Solution : ex2 — la chaîne qui ne finit jamais](solutions/lesson-019/ex2.md)
  - [Solution : ex3 — emballez l'état](solutions/lesson-019/ex3.md)
  - [Solution : ex4 — pourquoi trois phases](solutions/lesson-019/ex4.md)

- [Leçon 020 — la mesure du temps avec `clock_gettime`](lessons/part-0/lesson-020-timing.md)
  - [Solution : ex1 — prédire les ticks](solutions/lesson-020/ex1.md)
  - [Solution : ex2 — ce que coûte le plafond de frames](solutions/lesson-020/ex2.md)
  - [Solution : ex3 — monotone contre horloge murale](solutions/lesson-020/ex3.md)
  - [Solution : ex4 — une chose qui bouge](solutions/lesson-020/ex4.md)

- [Leçon 021 — la saisie brute au terminal avec les codes d'échappement](lessons/part-0/lesson-021-terminal-input.md)
  - [Solution : ex1 — des octets jusqu'au bout](solutions/lesson-021/ex1.md)
  - [Solution : ex2 — la touche qui a disparu](solutions/lesson-021/ex2.md)
  - [Solution : ex3 — CSI, SS3, et votre terminal](solutions/lesson-021/ex3.md)
  - [Solution : ex4 — WASD](solutions/lesson-021/ex4.md)

- [Leçon 022 — la grille de caractères à double tampon](lessons/part-0/lesson-022-double-buffer.md)
  - [Solution : ex1 — compter le vidage](solutions/lesson-022/ex1.md)
  - [Solution : ex2 — un de trop, deux fois](solutions/lesson-022/ex2.md)
  - [Solution : ex3 — ce que le différentiel économise](solutions/lesson-022/ex3.md)
  - [Solution : ex4 — des suites, pas des cellules](solutions/lesson-022/ex4.md)

- [Leçon 023 — la machine à états : titre, jeu, mort](lessons/part-0/lesson-023-state-machine.md)
  - [Solution : ex1 — compter jusqu'au mur](solutions/lesson-023/ex1.md)
  - [Solution : ex2 — le redémarrage qui n'en était pas un](solutions/lesson-023/ex2.md)
  - [Solution : ex3 — pause](solutions/lesson-023/ex3.md)
  - [Solution : ex4 — pourquoi des états, pas des drapeaux](solutions/lesson-023/ex4.md)

- [Leçon 024 — la table de commandes à pointeurs de fonction](lessons/part-0/lesson-024-command-table.md)
  - [Solution : ex1 — WASD, c'est quatre lignes](solutions/lesson-024/ex1.md)
  - [Solution : ex2 — Entrée est un retour chariot](solutions/lesson-024/ex2.md)
  - [Solution : ex3 — deux lignes, une touche](solutions/lesson-024/ex3.md)
  - [Solution : ex4 — où vivent les fonctions](solutions/lesson-024/ex4.md)

- [Leçon 025 — le sous-ensemble C++ : classes et vtables](lessons/part-0/lesson-025-cpp-subset.md)
  - [Solution : ex1 — une troisième vue](solutions/lesson-025/ex1.md)
  - [Solution : ex2 — le mot caché](solutions/lesson-025/ex2.md)
  - [Solution : ex3 — la vtable sous la loupe](solutions/lesson-025/ex3.md)
  - [Solution : ex4 — la copie qui ne peut pas exister](solutions/lesson-025/ex4.md)

# Partie 1 — la couche plateforme

- [Leçon 026 — la base de code naît](lessons/part-1/lesson-026-birth.md)
  - [Solution : ex1 — le moteur dit son nom](solutions/lesson-026/ex1.md)
  - [Solution : ex2 — deux unités de traduction](solutions/lesson-026/ex2.md)

- [Leçon 027 — la couture plateforme et la première fenêtre X11](lessons/part-1/lesson-027-first-window.md)
  - [Solution : ex1 — la fenêtre prend votre titre](solutions/lesson-027/ex1.md)
  - [Solution : ex2 — où se trouve la frontière](solutions/lesson-027/ex2.md)

- [Leçon 028 — la pompe à événements : garder la fenêtre en vie et signaler la fermeture](lessons/part-1/lesson-028-event-pump.md)
  - [Solution : ex1 — le redimensionnement est une nouvelle aussi](solutions/lesson-028/ex1.md)
  - [Solution : ex2 — deux façons dont la nouvelle arrive](solutions/lesson-028/ex2.md)

- [Leçon 029 — fermeture propre et chemins d'erreur : les ressources de l'OS libérées à chaque sortie](lessons/part-1/lesson-029-clean-close.md)
  - [Solution : ex1 — la fenêtre qu'on ouvre deux fois](solutions/lesson-029/ex1.md)
  - [Solution : ex2 — le registre](solutions/lesson-029/ex2.md)

- [Leçon 030 — le framebuffer comme nos propres octets](lessons/part-1/lesson-030-framebuffer.md)
  - [Solution : ex1 — FillRect, de retour de la Partie 0](solutions/lesson-030/ex1.md)
  - [Solution : ex2 — les quatre octets](solutions/lesson-030/ex2.md)

- [Leçon 031 — la présentation à travers la couche plateforme](lessons/part-1/lesson-031-present.md)
  - [Solution : ex1 — la vérification de la présentation](solutions/lesson-031/ex1.md)
  - [Solution : ex2 — recouvrir et révéler](solutions/lesson-031/ex2.md)

- [Leçon 032 — l'état d'entrée par scrutation](lessons/part-1/lesson-032-polled-input.md)
  - [Solution : ex1 — vos propres touches](solutions/lesson-032/ex1.md)
  - [Solution : ex2 — l'appui qui a disparu](solutions/lesson-032/ex2.md)

- [Leçon 033 — mémoriser les appuis brefs et suivre le focus](lessons/part-1/lesson-033-latching.md)
  - [Solution : ex1 — le compteur d'appuis](solutions/lesson-033/ex1.md)
  - [Solution : ex2 — la touche qui ne veut pas lâcher](solutions/lesson-033/ex2.md)

- [Leçon 034 — la première frame interactive](lessons/part-1/lesson-034-first-frame.md)
  - [Solution : ex1 — huit directions](solutions/lesson-034/ex1.md)
  - [Solution : ex2 — la vitesse qui appartient au clavier](solutions/lesson-034/ex2.md)

- [Leçon 035 — l'horloge de la plateforme](lessons/part-1/lesson-035-clock.md)
  - [Solution : ex1 — la diagonale est trop rapide](solutions/lesson-035/ex1.md)
  - [Solution : ex2 — l'horloge qui ment](solutions/lesson-035/ex2.md)

- [Leçon 036 — le temps de frame comme donnée mesurée](lessons/part-1/lesson-036-frame-time.md)
  - [Solution : ex1 — la copie, isolée](solutions/lesson-036/ex1.md)
  - [Solution : ex2 — le budget de frames](solutions/lesson-036/ex2.md)

- [Leçon 037 — lectures de fichier entier](lessons/part-1/lesson-037-file-read.md)
  - [Solution : ex1 — lire dans votre propre mémoire](solutions/lesson-037/ex1.md)
  - [Solution : ex2 — le fichier qui ne finit jamais](solutions/lesson-037/ex2.md)

- [Leçon 038 — écritures de fichier entier et aller-retour](lessons/part-1/lesson-038-file-write.md)
  - [Solution : ex1 — la capture d'écran](solutions/lesson-038/ex1.md)
  - [Solution : ex2 — le périphérique qui est toujours plein](solutions/lesson-038/ex2.md)

- [Leçon 039 — plongée dans la mémoire virtuelle](lessons/part-1/lesson-039-virtual-memory.md)
  - [Solution : ex1 — trouvez votre mappage](solutions/lesson-039/ex1.md)
  - [Solution : ex2 — le fichier qui ment sur sa taille](solutions/lesson-039/ex2.md)

- [Leçon 040 — des tampons adossés à une réservation](lessons/part-1/lesson-040-reservations.md)
  - [Solution : ex1 — un octet, s'il vous plaît](solutions/lesson-040/ex1.md)
  - [Solution : ex2 — la page qui riposte](solutions/lesson-040/ex2.md)

- [Leçon 041 — les arenas](lessons/part-1/lesson-041-arenas.md)
  - [Solution : ex1 — la traînée du marqueur](solutions/lesson-041/ex1.md)
  - [Solution : ex2 — le bug qu'ASan ne voit pas](solutions/lesson-041/ex2.md)

- [Leçon 042 — l'interface comme contrat](lessons/part-1/lesson-042-contract.md)
  - [Solution : ex1 — le second OS](solutions/lesson-042/ex1.md)
  - [Solution : ex2 — ce que le contrôle ne peut pas voir](solutions/lesson-042/ex2.md)

- [Leçon 043 — la démo de clôture : la couche plateforme terminée](lessons/part-1/lesson-043-demo.md)
  - [Solution : ex1 — la couche plateforme terminée, sur votre machine](solutions/lesson-043/ex1.md)
  - [Solution : ex2 — la table d'acceptation](solutions/lesson-043/ex2.md)

# Partie 2 — le rendu logiciel

- [Leçon 044 — un sprite comme des octets chargés](lessons/part-2/lesson-044-sprite-bytes.md)
  - [Solution : ex1 — l'en-tête qui ment](solutions/lesson-044/ex1.md)
  - [Solution : ex2 — sprite, voici la fenêtre](solutions/lesson-044/ex2.md)

- [Leçon 045 — le blit découpé et transparent](lessons/part-2/lesson-045-blit.md)
  - [Solution : ex1 — votre propre clé](solutions/lesson-045/ex1.md)
  - [Solution : ex2 — les quatre coins](solutions/lesson-045/ex2.md)

- [Leçon 046 — le sprite se déplace](lessons/part-2/lesson-046-movable-sprite.md)
  - [Solution : ex1 — où passe le render ?](solutions/lesson-046/ex1.md)
  - [Solution : ex2 — le sprite qui se retourne](solutions/lesson-046/ex2.md)

- [Leçon 047 — plongée dans les caches](lessons/part-2/lesson-047-caches.md)
  - [Solution : ex1 — les coudes de votre machine](solutions/lesson-047/ex1.md)
  - [Solution : ex2 — le pas qui ne rentre pas dans la ligne](solutions/lesson-047/ex2.md)

- [Leçon 048 — l'assembleur compilé du blitter](lessons/part-2/lesson-048-assembly.md)
  - [Solution : ex1 — le budget d'instructions](solutions/lesson-048/ex1.md)
  - [Solution : ex2 — la signature du compilateur](solutions/lesson-048/ex2.md)

- [Leçon 049 — la lentille SIMD](lessons/part-2/lesson-049-simd.md)
  - [Solution : ex1 — deux remplissages, prédits](solutions/lesson-049/ex1.md)
  - [Solution : ex2 — la largeur de registre de votre compilateur](solutions/lesson-049/ex2.md)

- [Leçon 050 — la police bitmap comme asset](lessons/part-2/lesson-050-font.md)
  - [Solution : ex1 — le dump de la police](solutions/lesson-050/ex1.md)
  - [Solution : ex2 — les octets dont la planche n'a jamais entendu parler](solutions/lesson-050/ex2.md)

- [Leçon 051 — du texte à l'écran](lessons/part-2/lesson-051-text.md)
  - [Solution : ex1 — le HUD qui compte](solutions/lesson-051/ex1.md)
  - [Solution : ex2 — les deux sortes de rien](solutions/lesson-051/ex2.md)

- [Leçon 052 — le format d'asset du tilemap](lessons/part-2/lesson-052-tilemap.md)
  - [Solution : ex1 — l'échec qui se nomme lui-même](solutions/lesson-052/ex1.md)
  - [Solution : ex2 — la carte en caractères](solutions/lesson-052/ex2.md)

- [Leçon 053 — le dessin de tilemap](lessons/part-2/lesson-053-tiles.md)
  - [Solution : ex1 — la tuile sous le sprite](solutions/lesson-053/ex1.md)
  - [Solution : ex2 — le dessin à vide](solutions/lesson-053/ex2.md)

- [Leçon 054 — la caméra](lessons/part-2/lesson-054-camera.md)
  - [Solution : ex1 — la secousse qui décroît](solutions/lesson-054/ex1.md)
  - [Solution : ex2 — l'additif qui annule](solutions/lesson-054/ex2.md)

- [Leçon 055 — les types de tuile et la solidité](lessons/part-2/lesson-055-collision.md)
  - [Solution : ex1 — la politique est la vôtre](solutions/lesson-055/ex1.md)
  - [Solution : ex2 — le rectangle à la frontière](solutions/lesson-055/ex2.md)

- [Leçon 056 — le mover qui s'arrête aux murs](lessons/part-2/lesson-056-mover.md)
  - [Solution : ex1 — la hitbox](solutions/lesson-056/ex1.md)
  - [Solution : ex2 — le pas et le mur](solutions/lesson-056/ex2.md)

- [Leçon 057 — la démo de clôture : le monde, dessiné](lessons/part-2/lesson-057-demo.md)
  - [Solution : ex1 — la démo de la partie 2 sur votre machine](solutions/lesson-057/ex1.md)
  - [Solution : ex2 — la table d'acceptation](solutions/lesson-057/ex2.md)

- [Leçon 058 — la table du budget de frames](lessons/part-2/lesson-058-budget.md)
  - [Solution : ex1 — la ligne qui n'est pas là](solutions/lesson-058/ex1.md)
  - [Solution : ex2 — la colonne de la pire frame](solutions/lesson-058/ex2.md)

<!-- translation-source: book/SUMMARY.md @ cfeaecd -->
