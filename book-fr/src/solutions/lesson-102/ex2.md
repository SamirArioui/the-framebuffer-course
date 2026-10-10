# Solution : exercice 2 — La carte de l'épilogue sur votre machine

{{#include ../../stability-horizon.md}}

*Solution de l'[Exercice 2 — La carte de l'épilogue sur votre machine](../../lessons/part-5/lesson-102-retrospective.md) de la leçon 102.*

## Le diff

Aucun, à dessein : cet exercice n'écrit pas de code, donc `ex2.patch` est
vide — il n'y a rien à appliquer et rien à vérifier. Le livrable est la
carte ; une réponse travaillée pour une machine suit, et la vôtre
différera là où la machine diffère.

## Visite guidée

Un réordonnancement travaillé pour **la machine de rédaction elle-même**
(WSL2 sous Windows, Xvfb `:99`, pas de périphérique sonore), qui est le
cas dégénéré : une machine *déjà* à l'intérieur du monde Windows de la
couture et qui n'a rien vers quoi porter.

**Quelle pièce d'abord, et quelles lignes le décident.** La ligne
`present` du rapport décide : `0.438 ms` de temps mural pour `0.004 ms`
de CPU, c'est l'attente de la couture, et sur un vrai affichage de bureau
la *forme* de cette ligne change (le coût de la copie atterrit dans le
CPU du processus au lieu de l'attente du serveur) — mais la ligne que le
portage GPU effacerait est `render` (`0.799 ms` : `clear 0.244`,
`tilemap 0.538`). Sur cette machine le coût de la frame n'est pas le
problème (`1.313 ms` d'un budget de `16.667 ms`, `0 of 2583` frames
au-dessus), donc aucune des deux pièces n'est exigée par la mesure ici —
et selon la discipline c'est la réponse honnête : **l'ordre de la carte
est pour les machines qui souffrent, et celle-ci ne souffre pas.** Si
c'était le cas, les deux candidates se sépareraient proprement : un coût
de rendu (les lignes de `render`, liées au CPU) dit portage GPU ; un coût
de plateforme (`present`, ou un périphérique que les fichiers de cet OS
ne peuvent pas atteindre) dit module Windows.

**Le premier jalon du module, dans les termes de Windows.** La liste du
contrat de `platform.h`, dans l'API sur laquelle un `platform_win32.cpp`
s'assoirait : `OpenWindow`/`CloseWindow` (une classe de fenêtre Win32 et
un `HWND`), la paire d'état des touches et `HasFocus` (l'état
`WM_KEYDOWN` de la pompe à messages et `WM_SETFOCUS`), `Now`
(`QueryPerformanceCounter`), `PageSize` (`GetSystemInfo`), la paire de
réservation (`VirtualAlloc`/`VirtualFree`), le trio de fichiers
(`CreateFile`/`ReadFile`/`WriteFile`), `PumpEvents`/`CloseRequested`
(`PeekMessage` et `WM_CLOSE`), `Present` (les mêmes octets BGRA —
`SetDIBitsToDevice` ou une copie de tampon arrière, en gardant « quand il
renvoie true, les pixels sont à l'écran »), et l'alimentation de
`OpenAudioOutput` (WASAPI en mode partagé, le même contrat mono 16 bits à
44.1 kHz). Les échecs typés se mappent un pour un (`OPEN_NO_DISPLAY`,
`FILE_NOT_FOUND`, `AUDIO_NO_DEVICE`, …) et la liste d'implémentation de
la vérification de frontière grandit pour nommer les nouveaux fichiers —
aucun fichier du moteur ne change.

**Le premier jalon du portage, dans les termes du rapport.** Gardez le
contrat de pixels ; remplacez d'abord le chemin de présentation (le temps
mural de la ligne `present` devient un échange et sa part CPU devient
celle du pilote) ; puis le clear (`clear 0.244 ms` de CPU s'en va au
remplissage du GPU), puis le dessin de la carte (`tilemap 0.538 ms`
devient des appels de dessin — et avec lui le levier de la leçon 099
cesse de s'appliquer). Les lignes qui survivent intactes : `update`,
`entities`, `audio`, la logique de `text`. Ce qui remplace la boucle de
copie est un échange — et le compte de frame continue de mesurer, parce
que le rapport est la façon dont quiconque saura que le portage a
réellement aidé.

**Ce que ma machine a changé dans le tableau** — et ce que la vôtre
pourrait : rien ici (la machine est la machine) ; sur un bureau,
attendez-vous à deux décalages structurels que la leçon a nommés : le
partage CPU/attente de la ligne `present` bascule avec un affichage
local, et un périphérique sonore cadence la boucle à travers
l'alimentation au lieu des secousses — si bien que votre *fréquence* de
frames devient réelle avant même que le *coût* de vos frames soit
intéressant. Une machine sur un OS auquel la couture ne répond pas encore
renverse l'ordre carrément : le module n'est alors pas une optimisation
mais la porte.

Quelle que soit la pièce que votre machine nomme en premier, une exigence
reste fixe (D12) : les nombres de la carte portent le nom de votre
machine, et la pièce se construit quand la mesure le demande — jamais
parce que la carte existe.

*Page traduite de la version anglaise `book/solutions/lesson-102/ex2.md`,
révision `4ef60c6`.*

<!-- translation-source: book/solutions/lesson-102/ex2.md @ 4ef60c6 -->
