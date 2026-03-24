# BobTricks — Commandes et modes d'exécution

## Commandes Make

### Disponibles aujourd'hui

| Commande | Effet |
|---|---|
| `make build` | Compile le binaire SDL (`build/bobtricks_v4`) |
| `make run` | Compile et lance l'application SDL |
| `make build_headless` | Compile le binaire headless (`build/bobtricks_headless`) |
| `make test` | Lance `bobtricks_headless --all --quiet` ; exit 0 si tout passe |
| `make clean` | Supprime `build/` |
| `make analysis/<nom>` | Compile un outil d'analyse headless depuis `analysis/<nom>.cpp` |

### Prévues par le plan directeur (Bloque 1)

| Commande | Effet prévu |
|---|---|
| `make docs` | Génère la documentation Doxygen HTML |

`make build_asan` et `make test_mem` sont déjà dans le Makefile.

---

## Mode SDL

**Binaire :** `build/bobtricks_v4`  
**Commande :** `make run`

Flux d'exécution :

```text
main.cpp
└── Application::init()
    ├── ConfigIO::load("data/config.ini")
    └── SimulationCore(config)    ← noyau physique
└── Application::run()            ← boucle principale
    ├── SDL_PollEvent → handleEvent()
    │     ├── touches AZERTY (Q/D/Espace) → flags locomotion
    │     ├── clic droit + drag → set_velocity dans InputFrame
    │     └── scroll → zoom camera
    ├── SimulationLoop::runFrame(dt, stepSimulation)
    │     └── stepSimulation(dt)
    │           ├── construit InputFrame depuis les flags
    │           └── SimulationCore::step(dt, InputFrame)
    ├── Camera2D::update()
    └── render()
          ├── DebugUI::render() → AppRequests
          │     └── Application applique AppRequests dans la même frame
          ├── SceneRenderer, CharacterRenderer, DebugOverlayRenderer
          └── ImGui render
```

**Entrées disponibles en mode SDL :**

| Entrée | Effet |
|---|---|
| Q | Déplacement gauche (`move_left`) |
| D | Déplacement droite (`move_right`) |
| Espace | Saut (`jump`) |
| Clic droit + drag sur le CM | Définit la vitesse du CM (`set_velocity`) |
| Clic gauche + drag | Pan de caméra (si `follow_x` désactivé) |
| Molette | Zoom |
| Échap | Quitter |

**Debug ImGui :**  
Les panels ImGui permettent d'inspecter et de modifier `AppConfig` en direct
(physique, caméra, terrain, pas, balance, etc.). `DebugUI::render()` retourne
un `AppRequests` que `Application` traite dans la même frame.

---

## Mode headless

**Binaire :** `build/bobtricks_headless`  
**Commande :** `make build_headless`

Flux d'exécution :

```text
main_headless.cpp
└── ConfigIO::load("data/config.ini")
└── ScenarioLibrary — catalogue de ScenarioDef nommés
└── ScenarioRunner::runScenario(def, config, csv_out, report_out)
    ├── SimulationCore::reset(init)
    ├── boucle déterministe : SimulationCore::step(dt, input_fn(t))
    ├── TelemetryRecorder::record(state)  par tick
    └── TelemetryRecorder::runAssertions() en fin de run
```

**Interface CLI :**

| Option | Effet |
|---|---|
| `--scenario <nom>` | Lance un scénario nommé ; CSV vers stdout |
| `--all` / `-a` | Lance tous les scénarios enregistrés |
| `--quiet` / `-q` | Supprime les logs de debug du core |
| `--list` / `-l` | Affiche les noms des scénarios disponibles |
| `--help` / `-h` | Affiche l'aide |

**Sorties :**

- CSV de télémétrie vers `stdout` (mode `--scenario`)
- Rapport d'assertions et logs vers `stderr`
- Exit code : `0` si tout passe, `1` si au moins une assertion échoue

---

## Mode test

**Commande :** `make test`

Lance `bobtricks_headless --all --quiet`. Exit code propagé.

Tout test automatique non visuel doit pouvoir tourner sans display. Les
validations visuelles (comportement SDL, overlays) restent manuelles et sont
documentées dans `TESTING_AND_VALIDATION.md`.

---

## Mode debug terminal

Mécanisme actuel : flag global `g_sim_verbose` (défini dans `SimVerbosity.h`).

Quand activé (défaut), le core émet des logs `fprintf(stderr, ...)` pour :

- bootstrap des pieds ;
- déclenchement et heel-strike du step planner ;
- infeasibility d'un step.

Désactivé via `--quiet` en mode headless, ou manuellement depuis le panel
SimLoop de DebugUI.

---

## Mode debug ImGui

Accessible uniquement en mode SDL. Les panels disponibles :

| Panel | Contenu |
|---|---|
| SimLoop | FPS, dt, time scale, pause, step-once, step-back |
| Camera | Zoom, follow, smooth |
| Character | Hauteur corps, masse, ratio pelvis |
| Reconstruction | Filtres facing, seuils |
| CM Kinematics | Position, vitesse, accélération du CM (read-only) |
| CM Visualization | Affichage des overlays CM (trail, XCoM, support) |
| Locomotion | État locomoteur, step plan, heel-strikes (read-only) |
| Balance | XCoM, MoS, seuils |
| Physics | Gravité, friction, ressort, forces de déplacement |
| Terrain | Seed, génération, paramètres de génération procédurale |
| IP Test | Test analytique du pendule inversé vs simulation |

Toutes les modifications de config depuis ImGui peuvent être sauvegardées vers
`data/config.ini` via le bouton Save du panel concerné.

---

## Flux de commandes par mode : synthèse

```text
              ┌──────────────────┐
              │   SimulationCore │  ← InputFrame uniquement
              └────────┬─────────┘
                       │
     ┌─────────────────┼─────────────────┐
     ▼                 ▼                 ▼
Mode SDL          Mode headless     Mode test
─────────         ─────────────     ─────────
InputFrame        InputFrame        InputFrame
depuis SDL        depuis            depuis
events            input_fn(t)       input_fn(t)
                  de ScenarioDef    de ScenarioDef
RuntimeCommand    RuntimeCommand    (pas de
depuis DebugUI    (pas d'UI)        commandes runtime)
→ Application     → ScenarioRunner
```

`SimulationCore` ne connaît aucun de ces modes. Il reçoit des `InputFrame` et
avance la simulation.
