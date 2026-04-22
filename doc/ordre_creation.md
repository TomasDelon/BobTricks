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
| 8 | `StrokeRenderer` | Rendu |
| 9 | `AudioSystem` | Application |
| 10 | `SimulationLoop` | Runtime |

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
| 19 | `LegIK` | `Vec2` |
| 20 | `InputFrame` | `Vec2` |
| 21 | `Camera2D` | `Vec2` |
| 22 | `DustParticle` | `Vec2` |
| 23 | `Terrain` | `AppConfig`, `Vec2` |

## Vague 3 — Dépendent uniquement des vagues 0–2

| # | Module | Dépend de |
|---|--------|-----------|
| 24 | `StrokePath` | `Bezier` |
| 25 | `CharacterState` | `Vec2`, `CMState`, `FootState`, `AppConfig` |
| 26 | `BalanceComputer` | `BalanceState`, `CMState`, `SupportState`, `AppConfig` |
| 27 | `StandingController` | `SupportState`, `CMState`, `FootState`, `AppConfig` |
| 28 | `SceneRenderer` | `Camera2D`, `DustParticle`, `Terrain`, `AppConfig` |

## Vague 4 — Dépendent uniquement des vagues 0–3

| # | Module | Dépend de |
|---|--------|-----------|
| 29 | `ArmController` | `AppConfig`, `CMState`, `CharacterState` |
| 30 | `HeadController` | `AppConfig`, `CharacterState` |
| 31 | `TelemetryRow` | `CharacterState` |
| 32 | `SimState` | `CMState`, `CharacterState` |
| 33 | `GroundReference` | `AppConfig`, `CMState`, `CharacterState`, `Terrain` |
| 34 | `CharacterRenderer` | `Camera2D`, `CMState`, `CharacterState`, `Terrain`, `AppConfig`, `StrokeRenderer` |
| 35 | `DebugOverlayRenderer` | `Camera2D`, `CMState`, `CharacterState`, `Terrain`, `AppConfig`, `TrailPoint` |
| 36 | `DebugUI` | `SimulationLoop`, `Terrain`, `Camera2D`, `CMState`, `CharacterState`, `AppConfig` |

## Vague 5 — Dépendent uniquement des vagues 0–4

| # | Module | Dépend de |
|---|--------|-----------|
| 37 | `UpperBodyKinematics` | `AppConfig`, `ArmController`, `HeadController`, `UpperBodyTypes` |
| 38 | `TelemetryRecorder` | `TelemetryRow`, `SimState` |
| 39 | `SimulationCore` | `AppConfig`, `Terrain`, `SimState`, `InputFrame` |
| 40 | `EffectsSystem` | `AppConfig`, `SimState`, `DustParticle` |

## Vague 6 — Dépendent uniquement des vagues 0–5

| # | Module | Dépend de |
|---|--------|-----------|
| 41 | `InputController` | `AppConfig`, `InputFrame`, `Camera2D`, `SimulationCore` |
| 42 | `ScenarioDef` | `SimState`, `InputFrame`, `TelemetryRecorder` |

## Vague 7 — Dépendent uniquement des vagues 0–6

| # | Module | Dépend de |
|---|--------|-----------|
| 43 | `ScenarioLibrary` | `AppConfig`, `ScenarioDef` |
| 44 | `ScenarioRunner` | `AppConfig`, `ScenarioDef` |
| 45 | `Application` | `AppConfig`, `SimulationCore`, `SimulationLoop`, `Camera2D`, `SceneRenderer`, `CharacterRenderer`, `DebugOverlayRenderer`, `DebugUI`, `AudioSystem`, `EffectsSystem`, `InputController`, `TrailPoint` |
