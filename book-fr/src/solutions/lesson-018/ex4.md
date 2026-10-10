# Solution : exercice 4 — Prouvez la correction

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 4 — Prouvez la correction](../../lessons/part-0/lesson-018-optimizer-ub.md) de la leçon 018.*

## Le diff

```diff
{{#include ../../../../book/solutions/lesson-018/ex4.patch}}
```

## Visite guidée

La passe ajoutée compte les octets qui ne sont pas la valeur de fond, et le
programme répond de son propre vidage :

```
clear covered 144 bytes
clear verified: 0 wrong bytes
```

Le reste de la preuve est par comparaison, et tout passe : lancer les
constructions `-O0` et `-O2` vers deux fichiers et les comparer ne rapporte
aucune différence, et `file paint.bmp` de l'une ou l'autre construction dit
`PC bitmap, Windows 3.x format, 8 x 6 x 24 … cbSize 198`. Ce que prouve
chaque vérification vaut la peine d'être explicité. La passe de vérification
prouve que le vidage *a porté* — une propriété sémantique, vérifiée dans le
programme lui-même. Le `diff` prouve que les deux constructions sont
d'accord sur tout ce que le programme a imprimé. `file` prouve que l'artefact
de sortie est bien formé. Et ce que le `diff` des sorties ne peut pas
attraper : tout ce qui n'atteint jamais stdout — un octet faux dans le BMP
passerait au large d'un diff de console (comparez aussi les fichiers, comme le
fait la dernière étape de l'énoncé) — et, plus profondément, deux sorties
identiques ne prouvent jamais l'*absence* de comportement indéfini : les deux
constructions pourraient être fausses de la même façon. L'accord est un
symptôme de correction, pas une preuve de correction.

*Page traduite de la version anglaise `book/solutions/lesson-018/ex4.md`,
révision `2aa5b9e`.*

<!-- translation-source: book/solutions/lesson-018/ex4.md @ 2aa5b9e -->
