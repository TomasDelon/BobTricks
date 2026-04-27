# BobTricks

BobTricks est un système de locomotion procédurale 2D pour stickman en C++20.
Le projet repose sur un noyau physique `CM-first` (centre de masse) qui pilote
la marche, la course, le saut, la chute, l'accroupissement et le relevé.
Il peut être exécuté soit en mode interactif SDL2 + ImGui, soit en mode
headless via des scénarios déterministes.

## Démo vidéo

[![Voir la démo BobTricks sur YouTube](https://img.youtube.com/vi/3eavnAWpd80/hqdefault.jpg)](https://www.youtube.com/watch?v=3eavnAWpd80)

Lien direct : https://www.youtube.com/watch?v=3eavnAWpd80

## Prérequis

Le build graphique utilise :

- un compilateur C++20 (`g++`) ;
- SDL2 pour la fenêtre, les événements clavier/souris et le rendu 2D ;
- SDL2_mixer pour les sons de pas, de glissade et d'atterrissage ;
- OpenGL pour le linkage graphique ;
- Dear ImGui, fourni dans `vendor/imgui`, pour l'interface de debug.

Sous Debian/Ubuntu, les paquets utiles sont généralement :

```sh
sudo apt-get install g++ make libsdl2-dev libsdl2-mixer-dev doxygen valgrind
```

`doxygen` et `valgrind` ne sont nécessaires que pour `make docs` et
`make test_mem`.

## Compilation et exécution

```sh
make build              # compile l'application graphique SDL2
make run                # compile puis lance l'application graphique
make build_headless     # compile le binaire headless sans SDL/ImGui/rendu
make test               # unitaires + régressions + scénarios headless
make test_unit          # tests unitaires bas niveau
make test_regression    # tests de régression sur les scénarios
make test_headless      # tous les scénarios headless
make build_asan         # binaire headless avec AddressSanitizer + UBSan
make test_asan          # scénarios headless avec sanitizers
make test_mem           # Valgrind sur le binaire headless
make check_architecture # vérifie les invariants d'architecture
make docs               # génère la documentation Doxygen
make clean              # supprime build/
```

Le Makefile sépare volontairement deux chemins :

- `make build` compile l'application complète : `src/app`, `src/render`,
  `src/debug`, SDL2, SDL2_mixer et ImGui ;
- `make build_headless` compile seulement `src/core`, `src/config` et
  `src/headless`, sans SDL ni interface graphique.

Cette séparation prouve que le noyau de simulation est testable sans fenêtre.

## Utilisation de l'application graphique

```sh
make run
```

Contrôles principaux :

- `Q` / `D` : aller à gauche / droite ;
- `Shift` : courir ;
- `Espace` : sauter ;
- `P` : basculer le mode présentation ;
- `Echap` : quitter ;
- molette : zoom caméra ;
- glisser dans le vide : déplacer la caméra quand le suivi est désactivé ;
- glisser sur un pied ou une main : déplacer cette cible ;
- clic droit sur un pied ou une main : épingler ou désépingler.

L'interface ImGui permet de modifier les paramètres de simulation, de rendu, de
terrain, d'audio et d'overlays pendant l'exécution. Les boutons "Sauver la
config ..." écrivent les valeurs dans `data/config.ini`.

## Configuration et données

Le fichier principal de données est :

- `data/config.ini`

Il est chargé au démarrage par `ConfigIO` et regroupe les paramètres par
sections : boucle de simulation, caméra, personnage, reconstruction, bras,
rendu spline, mode présentation, centre de masse, physique, terrain, particules,
audio, marche, course et saut.

Les assets se trouvent dans :

- `data/audio/` pour les sons et musiques ;
- `data/gif/` pour les captures de démonstration ;
- `doc/presentation/data/` pour les vidéos et images de la soutenance.

## Utiliser le mode headless

Le mode headless compile et exécute un binaire séparé, sans SDL2, sans ImGui et
sans rendu. Il sert à valider le noyau de simulation via des scénarios
déterministes.

### 1. Compiler le binaire headless

```sh
make build_headless
```

Le binaire produit est :

```sh
build/bobtricks_headless
```

### 2. Afficher l'aide CLI

```sh
./build/bobtricks_headless --help
```

Options disponibles :

- `--scenario <name>` : exécute un scénario unique
- `--all` ou `-a` : exécute tous les scénarios enregistrés
- `--list` ou `-l` : affiche la liste des scénarios disponibles
- `--quiet` ou `-q` : supprime les logs verbeux du noyau
- `--help` ou `-h` : affiche l'aide

Si aucun scénario n'est fourni, le binaire exécute `walk_3s` par défaut.

### 3. Lister les scénarios disponibles

```sh
./build/bobtricks_headless --list
```

Scénarios actuellement enregistrés :

- `fast_walk`
- `jump_from_stand`
- `jump_from_walk`
- `perturbation_recovery`
- `run_3s`
- `stand_still`
- `walk_3s`
- `walk_left`
- `walk_max_from_start`
- `walk_then_stop`

### 4. Exécuter un scénario unique

```sh
./build/bobtricks_headless --scenario walk_3s --quiet
```

Comportement de sortie :

- `stdout` : télémétrie CSV complète, une ligne par tick
- `stderr` : en-tête d'exécution, rapport des assertions, statut final

Exemple pour sauvegarder la télémétrie :

```sh
./build/bobtricks_headless --scenario walk_3s --quiet > walk_3s.csv
```

Le CSV contient notamment :

- `t`
- `cm_x`, `cm_vx`, `cm_y`, `cm_vy`
- positions bassin / pieds
- états de contact au sol
- `heel_strike`
- `loco_state`
- mesures de terrain et de stabilité

### 5. Exécuter tous les scénarios

```sh
./build/bobtricks_headless --all --quiet
```

Dans ce mode, le binaire :

- exécute chaque scénario avec une copie propre de la configuration
- évalue les assertions de chaque scénario
- affiche `PASS` ou `FAIL` pour chaque cas
- retourne `0` si tout passe, `1` sinon

Quand `--all` est utilisé, la télémétrie CSV n'est pas le livrable principal :
on cherche surtout un verdict de validation reproductible.

### 6. Commandes Make liées au headless

```sh
make build_headless   # compile build/bobtricks_headless
make test_headless    # exécute tous les scénarios headless
make test             # unitaires + régression + headless
make build_asan       # build headless avec AddressSanitizer + UBSan
make test_asan        # exécute tous les scénarios sur le build ASan
make test_mem         # Valgrind sur le binaire headless
```

En pratique :

- `make test_headless` exécute `build/bobtricks_headless --all --quiet`
- `make test` est la porte de qualité minimale avant un changement significatif

### 7. Configuration utilisée par le binaire

Le binaire headless charge `data/config.ini` au démarrage. Si le fichier est
absent, les valeurs par défaut de `AppConfig` sont utilisées silencieusement.

### 8. Où ajouter ou modifier des scénarios

Les scénarios prédéfinis sont centralisés dans :

- `src/headless/ScenarioLibrary.cpp`

L'exécutable CLI est défini dans :

- `src/headless/main_headless.cpp`

Le moteur d'exécution des scénarios et des assertions est défini dans :

- `src/headless/ScenarioRunner.cpp`

Les tests de régression qui s'appuient sur ces scénarios se trouvent dans :

- `tests/regression/test_headless_scenarios.cpp`

Pour ajouter un nouveau scénario headless :

1. Définir une nouvelle factory dans `src/headless/ScenarioLibrary.cpp`
2. Renseigner l'initialisation, la durée, les entrées simulées et les assertions
3. L'enregistrer dans la bibliothèque retournée par `scenarioLibrary()`
4. Valider avec `make test_headless` puis `make test`

## Règles d'architecture

- `src/core/` est sans SDL et sans ImGui. Aucune exception.
- `SimulationCore` est l'unique propriétaire de l'état physique. Aucune logique
  de simulation dans `Application`, les renderers ou l'UI.
- `InputFrame` transporte l'entrée physique par tick. `RuntimeCommand`
  transporte le contrôle du shell. On ne les mélange pas.
- Le mode headless est un chemin de validation de premier rang, pas un outil secondaire.
- `make test` est la porte de qualité minimale avant tout changement significatif.
- `make check_architecture` protège les invariants de dépôt avant push.

## Boucle de validation

1. Définir le comportement attendu avant d'implémenter.
2. Implémenter le changement.
3. Exécuter `make test` — il doit passer.
4. Exécuter `make check_architecture` — il doit passer.
5. Pour les changements numériques sensibles : exécuter `make test_asan`.
6. Pour les changements de durée de vie mémoire : exécuter `make test_mem`.
7. Pour les changements qui affectent SDL / render / UI : valider aussi
   manuellement dans l'application SDL.
8. Si le comportement observé contredit l'attendu, investiguer avant de clôturer.

## Documents clés

- `doc/class_diagram.md` — diagramme des modules et relations principales ;
- `doc/gantt_bobtricks.xlsx` — planning ;
- `doc/presentation/` — présentation orale, scripts et assets.

La documentation Doxygen des headers se génère avec :

```sh
make docs
```

La sortie HTML est produite dans `doc/doxygen/html/index.html`.
