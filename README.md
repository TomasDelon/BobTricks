# BobTricks

BobTricks est un système de locomotion procédurale 2D pour stickman en C++20.
Le projet repose sur un noyau physique `CM-first` (centre de masse) qui pilote
la marche, la course, le saut, la chute, l'accroupissement et le relevé.
Il peut être exécuté soit en mode interactif SDL2 + ImGui, soit en mode
headless via des scénarios déterministes.

## Commandes disponibles aujourd'hui

```sh
make build           # compile l'application SDL
make run             # compile puis lance l'application SDL
make build_headless  # compile le binaire headless (sans SDL)
make test            # exécute les scénarios headless actuels
make clean
```

## Commandes prévues par le plan directeur

Les commandes suivantes sont documentées comme objectifs d'architecture, mais
elles ne sont pas encore toutes disponibles dans le dépôt actif :

```sh
make build_asan      # compilation avec -fsanitize=address,undefined
make test_mem        # Valgrind sur bobtricks_headless --all
make docs            # génération de la documentation Doxygen
```

Le README sera mis à jour dès que ces cibles Makefile existeront réellement.

## Règles d'architecture

- `src/core/` est sans SDL et sans ImGui. Aucune exception.
- `SimulationCore` est l'unique propriétaire de l'état physique. Aucune logique
  de simulation dans `Application`, les renderers ou l'UI.
- `InputFrame` transporte l'entrée physique par tick. `RuntimeCommand`
  transporte le contrôle du shell. On ne les mélange pas.
- Le mode headless est un chemin de validation de premier rang, pas un outil secondaire.
- `make test` est la porte de qualité minimale avant tout changement significatif.

## Boucle de validation

1. Définir le comportement attendu avant d'implémenter.
2. Implémenter le changement.
3. Exécuter `make test` — il doit passer.
4. Pour les changements qui affectent SDL / render / UI : valider aussi
   manuellement dans l'application SDL.
5. Si le comportement observé contredit l'attendu, investiguer avant de clôturer.

## Documents clés

- `doc/ARCHITECTURE.md` — arbre des modules, règles de dépendance, modes d'exécution
- `doc/VISION.md` — scope, régimes locomoteurs, critères de complétude
- `doc/STATE_MODEL.md` — état autoritatif, invariants, reset
- `doc/LOCOMOTION_SPEC.md` — spécification technique par régime
- `doc/TESTING_AND_VALIDATION.md` — types de tests, commandes, format des scénarios
- `doc/ROADMAP_locomotion.md` — état d'avancement des phases de locomotion
