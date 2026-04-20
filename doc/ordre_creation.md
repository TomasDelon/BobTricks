# Ordre de création des modules

Tri topologique du graphe de dépendances : chaque module n'apparaît qu'après
que tous les modules dont il dépend ont déjà été créés.
Les modules d'une même vague n'ont aucune dépendance entre eux et peuvent
être créés dans n'importe quel ordre (ou en parallèle).

---

## Vague 0 — Aucune dépendance

| # | Module | Groupe |
|---|--------|--------|
| 1 | `MathConstants` | Math |
| 2 | `AppConfig` | Config |
| 3 | `BalanceState` | Données personnage |
| 4 | `SupportState` | Données personnage |
| 5 | `WorldConstants` | Simulation |
| 6 | `SimVerbosity` | Simulation |
| 7 | `StepTriggerType` | Locomotion |
| 8 | `StepPlanner` | Locomotion |
| 9 | `StrokeRenderer` | Rendu |
| 10 | `AudioSystem` | Application |
| 11 | `SimulationLoop` | Runtime |

## Vague 1 — Dépendent uniquement de la vague 0

| # | Module | Dépend de |
|---|--------|-----------|
| 12 | `Vec2` | `MathConstants` |
| 13 | `ConfigIO` | `AppConfig` |

## Vague 2 — Dépendent uniquement des vagues 0–1

| # | Module | Dépend de |
|---|--------|-----------|
| 14 | `Bezier` | `Vec2` |
| 15 | `CMState` | `Vec2` |
| 16 | `FootState` | `Vec2` |
| 17 | `TrailPoint` | `Vec2` |
| 18 | `UpperBodyTypes` | `Vec2` |
| 19 | `StepPlan` | `Vec2` |
| 20 | `LegIK` | `Vec2` |
| 21 | `InputFrame` | `Vec2` |
| 22 | `Camera2D` | `Vec2` |
| 23 | `DustParticle` | `Vec2` |
| 24 | `Terrain` | `AppConfig`, `Vec2` |

## Vague 3 — Dépendent uniquement des vagues 0–2

| # | Module | Dépend de |
|---|--------|-----------|
| 25 | `StrokePath` | `Bezier` |
| 26 | `CharacterState` | `Vec2`, `CMState`, `FootState`, `AppConfig` |
| 27 | `BalanceComputer` | `BalanceState`, `CMState`, `SupportState`, `AppConfig` |
| 28 | `StandingController` | `SupportState`, `CMState`, `FootState`, `AppConfig` |
| 29 | `SceneRenderer` | `Camera2D`, `DustParticle`, `Terrain`, `AppConfig` |

## Vague 4 — Dépendent uniquement des vagues 0–3

| # | Module | Dépend de |
|---|--------|-----------|
| 30 | `ArmController` | `AppConfig`, `CMState`, `CharacterState` |
| 31 | `HeadController` | `AppConfig`, `CharacterState` |
| 32 | `TelemetryRow` | `CharacterState` |
| 33 | `SimState` | `CMState`, `CharacterState` |
| 34 | `GroundReference` | `AppConfig`, `CMState`, `CharacterState`, `Terrain` |
| 35 | `CharacterRenderer` | `Camera2D`, `CMState`, `CharacterState`, `Terrain`, `AppConfig`, `StrokeRenderer` |
| 36 | `DebugOverlayRenderer` | `Camera2D`, `CMState`, `CharacterState`, `Terrain`, `AppConfig`, `TrailPoint` |
| 37 | `DebugUI` | `SimulationLoop`, `Terrain`, `Camera2D`, `CMState`, `CharacterState`, `AppConfig` |

## Vague 5 — Dépendent uniquement des vagues 0–4

| # | Module | Dépend de |
|---|--------|-----------|
| 38 | `UpperBodyKinematics` | `AppConfig`, `ArmController`, `HeadController`, `UpperBodyTypes` |
| 39 | `TelemetryRecorder` | `TelemetryRow`, `SimState` |
| 40 | `SimulationCore` | `AppConfig`, `Terrain`, `SimState`, `InputFrame` |
| 41 | `EffectsSystem` | `AppConfig`, `SimState`, `DustParticle` |

## Vague 6 — Dépendent uniquement des vagues 0–5

| # | Module | Dépend de |
|---|--------|-----------|
| 42 | `InputController` | `AppConfig`, `InputFrame`, `Camera2D`, `SimulationCore` |
| 43 | `ScenarioDef` | `SimState`, `InputFrame`, `TelemetryRecorder` |

## Vague 7 — Dépendent uniquement des vagues 0–6

| # | Module | Dépend de |
|---|--------|-----------|
| 44 | `ScenarioLibrary` | `AppConfig`, `ScenarioDef` |
| 45 | `ScenarioRunner` | `AppConfig`, `ScenarioDef` |
| 46 | `Application` | `AppConfig`, `SimulationCore`, `SimulationLoop`, `Camera2D`, `SceneRenderer`, `CharacterRenderer`, `DebugOverlayRenderer`, `DebugUI`, `AudioSystem`, `EffectsSystem`, `InputController`, `TrailPoint` |
