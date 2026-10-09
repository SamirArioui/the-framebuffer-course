# Sommaire

[Accueil](index.md)

<!-- L'édition française du cours. Les entrées ci-dessous suivent exactement
     l'ordre du programme de book/SUMMARY.md ; les lots 1 (leçons 001-003), 2
     (leçons 004-007) et 3 (leçons 008-012) sont traduits — tout l'arc
     wordcount et tout l'arc ds-kit — et les lots suivants ajoutent la suite
     sans rien réordonner. Les liens du cours non encore traduit mènent à
     l'édition anglaise, et le disent. -->

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

<!-- translation-source: book/SUMMARY.md @ cfeaecd -->
