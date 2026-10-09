# Solution : exercice 2 — Le fichier qui se ferme deux fois

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — Le fichier qui se ferme deux fois](../../lessons/part-0/lesson-002-gdb.md) de la leçon 002.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-002/ex2.patch}}
```

## Visite guidée

Exécuter la variante boguée sur un terminal meurt comme cela (les compteurs
de `a.txt` s'affichent d'abord ; à travers un tube, l'arrêt brutal peut
avaler la sortie en tampon) :

```
3 a.txt
free(): double free detected in tcache 2

Program received signal SIGABRT, Aborted.
...
#8  0x00007ffff7cadeae in __GI___libc_free (mem=mem@entry=0x5555555592a0) at ./malloc/malloc.c:3398
#9  0x00007ffff7c85410 in _IO_deallocate_file (fp=0x5555555592a0) at ./libio/libioP.h:958
#10 _IO_new_fclose (fp=0x5555555592a0) at ./libio/iofclose.c:74
#11 0x0000555555555320 in main (argc=2, argv=0x7fffffffdaf8) at wordcount.c:33
```

La réponse en une phrase : le fichier est fermé deux fois — une fois dans
`CountBytes` et une fois dans `main` — et le second `fclose` libère un objet
`FILE` que le premier a déjà relâché, sur quoi le vérificateur de tas de la
bibliothèque C s'arrête en catastrophe. Les trames `#0` à `#10` sont la
machinerie de signal et d'allocation de la bibliothèque C ; remontez
jusqu'à atteindre `main`, et la trame la plus profonde qui est *votre* code
nomme l'appel coupable. La correction est « fermer exactement une fois », et
ce patch prend la règle du collègue au sérieux : `CountBytes` consomme le
flux, donc il le ferme, avec la règle de propriété dans un commentaire là où
le lecteur suivant trébuchera dessus. Supprimer le `fclose` de l'appelé à la
place est un programme également correct — la classe du bug est deux
fermetures, pas laquelle des fermetures. (Les adresses dans la trace varient
par machine et par exécution ; les formes des trames, non.) Le comportement est
inchangé : `./wordcount a.txt b.txt` affiche toujours `3 a.txt` et `6 b.txt`.

*Page traduite de la version anglaise `book/solutions/lesson-002/ex2.md`,
révision `bb8d4ab`.*

<!-- translation-source: book/solutions/lesson-002/ex2.md @ bb8d4ab -->
