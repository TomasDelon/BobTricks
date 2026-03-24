# BobTricks — Architecture

## Arbre des modules

```text
src/core/                    domaine sans SDL et sans ImGui
├── math/                    Vec2, utilitaires numériques
├── physics/                 Geometry.h, WorldConstants.h
├── character/               CMState, CharacterState, FootState,
│                            BalanceState, SupportState, StepPlan
├── locomotion/              LegIK, BalanceComputer, StepPlanner,
│                            StandingController
│                            ← prévu : LocomotionCoordinator,
│                              JumpController, RunController, FallController
├── pose/                    ← prévu : PoseState, PoseController
├── simulation/              SimulationCore, SimState, InputFrame, SimVerbosity
├── telemetry/               TelemetryRecorder, TelemetryRow
├── terrain/                 Terrain
├── curves/                  ← prévu : modules promus depuis curves_lab
└── analysis/                ← prévu : fonctions pures sur SimState / TelemetryRow

src/config/                  AppConfig (structs de paramètres), ConfigIO
src/headless/                ScenarioDef, ScenarioRunner, ScenarioLibrary,
                             main_headless.cpp
src/app/                     Application (shell SDL), main.cpp
src/render/                  SceneRenderer, CharacterRenderer,
                             DebugOverlayRenderer, Camera2D
src/debug/                   DebugUI (widgets ImGui)

tests/
├── unit/                    tests isolés de fonctions et modules
├── scenario/                tests comportementaux à base de scénarios
└── regression/              sorties de télémétrie comparées à des plages attendues

curves_lab/                  incubateur temporaire — pas dépendance stable
third_party/imgui/           externe, non modifié
```

Les éléments marqués « prévu » appartiennent au plan directeur, mais ne sont
pas encore tous présents dans le dépôt actif.

## Règles de dépendance

Ces règles sont des invariants, pas des recommandations.

```text
core/**          ne dépend d'aucune bibliothèque externe
config/          ne dépend d'aucune bibliothèque externe
headless/        dépend de core + config
app/             dépend de core + config + render + debug
render/          dépend de core + config + SDL
debug/           dépend de core + config + ImGui
core/analysis/   dépend de core + config (sans SDL, sans ImGui, sans IO)
tests/           dépend de core + config + headless (sans SDL, sans ImGui)
```

Le caractère SDL-free de `src/core/` est déjà un engagement d'architecture. La
vérification automatique associée doit rester ou être réintroduite si elle
disparaît pendant un refactor.

## Trois couches d'entrée

### InputFrame — entrée physique par tick

Consommé exclusivement par `SimulationCore::step()`. Il ne contient que ce qui
a un effet physique direct sur la simulation.

```cpp
struct InputFrame {
    bool move_left  = false;
    bool move_right = false;
    bool jump       = false;
    // surcharge optionnelle de vitesse (scénarios, debug)
    std::optional<Vec2> set_velocity;
};
```

### RuntimeCommand — contrôle interactif du shell

Consommé par `Application` ou `ScenarioRunner`. Traité de façon synchrone dans
la frame où il est émis. Il ne mutile jamais `SimState` directement ; il
déclenche des opérations exposées par le shell actif.

État actuel :
- `AppRequests` retourné par `DebugUI::render()`.

État visé par le plan :
- un `RuntimeCommand` typé avec payload quand nécessaire, au lieu d'un simple
  enum nu, afin de couvrir par exemple `SetTimeScale`, `LoadScenario` ou
  `SetVerbosity`.

Exemples : pause, step-once, reset, sauvegarde config, régénération du terrain.

### Fonctions d'analyse — observabilité pure

Fonctions dans `src/core/analysis/` qui calculent des métriques dérivées à
partir de `SimState`, `AppConfig` ou `std::vector<TelemetryRow>`. Pas d'IO, pas
de formatage UI, pas d'effet de bord. Elles sont consommées par la sortie
terminal, les assertions de scénarios et les panels ImGui.

`DebugQuery` reste une catégorie conceptuelle, pas un type formel, tant qu'il
n'existe pas de friction réelle justifiant cette formalisation.

## Modes d'exécution

### Mode SDL (`make build` / `make run`)

```text
main.cpp
  └── Application::init() + run()
        ├── ConfigIO::load()
        ├── SimulationCore (propriétaire de la physique)
        ├── SimulationLoop (accumulateur à pas fixe)
        ├── Camera2D, SceneRenderer, CharacterRenderer, DebugOverlayRenderer
        └── DebugUI (ImGui) → AppRequests → Application traite les commandes
```

`Application::stepSimulation()` construit un `InputFrame` à partir de l'état des
touches SDL et appelle `SimulationCore::step()`. Toute la physique reste dans le
core.

### Mode headless (`make build_headless` / `make test`)

```text
main_headless.cpp
  └── ScenarioLibrary (catalogue de ScenarioDef)
        └── ScenarioRunner::runScenario()
              ├── SimulationCore (même core que le mode SDL)
              ├── TelemetryRecorder (CSV + assertions)
              └── exit 0 si tout passe, 1 sinon
```

Pas de SDL, pas d'ImGui, pas de `Camera2D`. Le même `SimulationCore` tourne de
manière identique.

### Mode test (`make test`)

Aujourd'hui, `make test` lance les scénarios headless existants. Le contrat à
tenir est : tous les tests automatiques non visuels doivent pouvoir tourner
sans display.

## Gestion de l'état locomoteur

État actuel :
- logique essentiellement binaire (`Walking` / `Standing`) directement dans
  `SimulationCore::step()`.

État visé par le plan (Bloque 5A) :
- `LocomotionCoordinator` comme module dédié, responsable des transitions entre
  régimes et appelé depuis `SimulationCore::step()`.

Cela permet de séparer la logique de préconditions et de transitions du tick
physique, sans changer le fait que `SimulationCore` reste propriétaire de la
simulation.

## Reconstruction de pose

État actuel :
- pelvis, torse et genoux sont calculés dans `CharacterState.cpp` /
  `updateCharacterState()`.

État visé par le plan (Bloque 5A) :
- `PoseController` dans `src/core/pose/` produit un `PoseState` (géométrie
  pure) à chaque tick.
- le lissage et l'inertie éventuels restent privés au controller.
- la tête et les bras y sont ajoutés comme secondary motion.

## Notes sur la classification actuelle

- `Camera2D` est dans `src/render/` (déplacé depuis `src/core/runtime/` au Bloque 0).
- `SimulationLoop` vit dans `src/core/runtime/`. C'est de l'orchestration, pas
  du domaine. Sa position est provisoire ; aucun déplacement immédiat n'est requis.
- `curves_lab/` est un incubateur temporaire, exclu du build principal et du
  build headless. Le render du personnage utilise des lignes droites (squelette).
  Lorsqu'un module de courbes devient une dépendance fonctionnelle réelle
  (trajectoire de swing, render de pose), il migre vers `src/core/curves/` avec
  inclusion dans le build headless et tests associés.
