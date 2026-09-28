# Journal de bord du portage

Chaque entrée a été écrite à partir de ce que le terminal a réellement affiché. Les symptômes sont recopiés tels quels.

## Entrée 1

Symptôme : ModuleNotFoundError: No module named 'Jenga' (dernière ligne de la traceback Python affichée par jenga.exe)
J'ai cru : que Jenga était déjà installé sur ma machine, parce que je voyais un dossier .jenga dans mon profil Windows.
C'était : une vieille installation en mode editable (version 2.0.1, pointant vers un dossier de travail du bureau) qui ne correspondait plus au point d'entrée jenga.exe. Il a fallu la désinstaller et installer le wheel officiel 2.8.4.
Temps perdu : 25 minutes

## Entrée 2

Symptôme : No .jenga workspace file found.
J'ai cru : que jenga info -v marchait depuis n'importe quel dossier, comme une commande système.
C'était : je l'avais lancé dans C:\WINDOWS\system32, où il n'y a aucun fichier .jenga. Jenga cherche le fichier de projet dans le dossier courant.
Temps perdu : 10 minutes

## Entrée 3

Symptôme : LINK : fatal error LNK1104: impossible d'ouvrir le fichier 'LIBCMT.lib'
J'ai cru : que mon main.cpp ou mon projet.jenga était en cause, puisque la compilation de main.cpp avait pourtant réussi.
C'était : le PowerShell classique n'a pas les variables d'environnement de MSVC (LIB, INCLUDE). Le même jenga build lancé depuis le Developer PowerShell de Visual Studio 2022 a réussi du premier coup.
Temps perdu : 10 minutes

## Entrée 4

Symptôme : Ô£ô Built: Build\Bin\Debug-Windows\MaSalle\MaSalle.exe avec type, puis âœ“ Built: Build\Bin\Debug-Windows\MaSalle\MaSalle.exe après cmd /c, puis [32m✓[0m Built: Build\Bin\Debug-Windows\MaSalle\MaSalle.exe dans Notepad
J'ai cru : que sortie-build.txt était corrompu et qu'il fallait le régénérer avec le bon encodage.
C'était : deux choses sans gravité. La console PowerShell affichait mal un fichier UTF-8 correct, et Jenga écrit des codes couleur ANSI quand on redirige sa sortie vers un fichier. Le fichier était bon depuis le début (UTF-8, avec BOM).
Temps perdu : 15 minutes

## Entrée 5

Symptôme : mkdir : Impossible de trouver un paramètre positionnel acceptant l'argument « Crée ».
J'ai cru : que la commande mkdir était mal écrite.
C'était : j'avais copié la phrase d'explication qui suivait la commande, en plus de la commande elle-même. Même piège avec « d : Le terme «d» n'est pas reconnu », où le cd était tronqué et plusieurs commandes collées sur une seule ligne.
Temps perdu : 3 minutes

## Entrée 6

Symptôme : Exception lors de l'appel de «SetEnvironmentVariable» avec «3» argument(s): «Tentative d'exécution d'une opération non autorisée.»
J'ai cru : qu'il me fallait les droits administrateur pour définir une variable d'environnement.
C'était : je n'ai pas identifié la cause exacte. La commande setx a fonctionné à la place pour ANDROID_SDK_ROOT.
Temps perdu : 5 minutes

## Entrée 7

Symptôme : sdkmanager : Le terme «sdkmanager» n'est pas reconnu comme nom d'applet de commande, fonction, fichier de script ou programme exécutable.
J'ai cru : que l'installation des command-line tools avait échoué, ou que le fichier n'était pas au bon endroit.
C'était : sdkmanager.bat était bien dans C:\Android\cmdline-tools\latest\bin. Le nouveau Path n'avait pas été enregistré. J'ai contourné le problème avec $env:Path += ";..." dans la fenêtre PowerShell en cours.
Temps perdu : 30 minutes

## Entrée 8

Symptôme : La sortie ne correspond pas à celle attendue. attendu : A / CYCLE obtenu : CYCLE (test « un cycle plus loin dans le graphe » de l'exo32, les retours à la ligne du rapport sont notés /)
J'ai cru : que l'énoncé, qui dit « affichez une seule ligne : CYCLE », voulait dire de ne rien afficher d'autre dès qu'il y a un cycle.
C'était : le correcteur attend que les modules déjà sortis avant le blocage restent affichés, puis CYCLE. Il fallait afficher chaque module au moment de sa sortie, et non à la fin.
Temps perdu : 15 minutes

## Comment j'ai obtenu les durées

Ces durées ne sont pas chronométrées, ce sont des estimations arrondies à quelques minutes. Je ne notais pas l'heure au moment des pannes, et j'ai reconstitué les chiffres après coup avec l'aide d'un assistant IA.

Pour les entrées 2, 3, 4 et 5, je me suis appuyé sur les dates de création et de modification des fichiers du projet (main.cpp créé à 21:33, projet.jenga écrit à 21:36, dossier Build créé à 21:38, dernier sortie-build.txt à 21:55 le 26/09, dossier C:\Android créé à 22:08 le 27/09). Ces dates donnent des bornes de séance, pas la durée de chaque erreur, donc les chiffres restent approximatifs.

Pour les entrées 1, 6, 7 et 8, aucune date ne m'aidait. J'ai estimé d'après le nombre d'échanges qu'il a fallu pour régler chaque problème, sans pouvoir savoir combien de temps s'écoulait entre deux messages. Ce sont les chiffres les moins fiables du journal.

La prochaine fois, je noterai l'heure du premier message d'erreur et celle de la résolution, au moment où ça casse.