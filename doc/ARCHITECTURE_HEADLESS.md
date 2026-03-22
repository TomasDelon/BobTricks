# BobTricks V4 — Architecture Headless

Objectif : permettre au noyau de simulation de tourner en mode fenêtre, en mode
headless (sans SDL visible), et en mode test console avec exit code 0/1.

---

## État des phases

| Phase | Titre | État |
|-------|-------|------|
| **0** | Consolidation sans changement de comportement | ✅ DONE |
| **1** | `SimulationCore` + `InputFrame` | ✅ DONE |
| **2** | `TelemetryRecorder` + `Assertion` | ✅ DONE |
| **3** | `HeadlessApp` + `ScenarioDef` + `ScenarioRunner` | ✅ DONE |
| **4** | `ScenarioLibrary` + `make test` + exit codes | ✅ DONE |
| **5** | Panel ImGui de scénarios dans `SDLViewerApp` | optionnel |

---

## Fase 0 — Livrée (détail)

Objectif : éliminer les redondances et stabiliser les types de base avant d'extraire
`SimulationCore`. Aucun changement de comportement visible.

### Fichiers créés

| Fichier | Contenu |
|---------|---------|
| `src/core/simulation/WorldConstants.h` | `World::GROUND_Y = 0.0` — source unique |
| `src/core/physics/Geometry.h` | `computeNominalY(L, d_pref, ratio)` — formule correcte |
| `src/core/simulation/InputFrame.h` | POD d'entrée joueur/scripté, sans SDL |

### Fichiers modifiés

| Fichier | Changement |
|---------|------------|
| `src/core/character/CharacterState.h` | `Falling` supprimé du enum (état zombie) |
| `src/core/character/CharacterState.cpp` | Guard `Falling` sticky supprimé |
| `src/core/locomotion/StepPlanner.h/.cpp` | `StepTriggerType` {None, Normal, Emergency} ajouté ; `shouldStep()` reçoit `out_trigger` |
| `src/app/Application.cpp` | `nominalCMPos()` supprimé ; `GROUND_Y` → `World::GROUND_Y` ; `nominal_y` → `computeNominalY()` |
| `src/render/DebugOverlayRenderer.h/.cpp` | `renderForeground()` reçoit `StandingConfig` ; `cm_offset` → `computeNominalY()` |
| `src/debug/DebugUI.cpp` | cas `Falling` supprimé ; `nominal_y` → `computeNominalY()` |

### Critère de sortie Fase 0

- `make -B build` propre (zero warnings)  ✅
- `Geometry.h`, `WorldConstants.h`, `InputFrame.h` compilent sans SDL  ✅
- aucun symbole `Falling` dans le codebase  ✅
- `nominal_y` calculé par `computeNominalY()` dans tous les fichiers  ✅

---

## Fase 1 — Livrée (détail)

### Fichiers créés

| Fichier | Contenu |
|---------|---------|
| `src/core/simulation/SimState.h` | Snapshot `{ CMState, CharacterState, double t }` + `ScenarioInit` |
| `src/core/simulation/SimulationCore.h` | Interface publique — SDL-free |
| `src/core/simulation/SimulationCore.cpp` | Noyau extrait de `Application::stepSimulation()` |

### Fichiers modifiés

| Fichier | Changement |
|---------|------------|
| `src/core/locomotion/StepPlanner.cpp` | `#include <SDL2/SDL.h>` → `#include <cstdio>` ; `SDL_Log` → `fprintf(stderr, ...)` |
| `src/app/Application.h` | `m_cm`, `m_character`, `m_terrain`, `m_feet_initialized` supprimés ; `m_core` (`unique_ptr<SimulationCore>`) ajouté ; `StepSnapshot` utilise `SimState` |
| `src/app/Application.cpp` | `stepSimulation()` devient un thin wrapper ; tout l'état lu depuis `m_core->state()` |

### Critère de sortie Fase 1

- `make -B build` propre (zero warnings)  ✅
- `g++ -std=c++20 -c SimulationCore.cpp -Isrc -lm` sans SDL  ✅
- comportement visuel identique  ✅

---

## Fase 1 — Plan (référence historique)

### Objectif

`SimulationCore` existe et compile sans SDL.

```sh
g++ -std=c++20 src/core/simulation/SimulationCore.cpp [core deps] -Isrc -lm -o /dev/null
# aucune erreur, aucun #include SDL
```

### Nouveaux fichiers

```
src/core/simulation/SimState.h          — snapshot read-only { CMState, CharacterState, double t }
src/core/simulation/SimulationCore.h    — interface publique
src/core/simulation/SimulationCore.cpp  — noyau extrait de stepSimulation()
```

