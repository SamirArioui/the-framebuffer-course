# Solution : exercice 1 — Deux remplissages, prédits

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 1 — Deux remplissages, prédits](../../lessons/part-2/lesson-049-simd.md) de la leçon 049.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-049/ex1.patch}}
```

## Visite guidée

La prédiction est là où vit l'exercice, alors prenez-la au sérieux. Les deux
boucles écrivent des octets en mémoire dans l'ordre — la forme pour laquelle
`CopySequential` a été vectorisée. Mais regardez *ce qu'*elles écrivent :

- **`FillPattern`** écrit quatre octets **différents** par pixel — `a`,
  `b`, `c`, `0` — répétés. Un déplacement vectoriel de 16 octets aurait
  besoin de ces quatre octets dupliqués quatre fois dans un même registre :
  le compilateur devrait construire le motif dans un registre (une
  permutation ou un chargement de constante), puis le stocker. Possible en
  principe ; dans le modèle de coût de `-O3`, improbable. La prédiction :
  **reste scalaire**, quatre stockages par pixel, comme `ClearBuffer`.
- **`FillUniform`** écrit **une seule** valeur, partout. Ce n'est pas une
  copie et pas un motif — c'est la définition d'école de `memset`. Le
  détecteur de motifs du compilateur cherche exactement cette boucle. La
  prédiction : elle ne restera pas une boucle — elle deviendra **un appel à
  `memset`**.

Le recensement, en `-O3` :

```
$ ./tools/disasm.sh --census | grep -E 'Fill|Copy|Blit|Clear'
    2  engine::CopyStrided(unsigned char*, unsigned char const*, unsigned long, unsigned long)
    2  engine::CopySequential(unsigned char*, unsigned char const*, unsigned long, unsigned long)
```

Ni l'un ni l'autre des remplissages n'apparaît — **zéro instruction sur
registres vectoriels pour les deux.** Les listes disent où chacun est allé :

```
$ ./tools/disasm.sh FillPattern
0000000000001c30 <engine::FillPattern(unsigned char*, unsigned long, unsigned char, unsigned char, unsigned char)>:
    1c30:	endbr64
    1c34:	test   %rsi,%rsi
    1c37:	je     1c5a <engine::FillPattern(...)+0x2a>
    1c39:	xor    %eax,%eax
    1c40:	mov    %dl,(%rdi,%rax,1)
    1c43:	mov    %cl,0x1(%rdi,%rax,1)
    1c47:	mov    %r8b,0x2(%rdi,%rax,1)
    1c4c:	movb   $0x0,0x3(%rdi,%rax,1)
    1c51:	add    $0x4,%rax
    1c55:	cmp    %rsi,%rax
    1c58:	jb     1c40 <engine::FillPattern(...)+0x10>
    1c5a:	ret
```

`FillPattern` est l'anatomie de la leçon 048 en miniature : quatre stockages
d'octets, le pas de 4, le test en bas. Scalaire, exactement comme prédit — et
exactement pourquoi `ClearBuffer`, qui est cette boucle avec d'autres
arguments, manque lui aussi au recensement vectoriel.

```
$ ./tools/disasm.sh FillUniform
0000000000001c60 <engine::FillUniform(unsigned char*, unsigned long, unsigned char)>:
    1c60:	endbr64
    1c64:	mov    %rsi,%rax
    1c67:	test   %rsi,%rsi
    1c6a:	je     1c80 <engine::FillUniform(...)+0x20>
    1c6c:	movzbl %dl,%esi
    1c6f:	mov    %rax,%rdx
    1c72:	jmp    1330 <memset@plt>
    1c80:	ret
```

`FillUniform` n'a **pas de boucle**. Tout le corps, c'est « préparer trois
registres et `jmp memset@plt` » — un appel terminal vers le `memset` de la
bibliothèque C, qui est lui-même de l'assembleur vectorisé à la main au sein
de glibc, écrit par des gens qui ont lu beaucoup de listes comme celles-ci.
Le compilateur n'a pas vectorisé votre boucle ; il l'a *supprimée* au profit
d'une meilleure qui existe déjà.

Voilà le cinquième sort, et le tableau de la leçon 049 est maintenant
complet : une boucle peut être **vectorisée**, **partiellement vectorisée**,
**scalaire mais allouée en registres**, **scalaire**, ou **reconnue et
remplacée**. Le recensement vous dit *si* des registres vectoriels sont
impliqués ; seule la liste vous dit *ce qui s'exécute réellement*. Prédire le
compilateur est une compétence qui se construit exactement ainsi — écrivez la
prédiction, lisez la liste, et lorsque la réalité vous surprend (comme
`jmp memset` surprend la plupart des gens la première fois), la surprise est
la leçon.

*Page traduite de la version anglaise `book/solutions/lesson-049/ex1.md`,
révision `8c45291`.*

<!-- translation-source: book/solutions/lesson-049/ex1.md @ 8c45291 -->
