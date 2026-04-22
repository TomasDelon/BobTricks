# Ordre de création des modules

Tri topologique du graphe de dépendances : chaque module n'apparaît qu'après
que tous les modules dont il dépend ont déjà été créés.
Les modules d'une même vague n'ont aucune dépendance entre eux et peuvent
être créés dans n'importe quel ordre (ou en parallèle).

Ce document décrit l'ordre des **modules logiques**, pas l'ordre des fichiers
`.cpp`. Ainsi, `SimulationCore` et `DebugUI` apparaissent chacun une seule fois
même si leur implémentation est désormais répartie sur plusieurs fichiers.

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
| 7 | `StrokeRenderer` | Rendu |
| 8 | `AudioSystem` | Application |
| 9 | `SimulationLoop` | Runtime |

## Vague 1 — Dépendent uniquement de la vague 0

| # | Module | Dépend de |
|---|--------|-----------|
| 10 | `Vec2` | `MathConstants` |
| 11 | `ConfigIO` | `AppConfig` |

## Vague 2 — Dépendent uniquement des vagues 0–1

| # | Module | Dépend de |
|---|--------|-----------|
| 12 | `Bezier` | `Vec2` |
| 13 | `CMState` | `Vec2` |
| 14 | `FootState` | `Vec2` |
| 15 | `TrailPoint` | `Vec2` |
| 16 | `UpperBodyTypes` | `Vec2` |
| 17 | `LegIK` | `Vec2` |
| 18 | `InputFrame` | `Vec2` |
| 19 | `Camera2D` | `Vec2` |
| 20 | `DustParticle` | `Vec2` |
| 21 | `Terrain` | `AppConfig`, `Vec2` |

## Vague 3 — Dépendent uniquement des vagues 0–2

| # | Module | Dépend de |
|---|--------|-----------|
| 22 | `StrokePath` | `Bezier` |
| 23 | `CharacterState` | `Vec2`, `CMState`, `FootState`, `AppConfig` |
| 24 | `BalanceComputer` | `BalanceState`, `CMState`, `SupportState`, `AppConfig` |
| 25 | `StandingController` | `SupportState`, `CMState`, `FootState`, `AppConfig` |
| 26 | `SceneRenderer` | `Camera2D`, `DustParticle`, `Terrain`, `AppConfig` |

## Vague 4 — Dépendent uniquement des vagues 0–3

| # | Module | Dépend de |
|---|--------|-----------|
| 27 | `ArmController` | `AppConfig`, `CMState`, `CharacterState` |
| 28 | `HeadController` | `AppConfig`, `CharacterState` |
| 29 | `TelemetryRow` | `CharacterState` |
| 30 | `SimState` | `CMState`, `CharacterState` |
| 31 | `GroundReference` | `AppConfig`, `CMState`, `CharacterState`, `Terrain` |
| 32 | `CharacterRenderer` | `Camera2D`, `CMState`, `CharacterState`, `Terrain`, `AppConfig`, `StrokeRenderer` |
| 33 | `DebugOverlayRenderer` | `Camera2D`, `CMState`, `CharacterState`, `Terrain`, `AppConfig`, `TrailPoint` |
| 34 | `DebugUI` | `SimulationLoop`, `Terrain`, `Camera2D`, `CMState`, `CharacterState`, `AppConfig` |

## Vague 5 — Dépendent uniquement des vagues 0–4

| # | Module | Dépend de |
|---|--------|-----------|
| 35 | `UpperBodyKinematics` | `AppConfig`, `ArmController`, `HeadController`, `UpperBodyTypes` |
| 36 | `TelemetryRecorder` | `TelemetryRow`, `SimState` |
| 37 | `SimulationCore` | `AppConfig`, `Terrain`, `SimState`, `InputFrame` |
| 38 | `EffectsSystem` | `AppConfig`, `SimState`, `DustParticle` |

## Vague 6 — Dépendent uniquement des vagues 0–5

| # | Module | Dépend de |
|---|--------|-----------|
| 39 | `InputController` | `AppConfig`, `InputFrame`, `Camera2D`, `SimulationCore` |
| 40 | `ScenarioDef` | `SimState`, `InputFrame`, `TelemetryRecorder` |

## Vague 7 — Dépendent uniquement des vagues 0–6

| # | Module | Dépend de |
|---|--------|-----------|
| 41 | `ScenarioLibrary` | `AppConfig`, `ScenarioDef` |
| 42 | `ScenarioRunner` | `AppConfig`, `ScenarioDef` |
| 43 | `Application` | `AppConfig`, `SimulationCore`, `SimulationLoop`, `Camera2D`, `SceneRenderer`, `CharacterRenderer`, `DebugOverlayRenderer`, `DebugUI`, `AudioSystem`, `EffectsSystem`, `InputController`, `TrailPoint` |
