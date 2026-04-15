# CLAUDE.md — BobTricks

Contexte de travail pour Claude Code. À lire en début de session.

---

## Ce qu'est ce projet

Simulation de locomotion bipède procédurale 2D en C++20. Un stickman piloté
par un noyau physique centré sur le centre de masse (CM). Pas de keyframes.
La pose et le placement des pieds émergent des règles locomotrices.

Trois frontends sur un seul noyau : SDL2 interactif, headless déterministe,
outils d'analyse. Le noyau (`src/core/`) est SDL-free et ImGui-free.

Deadline : fin avril. Voir `doc/ROADMAP.md` pour l'état d'avancement.

---

## BRANCHE walking-redesign — contexte spécifique

Cette branche est une réécriture complète de la locomotion à partir de zéro.

**Référence principale : `doc/WALKING_SPEC_V2.md`**
La branche suit ce redesign, mais l'état effectivement implémenté doit rester
cohérent avec `doc/LOCOMOTION_SPEC.md` et `doc/ROADMAP_locomotion.md`.

État actuel de l'implémentation :
- `src/core/simulation/SimulationCore.cpp` est le noyau actif de locomotion
- `src/core/locomotion/StepPlanner.*` est un stub de compatibilité ; la logique
  de planification de pas vit actuellement dans `SimulationCore`

Fichiers conservés (ne pas modifier sans raison) :
- `src/core/locomotion/LegIK.cpp` — IK correcte
- `src/core/locomotion/BalanceComputer.cpp` — XCoM correcte
- `src/core/locomotion/StandingController.cpp` — utilitaires géométriques
- `src/core/character/CharacterState.cpp` — reconstruction de pose

**Règle absolue dans cette branche** : garder le code, le spec et la doc runtime
alignés. Ne pas réintroduire d'ancienne implémentation en parallèle sans mettre
à jour les documents de référence.

---

## Commandes essentielles

```sh
make build           # compile le binaire SDL
make run             # compile + lance
make test            # gate de qualité obligatoire — doit rester vert
make build_headless  # core + config sans SDL
make build_asan      # headless avec ASan + UBSan
make test_asan       # build_asan + run all scenarios
make test_mem        # Valgrind sur headless
make clean
```

**Règle absolue :** `make test` doit passer avant tout commit. Si ça casse,
c'est un bloquant.

---

## Architecture en une page

```
src/core/          domaine SDL-free — SimulationCore, CMState, CharacterState,
                   FootState, BalanceState, SupportState, StepPlan,
                   LegIK, BalanceComputer, StepPlanner, StandingController,
                   TelemetryRecorder, Terrain
src/config/        AppConfig (structs de paramètres), ConfigIO
src/headless/      ScenarioDef, ScenarioRunner, ScenarioLibrary, main_headless
src/app/           Application (shell SDL), main.cpp
src/render/        SceneRenderer, CharacterRenderer, DebugOverlayRenderer, Camera2D
src/debug/         DebugUI (ImGui)

experiments/curves_lab/  bac à sable séparé — exclu du build principal et headless
vendor/imgui/            externe, non modifié
```

Règles de dépendance dures :
- `core/` ne dépend d'aucune lib externe
- `headless/` dépend de `core/` + `config/` uniquement
- `render/` dépend de `core/` + `config/` + SDL
- `debug/` dépend de `core/` + `config/` + ImGui

---

## Flux de données par frame (mode SDL)

```
SDL events → handleEvent() → flags locomotion / InputFrame
SimulationLoop::runFrame() → stepSimulation(dt)
  └── InputFrame → SimulationCore::step(dt, input)   ← toute la physique ici
Camera2D::update()
render()
  ├── DebugUI::render() → AppRequests → Application traite (save, step-back, etc.)
  ├── SceneRenderer, CharacterRenderer, DebugOverlayRenderer
  └── ImGui render
```

`SimulationCore` ne connaît ni SDL ni ImGui. Il reçoit des `InputFrame` et
expose un `SimState` en lecture seule.

---

## État locomoteur actuel

```cpp
enum class LocomotionState { Standing, Walking, Airborne };
```

Walking est implémenté dans le noyau CM-first actuel avec trigger unifié et
recovery géré directement par `SimulationCore`. Voir
`doc/ROADMAP_locomotion.md` pour l'état exact et `doc/LOCOMOTION_SPEC.md` pour
la spécification runtime.

Extension prévue (Bloque 5A) : `LocomotionMode` + `PostureMode` orthogonal
(`Crouching` est un modificateur, pas un mode exclusif).

---

## Paramètres

Tous les paramètres vivent dans `AppConfig` (`src/config/AppConfig.h`).
Chargé depuis `data/config.ini`. Muté en direct depuis ImGui sans redémarrer.

**Règle :** aucun magic number dans le code. Tout paramètre réglable passe
par `AppConfig`.

---

## Tests et validation

- `make test` : scénarios headless déterministes avec assertions CSV
- Ajouter un scénario : `src/headless/ScenarioLibrary.cpp`
- Pour inspecter un run : `./build/bobtricks_headless --scenario walk_3s > out.csv`
- Validation visuelle manuelle obligatoire pour tout changement de rendu ou
  de comportement perceptible — voir `doc/TESTING_AND_VALIDATION.md`

---

## Conventions de travail

- Écrire la spec dans `LOCOMOTION_SPEC.md` avant d'implémenter un régime
- `make test` vert = prérequis au commit
- Pas de splines / experiments/curves_lab dans le code de production tant qu'il n'y a
  pas de dépendance fonctionnelle réelle (swing trajectory, pose render)
- Le render du personnage est un squelette de lignes — pas de silhouette pour l'instant
- Pas de `Co-Authored-By` Claude dans les commits

---

## Documents de référence

| Document | Contenu |
|---|---|
| `doc/VISION.md` | Objectif, scope, définition de "complet" |
| `doc/ARCHITECTURE.md` | Modules, règles de dépendance, flux |
| `doc/STATE_MODEL.md` | CMState, CharacterState, SimState, AppConfig |
| `doc/ROADMAP.md` | Blocs de travail, état d'avancement |
| `doc/ROADMAP_locomotion.md` | Phases 1–6 walking, état exact |
| `doc/LOCOMOTION_SPEC.md` | Spéc technique de chaque régime |
| `doc/TESTING_AND_VALIDATION.md` | Matrice de validation, procédures |
| `doc/COMMANDS_AND_MODES.md` | Commandes make, modes d'exécution, CLI |