### API de SimulationCore

```cpp
class SimulationCore {
public:
    SimulationCore(AppConfig config, uint32_t terrain_seed);
    void step(double dt, const InputFrame& input);
    void reset(const ScenarioInit& init);
    const SimState& state() const;
    double time() const;

    // Callbacks optionnels — appelés depuis step()
    std::function<void(StepTriggerType, double xcom_ahead)> on_step_trigger;
    std::function<void(bool swing_right, Vec2 land_pos)>    on_heel_strike;
    std::function<void(const SimState&)>                    on_tick;
};
```

### reset() — état complet à réinitialiser

```
CMState:        position, velocity = init values, acceleration = {0,0}
CharacterState: tout à défaut   (= CharacterState{})
  feet_initialized  = false     → re-bootstrap au premier step()
  facing            = 1.0
  facing_vel        = init.cm_vel.x
  last_heel_strike_t = -1.0     (contrat CharacterState.h: -1 = jamais)
  step_plan.active  = false
Terrain:        regenerate si seed changée
sim_time:       0.0
```

### Critère de sortie Fase 1

- `make -B build` propre
- `g++ SimulationCore.cpp [deps] -Isrc -lm` sans SDL
- comportement visuel identique à avant

---

## Fase 2 — Livrée (détail)

### Fichiers créés

| Fichier | Contenu |
|---------|---------|
| `src/core/locomotion/StepTriggerType.h` | Enum extrait — inclus par `StepPlanner.h` et `CharacterState.h` |
| `src/core/telemetry/TelemetryRow.h` | POD avec les 17 colonnes CSV |
| `src/core/telemetry/TelemetryRecorder.h/.cpp` | `record()`, `writeCsv()`, `addAssertion()`, `runAssertions()` |

### Fichiers modifiés

| Fichier | Changement |
|---------|------------|
| `src/core/character/CharacterState.h` | `last_trigger: StepTriggerType` ajouté |
| `src/core/locomotion/StepPlanner.h` | Inclut `StepTriggerType.h` au lieu de définir l'enum |
| `src/core/simulation/SimulationCore.cpp` | `last_trigger` remis à None chaque tick ; assigné quand step planifié |
| `Makefile` | `src/core/telemetry` ajouté à `SRC_DIRS` |

### Format CSV — colonnes figées (ne pas modifier sans versionner)

```
t,cm_x,cm_vx,cm_y,cm_vy,pelvis_x,xcom,mos,support_left,support_right,support_width,foot_L_x,foot_R_x,step_active,swing_right,trigger,loco_state
```

### Critère de sortie Fase 2

- `make -B build` propre (zero warnings)  ✅
- `g++ -std=c++20 -c TelemetryRecorder.cpp -Isrc` sans SDL  ✅
- `CharacterState::last_trigger` propagé depuis SimulationCore  ✅

---

## Fase 3 — Livrée (détail)

### Fichiers créés

| Fichier | Contenu |
|---------|---------|
| `src/headless/ScenarioDef.h` | Struct avec `name`, `duration_s`, `init`, `input_fn`, `setup_asserts` |
| `src/headless/ScenarioRunner.h/.cpp` | `runScenario()` — boucle déterministe, CSV + assertions |
| `src/headless/main_headless.cpp` | Entry point + scénarios `walk_3s` et `stand_still` |

### Fichiers modifiés

| Fichier | Changement |
|---------|------------|
| `Makefile` | `HEADLESS_DIRS`, `HEADLESS_SRCS`, cible `build_headless` |

### Utilisation

```sh
make build_headless
./build/bobtricks_headless --scenario walk_3s  > out.csv   # exit 0 = PASS
./build/bobtricks_headless --scenario stand_still > /dev/null
```

### Critère de sortie Fase 3

- `make build_headless` propre (zero warnings)  ✅
- `./bobtricks_headless --scenario walk_3s` → exit 0, 6 heel-strikes, cm_x > 1.5  ✅
- `./bobtricks_headless --scenario stand_still` → exit 0, 0 heel-strikes, cm_x=0  ✅
- Aucun SDL dans la chaîne de compilation  ✅

---

## Boucle IA complète (cible Fase 3+)

```
1. tarea definida
2. editar ScenarioLibrary o args CLI
3. predicción escrita antes de ejecutar
4. make build_headless
5. ./bobtricks_headless --scenario X > out.csv
6. awk/grep sobre out.csv → extraer métricas clave
7. comparar con predicción
8. si FAIL: diagnosticar columnas, fix, repetir desde 4
9. si PASS: done
```

Aucune fenêtre nécessaire à aucune étape.
